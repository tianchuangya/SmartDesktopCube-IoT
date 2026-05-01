#include "DataRW.h"
#include "BH1750.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>

// 光照传感器硬件
extern TwoWire Wire_BH1750;
extern BH1750 bh1750;
extern Adafruit_AHTX0 aht;
extern DFRobot_ENS160_I2C ens;

// 读取所有传感器数据 → 数据池
void DataRead_ReadAll(void)
{
    // ==================== 读取 Bh1750光照 ====================
  if (status.sensor_bh1750) {
    sensorData.light = bh1750.readLightLevel();
    delay(50);
  } else {
    sensorData.light = -1;
  }

  // ==================== 读取 AHT20 温湿度 ====================
    if (status.sensor_aht21) {
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);
        sensorData.temp = temp.temperature;
        sensorData.humi = humidity.relative_humidity;
        delay(50);
    } else {
        sensorData.temp = -99;
        sensorData.humi = -99;
    }

    // ==================== 读取 ENS160 空气质量 ====================
    if (status.sensor_ens160) {
        sensorData.tvoc = ens.getTVOC();
        sensorData.eco2 = ens.getECO2();
        delay(50);
    } else {
        sensorData.tvoc = -1;
        sensorData.eco2 = -1;
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
        Serial.printf("TVOC: %.0f | eCO2: %.0f\n", sensorData.tvoc, sensorData.eco2);
    else
        Serial.println("空气质量: 离线");

    Serial.println("----------------------------------------");
}
