/*
 * Copyright 2026 NXP
 * NXP Proprietary.
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
	/* ---- bootScreen ---- */
	lv_obj_t *bootScreen;
	bool bootScreen_del;
	lv_obj_t *bootScreen_img_boot;
	lv_obj_t *bootScreen_label_status;

	/* ---- mainScreen ---- */
	lv_obj_t *mainScreen;
	bool mainScreen_del;
	lv_obj_t *mainScreen_label_time;
	lv_obj_t *mainScreen_label_wifi;
	lv_obj_t *mainScreen_label_summary;
	lv_obj_t *mainScreen_cont_aqi_card;
	lv_obj_t *mainScreen_label_aqi_val;
	lv_obj_t *mainScreen_label_aqi_title;
	lv_obj_t *mainScreen_cont_eco2_card;
	lv_obj_t *mainScreen_label_eco2_val;
	lv_obj_t *mainScreen_label_eco2_title;
	lv_obj_t *mainScreen_label_ai_tip;
	lv_obj_t *mainScreen_cont_role;
	lv_obj_t *mainScreen_img_role;
	lv_obj_t *mainScreen_label_role_name;
	lv_obj_t *mainScreen_cont_menu;
	lv_obj_t *mainScreen_btn_menu_air;
	lv_obj_t *mainScreen_btn_menu_air_label;
	lv_obj_t *mainScreen_btn_menu_env;
	lv_obj_t *mainScreen_btn_menu_env_label;
	lv_obj_t *mainScreen_btn_menu_focus;
	lv_obj_t *mainScreen_btn_menu_focus_label;
	lv_obj_t *mainScreen_btn_menu_settings;
	lv_obj_t *mainScreen_btn_menu_settings_label;

	/* ---- airScreen ---- */
	lv_obj_t *airScreen;
	bool airScreen_del;
	lv_obj_t *airScreen_btn_back;
	lv_obj_t *airScreen_btn_back_label;
	lv_obj_t *airScreen_label_title;
	lv_obj_t *airScreen_cont_tvoc_card;
	lv_obj_t *airScreen_label_tvoc_val;
	lv_obj_t *airScreen_label_tvoc_title;
	lv_obj_t *airScreen_cont_eco2_card;
	lv_obj_t *airScreen_label_eco2_val;
	lv_obj_t *airScreen_label_eco2_title;
	lv_obj_t *airScreen_label_aqi_badge;
	lv_obj_t *airScreen_label_mold;

	/* ---- envScreen ---- */
	lv_obj_t *envScreen;
	bool envScreen_del;
	lv_obj_t *envScreen_btn_back;
	lv_obj_t *envScreen_btn_back_label;
	lv_obj_t *envScreen_label_title;
	lv_obj_t *envScreen_label_temp;
	lv_obj_t *envScreen_label_humi;
	lv_obj_t *envScreen_label_light;
	lv_obj_t *envScreen_label_tvoc;
	lv_obj_t *envScreen_label_eco2;

	/* ---- focusScreen ---- */
	lv_obj_t *focusScreen;
	bool focusScreen_del;
	lv_obj_t *focusScreen_btn_exit;
	lv_obj_t *focusScreen_btn_exit_label;
	lv_obj_t *focusScreen_label_title;
	lv_obj_t *focusScreen_label_timer;
	lv_obj_t *focusScreen_label_light_info;
	lv_obj_t *focusScreen_img_role;
	lv_obj_t *focusScreen_label_status;
	lv_obj_t *focusScreen_btn_presence;
	lv_obj_t *focusScreen_btn_presence_label;

	/* ---- settingsScreen ---- */
	lv_obj_t *settingsScreen;
	bool settingsScreen_del;
	lv_obj_t *settingsScreen_btn_back;
	lv_obj_t *settingsScreen_btn_back_label;
	lv_obj_t *settingsScreen_label_title;
	lv_obj_t *settingsScreen_label_brightness;
	lv_obj_t *settingsScreen_slider_brightness;
	lv_obj_t *settingsScreen_cb_night;
	lv_obj_t *settingsScreen_label_night;
	lv_obj_t *settingsScreen_btn_lock;
	lv_obj_t *settingsScreen_btn_lock_label;
	lv_obj_t *settingsScreen_label_lock_hint;

	/* ---- lockScreen ---- */
	lv_obj_t *lockScreen;
	bool lockScreen_del;
	lv_obj_t *lockScreen_img_role;
	lv_obj_t *lockScreen_label_title;
	lv_obj_t *lockScreen_btn_unlock;
	lv_obj_t *lockScreen_btn_unlock_label;

} lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del,
                           ui_setup_scr_t setup_scr, lv_scr_load_anim_t anim_type,
                           uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value,
                  lv_anim_path_cb_t path_cb, uint16_t repeat_cnt, uint32_t repeat_delay,
                  uint32_t playback_time, uint32_t playback_delay, lv_anim_exec_xcb_t exec_cb,
                  lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);

void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;

void setup_scr_bootScreen(lv_ui *ui);
void setup_scr_mainScreen(lv_ui *ui);
void setup_scr_airScreen(lv_ui *ui);
void setup_scr_envScreen(lv_ui *ui);
void setup_scr_focusScreen(lv_ui *ui);
void setup_scr_settingsScreen(lv_ui *ui);
void setup_scr_lockScreen(lv_ui *ui);

LV_FONT_DECLARE(lv_font_yezi_hei_10)
LV_FONT_DECLARE(lv_font_montserratMedium_12)

#ifdef __cplusplus
}
#endif
#endif
