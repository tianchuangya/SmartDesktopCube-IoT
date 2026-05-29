#include <Arduino.h>
#include "DataPool.h"
#include "DataRW.h"
#include "DeviceInit.h"
#include "Task_Network.h"
#include "Task_Control.h"
#include "Task_LVGL.h"
#include "MqttTask.h"
#include "MqttReconnectTask.h"
#include "MqttCom.h"
SemaphoreHandle_t dataMutex;
//xTaskCreatePinnedToCore( 任务函数名, "任务名字", 堆栈大小, 传递参数, 优先级, 任务句柄, 核心编号 );
void setup()
{
    Serial.begin(115200);
    Device_InitAll();// 初始化传感器 + LVGL 屏幕
    mqttInit();
    Serial.println("正在预热设备以校准传感器数据，需3min请耐心等待");
    //vTaskDelay(180000 / portTICK_PERIOD_MS);//3min

    // 创建锁
    dataMutex = xSemaphoreCreateMutex();

    // 核心 1：UI + 控制 + 数据采集
    // LVGL优先级最高(4)，确保UI不卡顿
    Task_LVGL_Init();  // LVGL 定时器调度，核心1，优先级4
    xTaskCreatePinnedToCore(Task_Realtime_Control, "RealtimeCtrl", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Task_DataRead_Init,   "DataRead",    4096, NULL, 2, NULL, 1);
    //xTaskCreatePinnedToCore(Task_MqttReconnect,    "MQTTRecon",   8192, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Task_Focus_Init,       "FocusReport", 4096, NULL, 1, NULL, 1);

    // 核心 0：网络相关 (非阻塞)
    //xTaskCreatePinnedToCore(Task_Network_Init,     "WiFi",        4096, NULL, 1, NULL, 0);
    //xTaskCreatePinnedToCore(mqttHeartbeatTask,     "Heartbeat",   8192, NULL, 1, NULL, 0);
}

void loop()
{
    vTaskDelay(portMAX_DELAY);//loop休眠
}
