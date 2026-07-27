#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "DataPool.h"
#include "BlinkerApp.h"
#include "ControlPoll.h"
#include <time.h>
#include "esp_heap_caps.h"

/*
 * 堆碎片整理：每 2 分钟执行一次
 * 通过大块分配→缩小→释放，触发 ESP-IDF 分配器的空闲块合并
 */
static void heap_defrag(void) {
    static uint32_t last_defrag = 0;
    uint32_t now = millis();
    if (now - last_defrag < 120000) return;
    last_defrag = now;

    uint32_t before = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);

    // 从大到小尝试 realloc，迫使分配器移动块并合并空闲空间
    for (size_t sz = 16384; sz >= 512; sz >>= 1) {
        void *p = malloc(sz);
        if (p) {
            void *q = realloc(p, 64);
            if (q) {
                free(q);
            } else {
                free(p);
            }
        }
    }

    uint32_t after = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    if (after > before) {
        Serial.printf("[Heap] defrag: max_alloc %u -> %u (+%u)\n",
                      before, after, after - before);
    }
}

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

    // 初始化 HTTP 控制轮询
    ControlPoll_Init();

    while (1) {
        // ---- 0. OTA 期间：完全静默，不连接不收发 ----
        if (status.ota_in_progress) {
            if (mqttIsConnected()) mqttDisconnect();
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // ---- 1. 等待 WiFi 就绪 ----
        if (!status.wifi_connected) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 2. MQTT 重连（优先于 Blinker，避免 Blinker HTTPS 阻塞重连）----
        if (!mqttIsConnected()) {
            if (security.token_ok) {
                security.token_ok = false;
                security.token[0] = '\0';
                security.token_expire_time = 0;
                handshake_sent = false;
            }
            // MQTT 断开 → 立即重置 OTA 检测状态（不让 UI 卡住）
            if (status.ota_check_status == 1) {
                status.ota_check_status = 4;  // failed - MQTT disconnected
                Serial.println("[OTA] MQTT 断开，版本检测已取消");
            }
            status.mqtt_connected = false;
            if (millis() - last_retry >= retry_interval) {
                last_retry = millis();
                Serial.println("[MQTT] 尝试重连...");
                if (mqttConnect(3000)) {
                    status.mqtt_connected = true;
                    Serial.println("[MQTT] 重连成功 ✅");
                } else {
                    Serial.println("[MQTT] 重连失败，5s后重试");
                }
            }
            // Blinker 在 MQTT 断开时也运行（它有独立连接），但放在重连之后不阻塞
            // OTA 期间禁止 Blinker（TLS 争抢硬件 SHA → Double exception）
            if (!status.ota_in_progress) {
                BlinkerApp_Init();
                BlinkerApp_Run();
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // ---- 2.5 MQTT 已连接，运行 Blinker（OTA 期间跳过，防止 TLS 冲突）----
        if (!status.ota_in_progress) {
            BlinkerApp_Init();
            BlinkerApp_Run();
        }

        // ---- 3. 已连接：处理收发 ----
        uint32_t now = millis();

        // OTA 期间：只保持 MQTT 连接（用于上报 OTA 状态），跳过数据推送
        if (status.ota_in_progress) {
            mqttLoop();
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (!handshake_sent && status.time_synced && now - last_handshake >= 5000) {
            mqttSendHandshake();
            handshake_sent = true;
            last_handshake = now;
        }
        if (!security.token_ok) {
            handshake_sent = false;
        }
        // Token 过期检查：仅在 NTP 已同步后才有意义（避免 epoch≈0 时误判过期）
        if (status.time_synced && security.token_ok && security.token_expire_time > 0 && time(NULL) > security.token_expire_time) {
            security.token_ok = false;
            security.token[0] = '\0';
            security.token_expire_time = 0;
            handshake_sent = false;
        }

        mqttLoop();

        // ---- 3.5 手动检查更新请求 ----
        if (status.ota_check_requested) {
            status.ota_check_requested = false;
            if (!security.token_ok) {
                // 握手未完成/token 无效，不发请求（后端会返回401）
                status.ota_check_status = 4;
                Serial.println("[OTA] Token 未就绪（握手未完成），无法检测版本");
            } else {
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

        // 1秒数据上报
        if (now - last_report >= 1000) {
            last_report = now;
            if (security.token_ok) {
                mqttSendDataReport();
            }
        }

        // 1秒推送数据到 Blinker App（独立于 MQTT）
        if (now - last_blinker >= 1000) {
            last_blinker = now;
            BlinkerApp_SendAll();
        }

        // 10秒检查报警阈值，超限时自动发微信通知
        if (now - last_alarm >= 10000) {
            last_alarm = now;
            BlinkerApp_CheckAlarms();
        }

        // HTTP 控制轮询（内部 3 秒间隔，拉取后端控制队列指令）
        ControlPoll_Check();

        // 定期堆碎片整理
        heap_defrag();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
