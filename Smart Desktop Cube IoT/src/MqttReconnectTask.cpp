#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "DataPool.h"
#include "BlinkerApp.h"
#include <time.h>

/*
 * Core 0 统一 MQTT 任务：连接管理 + 收发消息 + 心跳 + 数据上报
 * 所有 MQTT 操作合并到同一核心，避免 PubSubClient 跨核线程安全问题
 * Blinker 数据推送独立计时，2 秒一次，减轻 App UI 压力
 */
void Task_MqttReconnect(void *pvParameters) {

    uint32_t last_retry = 0;
    const uint32_t retry_interval = 5000;

    uint32_t last_heartbeat = 0;
    uint32_t last_report = 0;
    uint32_t last_blinker = 0;
    uint32_t last_alarm = 0;
    uint32_t last_handshake = 0;
    bool     handshake_sent = false;

    while (1) {
        // ---- 1. 等待 WiFi 就绪 ----
        if (!status.wifi_connected) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 1.5 Blinker 初始化 + 运行 ----
        BlinkerApp_Init();
        BlinkerApp_Run();

        // ---- 2. MQTT 重连 ----
        if (!mqttIsConnected()) {
            if (security.token_ok) {
                security.token_ok = false;
                security.token[0] = '\0';
                security.token_expire_time = 0;
                handshake_sent = false;
            }
            if (millis() - last_retry >= retry_interval) {
                last_retry = millis();
                if (mqttConnect(3000)) {
                    status.mqtt_connected = true;
                } else {
                    status.mqtt_connected = false;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 3. 已连接：处理收发 ----
        uint32_t now = millis();

        // OTA 期间：只保持 MQTT 连接（用于上报 OTA 状态），跳过数据推送
        if (status.ota_in_progress) {
            mqttLoop();
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (!handshake_sent && now - last_handshake >= 5000) {
            mqttSendHandshake();
            handshake_sent = true;
            last_handshake = now;
        }
        if (!security.token_ok) {
            handshake_sent = false;
        }
        if (security.token_ok && security.token_expire_time > 0 && time(NULL) > security.token_expire_time) {
            security.token_ok = false;
            security.token[0] = '\0';
            security.token_expire_time = 0;
            handshake_sent = false;
        }

        mqttLoop();

        // ---- 3.5 手动检查更新请求 ----
        if (status.ota_check_requested) {
            status.ota_check_requested = false;
            /* 正在检测中则忽略重复触发，防止连点导致重复发送 */
            if (status.ota_check_status != 1) {
                status.ota_check_status = 1;  // checking
                status.ota_check_time = millis();
                mqttSendVersionCheck();
            }
        }
        // 检测超时：5秒无响应 → 取消本轮检测（状态5=超时）
        if (status.ota_check_status == 1 && (now - status.ota_check_time > 5000)) {
            status.ota_check_status = 5;  // timeout
            Serial.println("[OTA] 版本检测超时（5s无响应），已取消本轮检测");
        }

        // ---- 4. 定时发送 ----
        if (handshake_sent && !security.token_ok && now - last_handshake >= 10000) {
            handshake_sent = false;
        }

        // 30秒心跳
        if (now - last_heartbeat >= 30000) {
            last_heartbeat = now;
            if (security.token_ok) {
                mqttSendHeartbeat();
            }
        }

        // 2秒数据上报
        if (now - last_report >= 2000) {
            last_report = now;
            if (security.token_ok) {
                mqttSendDataReport();
            }
        }

        // 2秒推送数据到 Blinker App（独立于 MQTT）
        if (now - last_blinker >= 2000) {
            last_blinker = now;
            BlinkerApp_SendAll();
        }

        // 10秒检查报警阈值，超限时自动发微信通知
        if (now - last_alarm >= 10000) {
            last_alarm = now;
            BlinkerApp_CheckAlarms();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
