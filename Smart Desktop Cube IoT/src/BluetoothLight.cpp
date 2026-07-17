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
// 替换原 BLE + ESP32-C3 方案：不需要额外控制器，ESP32 → WiFi → Yeelight 直连

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

    // 等待响应（最多 3 秒）
    unsigned long start = millis();
    while (millis() - start < 3000) {
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char buf[512];
            int len = udp.read((uint8_t*)buf, sizeof(buf) - 1);
            if (len > 0) buf[len] = '\0';

            // 解析 Location: yeelink://IP:PORT
            char* loc = strstr(buf, "Location: yeelink://");
            if (!loc) loc = strstr(buf, "Location:yeelink://");
            if (loc) {
                loc += strlen("Location: yeelink://");
                if (loc == buf || strncmp(loc - 1, "yeelink://", 10) != 0) {
                    // adjust if no space
                }
                // 找到 IP:PORT
                String ipStr = "";
                while (*loc && *loc != ':' && *loc != '\r' && *loc != '\n' && *loc != '/') {
                    ipStr += *loc;
                    loc++;
                }
                int port = YEELIGHT_PORT;
                if (*loc == ':') {
                    loc++;
                    port = atoi(loc);
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

    // 色温: 0-100 → 2700K-6500K
    int ct_kelvin = BL_ColorTempToKelvin(color_temp);

    if (brightness == 0) {
        // 关灯
        yl_sendRpc("set_power", "[\"off\",\"smooth\",300]");
    } else {
        // 开灯 + 设亮度 + 设色温
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

// ==================== 连接 Yeelight ====================
static bool yl_connect()
{
    if (blLight.connected) return true;
    if (!status.wifi_connected) return false;

    // 获取 IP
    if (!ylIPFound) {
        if (strlen(YEELIGHT_IP) > 0) {
            ylIP.fromString(YEELIGHT_IP);
            ylPort = YEELIGHT_PORT;
            ylIPFound = true;
        } else {
            yl_discover();
        }
    }

    if (!ylIPFound) {
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    // TCP 连接
    ylClient.stop();
    if (!ylClient.connect(ylIP, ylPort, 3000)) {
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    blLight.connected = true;
    blLight.connection_attempts = 0;
    blLight.last_connect_failed = false;

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
    blLight.pending_update = false;
}

// ==================== 灯具管理任务 ====================
static void yl_Task(void* pvParameters)
{
    uint32_t lastConnectAttempt = 0;
    const uint32_t reconnectInterval = 5000;

    while (1) {
        uint32_t now = millis();

        if (!blLight.connected) {
            if (now - lastConnectAttempt > reconnectInterval) {
                lastConnectAttempt = now;
                yl_connect();
            }
        } else {
            // 已连接：发送待更新指令
            if (blLight.pending_update) {
                if (!yl_sendCommand(blLight.target_brightness, blLight.target_color_temp)) {
                    yl_disconnect();
                }
            }

            // 检查 TCP 连接是否仍然有效
            if (!ylClient.connected()) {
                yl_disconnect();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ==================== 公开 API ====================

void BL_Init()
{
    // 创建灯具管理任务（运行在核心1，优先级2）
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

// ==================== 一键预设模板 ====================

void BL_PresetWarm() {
    BL_SetLight(100, 8);
}

void BL_PresetCool() {
    BL_SetLight(80, 100);
}

void BL_PresetWhite() {
    BL_SetLight(80, 47);
}

void BL_TurnOff() {
    if (blLight.brightness > 0) {
        lastBrightness = blLight.brightness;
    }
    BL_SetBrightness(0);
}

void BL_TurnOn() {
    BL_SetBrightness(lastBrightness > 0 ? lastBrightness : 80);
}

bool BL_IsConnected() {
    return blLight.connected;
}

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
