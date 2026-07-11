#include "WiFiManager.h"
#include "DataPool.h"
#include <WiFi.h>

static bool time_synced = false;      // NTP 时间同步标志
static bool wifi_started = false;     // 是否已调用 WiFi.begin()
static uint32_t wifi_begin_time = 0;  // 上次 WiFi.begin() 的时间戳
static uint32_t last_status_print = 0;// 上次打印状态的时间
static uint32_t last_ntp_try = 0;     // 上次 NTP 同步尝试的时间

// 初始化 NTP 时间同步（WiFi 连上后调用，失败后每 30s 重试）
static void syncTimeOnce() {
    if (time_synced) return;

    // 失败后每 30 秒重试一次
    if (last_ntp_try > 0 && millis() - last_ntp_try < 30000) return;
    last_ntp_try = millis();

    // CST-8 时区，阿里云 NTP（国内快）+ 国际 NTP 兜底
    configTzTime("CST-8", "ntp.aliyun.com", "ntp1.aliyun.com", "pool.ntp.org");

    Serial.println("[NTP] 正在同步时间...");
    struct tm timeinfo;
    // 最多等 10 秒，每 500ms 检查一次
    for (int retry = 0; retry < 20; retry++) {
        if (getLocalTime(&timeinfo, 500)) {
            time_synced = true;
            Serial.printf("[NTP] ✅ 时间同步成功: %04d-%02d-%02d %02d:%02d:%02d\n",
                          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            Serial.printf("[NTP] Unix timestamp: %ld\n", (long)time(NULL));
            return;
        }
        Serial.printf("[NTP] 等待中... (%d/20)\n", retry + 1);
    }
    Serial.println("[NTP] ⚠ 本次同步超时，30s 后重试");
}

bool WiFiManager_IsTimeSynced(void) {
    return time_synced;
}

void WiFiManager_Connect(void) {
    wl_status_t status_wifi = WiFi.status();

    // 已经连上了，只更新状态
    if (status_wifi == WL_CONNECTED) {
        status.wifi_connected = true;
        if (!time_synced) syncTimeOnce();
        return;
    }

    // === 未连接的处理 ===

    // 如果还没开始连接，或者距离上次 begin 超过 15 秒（连接超时，重试）
    if (!wifi_started || (millis() - wifi_begin_time > 15000)) {
        // 打印内存诊断信息
        Serial.printf("[WiFi] 开始连接: %s ...\n", wifi_config.ssid);
        Serial.printf("[MEM] 内部空闲堆: %u bytes, PSRAM: %s (%u bytes)\n",
                      ESP.getFreeHeap(),
                      psramFound() ? "可用" : "不可用",
                      psramFound() ? ESP.getPsramSize() : 0);
        WiFi.disconnect();
        delay(100);
        WiFi.begin(wifi_config.ssid, wifi_config.pwd);
        wifi_started = true;
        wifi_begin_time = millis();
        last_status_print = 0;  // 重置状态打印计时器
    }

    // 每 3 秒打印一次当前状态（避免刷屏）
    if (millis() - last_status_print > 3000) {
        last_status_print = millis();
        switch (status_wifi) {
            case WL_NO_SSID_AVAIL:
                Serial.println("[WiFi] ❌ 找不到该热点，请检查 WiFi 名称或频段（需 2.4GHz）");
                break;
            case WL_CONNECT_FAILED:
                Serial.println("[WiFi] ❌ 密码错误或认证失败");
                break;
            case WL_DISCONNECTED:
                Serial.println("[WiFi] ⚠ 已断开，等待重连...");
                break;
            case WL_IDLE_STATUS:
                Serial.println("[WiFi] ⏳ 正在连接中...");
                break;
            default:
                Serial.printf("[WiFi] 状态码: %d，等待中...\n", status_wifi);
                break;
        }
    }

    // 连接失败（明确失败，不是连接中）→ 标记为可重试
    if (status_wifi == WL_CONNECT_FAILED || status_wifi == WL_NO_SSID_AVAIL) {
        if (millis() - wifi_begin_time > 5000) {
            // 等了 5 秒还是失败，重置允许下次重试
            wifi_started = false;
        }
    }

    status.wifi_connected = false;
}

bool WiFiManager_IsConnected(void) {
    return WiFi.status() == WL_CONNECTED;
}