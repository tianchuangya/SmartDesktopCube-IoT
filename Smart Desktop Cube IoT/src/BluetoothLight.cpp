#include "BluetoothLight.h"
#include "DataPool.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// ==================== Yeelight LAN Control Protocol ====================
// 协议：TCP 连接到灯带 IP:55443，发送 JSON-RPC 命令，以 \r\n 结尾
// 关键命令：
//   set_power  - 开关灯 ["on"/"off", "smooth", 毫秒]
//   set_bright - 亮度 [1-100, "smooth", 毫秒]
//   set_ct_abx - 色温 [2700-6500K, "smooth", 毫秒]
//
// 三种断连场景处理：
//   1. 开机无 WiFi → 后续 WiFi 连上：WiFi 状态变化检测，触发搜索
//   2. WiFi 中途断开 → 重连：清除旧 IP，重新 SSDP 搜索（IP 可能变）
//   3. WiFi 正常但灯带关着 → 后续开启：TCP 重连 + 搜索退避，灯开即连

// ==================== 静态/全局变量 ====================

BluetoothLightState blLight = {
    .connected = false,
    .brightness = 80,
    .color_temperature = 50,
    .target_brightness = 80,
    .target_color_temp = 50,
    .pending_update = false,
    .connection_attempts = 0,
    .last_fail_time = 0,
    .last_connect_failed = false
};

// 上次非零亮度（用于关灯后恢复）
static uint8_t lastBrightness = 80;

// TCP 客户端
static WiFiClient ylClient;

// Yeelight IP（运行时填充）
static IPAddress ylIP;
static uint16_t  ylPort = YEELIGHT_PORT;
static bool      ylIPFound = false;

// 跟踪 WiFi 状态变化
static bool      lastWifiConnected = false;

// TCP 连续失败计数（超过阈值则清除 IP 重新搜索）
static int       tcp_fail_count = 0;
#define MAX_TCP_FAILS_REDISCOVER 6

// JSON-RPC id 自增
static int rpcId = 1;

// 任务句柄
static TaskHandle_t ylTaskHandle = nullptr;

