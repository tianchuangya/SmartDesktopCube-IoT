#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "DataPool.h"
#include <time.h>

/*
 * Core 0 统一 MQTT 任务：连接管理 + 收发消息 + 心跳 + 数据上报
 * 所有 MQTT 操作合并到同一核心，避免 PubSubClient 跨核线程安全问题
 */
void Task_MqttReconnect(void *pvParameters) {

    uint32_t last_retry = 0;
    const uint32_t retry_interval = 5000;  // 每5秒重试一次

    uint32_t last_heartbeat = 0;
    uint32_t last_report = 0;
    uint32_t last_handshake = 0;
    bool     handshake_sent = false;

    while (1) {
        // ---- 1. 等待 WiFi 就绪 ----
        if (!status.wifi_connected) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 2. MQTT 重连 ----
        if (!mqttIsConnected()) {
            // MQTT 断开时清除旧 token，重连后必须重新握手
            if (security.token_ok) {
                security.token_ok = false;
                security.token[0] = '\0';
                security.token_expire_time = 0;
                handshake_sent = false;
                Serial.println("[MQTT] Token 已失效，等待重连后重新握手");
            }
            if (millis() - last_retry >= retry_interval) {
                last_retry = millis();
                Serial.println("[MQTT] Core0 尝试重连...");
                if (mqttConnect(3000)) {
                    status.mqtt_connected = true;
                    Serial.println("[MQTT] 重连成功 ✅");
                } else {
                    status.mqtt_connected = false;
                    Serial.println("[MQTT] 重连失败 ❌");
                }
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 3. 已连接：处理收发 ----
        uint32_t now = millis();

        // 需要握手时发送（最多每5秒一次，避免风暴）
        if (!handshake_sent && now - last_handshake >= 5000) {
            mqttSendHandshake();
            handshake_sent = true;
            last_handshake = now;
            Serial.println("[MQTT] 握手已发送");
        }
        // Token 过期/失效时，重置标志位等待下次握手
        if (!security.token_ok) {
            handshake_sent = false;
        }
        // Token 已过期（当前时间超过过期时间戳），强制重新握手
        // 注意：token_expire_time 是 Unix 时间戳（秒），必须用 time(NULL) 比较
        if (security.token_ok && security.token_expire_time > 0 && time(NULL) > security.token_expire_time) {
            security.token_ok = false;
            security.token[0] = '\0';
            security.token_expire_time = 0;
            handshake_sent = false;
            Serial.println("[MQTT] ⚠ Token 已过期，将重新握手");
        }

        // 处理下行消息（必须频繁调用，驱动 MQTT keepalive）
        mqttLoop();

        // ---- 3.5 手动检查更新请求（来自 settingsScreen 按钮）----
        if (status.ota_check_requested) {
            status.ota_check_requested = false;
            mqttSendVersionCheck();
        }

        // ---- 4. 定时发送 ----
        // 超过10秒没收到 token 则补发握手（公共 broker 丢包常见，缩短重试）
        if (handshake_sent && !security.token_ok && now - last_handshake >= 10000) {
            handshake_sent = false;  // 触发上面重新握手
        }

        // 30秒心跳（Token 未就绪时跳过）
        if (now - last_heartbeat >= 30000) {
            last_heartbeat = now;
            if (security.token_ok) {
                mqttSendHeartbeat();
                Serial.println("[MQTT] 心跳已发送 ✅");
            } else {
                Serial.println("[MQTT] ⚠ 跳过心跳（等待Token）");
            }
        }

        // 5秒数据上报（Token 未就绪时跳过，等握手响应拿到 token 再发）
        if (now - last_report >= 5000) {
            last_report = now;
            if (security.token_ok) {
                mqttSendDataReport();
                Serial.println("[MQTT] 数据已发送 ✅");
            } else {
                Serial.println("[MQTT] ⚠ 跳过数据上报（等待Token）");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));   // 50ms 响应间隔，兼顾 keepalive 和 CPU 占用
    }
}