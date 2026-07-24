#include <Arduino.h>
#include "DataPool.h"
#include "DataRW.h"
#include "DeviceInit.h"
#include "Task_Network.h"
#include "Task_Control.h"
#include "Task_LVGL.h"
#include "MqttReconnectTask.h"
#include "MqttCom.h"
#include "OTAManager.h"
#include "LocalIntelligence.h"
#include "WebServerTask.h"
SemaphoreHandle_t dataMutex;

/* 模块加载任务：在启动画面显示期间后台初始化传感器 */
static void Task_ModuleLoader(void *pvParameters)
{
    Device_InitModules();
    vTaskDelete(NULL);
}

void setup()
{
    // Phase 1: 快速初始化显示（LVGL + 启动画面）
    Device_InitDisplay();

    // 创建锁
    dataMutex = xSemaphoreCreateMutex();

    // 立即启动 LVGL 任务，让启动画面渲染 + 定时器轮询模块状态
    Task_LVGL_Init();  // LVGL 定时器调度，优先级4

    // Phase 2: 后台加载传感器模块（启动画面期间运行，每个模块5次重试）
    xTaskCreatePinnedToCore(Task_ModuleLoader, "ModuleLoad", 8192, NULL, 3, NULL, 1);

    // 网络 & 通信初始化
    mqttInit();
    otaInit();        // 初始化 OTA 子系统
    LocalIntelligence_Init();  // 初始化本地智能决策（自动亮度、告警、历史数据）

    // ===== 核心 1：应用层（控制 + 数据采集 + OTA）=====
    xTaskCreatePinnedToCore(Task_Realtime_Control, "RealtimeCtrl", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Task_DataRead_Init,   "DataRead",    4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(Task_Focus_Init,       "FocusReport", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(otaTaskFunc,           "OTA",         16384, NULL, 2, NULL, 1);

    // ===== 核心 0：网络层（WiFi + MQTT 连接/收发/心跳/上报 + Web 服务器）=====
    xTaskCreatePinnedToCore(Task_Network_Init,     "WiFi",        4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Task_MqttReconnect,    "MQTT_All",    8192, NULL, 1, NULL, 0);
    WebServerTask_Init();  // Web 服务器（RESTful API）
}

void loop()
{
    vTaskDelay(portMAX_DELAY);//loop休眠
}
