#include <Arduino.h>
#include "WIFiManager.h"
#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "DataPool.h"
extern SemaphoreHandle_t dataMutex;
void Task_Network_Init(void *pvParameters){
    while(1)
    {
        // 始终调用：首次启动 AP+DNS，已连接时内部直接 return
        WiFiManager_Connect();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}