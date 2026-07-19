#include "LocalIntelligence.h"
#include "DataPool.h"
#include "lv_port_disp.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

// ==================== 配置阈值 ====================
#define AUTO_BRIGHT_LOW_THRESHOLD   100    // 光照 < 100 lux → 降低亮度
#define AUTO_BRIGHT_HIGH_THRESHOLD 500    // 光照 > 500 lux → 提高亮度
#define AUTO_BRIGHT_LOW_VALUE     40     // 暗环境亮度（0-100）
#define AUTO_BRIGHT_HIGH_VALUE    100    // 亮环境亮度

#define CO2_ALERT_THRESHOLD      1000   // CO2 > 1000ppm 告警
#define TVOC_ALERT_THRESHOLD     0.5f   // TVOC > 0.5 mg/m³ 告警
#define TEMP_LOW_THRESHOLD       18.0f  // 温度 < 18°C 告警
#define TEMP_HIGH_THRESHOLD      28.0f  // 温度 > 28°C 告警
#define HUMI_LOW_THRESHOLD       30.0f  // 湿度 < 30% 告警（干燥）
#define HUMI_HIGH_THRESHOLD      70.0f  // 湿度 > 70% 告警（潮湿）

#define FOCUS_AUTO_ENTER_SECONDS 300    // 人体存在 > 5 分钟自动进入专注
#define HISTORY_RECORD_INTERVAL  300    // 每 5 分钟记录一次历史

#define NIGHT_START_HOUR         22     // 免打扰开始（22:00）
#define NIGHT_END_HOUR           8      // 免打扰结束（08:00）

// ==================== 内部状态 ====================
static uint32_t last_history_record = 0;  // 上次记录历史的时间
static TaskHandle_t intelligenceTaskHandle = nullptr;

// ==================== 免打扰模式判断 ====================
static bool isNightTime() {
    if (!status.time_synced) return false;  // 时间未同步，不启用免打扰
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 500)) return false;
    
    int hour = timeinfo.tm_hour;
    return (hour >= NIGHT_START_HOUR || hour < NIGHT_END_HOUR);
}

// ==================== 自动亮度调节 ====================
static void adjustBrightness() {
    if (!status.auto_brightness_enabled) return;
    
    int target_brightness;
    if (sensorData.light < AUTO_BRIGHT_LOW_THRESHOLD) {
        target_brightness = AUTO_BRIGHT_LOW_VALUE;
    } else if (sensorData.light > AUTO_BRIGHT_HIGH_THRESHOLD) {
        target_brightness = AUTO_BRIGHT_HIGH_VALUE;
    } else {
        // 中间区域：线性插值
        float ratio = (sensorData.light - AUTO_BRIGHT_LOW_THRESHOLD) / 
                      (float)(AUTO_BRIGHT_HIGH_THRESHOLD - AUTO_BRIGHT_LOW_THRESHOLD);
        target_brightness = AUTO_BRIGHT_LOW_VALUE + 
                           ratio * (AUTO_BRIGHT_HIGH_VALUE - AUTO_BRIGHT_LOW_VALUE);
    }
    
    // 只在亮度变化超过 5 时才更新（避免频繁调节）
    if (abs(status.screen_brightness - target_brightness) > 5) {
        status.screen_brightness = target_brightness;
        lv_port_disp_set_backlight(target_brightness);
        Serial.printf("[智能] 自动亮度调节: %d%% (光照 %.0f lux)\n", 
                      target_brightness, sensorData.light);
    }
}

// ==================== 空气质量告警 ====================
static void checkAirQuality() {
    bool alert = false;
    
    if (sensorData.eco2 > CO2_ALERT_THRESHOLD) {
        alert = true;
        if (!status.air_quality_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 空气质量告警: CO2=%.0f ppm (> %d)\n", 
                          sensorData.eco2, CO2_ALERT_THRESHOLD);
        }
    }
    
    if (sensorData.tvoc > TVOC_ALERT_THRESHOLD) {
        alert = true;
        if (!status.air_quality_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 空气质量告警: TVOC=%.2f mg/m³ (> %.1f)\n", 
                          sensorData.tvoc, TVOC_ALERT_THRESHOLD);
        }
    }
    
    status.air_quality_alert = alert;
}

