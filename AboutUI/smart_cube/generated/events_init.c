/*
 * Copyright 2026 NXP
 * NXP Proprietary.
 */

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "custom.h"

/* ---------- State variables ---------- */
static bool menu_active = false;
static int system_mode = 0;  /* 0=normal, 1=focus, 2=locked */

/* ---------- bootScreen ---------- */
static void bootScreen_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
	case LV_EVENT_SCREEN_LOADED:
	{
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		/* Auto transition to mainScreen after 2s */
		lv_timer_t *boot_timer = lv_timer_create(NULL, 2000, (void *)ui);
		lv_timer_set_cb(boot_timer, (lv_timer_cb_t)lv_scr_load);
		lv_timer_set_user_data(boot_timer, ui->mainScreen);
		lv_timer_set_auto_delete(boot_timer, true);
		break;
	}
	default:
		break;
	}
}

void events_init_bootScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->bootScreen, bootScreen_event_handler, LV_EVENT_ALL, ui);
}

/* ---------- mainScreen ---------- */
static void mainScreen_role_toggle_menu(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		if (system_mode == 2) return; /* locked */
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		menu_active = !menu_active;
		if (menu_active) {
			lv_obj_clear_flag(ui->mainScreen_cont_menu, LV_OBJ_FLAG_HIDDEN);
		} else {
			lv_obj_add_flag(ui->mainScreen_cont_menu, LV_OBJ_FLAG_HIDDEN);
		}
	}
}

static void mainScreen_menu_close_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		lv_obj_add_flag(ui->mainScreen_cont_menu, LV_OBJ_FLAG_HIDDEN);
		menu_active = false;
	}
}

static void mainScreen_btn_air_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		ui_load_scr_animation(ui, &ui->airScreen, ui->airScreen_del, &ui->mainScreen_del,
		                      setup_scr_airScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 200, false, true);
		ui->mainScreen_del = true;
	}
}

static void mainScreen_btn_env_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		ui_load_scr_animation(ui, &ui->envScreen, ui->envScreen_del, &ui->mainScreen_del,
		                      setup_scr_envScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 200, false, true);
		ui->mainScreen_del = true;
	}
}

static void mainScreen_btn_focus_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		system_mode = 1;
		reset_focus_timer();
		ui_load_scr_animation(ui, &ui->focusScreen, ui->focusScreen_del, &ui->mainScreen_del,
		                      setup_scr_focusScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 200, false, true);
		ui->mainScreen_del = true;
	}
}

static void mainScreen_btn_settings_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		ui_load_scr_animation(ui, &ui->settingsScreen, ui->settingsScreen_del, &ui->mainScreen_del,
		                      setup_scr_settingsScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 200, false, true);
		ui->mainScreen_del = true;
	}
}

void events_init_mainScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->mainScreen_cont_role, mainScreen_role_toggle_menu, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->mainScreen_img_role, mainScreen_role_toggle_menu, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->mainScreen_cont_menu, mainScreen_menu_close_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->mainScreen_btn_menu_air, mainScreen_btn_air_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->mainScreen_btn_menu_env, mainScreen_btn_env_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->mainScreen_btn_menu_focus, mainScreen_btn_focus_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->mainScreen_btn_menu_settings, mainScreen_btn_settings_event, LV_EVENT_ALL, ui);
}

/* ---------- airScreen ---------- */
static void airScreen_btn_back_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		ui_load_scr_animation(ui, &ui->mainScreen, ui->mainScreen_del, &ui->airScreen_del,
		                      setup_scr_mainScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, false, true);
		ui->airScreen_del = true;
	}
}

void events_init_airScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->airScreen_btn_back, airScreen_btn_back_event, LV_EVENT_ALL, ui);
}

/* ---------- envScreen ---------- */
static void envScreen_btn_back_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		ui_load_scr_animation(ui, &ui->mainScreen, ui->mainScreen_del, &ui->envScreen_del,
		                      setup_scr_mainScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, false, true);
		ui->envScreen_del = true;
	}
}

void events_init_envScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->envScreen_btn_back, envScreen_btn_back_event, LV_EVENT_ALL, ui);
}

/* ---------- focusScreen ---------- */
static void focusScreen_btn_exit_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		system_mode = 0;
		stop_focus_timer();
		ui_load_scr_animation(ui, &ui->mainScreen, ui->mainScreen_del, &ui->focusScreen_del,
		                      setup_scr_mainScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, false, true);
		ui->focusScreen_del = true;
	}
}

static void focusScreen_btn_presence_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		toggle_presence();
	}
}

void events_init_focusScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->focusScreen_btn_exit, focusScreen_btn_exit_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->focusScreen_btn_presence, focusScreen_btn_presence_event, LV_EVENT_ALL, ui);
}

/* ---------- settingsScreen ---------- */
static void settingsScreen_btn_back_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		ui_load_scr_animation(ui, &ui->mainScreen, ui->mainScreen_del, &ui->settingsScreen_del,
		                      setup_scr_mainScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, false, true);
		ui->settingsScreen_del = true;
	}
}

static void settingsScreen_btn_lock_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		system_mode = 2;
		ui_load_scr_animation(ui, &ui->lockScreen, ui->lockScreen_del, &ui->settingsScreen_del,
		                      setup_scr_lockScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 200, false, true);
		ui->settingsScreen_del = true;
	}
}

static void settingsScreen_slider_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_VALUE_CHANGED) {
		lv_obj_t *slider = lv_event_get_target(e);
		int16_t val = (int16_t)lv_slider_get_value(slider);
		/* Brightness value stored, can be applied via PWM/MQTT */
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		char buf[32];
		snprintf(buf, sizeof(buf), "亮度: %d", val);
		lv_label_set_text(ui->settingsScreen_label_brightness, buf);
	}
}

static void settingsScreen_night_cb_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_VALUE_CHANGED) {
		lv_obj_t *cb = lv_event_get_target(e);
		bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
		/* Apply night mode filter */
		set_night_mode(checked);
	}
}

void events_init_settingsScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->settingsScreen_btn_back, settingsScreen_btn_back_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->settingsScreen_btn_lock, settingsScreen_btn_lock_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->settingsScreen_slider_brightness, settingsScreen_slider_event, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->settingsScreen_cb_night, settingsScreen_night_cb_event, LV_EVENT_ALL, ui);
}

/* ---------- lockScreen ---------- */
static void lockScreen_btn_unlock_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_CLICKED) {
		lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
		system_mode = 0;
		ui_load_scr_animation(ui, &ui->mainScreen, ui->mainScreen_del, &ui->lockScreen_del,
		                      setup_scr_mainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 200, false, true);
		ui->lockScreen_del = true;
	}
}

void events_init_lockScreen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->lockScreen_btn_unlock, lockScreen_btn_unlock_event, LV_EVENT_ALL, ui);
}

/* ---------- global events_init ---------- */
void events_init(lv_ui *ui)
{
	events_init_bootScreen(ui);
	events_init_mainScreen(ui);
	events_init_airScreen(ui);
	events_init_envScreen(ui);
	events_init_focusScreen(ui);
	events_init_settingsScreen(ui);
	events_init_lockScreen(ui);
}
