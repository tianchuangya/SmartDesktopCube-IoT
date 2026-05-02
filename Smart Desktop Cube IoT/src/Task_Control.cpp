#include <Arduino.h>
#include "DataPool.h"
#include "DataRW.h"
#include "Task_Control.h"
extern SemaphoreHandle_t dataMutex;

void Task_Control_Init(void *pvParameters){
while(1){
    // 加锁
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    DataRead_ReadAll();
    ALLData_ToSerial();
    // 解锁
    xSemaphoreGive(dataMutex);


    vTaskDelay(500 / portTICK_PERIOD_MS);//延时500ms
}
}