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

// 所有初始化（兼容旧调用）
void Device_InitAll(void)
{
    Device_InitDisplay();
    Device_InitModules();
}

// Phase 1: 显示初始化（快速，让启动画面尽快渲染）
void Device_InitDisplay(void)
{
    Serial.begin(115200);
    Serial.println("系统启动...");
    status.boot_time = millis();
    status.run_seconds = 0;

    //==================== 屏幕 =======================
    Serial.println("=== LVGL compile-time config (from DeviceInit.cpp) ===");
    Serial.printf("  LV_MEM_CUSTOM = %d (PSRAM)\n", LV_MEM_CUSTOM);
    Serial.printf("  LV_COLOR_DEPTH = %d\n", LV_COLOR_DEPTH);

    delay(200);
    lv_port_disp_init();
    delay(200);
    lv_port_touch_init();
    delay(200);
    setup_ui(&guider_ui);
    delay(200);
    events_init(&guider_ui);
    delay(200);
    custom_init(&guider_ui);

    Serial.println("=== Display init complete, boot screen active ===");
}

// Phase 2: 模块加载（传感器+雷达，每个5次重试，在启动画面显示期间运行）
void Device_InitModules(void)
{
    // 给传感器上电稳定时间
    strncpy(status.boot_status_text, "Powering sensors...", sizeof(status.boot_status_text)-1);
    delay(2000);

    // ========= BH1750 光照传感器（5次重试）=========
    strncpy(status.boot_status_text, "BH1750 Light...", sizeof(status.boot_status_text)-1);
    delay(200);
    i2c_bus_recovery(2, 1);
    Wire_BH1750.begin(2, 1);
    Wire_BH1750.setClock(50000);

    bool bh1750_ok = false;
    for (int retry = 0; retry < 5; retry++) {
        if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire_BH1750)) {
            bh1750_ok = true;
            break;
        }
        Serial.printf("BH1750 retry (%d/5)\n", retry + 1);
        Wire_BH1750.end();
        delay(500);
        i2c_bus_recovery(2, 1);
        Wire_BH1750.begin(2, 1);
        Wire_BH1750.setClock(50000);
    }
    status.sensor_bh1750 = bh1750_ok;
    Serial.printf("BH1750: %s\n", bh1750_ok ? "OK" : "FAIL");

    // ========= AHT20 温湿度（5次重试）=========
    strncpy(status.boot_status_text, "AHT20 Temp/Humi...", sizeof(status.boot_status_text)-1);
    delay(200);
    i2c_bus_recovery(6, 5);
    Wire_AHTENS.begin(6, 5);
    Wire_AHTENS.setClock(50000);

    bool aht_ok = false;
    for (int retry = 0; retry < 5; retry++) {
        if (aht.begin(&Wire_AHTENS)) {
            aht_ok = true;
            break;
        }
        Serial.printf("AHT20 retry (%d/5)\n", retry + 1);
        Wire_AHTENS.end();
        delay(500);
        i2c_bus_recovery(6, 5);
        Wire_AHTENS.begin(6, 5);
        Wire_AHTENS.setClock(50000);
    }
    status.sensor_aht21 = aht_ok;
    Serial.printf("AHT20: %s\n", aht_ok ? "OK" : "FAIL");

    // ========= ENS160 空气质量（5次重试）=========
    strncpy(status.boot_status_text, "ENS160 AirQ...", sizeof(status.boot_status_text)-1);
    delay(200);
    {
        bool ens_ok = false;
        const uint8_t addrs[] = {0x53, 0x52};
        for (int a = 0; a < 2 && !ens_ok; a++) {
            DFRobot_ENS160_I2C* trial = new DFRobot_ENS160_I2C(&Wire_AHTENS, addrs[a]);
            for (int retry = 0; retry < 5; retry++) {
                int ret = trial->begin();
                if (ret == NO_ERR) {
                    ens_ok = true;
                    ens_ptr = trial;
                    Serial.printf("ENS160 OK (0x%02X)\n", addrs[a]);
                    break;
                }
                Serial.printf("ENS160 0x%02X retry (%d/5)\n", addrs[a], retry + 1);
                Wire_AHTENS.end();
                delay(500);
                i2c_bus_recovery(6, 5);
                Wire_AHTENS.begin(6, 5);
                Wire_AHTENS.setClock(50000);
            }
            if (!ens_ok) delete trial;
        }
        if (ens_ok) {
            ens_ptr->setTempAndHum(25.0, 50.0);
            status.sensor_ens160 = true;
        } else {
            Serial.println("ENS160: FAIL");
            status.sensor_ens160 = false;
        }
    }

    // ========= LD2410B 雷达（5次重试）=========
    strncpy(status.boot_status_text, "Radar LD2410...", sizeof(status.boot_status_text)-1);
    for (int retry = 0; retry < 5; retry++) {
        radar_init();   // 内部设置全局 radar_ok
        if (radar_ok) break;
        Serial.printf("Radar retry (%d/5)\n", retry + 1);
        delay(500);
    }
    Serial.printf("Radar: %s\n", radar_ok ? "OK" : "FAIL");

    // ========= 启动超时保护：30 秒内必须完成，否则强制进入主界面 =========
    uint32_t init_elapsed = millis() - status.boot_time;
    if (init_elapsed > 30000) {
        Serial.printf("[Init] ⚠ Timeout after %lums, forcing ready\n", init_elapsed);
    }

    // ========= 全部完成 =========
    strncpy(status.boot_status_text, "Ready!", sizeof(status.boot_status_text)-1);
    status.modules_ready = true;
    Serial.println("=== All modules loaded ===");
}