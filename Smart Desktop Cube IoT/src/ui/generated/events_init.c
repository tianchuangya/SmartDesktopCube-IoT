#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#include "BluetoothLight.h"
#include "../../DataPool.h"
#include "../custom/ui_animations.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

static uint32_t _last_switch_ms = 0;

static void switch_to_screen(lv_obj_t *new_scr)
{
    if (!new_scr) return;  // 空指针保护

    lv_obj_t *act = lv_scr_act();

    // 已经在目标屏幕，跳过
    if (new_scr == act) return;

    // 防抖 500ms，避免手势连续触发多次切换导致崩溃
    uint32_t now = lv_tick_get();
    if (now - _last_switch_ms < 500) return;
    _last_switch_ms = now;

    // 300ms 淡入过渡动画（替代原来的 LV_SCR_LOAD_ANIM_NONE）
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
    // 移除已有 toast
    if (toast_box) {
        lv_obj_del(toast_box);
        toast_box = NULL;
    }
    if (toast_timer) {
        lv_timer_del(toast_timer);
        toast_timer = NULL;
    }

    // 创建半透明浮层
    toast_box = lv_obj_create(lv_scr_act());
    lv_obj_set_size(toast_box, 270, 64);
    lv_obj_center(toast_box);
    lv_obj_set_style_bg_color(toast_box, bg_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(toast_box, 240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(toast_box, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(toast_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(toast_box, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(toast_box, 128, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 文本
    lv_obj_t *label = lv_label_create(toast_box);
    lv_label_set_text(label, msg);
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, &lv_font_yezi_hei_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 3.5 秒后自动消失
    toast_timer = lv_timer_create(toast_hide_cb, 3500, NULL);
    lv_timer_set_repeat_count(toast_timer, 1);
}

static void show_ble_disconnected_toast(void)
{
    show_toast("BLE not connected\nCheck light power & range",
               lv_color_hex(0xE53935));  // 红色背景
}

// ==================== BLE 状态刷新定时器 ====================
static lv_timer_t *ble_status_timer = NULL;

static void ble_status_update_cb(lv_timer_t *timer)
{
    (void)timer;
    lv_ui *ui = &guider_ui;

    // 更新 settingsScreen 上的状态标签
    if (ui->settingsScreen_label_ble_status) {
        const char *msg = BL_GetStatusMsg();
        if (BL_IsConnected()) {
            lv_label_set_text(ui->settingsScreen_label_ble_status, "BLE: connected");
            lv_obj_set_style_text_color(ui->settingsScreen_label_ble_status,
                                        lv_color_hex(0x4CAF50), LV_PART_MAIN | LV_STATE_DEFAULT);  // 绿色
        } else {
            lv_label_set_text(ui->settingsScreen_label_ble_status, "BLE: disconnected");
            lv_obj_set_style_text_color(ui->settingsScreen_label_ble_status,
                                        lv_color_hex(0xE53935), LV_PART_MAIN | LV_STATE_DEFAULT);  // 红色
        }
    }

    // 检查是否有新的连接失败事件（消费标记后弹 toast）
    if (BL_ConsumeConnectFailedFlag()) {
        // 只有当用户在设置屏幕时才弹窗，避免干扰其他页面
        if (lv_scr_act() == ui->settingsScreen) {
            show_ble_disconnected_toast();
        }
    }
}

// ==================== 辅助：检查BLE并弹窗 ====================
// 返回 true 表示已连接，false 表示未连接（已弹窗）
static bool check_ble_or_toast(void)
{
    if (!BL_IsConnected()) {
        show_ble_disconnected_toast();
        return false;
    }
    return true;
}

// ==================== mainScreen ====================
static void mainScreen_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        printf("GESTURE on mainScreen dir=%d\n", (int)dir);
        switch (dir) {
        case LV_DIR_RIGHT:
            switch_to_screen(guider_ui.envScreen);
            break;
        case LV_DIR_LEFT:
            switch_to_screen(guider_ui.aircreen);
            break;
        default:
            break;
        }
        break;
    }
    case LV_EVENT_PRESSING:
        printf("PRESSING on mainScreen\n");
        break;
    case LV_EVENT_CLICKED:
        printf("CLICKED on mainScreen\n");
        break;
    default:
        break;
    }
}

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
        // 进入设置屏幕时启动 BLE 状态刷新定时器（每秒更新）
        if (!ble_status_timer) {
            ble_status_timer = lv_timer_create(ble_status_update_cb, 1000, NULL);
        }
        switch_to_screen(guider_ui.settingsScreen);
        break;
    default:
        break;
    }
}

void events_init_mainScreen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->mainScreen, mainScreen_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->mainScreen_img_boot_logo, mainScreen_img_boot_logo_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->mainScreen_img_wifi, mainScreen_img_wifi_event_handler, LV_EVENT_ALL, ui);
}

