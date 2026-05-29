/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <Arduino.h>
#include "lvgl.h"
#include "custom.h"
#include "../../DataPool.h"

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

/* ---- 根据环境数据选择角色图片 ---- */
static const lv_img_dsc_t* get_role_image(void)
{
    float t  = sensorData.temp;
    float h  = sensorData.humi;
    float e  = sensorData.eco2;
    float tv = sensorData.tvoc;
    int   a  = sensorData.aqi;

    // 严重告警：AQI >= 4 或关键指标严重超标
    if (a >= 4 || e > 2000 || tv > 3000 || t > 40 || t < 5) {
        return &_img_role_alert_alpha_100x100;
    }
    // 普通提醒：AQI >= 2 或单一指标轻微偏离舒适区
    if (a >= 2 || e > 1000 || tv > 1000 || t > 32 || t < 15 || h > 75 || h < 20) {
        return &img_role_remind;
    }
    // 正常
    return &_img_role_normal_alpha_60x60;
}

static void ui_refresh_timer_cb(lv_timer_t *timer)
{
    if (!gui_ui) return;

    char buf[64];
    lv_obj_t *active = lv_scr_act();

    if (!timer_ok_reported) {
        printf("[UI Timer] running, active scr=%p\n", active);
        timer_ok_reported = true;
    }

    // ========== mainScreen ==========
    if (active == gui_ui->mainScreen) {
        if (label_valid(gui_ui->mainScreen_label_aqi_val)) {
            snprintf(buf, sizeof(buf), "%d", sensorData.aqi);
            lv_label_set_text(gui_ui->mainScreen_label_aqi_val, buf);
        }
        if (label_valid(gui_ui->mainScreen_label_eco2_val)) {
            snprintf(buf, sizeof(buf), "%.0f", sensorData.eco2);
            lv_label_set_text(gui_ui->mainScreen_label_eco2_val, buf);
        }
        if (label_valid(gui_ui->mainScreen_label_summary)) {
            snprintf(buf, sizeof(buf), "%.1fC | %.0f%% | %.0f lx",
                     sensorData.temp, sensorData.humi, sensorData.light);
            lv_label_set_text(gui_ui->mainScreen_label_summary, buf);
        }
        // 角色图片随AQI切换
        if (label_valid(gui_ui->mainScreen_img_role)) {
            const lv_img_dsc_t *role = get_role_image();
            lv_img_set_src(gui_ui->mainScreen_img_role, role);
            // alert 图片是 100x100，其他是 60x60
            if (role == &_img_role_alert_alpha_100x100) {
                lv_obj_set_size(gui_ui->mainScreen_img_role, 100, 100);
                lv_obj_set_pos(gui_ui->mainScreen_img_role, 10, 110);
            } else {
                lv_obj_set_size(gui_ui->mainScreen_img_role, 60, 60);
                lv_obj_set_pos(gui_ui->mainScreen_img_role, 16, 121);
            }
        }
        // Prompt 提示（用英文，yezi_hei_12 字体不含"空气质量"等字模）
        if (label_valid(gui_ui->mainScreen_label_Prompt)) {
            int a = sensorData.aqi;
            if      (a >= 5) lv_label_set_text(gui_ui->mainScreen_label_Prompt, "Air: Hazardous!");
            else if (a >= 4) lv_label_set_text(gui_ui->mainScreen_label_Prompt, "Air: Unhealthy");
            else if (a >= 3) lv_label_set_text(gui_ui->mainScreen_label_Prompt, "Air: Moderate");
            else if (a >= 2) lv_label_set_text(gui_ui->mainScreen_label_Prompt, "Air: Good");
            else             lv_label_set_text(gui_ui->mainScreen_label_Prompt, "Air: Excellent");
        }
    }

    // ========== airScreen ==========
    if (active == gui_ui->aircreen) {
        if (label_valid(gui_ui->aircreen_label_tvoc_val)) {
            snprintf(buf, sizeof(buf), "%.0f", sensorData.tvoc);
            lv_label_set_text(gui_ui->aircreen_label_tvoc_val, buf);
        }
        if (label_valid(gui_ui->aircreen_label_eco2_val)) {
            snprintf(buf, sizeof(buf), "%.0f", sensorData.eco2);
            lv_label_set_text(gui_ui->aircreen_label_eco2_val, buf);
        }
    }

    // ========== envScreen ==========
    if (active == gui_ui->envScreen) {
        if (label_valid(gui_ui->envScreen_label_temp)) {
            snprintf(buf, sizeof(buf), "%.1f C", sensorData.temp);
            lv_label_set_text(gui_ui->envScreen_label_temp, buf);
        }
        if (label_valid(gui_ui->envScreen_label_humi)) {
            snprintf(buf, sizeof(buf), "%.0f %%", sensorData.humi);
            lv_label_set_text(gui_ui->envScreen_label_humi, buf);
        }
        if (label_valid(gui_ui->envScreen_label_light)) {
            snprintf(buf, sizeof(buf), "%.0f lx", sensorData.light);
            lv_label_set_text(gui_ui->envScreen_label_light, buf);
        }
        if (label_valid(gui_ui->envScreen_label_tvoc)) {
            snprintf(buf, sizeof(buf), "%.0f ppb", sensorData.tvoc);
            lv_label_set_text(gui_ui->envScreen_label_tvoc, buf);
        }
        if (label_valid(gui_ui->envScreen_label_eco2)) {
            snprintf(buf, sizeof(buf), "%.0f ppm", sensorData.eco2);
            lv_label_set_text(gui_ui->envScreen_label_eco2, buf);
        }
    }

    // ========== fouseScreen ==========
    if (active == gui_ui->fouseScreen) {
        // 专注计时器 HH:MM:SS
        if (label_valid(gui_ui->fouseScreen_label_timer)) {
            if (status.focus_mode) {
                uint32_t dur = sensorData.focus_duration;
                int hours = dur / 3600;
                int mins  = (dur % 3600) / 60;
                int secs  = dur % 60;
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, mins, secs);
            } else {
                snprintf(buf, sizeof(buf), "00:00:00");
            }
            lv_label_set_text(gui_ui->fouseScreen_label_timer, buf);
        }
        // 环境摘要
        if (label_valid(gui_ui->fouseScreen_label_status)) {
            snprintf(buf, sizeof(buf), "%.1fC | %.0f%% | AQI:%d",
                     sensorData.temp, sensorData.humi, sensorData.aqi);
            lv_label_set_text(gui_ui->fouseScreen_label_status, buf);
        }
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

    // ---- 把生成的角色图片指针写入数据池 ----
    img.role.normal    = (const uint8_t*)&_img_role_normal_alpha_60x60;
    img.role.remind    = (const uint8_t*)&img_role_remind;
    img.role.alert     = (const uint8_t*)&_img_role_alert_alpha_100x100;
    img.role.focus     = (const uint8_t*)&_img_role_focus_alpha_60x60;

    // ---- 修正 fouseScreen 标签尺寸 ----
    lv_obj_set_size(ui->fouseScreen_label_timer, 240, 50);
    lv_obj_set_pos(ui->fouseScreen_label_timer, 40, 80);
    lv_obj_set_style_text_align(ui->fouseScreen_label_timer, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_set_size(ui->fouseScreen_label_status, 210, 24);
    lv_obj_set_pos(ui->fouseScreen_label_status, 80, 160);
    lv_obj_set_style_text_align(ui->fouseScreen_label_status, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

    printf("[custom_init] role images wired, labels fixed\n");

    // 200ms 刷新
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
