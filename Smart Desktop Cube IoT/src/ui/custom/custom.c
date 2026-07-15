/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <Arduino.h>
#include <time.h>
#include <string.h>
#include "lvgl.h"
#include "custom.h"
#include "../../DataPool.h"
#include "../../focus_mode.h"

/* ---- 引入已生成的角色图片 ---- */
LV_IMG_DECLARE(_img_role_normal_alpha_60x60);
LV_IMG_DECLARE(_img_role_focus_alpha_60x60);
LV_IMG_DECLARE(_img_role_alert_alpha_100x100);
LV_IMG_DECLARE(img_role_remind);

static lv_ui *gui_ui = NULL;
static bool timer_ok_reported = false;

static bool label_valid(lv_obj_t *label)
{
    return (label != NULL) && (lv_obj_get_screen(label) != NULL);
}

// 安全更新标签：只在文本变化时才调用 lv_label_set_text
// 避免无变化时的 LVGL 重绘、内存分配、SPI 刷屏 → 减少内存碎片
static void safe_label_set(lv_obj_t *label, const char *new_text)
{
    if (!label_valid(label)) return;
    const char *old = lv_label_get_text(label);
    if (old == NULL || strcmp(old, new_text) != 0) {
        lv_label_set_text(label, new_text);
    }
}

/* ---- 根据环境数据选择角色图片 ---- */
static const lv_img_dsc_t* get_role_image(void)
{
    float t  = sensorData.temp;
    float h  = sensorData.humi;
    float e  = sensorData.eco2;
    float tv = sensorData.tvoc;
    int   a  = sensorData.aqi;

    if (a >= 4 || e > 2000 || tv > 3000 || t > 40 || t < 5) {
        return &_img_role_alert_alpha_100x100;
    }
    if (a >= 2 || e > 1000 || tv > 1000 || t > 32 || t < 15 || h > 75 || h < 20) {
        return &img_role_remind;
    }
    return &_img_role_normal_alpha_60x60;
}

