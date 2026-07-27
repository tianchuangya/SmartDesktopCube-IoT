#include "focus_mode.h"
#include "DataPool.h"
#include "radar.h"
#include "LocalIntelligence.h"

static unsigned long firstPresenceTime = 0;
static bool wasHumanLastCycle = false;
static unsigned long lastFocusIncrementMs = 0;
static bool manual_focus_active = false;  // 手动进入专注模式标记（禁止自动退出）

void focusMode_update() {
    bool humanNow = status.is_human_exist;

    // 检测上升沿：从无人 → 有人，记录首次时间
    if (humanNow && !wasHumanLastCycle) {
        firstPresenceTime = millis();
    }

    // 检测下降沿：从有人 → 无人，重置
    if (!humanNow && wasHumanLastCycle) {
        firstPresenceTime = 0;
    }

    // 冷却期检查：手动退出后的一段时间内禁止自动进入
    bool in_cooldown = (focusConfig.last_manual_exit_ms != 0 &&
                        millis() - focusConfig.last_manual_exit_ms < focusConfig.focus_cooldown_ms);

    // 自动进入专注模式（仅当雷达正常 + 用户在 mainScreen + 不在冷却期）
    if (radar_ok &&
        focusConfig.auto_enter_enabled &&
        status.on_main_screen &&
        humanNow &&
        !status.focus_mode &&
        !in_cooldown &&
        firstPresenceTime > 0 &&
        (millis() - firstPresenceTime >= focusConfig.auto_enter_delay_ms)) {

        status.focus_mode = true;
        status.request_focus_screen = true;
        sensorData.focus_duration = 0;
        lastFocusIncrementMs = millis();
        manual_focus_active = false;  // 自动进入，允许自动退出
        FocusSession_Start();         // 开始环境数据记录
        Serial.println("[Focus] ✅ 自动进入专注模式");
    }

    // 自动退出专注模式（仅当雷达正常工作时；手动进入时不自动退出）
    if (radar_ok &&
        focusConfig.auto_exit_enabled &&
        status.focus_mode &&
        !manual_focus_active &&
        !humanNow) {

        FocusSession_End();           // 生成总结 + 触发弹窗
        status.focus_mode = false;
        sensorData.focus_duration = 0;
        Serial.println("[Focus] 自动退出专注模式");
    }

    // 专注中：每秒递增一次
    if (status.focus_mode) {
        unsigned long now = millis();
        if (now - lastFocusIncrementMs >= 1000) {
            sensorData.focus_duration++;
            lastFocusIncrementMs = now;
        }
    }

    // 保存本次周期状态，用于下个周期检测边沿
    wasHumanLastCycle = humanNow;
}

// 用户手动退出专注模式时调用（UI 层触发）
void focusMode_notifyManualExit() {
    FocusSession_End();               // 生成总结 + 触发弹窗
    focusConfig.last_manual_exit_ms = millis();
    firstPresenceTime = 0;
    manual_focus_active = false;
    Serial.printf("[Focus] 🛑 手动退出 → 冷却 %lums 内禁止自动进入\n",
                  focusConfig.focus_cooldown_ms);
}

// 用户手动进入专注模式时调用（UI 层触发）
void focusMode_notifyManualEnter() {
    lastFocusIncrementMs = millis();
    manual_focus_active = true;
    FocusSession_Start();              // 开始环境数据记录
    Serial.println("[Focus] ▶ 手动进入专注模式，计时开始");
}
