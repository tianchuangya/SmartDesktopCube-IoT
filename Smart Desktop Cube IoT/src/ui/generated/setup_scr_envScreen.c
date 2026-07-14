/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "ui/custom/custom.h"



void setup_scr_envScreen(lv_ui *ui)
{
    //Write codes envScreen
    ui->envScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->envScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->envScreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for envScreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->envScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->envScreen, &_img_bg_main_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->envScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->envScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_temp
    ui->envScreen_label_temp = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_temp, "温度: 26.3°C");
    lv_label_set_long_mode(ui->envScreen_label_temp, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_temp, 9, 37);
    lv_obj_set_size(ui->envScreen_label_temp, 100, 32);

    //Write style for envScreen_label_temp, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_temp, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_temp, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_temp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_temp, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_humi
    ui->envScreen_label_humi = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_humi, "湿度: 54%");
    lv_label_set_long_mode(ui->envScreen_label_humi, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_humi, 9, 61);
    lv_obj_set_size(ui->envScreen_label_humi, 100, 32);

    //Write style for envScreen_label_humi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_humi, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_humi, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_humi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_humi, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_light
    ui->envScreen_label_light = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_light, "光照: 320 lx");
    lv_label_set_long_mode(ui->envScreen_label_light, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_light, 9, 83);
    lv_obj_set_size(ui->envScreen_label_light, 100, 17);

    //Write style for envScreen_label_light, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_light, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_light, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_light, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_light, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_tvoc
    ui->envScreen_label_tvoc = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_tvoc, "TVOC: 120 ppb");
    lv_label_set_long_mode(ui->envScreen_label_tvoc, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_tvoc, 9, 106);
    lv_obj_set_size(ui->envScreen_label_tvoc, 114, 15);

    //Write style for envScreen_label_tvoc, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_tvoc, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_tvoc, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_tvoc, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_tvoc, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_tvoc, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_eco2
    ui->envScreen_label_eco2 = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_eco2, "eCO2: 450 ppm");
    lv_label_set_long_mode(ui->envScreen_label_eco2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_eco2, 9, 128);
    lv_obj_set_size(ui->envScreen_label_eco2, 134, 17);

    //Write style for envScreen_label_eco2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_eco2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_eco2, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_eco2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_eco2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_eco2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_title
    ui->envScreen_label_title = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_title, "环境数据");
    lv_label_set_long_mode(ui->envScreen_label_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_title, 239, 6);
    lv_obj_set_size(ui->envScreen_label_title, 77, 21);

    //Write style for envScreen_label_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_title, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes envScreen_label_pm25
    ui->envScreen_label_pm25 = lv_label_create(ui->envScreen);
    lv_label_set_text(ui->envScreen_label_pm25, "PM2.5: -- ug/m3");
    lv_label_set_long_mode(ui->envScreen_label_pm25, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->envScreen_label_pm25, 9, 149);
    lv_obj_set_size(ui->envScreen_label_pm25, 128, 16);

    //Write style for envScreen_label_pm25, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->envScreen_label_pm25, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->envScreen_label_pm25, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->envScreen_label_pm25, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->envScreen_label_pm25, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->envScreen_label_pm25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of envScreen.


    //Update current screen layout.
    lv_obj_update_layout(ui->envScreen);

    // events_init_envScreen 已在 events_init() 统一调用，此处删除避免双重注册
}
