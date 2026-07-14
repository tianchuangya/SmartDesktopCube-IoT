/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{

	lv_obj_t *bootScreen;
	bool bootScreen_del;
	lv_obj_t *bootScreen_img_boot_sleep;
	lv_obj_t *bootScreen_label_status;
	lv_obj_t *mainScreen;
	bool mainScreen_del;
	lv_obj_t *mainScreen_label_time;
	lv_obj_t *mainScreen_label_summary;
	lv_obj_t *mainScreen_cont_aqi_card;
	lv_obj_t *mainScreen_label_aqi_title;
	lv_obj_t *mainScreen_label_eco2_val;
	lv_obj_t *mainScreen_cont_eco2_card;
	lv_obj_t *mainScreen_label_eco2_title;
	lv_obj_t *mainScreen_label_Prompt;
	lv_obj_t *mainScreen_img_role;
	lv_obj_t *mainScreen_label_aqi_val;
	lv_obj_t *mainScreen_img_boot_logo;
	lv_obj_t *mainScreen_img_wifi;
	lv_obj_t *aircreen;
	bool aircreen_del;
	lv_obj_t *aircreen_label_eco2_val;
	lv_obj_t *aircreen_label_eco2_title;
	lv_obj_t *aircreen_cont_2;
	lv_obj_t *aircreen_cont_1;
	lv_obj_t *aircreen_label_tvoc_title;
	lv_obj_t *aircreen_label_tvoc_val;
	lv_obj_t *aircreen_label_title;
	lv_obj_t *aircreen_img_role2;
	lv_obj_t *aircreen_label_ai_tip;
	lv_obj_t *envScreen;
	bool envScreen_del;
	lv_obj_t *envScreen_label_temp;
	lv_obj_t *envScreen_label_humi;
	lv_obj_t *envScreen_label_light;
	lv_obj_t *envScreen_label_tvoc;
	lv_obj_t *envScreen_label_eco2;
	lv_obj_t *envScreen_label_title;
	lv_obj_t *envScreen_label_pm25;
	lv_obj_t *fouseScreen;
	bool fouseScreen_del;
	lv_obj_t *fouseScreen_btn_1;
	lv_obj_t *fouseScreen_btn_1_label;
	lv_obj_t *fouseScreen_label_focus_timer;
	lv_obj_t *fouseScreen_label_light_info;
	lv_obj_t *fouseScreen_label_status;
	lv_obj_t *fouseScreen_img_role_focus;
	lv_obj_t *fouseScreen_label_1;
	lv_obj_t *settingsScreen;
	bool settingsScreen_del;
	lv_obj_t *settingsScreen_slider_Brightness;
	lv_obj_t *settingsScreen_sw_open;
	lv_obj_t *settingsScreen_label_Bluetooth_light;
	lv_obj_t *settingsScreen_label_1;
	lv_obj_t *settingsScreen_label_2;
	lv_obj_t *settingsScreen_label_3;
	lv_obj_t *settingsScreen_slider_Color_temperature;
	lv_obj_t *settingsScreen_btn_1;
	lv_obj_t *settingsScreen_btn_1_label;
	lv_obj_t *settingsScreen_label_4;
	lv_obj_t *settingsScreen_label_ble_status;
	lv_obj_t *settingsScreen_btn_ota;
	lv_obj_t *settingsScreen_btn_ota_label;
	lv_obj_t *lockScreen;
	bool lockScreen_del;
	lv_obj_t *lockScreen_img_1;
	lv_obj_t *lockScreen_label_title;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                       uint16_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                       lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_ui(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_bootScreen(lv_ui *ui);
void setup_scr_mainScreen(lv_ui *ui);
void setup_scr_aircreen(lv_ui *ui);
void setup_scr_envScreen(lv_ui *ui);
void setup_scr_fouseScreen(lv_ui *ui);
void setup_scr_settingsScreen(lv_ui *ui);
void setup_scr_lockScreen(lv_ui *ui);

LV_IMG_DECLARE(_img_bg_main_320x240);
LV_IMG_DECLARE(_img_boot_sleep_22_alpha_168x155);

LV_IMG_DECLARE(_img_bg_main_320x240);
LV_IMG_DECLARE(_img_role_normal_alpha_60x60);
LV_IMG_DECLARE(_img_boot_logo_alpha_60x60);
LV_IMG_DECLARE(_wifion_alpha_28x28);

LV_IMG_DECLARE(_img_bg_main_320x240);
LV_IMG_DECLARE(_img_role_normal_alpha_60x60);

LV_IMG_DECLARE(_img_bg_main_320x240);

LV_IMG_DECLARE(_img_bg_focus_320x240);
LV_IMG_DECLARE(_img_role_focus_alpha_60x60);

LV_IMG_DECLARE(_img_bg_main_320x240);

LV_IMG_DECLARE(_img_bg_report_320x240);
LV_IMG_DECLARE(_img_role_alert_alpha_100x100);

LV_FONT_DECLARE(lv_font_yezi_hei_16)
LV_FONT_DECLARE(lv_font_montserratMedium_18)
LV_FONT_DECLARE(lv_font_yezi_hei_12)
LV_FONT_DECLARE(lv_font_yezi_hei_15)
LV_FONT_DECLARE(lv_font_yezi_hei_18)
LV_FONT_DECLARE(lv_font_montserratMedium_40)
LV_FONT_DECLARE(lv_font_yezi_hei_14)
LV_FONT_DECLARE(lv_font_yezi_hei_20)
LV_FONT_DECLARE(lv_font_montserratMedium_16)


#ifdef __cplusplus
}
#endif
#endif
