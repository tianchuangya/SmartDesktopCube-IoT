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
DFRobot_ENS160_I2C* ens_ptr = nullptr;  // 运行时根据I2C扫描结果动态创建
lv_ui guider_ui;

// I2C 总线恢复：烧录后传感器可能卡死，发送 9 个 SCL 脉冲释放 SDA
static void i2c_bus_recovery(int sda, int scl) {
    pinMode(sda, INPUT_PULLUP);
    pinMode(scl, OUTPUT_OPEN_DRAIN);
    for (int i = 0; i < 9; i++) {
        digitalWrite(scl, LOW);
        delayMicroseconds(10);
        digitalWrite(scl, HIGH);
        delayMicroseconds(10);
        if (digitalRead(sda)) break;  // SDA 已释放，提前结束
    }
    // 发送 STOP 条件
    digitalWrite(scl, HIGH);
    delayMicroseconds(5);
    pinMode(sda, OUTPUT_OPEN_DRAIN);
    digitalWrite(sda, LOW);
    delayMicroseconds(5);
    digitalWrite(sda, HIGH);
    delayMicroseconds(5);
}

// 所有初始化
void Device_InitAll(void)
{
    // 初始化串口
    Serial.begin(115200);
    delay(5000);  // 烧录后给传感器充足上电稳定时间
    Serial.println("系统启动...");
     // ========= 记录开机时间 =====    ====
    status.boot_time = millis();
    status.run_seconds = 0;

    // ========= 初始化 光照传感器 =========
    delay(200);
    i2c_bus_recovery(2, 1);  // 先恢复可能卡死的 I2C 总线
    Wire_BH1750.begin(2, 1); // SDA 2, SCL 1
    Wire_BH1750.setClock(50000);  // 降速提高稳定性

    // 重试机制：最多5次，失败后复位I2C总线再试
    bool bh1750_ok = false;
    for (int retry = 0; retry < 5; retry++) {
        if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire_BH1750)) {
            bh1750_ok = true;
            break;
        }
        Serial.printf("⚠ BH1750 初始化失败，重试 (%d/5)...\n", retry + 1);
        Wire_BH1750.end();
        delay(500);
        i2c_bus_recovery(2, 1);
        Wire_BH1750.begin(2, 1);
        Wire_BH1750.setClock(50000);
    }

    status.sensor_bh1750 = bh1750_ok;
    if (bh1750_ok) {
        Serial.println("✅ BH1750 初始化成功");
    } else {
        Serial.println("❌ BH1750 模块初始化失败（已重试3次）");
    }

    // ==================== I2C 总线扫描（调试用）====================
    delay(200);
    i2c_bus_recovery(6, 5);  // 先恢复可能卡死的 I2C 总线
    Wire_AHTENS.begin(6,5);
    Wire_AHTENS.setClock(50000);  // 降速提高稳定性
    Serial.println("🔍 扫描 I2C 总线 (Wire_AHTENS, SDA=6, SCL=5)...");
    {
        int found = 0;
        for (byte addr = 1; addr < 127; addr++) {
            Wire_AHTENS.beginTransmission(addr);
            if (Wire_AHTENS.endTransmission() == 0) {
                Serial.printf("  发现设备: 0x%02X (%d)\n", addr, addr);
                found++;
            }
        }
        if (found == 0) Serial.println("  ⚠ 未发现任何 I2C 设备！检查接线/供电");
        else             Serial.printf("  共发现 %d 个设备\n", found);
    }

    // ==================== AHT20 ====================
    if (aht.begin(&Wire_AHTENS)) {
        Serial.println("✅ AHT20 初始化成功");
        status.sensor_aht21 = true;
    } else {
        Serial.println("❌ AHT20 初始化失败");
        status.sensor_aht21 = false;
    }

    // ==================== ENS160 ====================
    delay(200);
    {
        bool ens_ok = false;
        // 依次尝试 0x53 和 0x52（根据模块 ADDR 引脚决定）
        const uint8_t addrs[] = {0x53, 0x52};
        for (int a = 0; a < 2 && !ens_ok; a++) {
            DFRobot_ENS160_I2C* trial = new DFRobot_ENS160_I2C(&Wire_AHTENS, addrs[a]);
            for (int retry = 0; retry < 3; retry++) {
                int ret = trial->begin();
                if (ret == NO_ERR) {
                    ens_ok = true;
                    ens_ptr = trial;
                    Serial.printf("✅ ENS160 初始化成功 (地址 0x%02X)\n", addrs[a]);
                    break;
                }
                Serial.printf("⚠ ENS160 0x%02X 失败 (重试 %d/3), 错误码: %d ",
                              addrs[a], retry + 1, ret);
                if (ret == -1) Serial.println("→ I2C总线不通");
                else if (ret == -2) Serial.println("→ 芯片ID不匹配");

                Wire_AHTENS.end();
                delay(500);
                i2c_bus_recovery(6, 5);
                Wire_AHTENS.begin(6,5);
                Wire_AHTENS.setClock(50000);
            }
            if (!ens_ok) {
                delete trial;  // 尝试下一个地址前释放
            }
        }

        if (ens_ok) {
            ens_ptr->setTempAndHum(25.0, 50.0);
            status.sensor_ens160 = true;
        } else {
            Serial.println("❌ ENS160 模块初始化失败（0x52/0x53 均无法通信）");
            status.sensor_ens160 = false;
        }
    }    //==================== 屏幕 =======================
    Serial.println("=== LVGL compile-time config (from DeviceInit.cpp) ===");
    Serial.printf("  LV_MEM_CUSTOM = %d (PSRAM)\n", LV_MEM_CUSTOM);
#if LV_MEM_CUSTOM == 0
    Serial.printf("  LV_MEM_SIZE  = %u bytes (%u KB)\n", (unsigned)LV_MEM_SIZE, (unsigned)(LV_MEM_SIZE / 1024));
#endif
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