#include "Task_LVGL.h"
#include "lvgl.h"
#include <Arduino.h>

static void lvgl_task(void *pvParameters)
{
    const TickType_t xDelay = 5 / portTICK_PERIOD_MS;
    bool once = true;
    for (;;) {
        lv_timer_handler();
        if (once) {
            Serial.println("[LVGL task] running on core " + String(xPortGetCoreID()));
            once = false;
        }
        vTaskDelay(xDelay);
    }
}

void Task_LVGL_Init(void)
{
    xTaskCreatePinnedToCore(lvgl_task, "LVGL", 8192, NULL, 4, NULL, 1);
}
