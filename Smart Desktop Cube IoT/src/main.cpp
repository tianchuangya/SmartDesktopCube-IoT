#include "DataPool.h"
#include "DataRW.h"
#include "DeviceInit.h"
#include <Arduino.h>
void setup()
{
    Serial.begin(115200);
    Device_InitAll();// 初始化传感器

}

void loop()
{
    // 读取所有传感器
    DataRead_ReadAll();
    ALLData_ToSerial();
    status.run_seconds+=1;
    delay(1000);
}