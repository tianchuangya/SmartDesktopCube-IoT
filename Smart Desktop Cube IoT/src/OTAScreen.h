#ifndef OTA_SCREEN_H
#define OTA_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// 显示 OTA 更新屏幕（使用现有 lockScreen 作为基础，动态添加进度条和文字）
// currentVer: 当前固件版本
// newVer:     新固件版本
void otaScreenShow(const char* currentVer, const char* newVer);

// 更新进度条和状态文字
// percent: 0-100
// status:  状态描述文字（如 "正在下载固件...45%"）
void otaScreenUpdateProgress(int percent, const char* status);

// 显示错误信息
void otaScreenSetError(const char* error);

// 隐藏 OTA 屏幕（恢复之前的状态）
void otaScreenHide();

#ifdef __cplusplus
}
#endif

#endif
