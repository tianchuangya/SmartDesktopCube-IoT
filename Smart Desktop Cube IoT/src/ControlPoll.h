#ifndef CONTROL_POLL_H
#define CONTROL_POLL_H

#include <Arduino.h>

// 初始化 HTTP 控制轮询模块
void ControlPoll_Init(void);

// 每轮调用：检查是否有待执行的控制指令（建议 3 秒调用一次）
void ControlPoll_Check(void);

#endif
