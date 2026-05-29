#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

// 所有画面在 setup_ui() 中已创建，这里直接切换，不重建不删除
static uint32_t _last_switch_ms = 0;

static void switch_to_screen(lv_obj_t *new_scr)
{
    uint32_t now = lv_tick_get();
    if (now - _last_switch_ms < 250) return;
    _last_switch_ms = now;

    lv_scr_load_anim(new_scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
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

static void mainScreen_img_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
        printf("CLICKED img_1 -> fouseScreen\n");
        switch_to_screen(guider_ui.fouseScreen);
        break;
    default:
        break;
    }
}

void events_init_mainScreen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->mainScreen, mainScreen_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->mainScreen_img_1, mainScreen_img_1_event_handler, LV_EVENT_ALL, ui);
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


void events_init(lv_ui *ui)
{
    events_init_mainScreen(ui);
    events_init_aircreen(ui);
    events_init_envScreen(ui);
    events_init_fouseScreen(ui);
}
