#ifndef RADAR_H
#define RADAR_H
#include <Arduino.h>

extern bool radar_ok;        // 雷达是否初始化成功

void radar_init();
void radar_update();          // 每轮循环调用一次，更新数据池
bool radar_isHumanExist();    // 返回当前是否有人
#endif