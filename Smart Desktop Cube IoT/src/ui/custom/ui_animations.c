/*
 * ui_animations.c — 屏幕过渡动画 + 控件逐级显示实现
 *
 * 效果说明：
 *   切屏时先 300ms 淡入过渡，然后屏幕上的控件按顺序
 * 依次淡入+从下方滑入（每个间隔 70ms），形成流畅的逐级加载效果。
 */
#include "ui_animations.h"

/* ======================== 动画回调 ======================== */

/* 设置控件整体透明度 */
static void anim_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)var, v, LV_PART_MAIN);
}

/* 设置控件 Y 坐标 */
static void anim_y_cb(void *var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, v);
}

/* ======================== 通用工具函数 ======================== */

void ui_anim_fade_slide_in(lv_obj_t *obj, int16_t offset_y,
                           uint16_t delay_ms, uint16_t duration_ms)
{
    if (!obj) return;

    /* 记录原始 Y，设为隐藏状态 */
    int32_t orig_y = lv_obj_get_y(obj);
    lv_obj_set_style_opa(obj, 0, LV_PART_MAIN);
    lv_obj_set_y(obj, orig_y + offset_y);

    /* 淡入动画 */
    lv_anim_t a_opa;
    lv_anim_init(&a_opa);
    lv_anim_set_var(&a_opa, obj);
    lv_anim_set_values(&a_opa, 0, 255);
    lv_anim_set_time(&a_opa, duration_ms);
    lv_anim_set_delay(&a_opa, delay_ms);
    lv_anim_set_exec_cb(&a_opa, anim_opa_cb);
    lv_anim_set_path_cb(&a_opa, lv_anim_path_ease_out);
    lv_anim_start(&a_opa);

    /* 位移动画（从下方滑回原位） */
    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, obj);
    lv_anim_set_values(&a_y, orig_y + offset_y, orig_y);
    lv_anim_set_time(&a_y, duration_ms);
    lv_anim_set_delay(&a_y, delay_ms);
    lv_anim_set_exec_cb(&a_y, anim_y_cb);
    lv_anim_set_path_cb(&a_y, lv_anim_path_ease_out);
    lv_anim_start(&a_y);
}

