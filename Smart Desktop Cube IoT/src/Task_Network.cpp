#include <Arduino.h>
#include "WIFiManager.h"
#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "DataPool.h"
extern SemaphoreHandle_t dataMutex;
void Task_Network_Init(void *pvParameters){
    while(1)
    {
        
        if(!WiFiManager_IsConnected())WiFiManager_Connect(); 
        else status.wifi_connected = true;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}