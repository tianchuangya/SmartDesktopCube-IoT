#ifndef DATA_READ_H
#define DATA_READ_H

#include "DataPool.h"

// 读取所有传感器数据，并更新到数据池
void DataRead_ReadAll(void);

//全部数据输出到串口
void ALLData_ToSerial(void);
#endif