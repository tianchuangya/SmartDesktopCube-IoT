#include "DataRW.h"
#include "BH1750.h"
#include <Wire.h>

// 光照传感器硬件
extern TwoWire Wire_BH1750;
extern BH1750 Bh1750;

// 读取所有传感器数据 → 数据池
void DataRead_ReadAll(void)
{
    // 光照
  if (status.sensor_bh1750) {
    sensorData.light = Bh1750.readLightLevel();
  } else {
    sensorData.light = -1;
  }
}



// 输出数据 → 串口
void ALLData_ToSerial(void)
{
    Serial.print("Time:");
    Serial.print(status.run_seconds);
    Serial.println("===== 传感器数据 =====");
    //=========================光照
    Serial.print("光照强度：");
    Serial.print(sensorData.light);
    Serial.println(" lx");

    // 以后可以在这里打印更多数据
    // Serial.print("温度：");
    // Serial.print(sensorData.temp);
    // Serial.println(" ℃");
}
