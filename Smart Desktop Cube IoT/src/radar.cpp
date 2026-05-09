#include "radar.h"
#include "DataPool.h"
#include "MyLD2410.h"

#define RADAR_RX 7
#define RADAR_TX 15

MyLD2410 radar(Serial2);

static bool humanPresent = false;
static unsigned long lastPresenceTime = 0;

void radar_init() {
    Serial2.begin(256000, SERIAL_8N1, RADAR_RX, RADAR_TX);
    if (!radar.begin()) {
        Serial.println("❌ LD2410 雷达初始化失败");
        status.is_human_exist = false;
        return;
    }
    Serial.println("✅ LD2410 雷达初始化成功");
}

void radar_update() {
     radar.check();


    bool detected = radar.presenceDetected();
    unsigned long dist = 0;
    if (detected) {
        dist = radar.detectedDistance();
    }

    focusConfig.human_distance = dist;

    bool inRange = (detected && dist > 0 && dist <= focusConfig.radar_max_distance);

    static bool wasInRange = false;   // 上一轮的有效状态

    if (inRange) {
        wasInRange = true;
        lastPresenceTime = millis();
        humanPresent = true;
    } else {
        if (wasInRange) {

            if (millis() - lastPresenceTime > focusConfig.leave_timeout_ms) {
                wasInRange = false;
                humanPresent = false;
            }

        }
       
    }
    status.is_human_exist = humanPresent;
}

bool radar_isHumanExist() {
    return humanPresent;
}