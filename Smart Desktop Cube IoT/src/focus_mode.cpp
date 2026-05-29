#include "focus_mode.h"
#include "DataPool.h"

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

    // 自动进入专注模式
    if (focusConfig.auto_enter_enabled &&
        humanNow &&
        !status.focus_mode &&
        firstPresenceTime > 0 &&
        (millis() - firstPresenceTime >= focusConfig.auto_enter_delay_ms)) {

        status.focus_mode = true;
        sensorData.focus_duration = 0;
        lastFocusIncrementMs = millis();
        Serial.println("[Focus] ✅ 自动进入专注模式");
    }

    // 自动退出专注模式
    if (focusConfig.auto_exit_enabled &&
        status.focus_mode &&
        !humanNow) {

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