// ==================== airScreen ====================
static void aircreen_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        printf("GESTURE on aircreen dir=%d\n", (int)dir);
        switch (dir) {
        case LV_DIR_LEFT:
            switch_to_screen(guider_ui.envScreen);
            break;
        case LV_DIR_RIGHT:
            switch_to_screen(guider_ui.mainScreen);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_aircreen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->aircreen, aircreen_event_handler, LV_EVENT_ALL, ui);
}

// ==================== envScreen ====================
static void envScreen_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        printf("GESTURE on envScreen dir=%d\n", (int)dir);
        switch (dir) {
        case LV_DIR_LEFT:
            switch_to_screen(guider_ui.mainScreen);
            break;
        case LV_DIR_RIGHT:
            switch_to_screen(guider_ui.aircreen);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_envScreen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->envScreen, envScreen_event_handler, LV_EVENT_ALL, ui);
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
        // 离开设置屏幕时停止状态刷新定时器
        if (ble_status_timer) {
            lv_timer_del(ble_status_timer);
            ble_status_timer = NULL;
        }
        switch_to_screen(guider_ui.mainScreen);
        break;
    default:
        break;
    }
}

// 亮度滑块值变化 → 更新标签 + 发送BLE指令
static void settingsScreen_slider_Brightness_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int val = (int)lv_slider_get_value(slider);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", val);
        lv_label_set_text(guider_ui.settingsScreen_label_Bluetooth_light, buf);

        if (!check_ble_or_toast()) return;  // BLE 未连接则弹窗并跳过发送

        BL_SetBrightness((uint8_t)val);
        printf("[Settings] 亮度滑块: %d%%\n", val);
    }
}

// 色温滑块值变化 → 更新标签 + 发送BLE指令
static void settingsScreen_slider_ColorTemp_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int val = (int)lv_slider_get_value(slider);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", val);
        lv_label_set_text(guider_ui.settingsScreen_label_3, buf);

        if (!check_ble_or_toast()) return;  // BLE 未连接则弹窗并跳过发送

        BL_SetColorTemp((uint8_t)val);
        printf("[Settings] 色温滑块: %d (~%dK)\n", val, BL_ColorTempToKelvin((uint8_t)val));
    }
}

// 开关切换 → 开灯/关灯
static void settingsScreen_sw_open_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *sw = lv_event_get_target(e);
        bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);

        if (!check_ble_or_toast()) {
            // BLE 未连接：弹回开关到之前的状态
            if (on) {
                lv_obj_clear_state(sw, LV_STATE_CHECKED);
            } else {
                lv_obj_add_state(sw, LV_STATE_CHECKED);
            }
            return;
        }

        if (on) {
            BL_TurnOn();
            printf("[Settings] 开关: ON\n");
        } else {
            BL_TurnOff();
            printf("[Settings] 开关: OFF\n");
        }
    }
}

// OTA 检查更新按钮
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


// ==================== 屏幕加载完成回调（触发逐级显示动画）====================
static void screen_loaded_cb(lv_event_t *e)
{
    lv_obj_t *scr = lv_event_get_target(e);
    ui_anim_reveal_screen(&guider_ui, scr);
}

void events_init(lv_ui *ui)
{
    // 注册屏幕加载完成回调 → 触发控件逐级淡入动画
    lv_obj_add_event_cb(ui->mainScreen,    screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui->aircreen,      screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui->envScreen,     screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui->fouseScreen,   screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui->settingsScreen, screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    // 唯一的事件注册入口（各 setup_scr_*.c 末尾的调用已删除）
    events_init_mainScreen(ui);
    events_init_aircreen(ui);
    events_init_envScreen(ui);
    events_init_fouseScreen(ui);
    events_init_settingsScreen(ui);
}
