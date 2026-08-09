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
#define TVOC_ALERT_THRESHOLD     500.0f  // TVOC > 500 ppb 告警（ENS160 返回 ppb 单位）
#define TEMP_LOW_THRESHOLD       18.0f  // 温度 < 18°C 告警
#define TEMP_HIGH_THRESHOLD      28.0f  // 温度 > 28°C 告警
#define HUMI_LOW_THRESHOLD       30.0f  // 湿度 < 30% 告警（干燥）
#define HUMI_HIGH_THRESHOLD      70.0f  // 湿度 > 70% 告警（潮湿）

#define HISTORY_RECORD_INTERVAL  300    // 每 5 分钟记录一次历史

#define NIGHT_START_HOUR         22     // 免打扰开始（22:00）
#define NIGHT_END_HOUR           8      // 免打扰结束（08:00）

// 专注模式休息建议阈值（比常规告警稍低，提前预警）
#define FOCUS_BREAK_CO2          1200   // CO2 > 1200ppm → 建议通风休息
#define FOCUS_BREAK_TEMP_HIGH    28.0f  // 温度 > 28°C → 建议降温
#define FOCUS_BREAK_HUMI_HIGH    75.0f  // 湿度 > 75% → 建议除湿
#define FOCUS_BREAK_COOLDOWN_MS  180000 // 休息建议冷却 3 分钟

// ==================== 内部状态 ====================
static uint32_t last_history_record = 0;
static TaskHandle_t intelligenceTaskHandle = nullptr;

// 前向声明（内部函数）
static void computeFocusScore(float avg_temp, float avg_humi, float avg_co2);
static void predictCO2Trend(void);

// ==================== 免打扰模式判断 ====================
static bool isNightTime() {
    if (!status.time_synced) return false;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 500)) return false;

    int hour = timeinfo.tm_hour;
    return (hour >= NIGHT_START_HOUR || hour < NIGHT_END_HOUR);
}

// ==================== 自动亮度调节 ====================
static void adjustBrightness() {
    if (!status.auto_brightness_enabled) return;
    if (sensorData.light < 0) return;  // 光照传感器离线，跳过自动调节

    int target_brightness;
    if (sensorData.light < AUTO_BRIGHT_LOW_THRESHOLD) {
        target_brightness = AUTO_BRIGHT_LOW_VALUE;
    } else if (sensorData.light > AUTO_BRIGHT_HIGH_THRESHOLD) {
        target_brightness = AUTO_BRIGHT_HIGH_VALUE;
    } else {
        float ratio = (sensorData.light - AUTO_BRIGHT_LOW_THRESHOLD) /
                      (float)(AUTO_BRIGHT_HIGH_THRESHOLD - AUTO_BRIGHT_LOW_THRESHOLD);
        target_brightness = AUTO_BRIGHT_LOW_VALUE +
                           ratio * (AUTO_BRIGHT_HIGH_VALUE - AUTO_BRIGHT_LOW_VALUE);
    }

    if (abs(status.screen_brightness - target_brightness) > 5) {
        status.screen_brightness = target_brightness;
        lv_port_disp_set_backlight(target_brightness);
        Serial.printf("[智能] 自动亮度调节: %d%% (光照 %.0f lux)\n",
                      target_brightness, sensorData.light);
    }
}

// ==================== 空气质量告警 ====================
static uint32_t last_air_toast_ms = 0;
#define AIR_ALERT_TOAST_COOLDOWN_MS  120000  // 2分钟冷却

