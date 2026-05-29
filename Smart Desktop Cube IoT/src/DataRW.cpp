#include "DataRW.h"
#include "BH1750.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>
#include "DataPool.h"
// 光照传感器硬件
extern TwoWire Wire_BH1750;
extern BH1750 bh1750;
extern Adafruit_AHTX0 aht;
extern DFRobot_ENS160_I2C ens;

// 根据TVOC/eCO2计算AQI等级(1-5)
static int calcAQI(float tvoc, float eco2)
{
    // 综合TVOC和eCO2评定空气质量等级
    int aqi = 1;  // 默认优
    if (eco2 > 2000 || tvoc > 3000) aqi = 5;       // 劣
    else if (eco2 > 1500 || tvoc > 2000) aqi = 4;  // 差
    else if (eco2 > 1000 || tvoc > 1000) aqi = 3;  // 中
    else if (eco2 > 600  || tvoc > 300)  aqi = 2;  // 良
    return aqi;
}

// 读取所有传感器数据 → 数据池
void DataRead_ReadAll(void)
{
    status.run_seconds = (millis() - status.boot_time) / 1000;

    // ==================== 读取 Bh1750光照 ====================
    if (status.sensor_bh1750) {
        sensorData.light = bh1750.readLightLevel();
    } else {
        sensorData.light = -1;
    }

    // ==================== 读取 AHT20 温湿度 ====================
    if (status.sensor_aht21) {
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);
        sensorData.temp = temp.temperature;
        sensorData.humi = humidity.relative_humidity;
    } else {
        sensorData.temp = -99;
        sensorData.humi = -99;
    }

    // ==================== 读取 ENS160 空气质量 ====================
    if (status.sensor_ens160) {
        sensorData.tvoc = ens.getTVOC();
        sensorData.eco2 = ens.getECO2();
        sensorData.aqi  = calcAQI(sensorData.tvoc, sensorData.eco2);
    } else {
        sensorData.tvoc = -1;
        sensorData.eco2 = -1;
        sensorData.aqi  = 0;
    }
}



// 输出数据 → 串口
void ALLData_ToSerial(void)
{
    Serial.print("Time:");
    Serial.print(status.run_seconds);
    Serial.println("===== 传感器数据 =====");
    // 光照
    if (status.sensor_bh1750)
        Serial.printf("光照: %.2f lx | ", sensorData.light);
    else
        Serial.print("光照: 离线 | ");

    // 温湿度
    if (status.sensor_aht21)
        Serial.printf("温度: %.1f°C | 湿度: %.1f%% | ", sensorData.temp, sensorData.humi);
    else
        Serial.print("温湿度: 离线 | ");

    // 空气质量
    if (status.sensor_ens160)
        Serial.printf("TVOC: %.0f | eCO2: %.0f | AQI: %d\n", sensorData.tvoc, sensorData.eco2, sensorData.aqi);
    else
        Serial.print("空气质量: 离线");

    //wifi连接
    if(status.wifi_connected)
        Serial.printf("wifi名称: %s | wifi密码: %s | ", wifi_config.ssid, wifi_config.pwd);
    else
        Serial.println("WiFi状态: 离线|");

    //专注模式
    if(status.focus_mode)
         Serial.printf("专注模式：启用 |");
    else
        Serial.println("专注模式：关闭 |");
    Serial.println("----------------------------------------");
}