// ==================== SSDP 自动发现 Yeelight ====================
static bool yl_discover()
{
    WiFiUDP udp;
    udp.begin(1982);

    // 发送 M-SEARCH 多播
    IPAddress multicast(239, 255, 255, 250);
    String searchMsg =
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1982\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "ST: wifi_bulb\r\n"
        "\r\n";

    udp.beginPacket(multicast, 1982);
    udp.write((const uint8_t*)searchMsg.c_str(), searchMsg.length());
    udp.endPacket();

    // 等待响应（最多 2 秒）
    unsigned long start = millis();
    while (millis() - start < 2000) {
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char buf[512];
            int len = udp.read((uint8_t*)buf, sizeof(buf) - 1);
            if (len > 0) buf[len] = '\0';

            // 解析 Location: yeelink://IP:PORT
            char* loc = strstr(buf, "Location: yeelink://");
            if (!loc) loc = strstr(buf, "Location:yeelink://");
            if (loc) {
                // 跳过 "Location:" 或 "Location: " + "yeelink://"
                char* yl = strstr(loc, "yeelink://");
                if (yl) yl += strlen("yeelink://");
                else loc += strlen("Location: yeelink://");
                char* p = yl ? yl : loc;

                // 提取 IP
                String ipStr = "";
                while (*p && *p != ':' && *p != '\r' && *p != '\n' && *p != '/') {
                    ipStr += *p;
                    p++;
                }
                int port = YEELIGHT_PORT;
                if (*p == ':') {
                    p++;
                    port = atoi(p);
                }
                if (ipStr.length() > 0) {
                    ylIP.fromString(ipStr);
                    ylPort = port;
                    ylIPFound = true;
                    return true;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    return false;
}

// ==================== 发送 JSON-RPC 命令 ====================
static void yl_sendRpc(const char* method, const String& params)
{
    if (!ylClient.connected()) return;
    String cmd = "{\"id\":";
    cmd += rpcId++;
    cmd += ",\"method\":\"";
    cmd += method;
    cmd += "\",\"params\":";
    cmd += params;
    cmd += "}\r\n";
    ylClient.print(cmd);
}

// ==================== 实际发送控制指令 ====================
static bool yl_sendCommand(uint8_t brightness, uint8_t color_temp)
{
    if (!ylClient.connected()) return false;

    int ct_kelvin = BL_ColorTempToKelvin(color_temp);

    if (brightness == 0) {
        yl_sendRpc("set_power", "[\"off\",\"smooth\",300]");
    } else {
        yl_sendRpc("set_power", "[\"on\",\"smooth\",300]");
        String bParam = "[";
        bParam += brightness;
        bParam += ",\"smooth\",300]";
        yl_sendRpc("set_bright", bParam);

        String ctParam = "[";
        ctParam += ct_kelvin;
        ctParam += ",\"smooth\",300]";
        yl_sendRpc("set_ct_abx", ctParam);
    }

    blLight.brightness = brightness;
    blLight.color_temperature = color_temp;
    blLight.pending_update = false;
    return true;
}

// ==================== 尝试 TCP 连接（假设 IP 已知）====================
static bool yl_tryConnect()
{
    if (!ylIPFound) return false;

    ylClient.stop();
    if (!ylClient.connect(ylIP, ylPort, 2000)) {
        tcp_fail_count++;
        blLight.last_connect_failed = (tcp_fail_count == 1);
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    blLight.connected = true;
    blLight.connection_attempts = 0;
    blLight.last_connect_failed = false;
    tcp_fail_count = 0;

    // 连接后立即同步当前状态
    if (blLight.pending_update) {
        yl_sendCommand(blLight.target_brightness, blLight.target_color_temp);
    }
    return true;
}

// ==================== 断开 ====================
static void yl_disconnect()
{
    ylClient.stop();
    blLight.connected = false;
    // 不清 pending_update，重连后补发用户指令
}

// ==================== 重置搜索状态 ====================
static void yl_resetSearch()
{
    ylIPFound = false;
    tcp_fail_count = 0;
    blLight.last_connect_failed = false;
    blLight.connection_attempts = 0;
}

// ==================== 灯具管理任务 ====================
// 两层逻辑：
//   层1 — 搜索 IP（SSDP 或硬编码）：指数退避 5s→10s→20s→30s
//   层2 — TCP 连接（IP 已知）：固定 5s 间隔
//   TCP 连续失败 6 次（30s）→ 清 IP 回到层1
//   WiFi 断→连 → 全部重置，立即进入层1
static void yl_Task(void* pvParameters)
{
    uint32_t lastDiscover = 0;
    uint32_t lastConnect = 0;
    uint32_t discoveryInterval = 5000;     // 搜索退避，失败后翻倍
    const uint32_t maxDiscoveryInterval = 30000;
    const uint32_t connectInterval = 5000; // TCP 重连固定 5s

    while (1) {
        uint32_t now = millis();

        // ---- 检测 WiFi 断→连变化 ----
        // 场景1：开机无 WiFi 后续连上
        // 场景2：中途断开后续连上
        if (status.wifi_connected && !lastWifiConnected) {
            yl_disconnect();
            yl_resetSearch();
            discoveryInterval = 5000;
            lastDiscover = now;  // 重置计时，等下个周期再搜
        }
        lastWifiConnected = status.wifi_connected;

        // ---- WiFi 未连接：等待，不浪费 CPU ----
        if (!status.wifi_connected) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // ---- 已连接：维护连接 + 发送指令 ----
        if (blLight.connected) {
            if (blLight.pending_update) {
                if (!yl_sendCommand(blLight.target_brightness, blLight.target_color_temp)) {
                    yl_disconnect();
                }
            }
            // TCP 断了（灯被关掉/拔电源等）
            if (!ylClient.connected()) {
                yl_disconnect();
                // 不清 ylIPFound：灯可能只是被关了一下，IP 没变，优先 TCP 快速重连
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 未连接：层1 搜索 IP ----
        if (!ylIPFound) {
            if (now - lastDiscover >= discoveryInterval) {
                lastDiscover = now;
                bool found = false;
                if (strlen(YEELIGHT_IP) > 0) {
                    ylIP.fromString(YEELIGHT_IP);
                    ylPort = YEELIGHT_PORT;
                    ylIPFound = true;
                    found = true;
                } else {
                    found = yl_discover();
                }
                if (found) {
                    discoveryInterval = 5000;  // 成功→重置退避
                    yl_tryConnect();           // 立即尝试连接
                } else {
                    // 指数退避：5s→10s→20s→30s
                    discoveryInterval = discoveryInterval * 2;
                    if (discoveryInterval > maxDiscoveryInterval)
                        discoveryInterval = maxDiscoveryInterval;
                    blLight.connection_attempts++;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 未连接：层2 TCP 连接（IP 已知）----
        // 场景3：WiFi 正常但灯带关着，后续开启
        if (now - lastConnect >= connectInterval) {
            lastConnect = now;
            yl_tryConnect();
            // TCP 连续失败 6 次（30s）→ IP 可能过期，回到搜索
            if (tcp_fail_count >= MAX_TCP_FAILS_REDISCOVER) {
                yl_resetSearch();
                discoveryInterval = 5000;
                lastDiscover = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ==================== 公开 API ====================

void BL_Init()
{
    xTaskCreatePinnedToCore(
        yl_Task,
        "YeeLight",
        6144,
        NULL,
        2,
        &ylTaskHandle,
        1
    );
}

void BL_SetBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    blLight.target_brightness = brightness;
    blLight.pending_update = true;
    if (brightness > 0) lastBrightness = brightness;
}

void BL_SetColorTemp(uint8_t color_temp) {
    if (color_temp > 100) color_temp = 100;
    blLight.target_color_temp = color_temp;
    blLight.pending_update = true;
}

void BL_SetLight(uint8_t brightness, uint8_t color_temp) {
    if (brightness > 100) brightness = 100;
    if (color_temp > 100) color_temp = 100;
    blLight.target_brightness = brightness;
    blLight.target_color_temp = color_temp;
    blLight.pending_update = true;
    if (brightness > 0) lastBrightness = brightness;
}

void BL_PresetWarm()  { BL_SetLight(100, 8); }
void BL_PresetCool()  { BL_SetLight(80, 100); }
void BL_PresetWhite() { BL_SetLight(80, 47); }

void BL_TurnOff() {
    if (blLight.brightness > 0) lastBrightness = blLight.brightness;
    BL_SetBrightness(0);
}

void BL_TurnOn() {
    BL_SetBrightness(lastBrightness > 0 ? lastBrightness : 80);
}

bool BL_IsConnected() { return blLight.connected; }

bool BL_ConsumeConnectFailedFlag() {
    bool failed = blLight.last_connect_failed;
    blLight.last_connect_failed = false;
    return failed;
}

const char* BL_GetStatusMsg() {
    if (blLight.connected) return "已连接";
    if (blLight.connection_attempts > 0) return "连接失败";
    return "未连接";
}

uint16_t BL_ColorTempToKelvin(uint8_t ct) {
    return 2700 + (uint16_t)ct * 38;
}
