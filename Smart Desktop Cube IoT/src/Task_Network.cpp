#include <Arduino.h>
#include "WIFiManager.h"
extern SemaphoreHandle_t dataMutex;
void Task_Network_Init(void *pvParameters){
    while(1)
    {
        
        if(!WiFiManager_IsConnected())WiFiManager_Connect();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}