static void checkAirQuality() {
    // 传感器数据无效时跳过告警（-1 = 传感器离线或未初始化）
    if (sensorData.eco2 < 0 && sensorData.tvoc < 0) {
        status.air_quality_alert = false;
        return;
    }

    bool alert = false;
    const char* reason = "";

    if (sensorData.eco2 > 0 && sensorData.eco2 > CO2_ALERT_THRESHOLD) {
        alert = true;
        reason = "CO2";
        if (!status.air_quality_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 空气质量告警: CO2=%.0f ppm (> %d)\n",
                          sensorData.eco2, CO2_ALERT_THRESHOLD);
        }
    }

    if (sensorData.tvoc > 0 && sensorData.tvoc > TVOC_ALERT_THRESHOLD) {
        alert = true;
        if (!reason[0]) reason = "TVOC";
        if (!status.air_quality_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 空气质量告警: TVOC=%.0f ppb (> %.0f)\n",
                          sensorData.tvoc, TVOC_ALERT_THRESHOLD);
        }
    }

    // 告警状态由 false → true 时，弹出屏幕提示 + 标记待发送微信
    if (alert && !status.air_quality_alert) {
        uint32_t now = millis();
        status.pending_wechat_air = true;  // 暂存告警，等Blinker认证后补发微信
        if (last_air_toast_ms == 0 || now - last_air_toast_ms >= AIR_ALERT_TOAST_COOLDOWN_MS) {
            snprintf(status.pending_toast, sizeof(status.pending_toast),
                     "Air quality alert!\n%s high, ventilate", reason);
            status.toast_duration_ms = 5000;
            status.toast_pending = true;
            last_air_toast_ms = now;
        }
    }

    status.air_quality_alert = alert;
}

// ==================== 温湿度舒适度告警 ====================
static void checkTempComfort() {
    float temp = sensorData.temp;
    float humi = sensorData.humi;

    // 传感器数据无效时跳过告警
    if (temp < 0 || humi < 0) {
        status.temp_comfort_alert = false;
        return;
    }

    bool alert = false;

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

    // 闷热
    if (temp > 26.0f && humi > 60.0f) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 闷热 (%.1f°C + %.0f%%)\n", temp, humi);
        }
    }

    // 湿冷
    if (temp < 20.0f && humi > 70.0f) {
        alert = true;
        if (!status.temp_comfort_alert && !status.silent_mode) {
            Serial.printf("[智能] ⚠ 舒适度告警: 湿冷 (%.1f°C + %.0f%%)\n", temp, humi);
        }
    }

    status.temp_comfort_alert = alert;
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
    // 传感器未就绪时跳过，避免污染历史缓冲区
    if (sensorData.temp < 0 || sensorData.eco2 < 0) return;

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
    uint32_t now = millis() / 1000;

    if (now - last_history_record >= HISTORY_RECORD_INTERVAL) {
        last_history_record = now;
        RecordHistory();
        Serial.printf("[智能] 记录历史数据（已记录 %d 个点）\n", sensorHistory.history_count);
    }
}

// ==================== 专注会话：开始记录 ====================
void FocusSession_Start(void) {
    focusSession.start_time = millis();
    focusSession.duration_sec = 0;
    focusSession.temp_sum = 0.0f;
    focusSession.humi_sum = 0.0f;
    focusSession.eco2_sum = 0.0f;
    focusSession.sample_count = 0;
    focusSession.last_break_suggest = 0;
    focusSession.active = true;
    Serial.println("[专注] 会话记录开始");
}

// ==================== 专注会话：累积采样 ====================
static void focusSession_update(void) {
    if (!focusSession.active) return;

    // 传感器未就绪时跳过本次采样，避免 -1.0 拉低均值
    if (sensorData.temp < 0 || sensorData.humi < 0 || sensorData.eco2 < 0) return;

    // 每次 IntelligenceTask 循环（5秒）累积一次采样
    focusSession.temp_sum += sensorData.temp;
    focusSession.humi_sum += sensorData.humi;
    focusSession.eco2_sum += sensorData.eco2;
    focusSession.sample_count++;
}

