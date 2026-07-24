#include "WiFiManager.h"
#include "DataPool.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// ==================== 静态变量 ====================
static bool time_synced = false;
static bool wifi_started = false;
static uint32_t wifi_begin_time = 0;
static uint32_t last_status_print = 0;
static uint32_t last_ntp_try = 0;
static bool ap_started = false;

// NVS 存储
static Preferences prefs;
static const char* NVS_NAMESPACE = "wifi_cfg";

// Captive Portal DNS 服务器（将所有域名解析到 AP IP，触发手机弹出页面）
static DNSServer dnsServer;
static bool dns_started = false;

// ==================== NVS 读写 ====================
static void nvsLoadWifi() {
    prefs.begin(NVS_NAMESPACE, true);  // 只读
    String ssid = prefs.getString("ssid", "");
    String pwd  = prefs.getString("pwd", "");
    prefs.end();

    if (ssid.length() > 0) {
        strncpy(wifi_config.ssid, ssid.c_str(), sizeof(wifi_config.ssid) - 1);
        wifi_config.ssid[sizeof(wifi_config.ssid) - 1] = '\0';
        strncpy(wifi_config.pwd, pwd.c_str(), sizeof(wifi_config.pwd) - 1);
        wifi_config.pwd[sizeof(wifi_config.pwd) - 1] = '\0';
        Serial.printf("[WiFi] 从 NVS 加载: SSID=%s\n", wifi_config.ssid);
    } else {
        Serial.printf("[WiFi] NVS 无记录，使用默认: SSID=%s\n", wifi_config.ssid);
    }
}

static void nvsSaveWifi(const char* ssid, const char* pwd) {
    prefs.begin(NVS_NAMESPACE, false);  // 读写
    prefs.putString("ssid", ssid);
    prefs.putString("pwd", pwd);
    prefs.end();
    Serial.printf("[WiFi] 已保存到 NVS: SSID=%s\n", ssid);
}