void ui_anim_fade_in(lv_obj_t *obj, uint16_t delay_ms, uint16_t duration_ms)
{
    if (!obj) return;

    lv_obj_set_style_opa(obj, 0, LV_PART_MAIN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, duration_ms);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/* ======================== 屏幕切换过渡 ======================== */

void ui_anim_switch_screen(lv_obj_t *new_scr)
{
    if (!new_scr) return;

    lv_obj_t *act = lv_scr_act();
    if (new_scr == act) return;

    /* 300ms 淡入淡出过渡，保留旧屏幕（不自动删除） */
    lv_scr_load_anim(new_scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
}

/* ======================== 逐级显示 ======================== */

void ui_anim_reveal_screen(lv_ui *ui, lv_obj_t *screen)
{
    if (!ui || !screen) return;

    /* 动画参数 */
    const int16_t  OFFSET   = 20;   /* 从下方偏移像素 */
    const uint16_t DURATION = 300;  /* 单控件动画时长 ms */
    const uint16_t STEP     = 70;   /* 控件间延迟 ms */

    if (screen == ui->mainScreen) {
        /* 新主屏幕：状态栏 → 角色 → AQI卡片 → 温湿度 → 传感器网格 → 底部栏 */
        ui_anim_fade_in     (ui->mainScreen_bar_top,                  0,         DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_img_role,                OFFSET, STEP,      DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cont_aqi_hero,           OFFSET, STEP * 2,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cont_temp,               OFFSET, STEP * 3,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cont_humi,               OFFSET, STEP * 3,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cell_light,              OFFSET, STEP * 4,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cell_co2,                OFFSET, STEP * 4,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cell_tvoc,               OFFSET, STEP * 4,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cell_pm25,               OFFSET, STEP * 5,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cell_human,              OFFSET, STEP * 5,  DURATION);
        ui_anim_fade_slide_in(ui->mainScreen_cell_signal,             OFFSET, STEP * 5,  DURATION);
        ui_anim_fade_in     (ui->mainScreen_bar_bottom,                      STEP * 6,  DURATION);
        ui_anim_fade_in     (ui->mainScreen_img_boot_logo,                   STEP * 6,  DURATION);
        ui_anim_fade_in     (ui->mainScreen_img_wifi,                        STEP * 6,  DURATION);
    }
    else if (screen == ui->aircreen) {
        /* 空气屏幕：标题 → TVOC卡片 → eCO2卡片 → 角色 → AI提示 */
        ui_anim_fade_slide_in(ui->aircreen_label_title,  OFFSET, 0,         DURATION);
        ui_anim_fade_slide_in(ui->aircreen_cont_1,       OFFSET, STEP,      DURATION);
        ui_anim_fade_slide_in(ui->aircreen_cont_2,       OFFSET, STEP * 2,  DURATION);
        ui_anim_fade_slide_in(ui->aircreen_img_role2,    OFFSET, STEP * 3,  DURATION);
        ui_anim_fade_slide_in(ui->aircreen_label_ai_tip, OFFSET, STEP * 4,  DURATION);
    }
    else if (screen == ui->envScreen) {
        /* 环境屏幕：标题 → 温度 → 湿度 → 光照 → TVOC → eCO2 → PM2.5 */
        ui_anim_fade_slide_in(ui->envScreen_label_title, OFFSET, 0,         DURATION);
        ui_anim_fade_slide_in(ui->envScreen_label_temp,  OFFSET, STEP,      DURATION);
        ui_anim_fade_slide_in(ui->envScreen_label_humi,  OFFSET, STEP * 2,  DURATION);
        ui_anim_fade_slide_in(ui->envScreen_label_light, OFFSET, STEP * 3,  DURATION);
        ui_anim_fade_slide_in(ui->envScreen_label_tvoc,  OFFSET, STEP * 4,  DURATION);
        ui_anim_fade_slide_in(ui->envScreen_label_eco2,  OFFSET, STEP * 5,  DURATION);
        ui_anim_fade_slide_in(ui->envScreen_label_pm25,  OFFSET, STEP * 6,  DURATION);
    }
    else if (screen == ui->fouseScreen) {
        /* 专注屏幕：计时器 → 角色 → 状态 → 退出按钮 */
        ui_anim_fade_slide_in(ui->fouseScreen_label_focus_timer, OFFSET, 0,         DURATION);
        ui_anim_fade_slide_in(ui->fouseScreen_img_role_focus,     OFFSET, STEP,      DURATION);
        ui_anim_fade_slide_in(ui->fouseScreen_label_status,        OFFSET, STEP * 2,  DURATION);
        ui_anim_fade_slide_in(ui->fouseScreen_btn_1,               OFFSET, STEP * 3,  DURATION);
    }
    else if (screen == ui->settingsScreen) {
        /* 设置屏幕：标题 → 亮度 → 色温 → 开关 → 返回按钮 → OTA按钮 */
        ui_anim_fade_slide_in(ui->settingsScreen_label_1,                   OFFSET, 0,         DURATION);
        ui_anim_fade_slide_in(ui->settingsScreen_slider_Brightness,         OFFSET, STEP,      DURATION);
        ui_anim_fade_slide_in(ui->settingsScreen_slider_Color_temperature,   OFFSET, STEP * 2,  DURATION);
        ui_anim_fade_slide_in(ui->settingsScreen_sw_open,                   OFFSET, STEP * 3,  DURATION);
        ui_anim_fade_slide_in(ui->settingsScreen_btn_1,                     OFFSET, STEP * 4,  DURATION);
        ui_anim_fade_slide_in(ui->settingsScreen_btn_ota,                   OFFSET, STEP * 5,  DURATION);
    }
}
