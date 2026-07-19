#ifndef LOCAL_INTELLIGENCE_H
#define LOCAL_INTELLIGENCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 本地智能决策任务初始化（创建 FreeRTOS 任务）
void LocalIntelligence_Init(void);

// 记录传感器历史数据（每 5 分钟调用一次）
void RecordHistory(void);

// 获取历史数据（供 Web API 调用）
int GetHistoryCount(void);
void GetHistoryData(float* temp, float* humi, float* light, float* eco2, int max_count);

#ifdef __cplusplus
}
#endif

#endif
