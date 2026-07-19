#include <Arduino.h>
#include "DataPool.h"
#include "DataRW.h"
#include "Task_Control.h"
#include "focus_mode.h"
#include "radar.h"
extern SemaphoreHandle_t dataMutex;


void Task_Realtime_Control(void *pvParameters) {
    while (1) {
        // OTA 期间暂停雷达和专注模式
        if (status.ota_in_progress) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // 1. 高频更新雷达状态 (感知)
        radar_update(); 
        
        // 2. 高频更新专注模式状态机 (决策)
        focusMode_update();

        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms周期，极低延迟
    }
}

// 低频数据任务：负责传感器数据采集、串口输出和云端上报
void Task_DataRead_Init(void *pvParameters) {
    while (1) {
        // OTA 期间暂停传感器采集
        if (status.ota_in_progress) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        xSemaphoreTake(dataMutex, portMAX_DELAY);
        DataRead_ReadAll();      // 读取温湿度、光照、AQI等
        ALLData_ToSerial();      // 串口打印
        xSemaphoreGive(dataMutex);

        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms周期对这类数据足矣
    }
}

// 专注模式任务，只处理低频的总结汇报
void Task_Focus_Init(void *pvParameters) {
    while (1) {
        // 这里未来可以放退出专注模式后的“全量汇报”等低频逻辑
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}