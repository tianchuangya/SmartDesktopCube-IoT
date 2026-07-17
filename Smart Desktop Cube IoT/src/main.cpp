#include <Arduino.h>
#include "DataPool.h"
#include "DataRW.h"
#include "DeviceInit.h"
#include "Task_Network.h"
#include "Task_Control.h"
#include "Task_LVGL.h"
#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "BluetoothLight.h"
#include "OTAManager.h"
SemaphoreHandle_t dataMutex;

void setup()
{
    Serial.begin(115200);
    Device_InitAll();// 初始化传感器 + LVGL 屏幕
    mqttInit();
    BL_Init();        // 初始化 Yeelight 灯具控制
    otaInit();        // 初始化 OTA 子系统
    Serial.println("正在预热设备以校准传感器数据，需3min请耐心等待");
    //vTaskDelay(180000 / portTICK_PERIOD_MS);//3min

    // 创建锁
    dataMutex = xSemaphoreCreateMutex();

    // ===== 核心 1：应用层（UI + 控制 + 数据采集 + OTA）=====
    Task_LVGL_Init();  // LVGL 定时器调度，优先级4
    xTaskCreatePinnedToCore(Task_Realtime_Control, "RealtimeCtrl", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Task_DataRead_Init,   "DataRead",    4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(Task_Focus_Init,       "FocusReport", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(otaTaskFunc,           "OTA",         8192, NULL, 2, NULL, 1);

    // ===== 核心 0：网络层（WiFi + MQTT 连接/收发/心跳/上报 统一任务）=====
    xTaskCreatePinnedToCore(Task_Network_Init,     "WiFi",        4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Task_MqttReconnect,    "MQTT_All",    8192, NULL, 1, NULL, 0);
}

void loop()
{
    vTaskDelay(portMAX_DELAY);//loop休眠
}