static void ui_refresh_timer_cb(lv_timer_t *timer)
{
    if (!gui_ui) return;

    char buf[64];
    lv_obj_t *active = lv_scr_act();
    static lv_obj_t *last_active = NULL;

    if (!timer_ok_reported) {
        printf("[UI Timer] running, active scr=%p\n", active);
        timer_ok_reported = true;
    }

    // ---- 手动进入/退出专注模式（屏幕切换触发）----
    // 仅当雷达故障（不支持自动）或自动功能关闭时，由屏幕切换来手动控制
    if (active != last_active) {
        // 从其他屏幕 → fouseScreen：手动开启专注
        if (active == gui_ui->fouseScreen && !status.focus_mode) {
            status.focus_mode = true;
            sensorData.focus_duration = 0;
            focusConfig.last_manual_exit_ms = 0;  // 取消冷却，用户明确要专注
            printf("[Focus] 📱 手动进入专注模式\n");
        }
        // 从 fouseScreen → 其他屏幕：手动关闭专注
        if (last_active == gui_ui->fouseScreen && status.focus_mode) {
            status.focus_mode = false;
            sensorData.focus_duration = 0;
            focusMode_notifyManualExit();          // 记录失焦时间，启动冷却
            printf("[Focus] 📱 手动退出专注模式\n");
        }
        last_active = active;
    }

    // 自动进入专注模式 → 自动切换到 fouseScreen
    if (status.request_focus_screen) {
        status.request_focus_screen = false;
        if (active != gui_ui->fouseScreen && gui_ui->fouseScreen) {
            lv_scr_load_anim(gui_ui->fouseScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
            active = lv_scr_act();
        }
    }

    // 更新屏幕状态标记（供 focus_mode 等模块判断当前所在屏幕）
    status.on_main_screen = (active == gui_ui->mainScreen);

    // ========== mainScreen ==========
    if (active == gui_ui->mainScreen) {
        // 左上角时间
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        if (status.time_synced) {  // NTP已同步，断网后ESP32内部RTC继续走时
            snprintf(buf, sizeof(buf), "%02d:%02d", t->tm_hour, t->tm_min);
        } else {
            snprintf(buf, sizeof(buf), "--:--");
        }
        safe_label_set(gui_ui->mainScreen_label_time, buf);

        // AQI 值
        snprintf(buf, sizeof(buf), "%d", sensorData.aqi);
        safe_label_set(gui_ui->mainScreen_label_aqi_val, buf);

        // eCO2 值
        snprintf(buf, sizeof(buf), "%.0f", sensorData.eco2);
        safe_label_set(gui_ui->mainScreen_label_eco2_val, buf);

        // 环境摘要
        snprintf(buf, sizeof(buf), "%.1fC | %.0f%% | %.0f lx",
                 sensorData.temp, sensorData.humi, sensorData.light);
        safe_label_set(gui_ui->mainScreen_label_summary, buf);

        // 角色图片
        if (label_valid(gui_ui->mainScreen_img_role)) {
            const lv_img_dsc_t *role = get_role_image();
            lv_img_set_src(gui_ui->mainScreen_img_role, role);
            if (role == &_img_role_alert_alpha_100x100) {
                lv_obj_set_size(gui_ui->mainScreen_img_role, 100, 100);
                lv_obj_set_pos(gui_ui->mainScreen_img_role, 10, 110);
            } else {
                lv_obj_set_size(gui_ui->mainScreen_img_role, 60, 60);
                lv_obj_set_pos(gui_ui->mainScreen_img_role, 16, 121);
            }
        }

        // Prompt
        if (label_valid(gui_ui->mainScreen_label_Prompt)) {
            int a = sensorData.aqi;
            const char *msg;
            if      (a >= 5) msg = "Air: Hazardous!";
            else if (a >= 4) msg = "Air: Unhealthy";
            else if (a >= 3) msg = "Air: Moderate";
            else if (a >= 2) msg = "Air: Good";
            else             msg = "Air: Excellent";
            safe_label_set(gui_ui->mainScreen_label_Prompt, msg);
        }
    }

    // ========== airScreen ==========
    if (active == gui_ui->aircreen) {
        snprintf(buf, sizeof(buf), "%.0f", sensorData.tvoc);
        safe_label_set(gui_ui->aircreen_label_tvoc_val, buf);

        snprintf(buf, sizeof(buf), "%.0f", sensorData.eco2);
        safe_label_set(gui_ui->aircreen_label_eco2_val, buf);
    }

    // ========== envScreen ==========
    if (active == gui_ui->envScreen) {
        snprintf(buf, sizeof(buf), "温度: %.1f°C", sensorData.temp);
        safe_label_set(gui_ui->envScreen_label_temp, buf);

        snprintf(buf, sizeof(buf), "湿度: %.0f%%", sensorData.humi);
        safe_label_set(gui_ui->envScreen_label_humi, buf);

        snprintf(buf, sizeof(buf), "光照: %.0f lx", sensorData.light);
        safe_label_set(gui_ui->envScreen_label_light, buf);

        snprintf(buf, sizeof(buf), "TVOC: %.0f ppb", sensorData.tvoc);
        safe_label_set(gui_ui->envScreen_label_tvoc, buf);

        snprintf(buf, sizeof(buf), "eCO2: %.0f ppm", sensorData.eco2);
        safe_label_set(gui_ui->envScreen_label_eco2, buf);

        snprintf(buf, sizeof(buf), "PM2.5: %.1f ug/m3", sensorData.pm25);
        safe_label_set(gui_ui->envScreen_label_pm25, buf);
    }

    // ========== fouseScreen ==========
    if (active == gui_ui->fouseScreen) {
        // 专注计时器 HH:MM:SS
        if (status.focus_mode) {
            uint32_t dur = sensorData.focus_duration;
            int hours = dur / 3600;
            int mins  = (dur % 3600) / 60;
            int secs  = dur % 60;
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, mins, secs);
        } else {
            snprintf(buf, sizeof(buf), "00:00:00");
        }
        safe_label_set(gui_ui->fouseScreen_label_focus_timer, buf);

        // 环境摘要
        snprintf(buf, sizeof(buf), "%.1fC | %.0f%% | AQI:%d",
                 sensorData.temp, sensorData.humi, sensorData.aqi);
        safe_label_set(gui_ui->fouseScreen_label_status, buf);
    }
}

void custom_init(lv_ui *ui)
{
    gui_ui = ui;

    printf("--- LVGL memory after setup_ui ---\n");
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("LVGL free: %d / %d (frag: %d%%)\n",
           (int)mon.free_size, (int)mon.total_size, (int)mon.frag_pct);

    img.role.normal    = (const uint8_t*)&_img_role_normal_alpha_60x60;
    img.role.remind    = (const uint8_t*)&img_role_remind;
    img.role.alert     = (const uint8_t*)&_img_role_alert_alpha_100x100;
    img.role.focus     = (const uint8_t*)&_img_role_focus_alpha_60x60;

    lv_obj_set_size(ui->fouseScreen_label_focus_timer, 240, 50);
    lv_obj_set_pos(ui->fouseScreen_label_focus_timer, 40, 80);
    lv_obj_set_style_text_align(ui->fouseScreen_label_focus_timer, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_set_size(ui->fouseScreen_label_status, 210, 24);
    lv_obj_set_pos(ui->fouseScreen_label_status, 80, 160);
    lv_obj_set_style_text_align(ui->fouseScreen_label_status, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

    printf("[custom_init] role images wired, labels fixed, change-detection ON\n");

    lv_timer_create(ui_refresh_timer_cb, 200, NULL);
}

void Task_LVGL(void *pvParameters)
{
    const TickType_t xDelay = 5 / portTICK_PERIOD_MS;
    for (;;) {
        lv_timer_handler();
        vTaskDelay(xDelay);
    }
}
