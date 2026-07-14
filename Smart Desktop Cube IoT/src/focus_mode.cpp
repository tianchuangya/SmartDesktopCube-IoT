#include "focus_mode.h"
#include "DataPool.h"
#include "radar.h"
#include "BluetoothLight.h"

static unsigned long firstPresenceTime = 0;
static bool wasHumanLastCycle = false;
static unsigned long lastFocusIncrementMs = 0;

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
        BL_PresetWarm();  // 进入专注 → 暖光+最亮
        Serial.println("[Focus] ✅ 自动进入专注模式 → 灯光切换为暖光最亮");
    }

    // 自动退出专注模式（仅当雷达正常工作时；雷达故障时由手动按钮控制）
    if (radar_ok &&
        focusConfig.auto_exit_enabled &&
        status.focus_mode &&
        !humanNow) {

        status.focus_mode = false;
        sensorData.focus_duration = 0;
        BL_PresetWhite();  // 退出专注 → 恢复正常白光
        Serial.println("[Focus] 自动退出专注模式 → 灯光恢复白光");
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
    focusConfig.last_manual_exit_ms = millis();  // 记录退出时间，启动冷却
    firstPresenceTime = 0;                        // 重置累积时间，避免冷却期内误触发
    BL_PresetWhite();                             // 恢复白光
    Serial.printf("[Focus] 🛑 手动退出 → 冷却 %lums 内禁止自动进入\n",
                  focusConfig.focus_cooldown_ms);
}
