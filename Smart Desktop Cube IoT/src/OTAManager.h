#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>

// OTA 状态枚举
enum OtaState {
    OTA_IDLE = 0,
    OTA_DOWNLOADING,
    OTA_VERIFYING,
    OTA_WRITING,
    OTA_SUCCESS,
    OTA_ERROR
};

// OTA 进度信息（外部只读）
struct OtaProgress {
    OtaState state;
    int      progress;       // 0-100
    size_t   bytes_downloaded;
    size_t   total_bytes;
    char     new_version[32];
    char     error_msg[128];
};

// 初始化 OTA 子系统
void otaInit();

// 触发 OTA（非阻塞，下载+写入在独立 Task 中完成）
// url: 固件 .bin 的 HTTP 下载地址
// version: 新固件版本号（会与当前版本比较）
// md5: 固件 MD5 校验值（可选，空字符串跳过）
bool otaTrigger(const char* url, const char* version, const char* md5);

// 获取当前 OTA 进度（供 UI 轮询）
OtaProgress otaGetProgress();

// OTA 是否正在进行中
bool otaIsInProgress();

// OTA 任务主循环（由 FreeRTOS 任务调用）
void otaTaskFunc(void* pvParameters);

#endif
