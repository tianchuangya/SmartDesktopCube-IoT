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



void setup_scr_settingsScreen(lv_ui *ui)
{
    //Write codes settingsScreen
    ui->settingsScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->settingsScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->settingsScreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for settingsScreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->settingsScreen, &_img_bg_main_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->settingsScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->settingsScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes settingsScreen_slider_Brightness
    ui->settingsScreen_slider_Brightness = lv_slider_create(ui->settingsScreen);
    lv_slider_set_range(ui->settingsScreen_slider_Brightness, 0, 100);
    lv_slider_set_mode(ui->settingsScreen_slider_Brightness, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->settingsScreen_slider_Brightness, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->settingsScreen_slider_Brightness, 22, 62);
    lv_obj_set_size(ui->settingsScreen_slider_Brightness, 257, 31);

    //Write style for settingsScreen_slider_Brightness, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_slider_Brightness, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_slider_Brightness, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_slider_Brightness, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_slider_Brightness, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->settingsScreen_slider_Brightness, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_slider_Brightness, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for settingsScreen_slider_Brightness, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_slider_Brightness, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_slider_Brightness, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_slider_Brightness, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_slider_Brightness, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for settingsScreen_slider_Brightness, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_slider_Brightness, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_slider_Brightness, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_slider_Brightness, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_slider_Brightness, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes settingsScreen_sw_open
    ui->settingsScreen_sw_open = lv_switch_create(ui->settingsScreen);
    lv_obj_set_pos(ui->settingsScreen_sw_open, 122, 192);
    lv_obj_set_size(ui->settingsScreen_sw_open, 58, 34);

    //Write style for settingsScreen_sw_open, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_sw_open, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_sw_open, lv_color_hex(0x1bfebd), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_sw_open, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->settingsScreen_sw_open, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_sw_open, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_sw_open, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for settingsScreen_sw_open, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->settingsScreen_sw_open, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->settingsScreen_sw_open, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_sw_open, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->settingsScreen_sw_open, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for settingsScreen_sw_open, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_sw_open, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_sw_open, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_sw_open, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->settingsScreen_sw_open, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_sw_open, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes settingsScreen_label_Bluetooth_light
    ui->settingsScreen_label_Bluetooth_light = lv_label_create(ui->settingsScreen);
    lv_label_set_text(ui->settingsScreen_label_Bluetooth_light, "50");
    lv_label_set_long_mode(ui->settingsScreen_label_Bluetooth_light, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->settingsScreen_label_Bluetooth_light, 117, 26);
    lv_obj_set_size(ui->settingsScreen_label_Bluetooth_light, 61, 28);

    //Write style for settingsScreen_label_Bluetooth_light, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_label_Bluetooth_light, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_label_Bluetooth_light, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_label_Bluetooth_light, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_label_Bluetooth_light, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_label_Bluetooth_light, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes settingsScreen_label_1
    ui->settingsScreen_label_1 = lv_label_create(ui->settingsScreen);
    lv_label_set_text(ui->settingsScreen_label_1, "Light");
    lv_label_set_long_mode(ui->settingsScreen_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->settingsScreen_label_1, 22, 26);
    lv_obj_set_size(ui->settingsScreen_label_1, 90, 18);

    //Write style for settingsScreen_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_label_1, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes settingsScreen_label_2
    ui->settingsScreen_label_2 = lv_label_create(ui->settingsScreen);
    lv_label_set_text(ui->settingsScreen_label_2, "Temp");
    lv_label_set_long_mode(ui->settingsScreen_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->settingsScreen_label_2, 22, 108);
    lv_obj_set_size(ui->settingsScreen_label_2, 90, 32);

    //Write style for settingsScreen_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_label_2, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes settingsScreen_label_3
    ui->settingsScreen_label_3 = lv_label_create(ui->settingsScreen);
    lv_label_set_text(ui->settingsScreen_label_3, "50");
    lv_label_set_long_mode(ui->settingsScreen_label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->settingsScreen_label_3, 122, 108);
    lv_obj_set_size(ui->settingsScreen_label_3, 57, 32);

    //Write style for settingsScreen_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_label_3, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_label_3, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes settingsScreen_slider_Color_temperature
    ui->settingsScreen_slider_Color_temperature = lv_slider_create(ui->settingsScreen);
    lv_slider_set_range(ui->settingsScreen_slider_Color_temperature, 0, 100);
    lv_slider_set_mode(ui->settingsScreen_slider_Color_temperature, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->settingsScreen_slider_Color_temperature, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->settingsScreen_slider_Color_temperature, 22, 140);
    lv_obj_set_size(ui->settingsScreen_slider_Color_temperature, 257, 31);

    //Write style for settingsScreen_slider_Color_temperature, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_slider_Color_temperature, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_slider_Color_temperature, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_slider_Color_temperature, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_slider_Color_temperature, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->settingsScreen_slider_Color_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_slider_Color_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for settingsScreen_slider_Color_temperature, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_slider_Color_temperature, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_slider_Color_temperature, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_slider_Color_temperature, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_slider_Color_temperature, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for settingsScreen_slider_Color_temperature, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_slider_Color_temperature, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_slider_Color_temperature, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_slider_Color_temperature, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_slider_Color_temperature, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes settingsScreen_btn_1
    ui->settingsScreen_btn_1 = lv_btn_create(ui->settingsScreen);
    ui->settingsScreen_btn_1_label = lv_label_create(ui->settingsScreen_btn_1);
    lv_label_set_text(ui->settingsScreen_btn_1_label, "返回");
    lv_label_set_long_mode(ui->settingsScreen_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->settingsScreen_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->settingsScreen_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->settingsScreen_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->settingsScreen_btn_1, 7, 12);
    lv_obj_set_size(ui->settingsScreen_btn_1, 65, 26);

    //Write style for settingsScreen_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->settingsScreen_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_btn_1, lv_color_hex(0xb7d1e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->settingsScreen_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_btn_1, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_btn_1, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes settingsScreen_label_4
    ui->settingsScreen_label_4 = lv_label_create(ui->settingsScreen);
    lv_label_set_text(ui->settingsScreen_label_4, "台灯设置");
    lv_label_set_long_mode(ui->settingsScreen_label_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->settingsScreen_label_4, 229, 6);
    lv_obj_set_size(ui->settingsScreen_label_4, 100, 32);

    //Write style for settingsScreen_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_label_4, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_label_4, &lv_font_yezi_hei_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of settingsScreen.

    // 灯具连接状态标签
    ui->settingsScreen_label_ble_status = lv_label_create(ui->settingsScreen);
    lv_label_set_text(ui->settingsScreen_label_ble_status, "Light: checking...");
    lv_label_set_long_mode(ui->settingsScreen_label_ble_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->settingsScreen_label_ble_status, 50, 222);
    lv_obj_set_size(ui->settingsScreen_label_ble_status, 220, 18);
    lv_obj_set_style_text_color(ui->settingsScreen_label_ble_status, lv_color_hex(0x888888), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_label_ble_status, &lv_font_yezi_hei_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_label_ble_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->settingsScreen_label_ble_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // OTA 检查更新按钮
    ui->settingsScreen_btn_ota = lv_btn_create(ui->settingsScreen);
    ui->settingsScreen_btn_ota_label = lv_label_create(ui->settingsScreen_btn_ota);
    lv_label_set_text(ui->settingsScreen_btn_ota_label, "检查更新");
    lv_label_set_long_mode(ui->settingsScreen_btn_ota_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->settingsScreen_btn_ota_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->settingsScreen_btn_ota, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->settingsScreen_btn_ota_label, LV_PCT(100));
    lv_obj_set_pos(ui->settingsScreen_btn_ota, 220, 192);
    lv_obj_set_size(ui->settingsScreen_btn_ota, 80, 34);
    lv_obj_set_style_bg_opa(ui->settingsScreen_btn_ota, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->settingsScreen_btn_ota, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->settingsScreen_btn_ota, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->settingsScreen_btn_ota, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->settingsScreen_btn_ota, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->settingsScreen_btn_ota, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->settingsScreen_btn_ota, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->settingsScreen_btn_ota, &lv_font_yezi_hei_14, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->settingsScreen_btn_ota, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->settingsScreen_btn_ota, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Update current screen layout.
    lv_obj_update_layout(ui->settingsScreen);

    // events_init_settingsScreen 已在 events_init() 统一调用，此处删除避免双重注册
}
