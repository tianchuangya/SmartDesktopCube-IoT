#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "DataPool.h"

void Task_MqttReconnect(void *pvParameters) {
    
    uint32_t last_retry = 0;
    const uint32_t retry_interval = 5000;  // 每5秒重试一次

    while (1) {
        if(!status.wifi_connected){vTaskDelay(pdMS_TO_TICKS(200));continue;}
        if (!mqttIsConnected()) {
            if (millis() - last_retry >= retry_interval) {
                last_retry = millis();
                Serial.println("[MQTT] Core0 尝试重连...");
                if (mqttConnect(3000)) {   // 3秒超时连接
                    mqttSendHandshake();
                    status.mqtt_connected = true;
                    Serial.println("[MQTT] 重连成功 ✅");
                } else {
                    status.mqtt_connected = false;
                    Serial.println("[MQTT] 重连失败 ❌");
                }
            }
        } else {
            // 已连接时保持循环，接收下行指令
            mqttLoop();
        }
        vTaskDelay(pdMS_TO_TICKS(200));  // 100ms检查一次
    }
}