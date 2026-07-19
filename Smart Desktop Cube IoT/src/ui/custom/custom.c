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
#include "../../OTAScreen.h"

/* ---- 引入已生成的角色图片 ---- */
LV_IMG_DECLARE(_img_role_normal_alpha_60x60);
LV_IMG_DECLARE(_img_role_focus_alpha_60x60);
LV_IMG_DECLARE(_img_role_alert_alpha_60x60);
LV_IMG_DECLARE(img_role_remind);
LV_IMG_DECLARE(_img_role_celebrate_alpha_60x60);

static lv_ui *gui_ui = NULL;
static bool timer_ok_reported = false;

/* ---- Toast 弹窗 ---- */
static lv_obj_t *toast_obj = NULL;
static uint32_t toast_created_ms = 0;
#define TOAST_DURATION_MS 2500

static void toast_delete_cb(lv_timer_t *t)
{
    if (toast_obj) {
        lv_obj_del(toast_obj);
        toast_obj = NULL;
    }
    lv_timer_del(t);
}

static void show_toast(const char *msg)
{
    /* 如果已有 toast，先删掉旧的 */
    if (toast_obj) {
        lv_obj_del(toast_obj);
        toast_obj = NULL;
    }

    lv_obj_t *scr = lv_scr_act();
    toast_obj = lv_label_create(scr);
    lv_label_set_text(toast_obj, msg);
    lv_obj_set_style_text_color(toast_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(toast_obj, &lv_font_montserratMedium_16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(toast_obj, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(toast_obj, 180, LV_PART_MAIN);  /* 半透明 */
    lv_obj_set_style_radius(toast_obj, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(toast_obj, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(toast_obj, 10, LV_PART_MAIN);
    lv_obj_align(toast_obj, LV_ALIGN_TOP_MID, 0, 8);  /* 顶部弹窗 */
    lv_obj_clear_flag(toast_obj, LV_OBJ_FLAG_CLICKABLE);

    toast_created_ms = millis();

    /* 2.5秒后自动删除 */
    lv_timer_t *t = lv_timer_create(toast_delete_cb, TOAST_DURATION_MS, NULL);
    lv_timer_set_repeat_count(t, 1);
}

static bool label_valid(lv_obj_t *label)
{
    if (label == NULL) return false;
    if (lv_obj_get_screen(label) == NULL) return false;
    /* 确认对象确实是 label 类型，防止把容器/图片当 label 用导致崩溃 */
    if (!lv_obj_check_type(label, &lv_label_class)) return false;
    return true;
}

// 安全更新标签：只在文本变化时才调用 lv_label_set_text
static void safe_label_set(lv_obj_t *label, const char *new_text)
{
    if (!label_valid(label)) return;
    const char *old = lv_label_get_text(label);
    if (old == NULL || strcmp(old, new_text) != 0) {
        lv_label_set_text(label, new_text);
    }
}

/* 开机过渡回调：等待所有模块加载完成后才切换到主界面 */
static void boot_to_main_cb(lv_timer_t *t)
{
    extern lv_ui guider_ui;
    /* 模块还没加载完，继续等待 */
    if (!status.modules_ready) return;

    if (guider_ui.mainScreen && lv_scr_act() == guider_ui.bootScreen) {
        lv_scr_load_anim(guider_ui.mainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);
        printf("[Boot] modules ready -> mainScreen\n");
    }
    lv_timer_del(t);
}

/* ---- 根据环境数据选择角色图片（4个等级） ---- */
static const lv_img_dsc_t* get_role_image(void)
{
    float e  = sensorData.eco2;
    float tv = sensorData.tvoc;
    int   a  = sensorData.aqi;

    /* alert: 严重污染 */
    if (a > 150 || e > 2000 || tv > 3000) {
        return &_img_role_alert_alpha_60x60;
    }
    /* remind: 中度污染 */
    if (a > 100 || e > 1000 || tv > 1000) {
        return &img_role_remind;
    }
    /* normal: 轻度 */
    if (a > 50 || e > 800) {
        return &_img_role_normal_alpha_60x60;
    }
    /* celebrate: 空气优秀 */
    return &_img_role_celebrate_alpha_60x60;
}

/* ---- 获取角色心情文字 ---- */
static const char* get_role_mood(void)
{
    float e  = sensorData.eco2;
    float tv = sensorData.tvoc;
    int   a  = sensorData.aqi;

    if (a > 150 || e > 2000 || tv > 3000) return "Alert!";
    if (a > 100 || e > 1000 || tv > 1000) return "Ventilate";
    if (a > 50 || e > 800) return "OK";
    return "Great!";
}

/* ---- AQI 等级文字和颜色 ---- */
static void get_aqi_info(int aqi, const char **level, lv_color_t *color)
{
    if (aqi <= 50)       { *level = "Good";  *color = lv_color_hex(0x4ADE80); }
    else if (aqi <= 100) { *level = "Mod";   *color = lv_color_hex(0xFBBF24); }
    else if (aqi <= 150) { *level = "Bad";   *color = lv_color_hex(0xFB923C); }
    else if (aqi <= 200) { *level = "VBad";  *color = lv_color_hex(0xF43F5E); }
    else if (aqi <= 300) { *level = "Haz";   *color = lv_color_hex(0xDC2626); }
    else                 { *level = "Sev";   *color = lv_color_hex(0x991B1B); }
}

static void ui_refresh_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!gui_ui) return;

    /* ---- 跨核 Toast：WebServer(Core0) 设置标志，LVGL(Core1) 显示 ---- */
    if (status.toast_pending) {
        status.toast_pending = false;
        show_toast(status.pending_toast);
    }

    /* ---- 启动画面：实时显示模块加载进度文字 ---- */
    if (lv_scr_act() == gui_ui->bootScreen && gui_ui->bootScreen_label_status) {
        safe_label_set(gui_ui->bootScreen_label_status, status.boot_status_text);
    }

    /* ---- OTA 进度屏幕（LVGL 安全渲染）---- */
    static bool ota_screen_shown = false;
    if (status.ota_in_progress) {
        if (!ota_screen_shown) {
            otaScreenShow(FW_VERSION, status.ota_new_version);
            ota_screen_shown = true;
        }
        otaScreenUpdateProgress(status.ota_progress, status.ota_status_text);
        return;  /* OTA 期间不刷新其他 UI */
    } else if (ota_screen_shown) {
        /* OTA 结束（成功会重启，这里是错误恢复）*/
        otaScreenHide();
        ota_screen_shown = false;
        /* 回到主界面 */
        if (gui_ui->mainScreen) {
            lv_scr_load_anim(gui_ui->mainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
        }
    }

    char buf[64];
    lv_obj_t *active = lv_scr_act();
    static lv_obj_t *last_active = NULL;

    if (!timer_ok_reported) {
        printf("[UI Timer] running, active scr=%p\n", active);
        timer_ok_reported = true;
    }

    // ---- 手动进入/退出专注模式（屏幕切换触发）----
    if (active != last_active) {
        if (active == gui_ui->fouseScreen && !status.focus_mode) {
            status.focus_mode = true;
            sensorData.focus_duration = 0;
            focusConfig.last_manual_exit_ms = 0;
            focusMode_notifyManualEnter();
            printf("[Focus] 手动进入专注模式\n");
        }
        if (last_active == gui_ui->fouseScreen && status.focus_mode) {
            status.focus_mode = false;
            sensorData.focus_duration = 0;
            focusMode_notifyManualExit();
            printf("[Focus] 手动退出专注模式\n");
        }
        last_active = active;
    }

    // 自动进入专注模式
    if (status.request_focus_screen) {
        status.request_focus_screen = false;
        if (active != gui_ui->fouseScreen && gui_ui->fouseScreen) {
            lv_scr_load_anim(gui_ui->fouseScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
            active = lv_scr_act();
            /* 确保计时器启动 & 防止自动退出立即生效 */
            focusMode_notifyManualEnter();
        }
    }

    status.on_main_screen = (active == gui_ui->mainScreen);

    // ========== mainScreen ==========
    if (active == gui_ui->mainScreen) {
        /* 时钟 */
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        if (status.time_synced || now > 1700000000) {
            snprintf(buf, sizeof(buf), "%02d:%02d", t->tm_hour, t->tm_min);
        } else {
            snprintf(buf, sizeof(buf), "--:--");
        }
        safe_label_set(gui_ui->mainScreen_label_clock, buf);

        /* AQI 数值 + 颜色圈圈 */
        snprintf(buf, sizeof(buf), "AQI %d", sensorData.aqi);
        safe_label_set(gui_ui->mainScreen_label_aqi_big, buf);

        const char *aqi_level;
        lv_color_t aqi_color;
        get_aqi_info(sensorData.aqi, &aqi_level, &aqi_color);
        if (label_valid(gui_ui->mainScreen_label_aqi_big)) {
            lv_obj_set_style_text_color(gui_ui->mainScreen_label_aqi_big, aqi_color,
                                        LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        /* 更新圈圈颜色 */
        if (gui_ui->mainScreen_label_aqi_level) {
            lv_obj_set_style_border_color(gui_ui->mainScreen_label_aqi_level, aqi_color,
                                          LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        /* 温度（带单位） */
        snprintf(buf, sizeof(buf), "%.1f C", sensorData.temp);
        safe_label_set(gui_ui->mainScreen_label_temp_val, buf);

        /* 湿度（带单位） */
        snprintf(buf, sizeof(buf), "%.0f %%", sensorData.humi);
        safe_label_set(gui_ui->mainScreen_label_humi_val, buf);

        /* 光照（带单位） */
        snprintf(buf, sizeof(buf), "%.0f lx", sensorData.light);
        safe_label_set(gui_ui->mainScreen_label_light_val, buf);

        /* CO2（带单位） */
        snprintf(buf, sizeof(buf), "%.0f ppm", sensorData.eco2);
        safe_label_set(gui_ui->mainScreen_label_co2_val, buf);

        /* TVOC */
        snprintf(buf, sizeof(buf), "%.2f", sensorData.tvoc);
        safe_label_set(gui_ui->mainScreen_label_tvoc_val, buf);

        /* PM2.5（带单位） */
        snprintf(buf, sizeof(buf), "%.0f ug/m3", sensorData.pm25);
        safe_label_set(gui_ui->mainScreen_label_pm25_val, buf);

        /* 人体存在 */
        if (label_valid(gui_ui->mainScreen_label_human_val)) {
            safe_label_set(gui_ui->mainScreen_label_human_val,
                           status.is_human_exist ? "Yes" : "No");
        }

        /* 角色图片（固定 60x60，不 resize）+ 心情 */
        if (gui_ui->mainScreen_img_role) {
            const lv_img_dsc_t *role = get_role_image();
            lv_img_set_src(gui_ui->mainScreen_img_role, role);
        }
        safe_label_set(gui_ui->mainScreen_label_mood, get_role_mood());

        /* WiFi 状态点：已连接绿色，未连接红色 */
        if (gui_ui->mainScreen_dot_wifi) {
            lv_obj_set_style_bg_color(gui_ui->mainScreen_dot_wifi,
                                      status.wifi_connected ? lv_color_hex(0x4ADE80) : lv_color_hex(0xF43F5E),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        /* MQTT 状态点 */
        if (gui_ui->mainScreen_dot_mqtt) {
            lv_obj_set_style_bg_color(gui_ui->mainScreen_dot_mqtt,
                                      status.mqtt_connected ? lv_color_hex(0x60A5FA) : lv_color_hex(0xF43F5E),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        /* 专注模式指示灯 */
        if (gui_ui->mainScreen_dot_focus) {
            lv_obj_set_style_bg_opa(gui_ui->mainScreen_dot_focus,
                                    status.focus_mode ? 255 : 77,
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        /* 自动亮度状态 */
        if (label_valid(gui_ui->mainScreen_label_auto_brightness)) {
            safe_label_set(gui_ui->mainScreen_label_auto_brightness,
                           status.auto_brightness_enabled ? "Auto Bright ON" : "Auto Bright OFF");
        }
    }

    // ========== fouseScreen ==========
    if (active == gui_ui->fouseScreen) {
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

        /* 环境建议（根据传感器数据给出提示） */
        const char *advice = "Stay focused...";
        if (sensorData.temp > 28.0f) {
            advice = "Too hot, suggest AC/fan";
        } else if (sensorData.temp < 18.0f) {
            advice = "Too cold, suggest heater";
        } else if (sensorData.eco2 > 1000) {
            advice = "CO2 high, ventilate!";
        } else if (sensorData.humi > 70.0f) {
            advice = "Humid, suggest dehumidify";
        } else if (sensorData.humi < 30.0f) {
            advice = "Dry air, suggest humidifier";
        } else if (sensorData.aqi > 100) {
            advice = "Air poor, close window";
        }
        safe_label_set(gui_ui->fouseScreen_label_status, advice);
    }

    // ========== settingsScreen ==========
    if (active == gui_ui->settingsScreen) {
        /* 蓝牙灯已移除，设置页不再显示灯光状态 */
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
    img.role.alert     = (const uint8_t*)&_img_role_alert_alpha_60x60;
    img.role.focus     = (const uint8_t*)&_img_role_focus_alpha_60x60;
    img.role.celebrate = (const uint8_t*)&_img_role_celebrate_alpha_60x60;

    /* 专注模式计时器标签尺寸修正 */
    if (ui->fouseScreen_label_focus_timer) {
        lv_obj_set_size(ui->fouseScreen_label_focus_timer, 240, 50);
        lv_obj_set_pos(ui->fouseScreen_label_focus_timer, 40, 80);
        lv_obj_set_style_text_align(ui->fouseScreen_label_focus_timer, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }

    if (ui->fouseScreen_label_status) {
        lv_obj_set_size(ui->fouseScreen_label_status, 240, 40);
        lv_obj_set_pos(ui->fouseScreen_label_status, 40, 150);
        lv_obj_set_style_text_align(ui->fouseScreen_label_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }

    printf("[custom_init] role images wired, labels fixed, new mainScreen layout active\n");

    lv_timer_create(ui_refresh_timer_cb, 200, NULL);

    /* 开机过渡：每500ms检查模块是否加载完成，完成后自动切换到主界面 */
    lv_timer_create(boot_to_main_cb, 500, NULL);
}

void Task_LVGL(void *pvParameters)
{
    const TickType_t xDelay = 5 / portTICK_PERIOD_MS;
    for (;;) {
        lv_timer_handler();
        vTaskDelay(xDelay);
    }
}