// ==================== 专注会话：结束 + 生成总结 ====================
void FocusSession_End(void) {
    if (!focusSession.active) return;

    // 计算会话时长
    focusSession.duration_sec = (millis() - focusSession.start_time) / 1000;

    if (focusSession.sample_count > 0 && focusSession.duration_sec >= 5) {
        float avg_temp = focusSession.temp_sum / focusSession.sample_count;
        float avg_humi = focusSession.humi_sum / focusSession.sample_count;
        float avg_co2  = focusSession.eco2_sum / focusSession.sample_count;

        // 计算专注质量评分
        computeFocusScore(avg_temp, avg_humi, avg_co2);

        int mins = focusSession.duration_sec / 60;
        int secs = focusSession.duration_sec % 60;

        // 评分等级文字
        const char* grade;
        if (focusSession.quality_score >= 90)      grade = "Excellent";
        else if (focusSession.quality_score >= 75) grade = "Good";
        else if (focusSession.quality_score >= 60) grade = "Fair";
        else                                       grade = "Improve";

        // 生成总结文本（含评分）
        snprintf(status.focus_summary_text, sizeof(status.focus_summary_text),
                 "Focus %dm %ds  Score: %d\nAvg %.1fC | %.0f%% | %.0fppm\nAir: %s",
                 mins, secs, focusSession.quality_score,
                 avg_temp, avg_humi, avg_co2, grade);

        status.focus_summary_pending = true;

        Serial.printf("[专注] 会话总结: %d分%d秒, 评分%d(%s), T=%.1fC H=%.0f%% CO2=%.0fppm\n",
                      mins, secs, focusSession.quality_score, grade,
                      avg_temp, avg_humi, avg_co2);
    } else {
        Serial.println("[专注] 会话结束（时长过短，不生成总结）");
    }

    focusSession.active = false;
}

// ==================== 专注质量评分算法 ====================
// 根据专注期间的平均环境条件打分（0-100）
// 权重：CO2 40% + 温度 35% + 湿度 25%
static void computeFocusScore(float avg_temp, float avg_humi, float avg_co2) {
    int score = 100;

    // CO2 扣分（权重 40%）：>600ppm 开始扣分
    if (avg_co2 > 600) {
        int penalty = (int)((avg_co2 - 600) / 14.0f);
        if (penalty > 40) penalty = 40;
        score -= penalty;
    }

    // 温度扣分（权重 35%）：舒适区 22-26°C
    if (avg_temp < 22.0f) {
        int penalty = (int)((22.0f - avg_temp) * 8.75f);
        if (penalty > 35) penalty = 35;
        score -= penalty;
    } else if (avg_temp > 26.0f) {
        int penalty = (int)((avg_temp - 26.0f) * 8.75f);
        if (penalty > 35) penalty = 35;
        score -= penalty;
    }

    // 湿度扣分（权重 25%）：舒适区 40-60%
    if (avg_humi < 40.0f) {
        int penalty = (int)((40.0f - avg_humi) * 1.25f);
        if (penalty > 25) penalty = 25;
        score -= penalty;
    } else if (avg_humi > 60.0f) {
        int penalty = (int)((avg_humi - 60.0f) * 1.25f);
        if (penalty > 25) penalty = 25;
        score -= penalty;
    }

    if (score < 0) score = 0;
    focusSession.quality_score = (uint8_t)score;
}

// ==================== CO2 趋势预测（线性回归）====================
// 用最近 30 分钟的传感器历史数据做线性回归，预测 CO2 何时超标
#define TREND_POINTS      6       // 取最近 6 个点（6×5min = 30min）
#define TREND_INTERVAL_MS 30000   // 每 30 秒预测一次
#define CO2_PREDICT_THRESHOLD 1000 // 预测目标阈值（ppm）

static uint32_t last_trend_predict = 0;

