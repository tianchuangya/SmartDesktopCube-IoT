#include "DeviceInit.h"
#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include "DataPool.h"
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>
// 硬件对象定义（全局）
TwoWire Wire_BH1750(0);   // 光照
TwoWire Wire_AHTENS(1);   // 温湿度 + 空气质量 一体模块

// 传感器对象
BH1750              bh1750;
Adafruit_AHTX0      aht;
DFRobot_ENS160_I2C  ens(&Wire_AHTENS, 0x53);

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

    // ========= 初始化 光照传感器=========
    delay(200);
    Wire_BH1750.begin(2, 1); // SDA 2, SCL 1
    Wire_BH1750.setClock(100000);
    if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE,0x23, &Wire_BH1750)) {
     status.sensor_bh1750 = true; 
     Serial.println("✅ BH1750 初始化成功");  
    } else {
    status.sensor_bh1750 = false;
    Serial.println("❌ BH175模块初始化失败");
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
}