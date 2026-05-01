#include "DeviceInit.h"
#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include "DataPool.h"

// 硬件对象定义（全局）
TwoWire Wire_BH1750(1);
BH1750 Bh1750;

// 所有初始化
void Device_InitAll(void)
{
    // 初始化串口
    Serial.begin(115200);
    delay(3000);
    Serial.println("系统启动...");
     // ========= 记录开机时间 =========
    status.boot_time = millis();
    status.run_seconds = 0;
    // ========= 初始化 光照传感器=========
    Wire_BH1750.begin(2, 1); // SDA 2, SCL 1
    if (Bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE,0x23, &Wire_BH1750)) {
     status.sensor_bh1750 = true;   
    } else {
    status.sensor_bh1750 = false;
    Serial.println("BH175模块初始化失败！！！");
    }

    // 以后加 AHT21、ENS160、屏幕、WiFi 都放这里！
}