#ifndef FOCUS_MODE_H
#define FOCUS_MODE_H
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// 每轮主循环调用一次，检查并处理专注模式的自动进入/退出
void focusMode_update();

// 通知手动退出：记录失焦时间戳，启动冷却期（冷却期内禁止自动进入）
// 由 UI 层在用户切离 fouseScreen 时调用
void focusMode_notifyManualExit();

// 通知手动进入：初始化计时起点，并禁止自动退出（直到手动退出）
void focusMode_notifyManualEnter();

#ifdef __cplusplus
}
#endif

#endif