// ==================== 温湿度舒适度告警（人体舒适度指数） ====================
static void checkTempComfort() {
    float temp = sensorData.temp;
    float humi = sensorData.humi;
    bool alert = false;
    
    // 温度超出舒适范围
    if (temp < TEMP_LOW_THRESHOLD) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 温度 %.1f°C (< %.0f°C)\n", 
                          temp, TEMP_LOW_THRESHOLD);
        }
    }
    if (temp > TEMP_HIGH_THRESHOLD) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 温度 %.1f°C (> %.0f°C)\n", 
                          temp, TEMP_HIGH_THRESHOLD);
        }
    }
    
    // 湿度超出舒适范围
    if (humi < HUMI_LOW_THRESHOLD) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 湿度 %.0f%% (< %.0f%%，干燥)\n", 
                          humi, HUMI_LOW_THRESHOLD);
        }
    }
    if (humi > HUMI_HIGH_THRESHOLD) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 湿度 %.0f%% (> %.0f%%，潮湿)\n", 
                          humi, HUMI_HIGH_THRESHOLD);
        }
    }
    
    // 综合效应：高温+高湿 → 体感更热（闷热）
    if (temp > 26.0f && humi > 60.0f) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 闷热 (%.1f°C + %.0f%%)\n", temp, humi);
        }
    }
    
    // 综合效应：低温+高湿 → 体感更冷（湿冷）
    if (temp < 20.0f && humi > 70.0f) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 湿冷 (%.1f°C + %.0f%%)\n", temp, humi);
        }
    }
    
    status.temp_comfort_alert = alert;
}

// ==================== 专注模式自动进入 ====================
static void checkAutoFocus() {
    // 已在专注模式，不重复进入
    if (status.focus_mode) return;
    
    // 专注模式配置未启用自动进入
    if (!focusConfig.auto_enter_enabled) return;
    
    // 检测到人体存在超过阈值
    if (sensorData.human_duration >= FOCUS_AUTO_ENTER_SECONDS) {
        status.focus_mode = true;
        status.request_focus_screen = true;
        Serial.printf("[智能] 自动进入专注模式（人体存在 %d 秒 > %d 秒）\n",
                      sensorData.human_duration, FOCUS_AUTO_ENTER_SECONDS);
    }
}

// ==================== 免打扰模式更新 ====================
static void updateSilentMode() {
    bool was_silent = status.silent_mode;
    status.silent_mode = isNightTime();
    
    if (status.silent_mode && !was_silent) {
        Serial.println("[智能] 进入免打扰模式（夜间）");
    } else if (!status.silent_mode && was_silent) {
        Serial.println("[智能] 退出免打扰模式");
    }
}

// ==================== 记录历史数据 ====================
void RecordHistory(void) {
    sensorHistory.temp[sensorHistory.history_index] = sensorData.temp;
    sensorHistory.humi[sensorHistory.history_index] = sensorData.humi;
    sensorHistory.light[sensorHistory.history_index] = sensorData.light;
    sensorHistory.eco2[sensorHistory.history_index] = sensorData.eco2;
    
    sensorHistory.history_index = (sensorHistory.history_index + 1) % HISTORY_SIZE;
    if (sensorHistory.history_count < HISTORY_SIZE) {
        sensorHistory.history_count++;
    }
}

// ==================== 定时记录历史 ====================
static void checkHistoryRecord() {
    uint32_t now = millis() / 1000;  // 转换为秒
    
    if (now - last_history_record >= HISTORY_RECORD_INTERVAL) {
        last_history_record = now;
        RecordHistory();
        Serial.printf("[智能] 记录历史数据（已记录 %d 个点）\n", sensorHistory.history_count);
    }
}

// ==================== 智能决策主循环 ====================
static void IntelligenceTask(void* pvParameters) {
    Serial.println("[智能] 本地智能决策任务已启动");
    
    while (1) {
        // OTA 期间暂停本地智能决策
        if (status.ota_in_progress) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // 1. 更新免打扰模式
        updateSilentMode();
        
        // 2. 自动亮度调节
        adjustBrightness();
        
        // 3. 空气质量告警
        checkAirQuality();
        
        // 4. 温度舒适度告警
        checkTempComfort();
        
        // 5. 专注模式自动进入
        checkAutoFocus();
        
        // 6. 定时记录历史数据
        checkHistoryRecord();
        
        // 每 5 秒检查一次
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ==================== 初始化 ====================
void LocalIntelligence_Init(void) {
    xTaskCreatePinnedToCore(
        IntelligenceTask,
        "Intelligence",
        4096,       // 栈大小
        NULL,
        2,          // 优先级（中等）
        &intelligenceTaskHandle,
        1           // 核心 1（应用层）
    );
}

// ==================== 历史数据查询 API ====================
int GetHistoryCount(void) {
    return sensorHistory.history_count;
}

void GetHistoryData(float* temp, float* humi, float* light, float* eco2, int max_count) {
    int count = (max_count < sensorHistory.history_count) ? max_count : sensorHistory.history_count;
    
    for (int i = 0; i < count; i++) {
        // 从最旧到最新读取（环形缓冲区）
        int idx = (sensorHistory.history_index - sensorHistory.history_count + i + HISTORY_SIZE) % HISTORY_SIZE;
        temp[i] = sensorHistory.temp[idx];
        humi[i] = sensorHistory.humi[idx];
        light[i] = sensorHistory.light[idx];
        eco2[i] = sensorHistory.eco2[idx];
    }
}