static void predictCO2Trend(void) {
    uint32_t now = millis();
    if (now - last_trend_predict < TREND_INTERVAL_MS) return;
    last_trend_predict = now;

    int count = sensorHistory.history_count;
    if (count < 3) {
        status.co2_trend_text[0] = '\0';
        return;
    }

    int n = (count < TREND_POINTS) ? count : TREND_POINTS;

    // 从环形缓冲区取出最近 n 个 CO2 值（按时间顺序）
    float y[TREND_POINTS];
    for (int i = 0; i < n; i++) {
        int idx = (sensorHistory.history_index - n + i + HISTORY_SIZE) % HISTORY_SIZE;
        y[i] = sensorHistory.eco2[idx];
    }

    // 最小二乘法线性回归：y = a + b*x
    float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    for (int i = 0; i < n; i++) {
        float x = (float)i;
        sum_x += x;
        sum_y += y[i];
        sum_xy += x * y[i];
        sum_x2 += x * x;
    }

    float denom = (float)n * sum_x2 - sum_x * sum_x;
    if (fabsf(denom) < 0.001f) {
        snprintf(status.co2_trend_text, sizeof(status.co2_trend_text),
                 "CO2 stable ~%.0fppm", y[n - 1]);
        return;
    }

    float b = ((float)n * sum_xy - sum_x * sum_y) / denom;  // 斜率（ppm/5min）
    float current = y[n - 1];

    if (b > 5.0f && current < CO2_PREDICT_THRESHOLD) {
        // CO2 在上升，预测多久后超标
        float remain = CO2_PREDICT_THRESHOLD - current;
        float intervals = remain / b;           // 以 5 分钟为单位
        int minutes = (int)(intervals * 5.0f);

        if (minutes > 0 && minutes <= 120) {
            snprintf(status.co2_trend_text, sizeof(status.co2_trend_text),
                     "CO2 rising ~%dmin to ventilate", minutes);
        } else if (minutes > 120) {
            snprintf(status.co2_trend_text, sizeof(status.co2_trend_text),
                     "CO2 rising slowly, OK for now");
        } else {
            snprintf(status.co2_trend_text, sizeof(status.co2_trend_text),
                     "CO2 rising, ventilate soon");
        }
    } else if (b < -5.0f) {
        snprintf(status.co2_trend_text, sizeof(status.co2_trend_text),
                 "CO2 dropping, air improving");
    } else {
        snprintf(status.co2_trend_text, sizeof(status.co2_trend_text),
                 "CO2 stable ~%.0fppm", current);
    }
}

// ==================== 专注模式：休息建议弹窗 ====================
static void checkFocusBreak(void) {
    if (!status.focus_mode || !focusSession.active) return;

    uint32_t now = millis();
    if (focusSession.last_break_suggest != 0 &&
        now - focusSession.last_break_suggest < FOCUS_BREAK_COOLDOWN_MS) {
        return;  // 冷却期内不重复提示
    }

    // CO2 过高 → 建议通风
    if (sensorData.eco2 > FOCUS_BREAK_CO2) {
        snprintf(status.pending_toast, sizeof(status.pending_toast),
                 "CO2 %.0fppm high!\nTake a break, ventilate", sensorData.eco2);
        status.toast_duration_ms = 5000;
        status.toast_pending = true;
        focusSession.last_break_suggest = now;
        Serial.printf("[专注] 休息建议: CO2=%.0f > %d\n", sensorData.eco2, FOCUS_BREAK_CO2);
        return;
    }

    // 温度过高 → 建议降温
    if (sensorData.temp > FOCUS_BREAK_TEMP_HIGH) {
        snprintf(status.pending_toast, sizeof(status.pending_toast),
                 "%.1fC too hot!\nSuggest AC or a break", sensorData.temp);
        status.toast_duration_ms = 5000;
        status.toast_pending = true;
        focusSession.last_break_suggest = now;
        Serial.printf("[专注] 休息建议: temp=%.1f > %.0f\n", sensorData.temp, FOCUS_BREAK_TEMP_HIGH);
        return;
    }

    // 湿度过高 → 建议除湿
    if (sensorData.humi > FOCUS_BREAK_HUMI_HIGH) {
        snprintf(status.pending_toast, sizeof(status.pending_toast),
                 "Humidity %.0f%% high\nSuggest dehumidify", sensorData.humi);
        status.toast_duration_ms = 5000;
        status.toast_pending = true;
        focusSession.last_break_suggest = now;
        Serial.printf("[专注] 休息建议: humi=%.0f > %.0f\n", sensorData.humi, FOCUS_BREAK_HUMI_HIGH);
        return;
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

        // 5. 专注模式数据记录（累积采样）
        focusSession_update();

        // 6. 专注模式休息建议
        checkFocusBreak();

        // 7. CO2 趋势预测（线性回归）
        predictCO2Trend();

        // 8. 定时记录历史数据
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
        6144,       // 栈大小（增加了专注会话逻辑）
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
        int idx = (sensorHistory.history_index - sensorHistory.history_count + i + HISTORY_SIZE) % HISTORY_SIZE;
        temp[i] = sensorHistory.temp[idx];
        humi[i] = sensorHistory.humi[idx];
        light[i] = sensorHistory.light[idx];
        eco2[i] = sensorHistory.eco2[idx];
    }
}
