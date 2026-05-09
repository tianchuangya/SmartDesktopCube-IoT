#include "focus_mode.h"
#include "DataPool.h"

static unsigned long firstPresenceTime = 0;
static bool wasHumanLastCycle = false;

void focusMode_update() {
    bool humanNow = status.is_human_exist;
    // Serial.printf("[Focus] human=%d mode=%d firstTime=%lu distance=%u cm\n",
    //           humanNow, status.focus_mode, firstPresenceTime, focusConfig.human_distance);//仅做调试用
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
        Serial.println("[Focus] ✅ 自动进入专注模式");
    }

    // 自动退出专注模式
    if (focusConfig.auto_exit_enabled &&
        status.focus_mode &&
        !humanNow) {
        
        status.focus_mode = false;
        Serial.println("[Focus] 自动退出专注模式");
    }

    // 保存本次周期状态，用于下个周期检测边沿
    wasHumanLastCycle = humanNow;
}