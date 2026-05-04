#include "MqttTask.h"
#include "MqttCom.h"
#include <Arduino.h>
#include "DataPool.h"
// 仅负责定时心跳、上报（只读写数据池，不操作MQTT连接）
void mqttHeartbeatTask(void *pvParameters)
{
    static uint32_t last_heartbeat = 0;
    static uint32_t last_report = 0;

    while (1)
    {
        // 仅当MQTT连接成功时，才发送数据（避免无效发包）
        if (mqttIsConnected())
        {
            // 30秒心跳
            if (millis() - last_heartbeat >= 30000 && mqttSendHandshake())
            {
                last_heartbeat = millis();
                mqttSendHeartbeat();
                Serial.println("[MQTT] 心跳已发送 ✅");
            }

            // 10秒上报数据
            if (millis() - last_report >= 10000 && mqttSendHandshake())
            {
                last_report = millis();
                mqttSendDataReport();
                Serial.println("[MQTT] 数据已发送 ✅");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // 最小延时，避免占用CPU
    }
}