// ==================== NTP 时间同步 ====================
static void syncTimeOnce() {
    if (time_synced) return;
    if (last_ntp_try > 0 && millis() - last_ntp_try < 30000) return;
    last_ntp_try = millis();

    configTzTime("CST-8", "ntp.aliyun.com", "ntp1.aliyun.com", "pool.ntp.org");

    Serial.println("[NTP] 正在同步时间...");
    struct tm timeinfo;
    for (int retry = 0; retry < 20; retry++) {
        if (getLocalTime(&timeinfo, 500)) {
            time_synced = true;
            status.time_synced = true;
            Serial.printf("[NTP] ✅ 时间同步成功: %04d-%02d-%02d %02d:%02d:%02d\n",
                          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            return;
        }
    }
    Serial.println("[NTP] ⚠ 本次同步超时，30s 后重试");
}

// ==================== 公共 API ====================

bool WiFiManager_IsTimeSynced(void) {
    return time_synced;
}

bool WiFiManager_IsConnected(void) {
    return WiFi.status() == WL_CONNECTED;
}

const char* WiFiManager_GetSSID(void) {
    return wifi_config.ssid;
}

uint8_t WiFiManager_GetAPClientCount(void) {
    return WiFi.softAPgetStationNum();
}

void WiFiManager_Connect(void) {
    // === 首次进入：开启 AP+STA 双模 + 加载 NVS ===
    if (!ap_started) {
        // 从 NVS 加载 WiFi 凭据（覆盖 DataPool 中的默认值）
        nvsLoadWifi();

        WiFi.mode(WIFI_AP_STA);
        bool ap_ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
        if (ap_ok) {
            Serial.printf("[WiFi] ✅ AP 已开启: SSID=%s, IP=%s\n",
                          AP_SSID, WiFi.softAPIP().toString().c_str());
        } else {
            Serial.println("[WiFi] ❌ AP 开启失败");
        }

        // 启动 Captive Portal DNS（端口53，所有域名 → AP IP）
        dnsServer.start(53, "*", WiFi.softAPIP());
        dns_started = true;
        Serial.println("[WiFi] Captive Portal DNS 已启动（手机连接后自动弹出页面）");

        ap_started = true;
    }

    wl_status_t status_wifi = WiFi.status();

    // 已经连上了
    if (status_wifi == WL_CONNECTED) {
        status.wifi_connected = true;
        if (!time_synced) syncTimeOnce();
        return;
    }

    // === 未连接：尝试连接 ===
    if (!wifi_started || (millis() - wifi_begin_time > 15000)) {
        Serial.printf("[WiFi] 开始连接: %s ...\n", wifi_config.ssid);
        WiFi.disconnect();
        delay(100);
        WiFi.begin(wifi_config.ssid, wifi_config.pwd);
        wifi_started = true;
        wifi_begin_time = millis();
        last_status_print = 0;
    }

    // 每 3 秒打印状态
    if (millis() - last_status_print > 3000) {
        last_status_print = millis();
        switch (status_wifi) {
            case WL_NO_SSID_AVAIL:
                Serial.println("[WiFi] ❌ 找不到该热点（需 2.4GHz）");
                break;
            case WL_CONNECT_FAILED:
                Serial.println("[WiFi] ❌ 密码错误或认证失败");
                break;
            case WL_DISCONNECTED:
                Serial.println("[WiFi] ⚠ 已断开，等待重连...");
                break;
            default:
                Serial.println("[WiFi] ⏳ 正在连接中...");
                break;
        }
    }

    // 明确失败 → 允许重试
    if (status_wifi == WL_CONNECT_FAILED || status_wifi == WL_NO_SSID_AVAIL) {
        if (millis() - wifi_begin_time > 5000) {
            wifi_started = false;
        }
    }

    status.wifi_connected = false;
}

// ==================== WiFi 扫描 ====================
String WiFiManager_Scan(void) {
    int n = WiFi.scanNetworks(false, false, false, 300);
    JsonDocument doc;
    JsonArray arr = doc["networks"].to<JsonArray>();

    for (int i = 0; i < n; i++) {
        // 跳过隐藏 SSID 和重复
        if (WiFi.SSID(i).length() == 0) continue;

        JsonObject net = arr.add<JsonObject>();
        net["ssid"] = WiFi.SSID(i);
        net["rssi"] = WiFi.RSSI(i);
        net["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    WiFi.scanDelete();

    String result;
    serializeJson(doc, result);
    return result;
}

// ==================== 连接新 WiFi ====================
bool WiFiManager_ConnectTo(const char* ssid, const char* password) {
    if (strlen(ssid) == 0 || strlen(ssid) >= sizeof(wifi_config.ssid)) return false;

    // 保存到 NVS（重启后自动连接）
    nvsSaveWifi(ssid, password);

    // 更新运行时配置
    strncpy(wifi_config.ssid, ssid, sizeof(wifi_config.ssid) - 1);
    wifi_config.ssid[sizeof(wifi_config.ssid) - 1] = '\0';
    strncpy(wifi_config.pwd, password, sizeof(wifi_config.pwd) - 1);
    wifi_config.pwd[sizeof(wifi_config.pwd) - 1] = '\0';

    // 立即尝试连接
    WiFi.disconnect();
    delay(200);
    WiFi.begin(wifi_config.ssid, wifi_config.pwd);
    wifi_started = true;
    wifi_begin_time = millis();

    Serial.printf("[WiFi] 正在连接新网络: %s\n", ssid);

    // 等待最多 10 秒
    for (int i = 0; i < 20; i++) {
        delay(500);
        if (WiFi.status() == WL_CONNECTED) {
            status.wifi_connected = true;
            time_synced = false;  // 重新同步时间
            Serial.printf("[WiFi] ✅ 已连接: %s, IP=%s\n",
                          ssid, WiFi.localIP().toString().c_str());
            return true;
        }
    }

    Serial.println("[WiFi] ❌ 连接超时（10s），将在后台继续重试");
    return false;
}

// ==================== DNS Loop（供外部任务调用）====================
void WiFiManager_DNSLoop(void) {
    if (dns_started) {
        dnsServer.processNextRequest();
    }
}
