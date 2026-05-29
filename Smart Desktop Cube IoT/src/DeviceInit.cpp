#include "DeviceInit.h"
#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include "DataPool.h"
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>
#include "radar.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "lv_port_disp.h"
#include "lv_port_touch.h"
#include "ui/custom/custom.h"
#include "ui/generated/gui_guider.h"
#include "ui/generated/events_init.h"
#include "Task_LVGL.h"
// 硬件对象定义（全局）
TwoWire Wire_BH1750(0);   // 光照
TwoWire Wire_AHTENS(1);   // 温湿度 + 空气质量 一体模块

// 传感器对象
BH1750              bh1750;
Adafruit_AHTX0      aht;
DFRobot_ENS160_I2C  ens(&Wire_AHTENS, 0x53);
lv_ui guider_ui;

// 所有初始化
void Device_InitAll(void)
{
    // 初始化串口
    Serial.begin(115200);
    delay(3000);
    Serial.println("系统启动...");
     // ========= 记录开机时间 =====    ====
    status.boot_time = millis();
    status.run_seconds = 0;

    // ========= 初始化 光照传感器 =========
    delay(500);  // BH1750 上电稳定需要更长时间
    Wire_BH1750.begin(2, 1); // SDA 2, SCL 1
    Wire_BH1750.setClock(100000);

    // 重试机制：最多3次，失败后复位I2C总线再试
    bool bh1750_ok = false;
    for (int retry = 0; retry < 3; retry++) {
        if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire_BH1750)) {
            bh1750_ok = true;
            break;
        }
        Serial.printf("⚠ BH1750 初始化失败，重试 (%d/3)...\n", retry + 1);
        Wire_BH1750.end();               // 释放I2C总线
        delay(300);
        Wire_BH1750.begin(2, 1);         // 重新初始化I2C
        Wire_BH1750.setClock(100000);
    }

    status.sensor_bh1750 = bh1750_ok;
    if (bh1750_ok) {
        Serial.println("✅ BH1750 初始化成功");
    } else {
        Serial.println("❌ BH1750 模块初始化失败（已重试3次）");
    }

    // ==================== AHT20 ====================
    delay(200);
    Wire_AHTENS.begin(6,5);
    if (aht.begin(&Wire_AHTENS)) {
        Serial.println("✅ AHT20 初始化成功");
        status.sensor_aht21 = true;
    } else {
        Serial.println("❌ AHT20 初始化失败");
        status.sensor_aht21 = false;
    }
    
    // ==================== ENS160 ====================
    delay(200);
    if (ens.begin()== NO_ERR) {
        Serial.println("✅ ENS160 初始化成功");
        // 设置为标准测量模式
        ens.setPWRMode(ENS160_STANDARD_MODE);
        Serial.println("ENS160 已进入标准测量模式");
        //设置温湿度补偿
        ens.setTempAndHum(25.0, 50.0);
        status.sensor_ens160 = true;
    } else {
        Serial.println("❌ ENS160 初始化失败");
        status.sensor_ens160 = false;
    }
    //==================== 屏幕 =======================
    Serial.println("=== LVGL compile-time config (from DeviceInit.cpp) ===");
    Serial.printf("  LV_MEM_SIZE  = %u bytes (%u KB)\n", (unsigned)LV_MEM_SIZE, (unsigned)(LV_MEM_SIZE / 1024));
    Serial.printf("  LV_COLOR_DEPTH = %d\n", LV_COLOR_DEPTH);
    Serial.printf("  LV_TICK_CUSTOM = %d\n", LV_TICK_CUSTOM);
    Serial.printf("  LV_FONT_MONTSERRAT_14 = %d\n", LV_FONT_MONTSERRAT_14);
    Serial.printf("  LV_FONT_MONTSERRAT_16 = %d\n", LV_FONT_MONTSERRAT_16);

    delay(200);
    Serial.println("\n--- Step 1: lv_port_disp_init() ---");
    lv_port_disp_init();// 初始化 LVGL + TFT 显示驱动（内含 lv_init）

    {
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        Serial.printf("  After lv_init: LVGL memory free=%d / total=%d (frag=%d%%)\n",
                      (int)mon.free_size, (int)mon.total_size, (int)mon.frag_pct);
    }

    delay(200);
    Serial.println("\n--- Step 2: lv_port_touch_init() ---");
    lv_port_touch_init();// 初始化 XPT2046 触摸驱动

    delay(200);
    Serial.println("\n--- Step 3: setup_ui(&guider_ui) ---");
    setup_ui(&guider_ui);// 创建界面
    Serial.printf("  scr: bootScreen=%p, mainScreen=%p, airclean=%p, envScreen=%p, fouseScreen=%p\n",
              guider_ui.bootScreen, guider_ui.mainScreen, guider_ui.aircreen,
              guider_ui.envScreen, guider_ui.fouseScreen);

    {
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        Serial.printf("  After setup_ui: LVGL memory free=%d / total=%d (frag=%d%%)\n",
                      (int)mon.free_size, (int)mon.total_size, (int)mon.frag_pct);
    }

    if (guider_ui.aircreen == NULL) {
        Serial.println("  FATAL: airclean screen is NULL! Halting...");
        while(1) vTaskDelay(1000);
    }

    delay(200);
    Serial.println("\n--- Step 4: events_init(&guider_ui) ---");
    events_init(&guider_ui);// 初始化所有事件
    Serial.println("  events_init() done");

    delay(200);
    Serial.println("\n--- Step 5: custom_init(&guider_ui) ---");
    custom_init(&guider_ui);
    Serial.println("  custom_init() done (500ms timer created)");

    Serial.println("\n=== LVGL initialization complete ===\n");

    //==================== LD2410c ====================
    delay(200);
    radar_init();  // 雷达初始化函数
}