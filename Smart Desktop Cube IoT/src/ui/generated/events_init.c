#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#include "../../DataPool.h"
#include "../custom/ui_animations.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

static uint32_t _last_switch_ms = 0;

static void switch_to_screen(lv_obj_t *new_scr)
{
    if (!new_scr) return;
    lv_obj_t *act = lv_scr_act();
    if (new_scr == act) return;
    uint32_t now = lv_tick_get();
    if (now - _last_switch_ms < 500) return;
    _last_switch_ms = now;
    lv_scr_load_anim(new_scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
}

// ==================== Toast 弹窗系统 ====================
static lv_obj_t *toast_box = NULL;
static lv_timer_t *toast_timer = NULL;

static void toast_hide_cb(lv_timer_t *timer)
{
    (void)timer;
    if (toast_box) {
        lv_obj_del(toast_box);
        toast_box = NULL;
    }
    toast_timer = NULL;
}

static void show_toast(const char *msg, lv_color_t bg_color)
{
    if (toast_box) { lv_obj_del(toast_box); toast_box = NULL; }
    if (toast_timer) { lv_timer_del(toast_timer); toast_timer = NULL; }

    toast_box = lv_obj_create(lv_scr_act());
    lv_obj_set_size(toast_box, 270, 64);
    lv_obj_center(toast_box);
    lv_obj_set_style_bg_color(toast_box, bg_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(toast_box, 240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(toast_box, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(toast_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(toast_box, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(toast_box, 128, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label = lv_label_create(toast_box);
    lv_label_set_text(label, msg);
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, &lv_font_yezi_hei_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    toast_timer = lv_timer_create(toast_hide_cb, 3500, NULL);
    lv_timer_set_repeat_count(toast_timer, 1);
}

static void show_ble_disconnected_toast(void)
{
    show_toast("Light not connected\nCheck Yeelight power & WiFi",
               lv_color_hex(0xE53935));
}

// ==================== mainScreen ====================
/* 新设计：主界面不再支持滑动切换页面，所有数据一屏显示 */
static void mainScreen_img_boot_logo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
        printf("CLICKED img_boot_logo -> fouseScreen\n");
        switch_to_screen(guider_ui.fouseScreen);
        break;
    default:
        break;
    }
}

static void mainScreen_img_wifi_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
        printf("CLICKED img_wifi -> settingsScreen\n");
        switch_to_screen(guider_ui.settingsScreen);
        break;
    default:
        break;
    }
}

void events_init_mainScreen(lv_ui *ui)
{
    /* 不再注册 LV_EVENT_GESTURE，取消滑动切屏 */
    if (ui->mainScreen_img_boot_logo) {
        lv_obj_add_event_cb(ui->mainScreen_img_boot_logo, mainScreen_img_boot_logo_event_handler, LV_EVENT_ALL, ui);
    }
    if (ui->mainScreen_img_wifi) {
        lv_obj_add_event_cb(ui->mainScreen_img_wifi, mainScreen_img_wifi_event_handler, LV_EVENT_ALL, ui);
    }
}

// ==================== fouseScreen ====================
static void fouseScreen_btn_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
        printf("CLICKED btn_1 -> mainScreen\n");
        switch_to_screen(guider_ui.mainScreen);
        break;
    default:
        break;
    }
}

void events_init_fouseScreen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->fouseScreen_btn_1, fouseScreen_btn_1_event_handler, LV_EVENT_ALL, ui);
}

// ==================== settingsScreen ====================
static void settingsScreen_btn_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
        printf("CLICKED settings btn_1 -> mainScreen\n");
        switch_to_screen(guider_ui.mainScreen);
        break;
    default:
        break;
    }
}

static void settingsScreen_slider_Brightness_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int val = (int)lv_slider_get_value(slider);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", val);
        lv_label_set_text(guider_ui.settingsScreen_label_Bluetooth_light, buf);
        printf("[Settings] 亮度滑块: %d%%\n", val);
    }
}

static void settingsScreen_slider_ColorTemp_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int val = (int)lv_slider_get_value(slider);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", val);
        lv_label_set_text(guider_ui.settingsScreen_label_3, buf);
        printf("[Settings] 色温滑块: %d\n", val);
    }
}

static void settingsScreen_sw_open_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *sw = lv_event_get_target(e);
        bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);
        printf("[Settings] 开关: %s\n", on ? "ON" : "OFF");
    }
}

static void settingsScreen_btn_ota_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        status.ota_check_requested = true;
        show_toast("已发送版本检查请求\n请稍候...", lv_color_hex(0x2195F6));
        printf("[OTA] 用户手动请求检查更新\n");
    }
}

void events_init_settingsScreen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->settingsScreen_btn_1, settingsScreen_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->settingsScreen_slider_Brightness, settingsScreen_slider_Brightness_event_handler, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_add_event_cb(ui->settingsScreen_slider_Color_temperature, settingsScreen_slider_ColorTemp_event_handler, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_add_event_cb(ui->settingsScreen_sw_open, settingsScreen_sw_open_event_handler, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_add_event_cb(ui->settingsScreen_btn_ota, settingsScreen_btn_ota_event_handler, LV_EVENT_CLICKED, ui);
}

// ==================== 屏幕加载完成回调 ====================
static void screen_loaded_cb(lv_event_t *e)
{
    lv_obj_t *scr = lv_event_get_target(e);
    ui_anim_reveal_screen(&guider_ui, scr);
}

void events_init(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->mainScreen,    screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui->fouseScreen,   screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui->settingsScreen, screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    events_init_mainScreen(ui);
    events_init_fouseScreen(ui);
    events_init_settingsScreen(ui);
}
