#include "DataPool.h"
#include "DataRW.h"
#include "DeviceInit.h"
#include <Arduino.h>
void setup()
{
    Serial.begin(115200);
    Device_InitAll();// 初始化传感器
    Serial.println("正在预热设备以校准传感器数据，需3min请耐心等待");
    delay(180000);//3min
}

void loop()
{
    
    // 读取所有传感器
    DataRead_ReadAll();
    ALLData_ToSerial();
    status.run_seconds+=1;
    delay(1000);
}