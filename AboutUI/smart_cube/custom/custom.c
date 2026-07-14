/*
 * Copyright 2023 NXP
 * NXP Proprietary.
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <time.h>
#include "lvgl.h"
#include "custom.h"

/*
 * Connect to your IoT project's DataPool.
 * Include your DataPool header to access sensorData, status, etc.
 */
#include "DataPool.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *  GLOBAL VARIABLES
 *  (stub data for simulator; real data from ESP32)
 **********************/
SensorData sensorData = {
    .temp = 26.0f,
    .humi = 54.0f,
    .tvoc = 120.0f,
    .eco2 = 450.0f,
    .light = 320.0f,
    .pm25 = 15.0f,
    .aqi = 1,
    .human_duration = 0.0f,
};

DeviceStatus status = {
    .wifi_connected = true,
    .mqtt_connected = true,
    .is_human_exist = true,
    .screen_brightness = 80,
    .focus_mode = false,
    .device_lock = false,
};

/**********************
 *  STATIC VARIABLES
 **********************/
static int focus_seconds = 0;
static lv_timer_t *focus_timer = NULL;
static lv_timer_t *ui_update_timer = NULL;
static bool is_user_present = true;
static bool night_mode_on = false;

/**********************
 *  CALLBACKS
 **********************/

/**
 * Main UI update timer — reads sensorData & status from DataPool
 * and updates ALL screen labels. Runs every 500ms.
 */
void ui_update_timer_cb(lv_timer_t *t)
{
	lv_ui *ui = (lv_ui *)t->user_data;

	/* ---- Update time on mainScreen ---- */
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char time_buf[16];
	snprintf(time_buf, sizeof(time_buf), "%02d:%02d", tm_info->tm_hour, tm_info->tm_min);
	lv_label_set_text(ui->mainScreen_label_time, time_buf);

	/* ---- WiFi status from DataPool ---- */
	if (status.wifi_connected) {
		lv_label_set_text(ui->mainScreen_label_wifi, "WiFi ✓");
	} else {
		lv_label_set_text(ui->mainScreen_label_wifi, "WiFi ✗");
	}

	/* ---- MainScreen: temp/humi/light summary ---- */
	char summary_buf[64];
	snprintf(summary_buf, sizeof(summary_buf),
	         "%.1f°C | %.0f%% | %.0f lx",
	         sensorData.temp, sensorData.humi, sensorData.light);
	lv_label_set_text(ui->mainScreen_label_summary, summary_buf);

	/* ---- MainScreen: AQI card ---- */
	char aqi_buf[8];
	snprintf(aqi_buf, sizeof(aqi_buf), "%d", sensorData.aqi);
	lv_label_set_text(ui->mainScreen_label_aqi_val, aqi_buf);

	/* ---- MainScreen: eCO2 card ---- */
	char eco2_buf[8];
	snprintf(eco2_buf, sizeof(eco2_buf), "%.0f", sensorData.eco2);
	lv_label_set_text(ui->mainScreen_label_eco2_val, eco2_buf);

	/* ---- MainScreen: AI tip based on AQI ---- */
	const char *ai_tip;
	if (sensorData.aqi <= 1) {
		ai_tip = "AI建议：当前环境舒适";
	} else if (sensorData.aqi <= 2) {
		ai_tip = "AI建议：建议开窗通风";
	} else if (sensorData.aqi <= 3) {
		ai_tip = "AI建议：空气质量一般";
	} else {
		ai_tip = "AI建议：请开启空气净化";
	}
	lv_label_set_text(ui->mainScreen_label_ai_tip, ai_tip);

	/* ---- airScreen: TVOC value ---- */
	char tvoc_buf[8];
	snprintf(tvoc_buf, sizeof(tvoc_buf), "%.0f", sensorData.tvoc);
	lv_label_set_text(ui->airScreen_label_tvoc_val, tvoc_buf);

	/* ---- airScreen: eCO2 value ---- */
	lv_label_set_text(ui->airScreen_label_eco2_val, eco2_buf);

	/* ---- airScreen: AQI badge ---- */
	char aqi_badge[32];
	const char *aqi_label;
	lv_color_t badge_color;
	if (sensorData.aqi <= 1) {
		aqi_label = "优";
		badge_color = lv_color_hex(0x4caf50);
	} else if (sensorData.aqi <= 2) {
		aqi_label = "良";
		badge_color = lv_color_hex(0xff9800);
	} else if (sensorData.aqi <= 3) {
		aqi_label = "中";
		badge_color = lv_color_hex(0xff9800);
	} else {
		aqi_label = "差";
		badge_color = lv_color_hex(0xf44336);
	}
	snprintf(aqi_badge, sizeof(aqi_badge), "AQI %d %s", sensorData.aqi, aqi_label);
	lv_label_set_text(ui->airScreen_label_aqi_badge, aqi_badge);
	lv_obj_set_style_bg_color(ui->airScreen_label_aqi_badge, badge_color,
	                          LV_PART_MAIN|LV_STATE_DEFAULT);

	/* ---- envScreen: all sensor data ---- */
	char env_buf[64];
	snprintf(env_buf, sizeof(env_buf), "温度: %.1f°C", sensorData.temp);
	lv_label_set_text(ui->envScreen_label_temp, env_buf);

	snprintf(env_buf, sizeof(env_buf), "湿度: %.0f%%", sensorData.humi);
	lv_label_set_text(ui->envScreen_label_humi, env_buf);

	snprintf(env_buf, sizeof(env_buf), "光照: %.0f lx", sensorData.light);
	lv_label_set_text(ui->envScreen_label_light, env_buf);

	snprintf(env_buf, sizeof(env_buf), "TVOC: %.0f ppb", sensorData.tvoc);
	lv_label_set_text(ui->envScreen_label_tvoc, env_buf);

	snprintf(env_buf, sizeof(env_buf), "eCO2: %.0f ppm", sensorData.eco2);
	lv_label_set_text(ui->envScreen_label_eco2, env_buf);
}

/**
 * Focus mode timer — counts up every second.
 */
void focus_count_timer_cb(lv_timer_t *t)
{
	lv_ui *ui = (lv_ui *)t->user_data;
	focus_seconds++;

	int mins = focus_seconds / 60;
	int secs = focus_seconds % 60;
	char buf[8];
	snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
	lv_label_set_text(ui->focusScreen_label_timer, buf);
}

void reset_focus_timer(void)
{
	focus_seconds = 0;
	if (focus_timer) {
		lv_timer_del(focus_timer);
	}
	focus_timer = lv_timer_create(focus_count_timer_cb, 1000, &guider_ui);
}

void stop_focus_timer(void)
{
	if (focus_timer) {
		lv_timer_del(focus_timer);
		focus_timer = NULL;
	}
	focus_seconds = 0;
}

void toggle_presence(void)
{
	is_user_present = !is_user_present;
	lv_label_set_text(guider_ui.focusScreen_btn_presence_label,
	                  is_user_present ? "模拟离开" : "模拟回来");
}

void set_night_mode(bool enabled)
{
	night_mode_on = enabled;
	lv_obj_t *scr = lv_scr_act();
	if (enabled) {
		lv_obj_set_style_bg_opa(scr, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(scr, lv_color_hex(0x0d0d1a), LV_PART_MAIN|LV_STATE_DEFAULT);
	}
	/* Restore handled by screen's own setup_scr when screen changes */
}

/**********************
 *  custom_init
 **********************/
void custom_init(lv_ui *ui)
{
	/* Start the main UI update timer — refreshes sensor data every 500ms */
	ui_update_timer = lv_timer_create(ui_update_timer_cb, 500, ui);
}
