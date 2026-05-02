#include <Arduino.h>
#include "DataPool.h"
#include "DataRW.h"
#include "DeviceInit.h"
#include "Task_Network.h"
#include "Task_Control.h"
SemaphoreHandle_t dataMutex;
//xTaskCreatePinnedToCore( 任务函数名, "任务名字", 堆栈大小, 传递参数, 优先级, 任务句柄, 核心编号 );
void setup()
{
    Serial.begin(115200);
    Device_InitAll();// 初始化传感器
    Serial.println("正在预热设备以校准传感器数据，需3min请耐心等待");
    //vTaskDelay(180000 / portTICK_PERIOD_MS);//3min

    // 创建锁
    dataMutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(Task_Control_Init,"core 1",16384,NULL,2,NULL,1);//核心1：本地控制大任务
    xTaskCreatePinnedToCore(Task_Network_Init,"core 0",8192,NULL,1,NULL,0);//核心0：网络任务
    

}

void loop()
{
    vTaskDelay(portMAX_DELAY);//loop休眠
}