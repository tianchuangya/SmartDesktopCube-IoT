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



void setup_scr_mainScreen(lv_ui *ui)
{
    //Write codes mainScreen
    ui->mainScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->mainScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->mainScreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for mainScreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->mainScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->mainScreen, &_img_bg_main_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->mainScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->mainScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_time
    ui->mainScreen_label_time = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_time, "12:36");
    lv_label_set_long_mode(ui->mainScreen_label_time, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_time, 4, 8);
    lv_obj_set_size(ui->mainScreen_label_time, 46, 15);

    //Write style for mainScreen_label_time, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_time, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_time, &lv_font_montserratMedium_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_time, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_time, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_summary
    ui->mainScreen_label_summary = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_summary, "26°C | 54% | 320 lx");
    lv_label_set_long_mode(ui->mainScreen_label_summary, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_summary, 8, 34);
    lv_obj_set_size(ui->mainScreen_label_summary, 118, 15);

    //Write style for mainScreen_label_summary, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_summary, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_summary, &lv_font_yezi_hei_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_summary, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_summary, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_summary, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_cont_aqi_card
    ui->mainScreen_cont_aqi_card = lv_obj_create(ui->mainScreen);
    lv_obj_set_pos(ui->mainScreen_cont_aqi_card, 16, 58);
    lv_obj_set_size(ui->mainScreen_cont_aqi_card, 138, 55);
    lv_obj_set_scrollbar_mode(ui->mainScreen_cont_aqi_card, LV_SCROLLBAR_MODE_OFF);

    //Write style for mainScreen_cont_aqi_card, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_cont_aqi_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_cont_aqi_card, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_cont_aqi_card, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->mainScreen_cont_aqi_card, lv_color_hex(0x00eeff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->mainScreen_cont_aqi_card, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_cont_aqi_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_cont_aqi_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_cont_aqi_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_cont_aqi_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_cont_aqi_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_aqi_title
    ui->mainScreen_label_aqi_title = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_aqi_title, "AQI");
    lv_label_set_long_mode(ui->mainScreen_label_aqi_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_aqi_title, 32, 89);
    lv_obj_set_size(ui->mainScreen_label_aqi_title, 100, 32);

    //Write style for mainScreen_label_aqi_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_aqi_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_aqi_title, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_aqi_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_aqi_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_aqi_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_eco2_val
    ui->mainScreen_label_eco2_val = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_eco2_val, "450");
    lv_label_set_long_mode(ui->mainScreen_label_eco2_val, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_eco2_val, 162, 66);
    lv_obj_set_size(ui->mainScreen_label_eco2_val, 138, 30);

    //Write style for mainScreen_label_eco2_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_eco2_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_eco2_val, &lv_font_yezi_hei_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_eco2_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_eco2_val, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_cont_eco2_card
    ui->mainScreen_cont_eco2_card = lv_obj_create(ui->mainScreen);
    lv_obj_set_pos(ui->mainScreen_cont_eco2_card, 166, 58);
    lv_obj_set_size(ui->mainScreen_cont_eco2_card, 138, 55);
    lv_obj_set_scrollbar_mode(ui->mainScreen_cont_eco2_card, LV_SCROLLBAR_MODE_OFF);

    //Write style for mainScreen_cont_eco2_card, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_cont_eco2_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_cont_eco2_card, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_cont_eco2_card, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->mainScreen_cont_eco2_card, lv_color_hex(0x00eeff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->mainScreen_cont_eco2_card, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_cont_eco2_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_cont_eco2_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_cont_eco2_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_cont_eco2_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_cont_eco2_card, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_eco2_title
    ui->mainScreen_label_eco2_title = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_eco2_title, "eCO2 ppm");
    lv_label_set_long_mode(ui->mainScreen_label_eco2_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_eco2_title, 189, 93);
    lv_obj_set_size(ui->mainScreen_label_eco2_title, 100, 32);

    //Write style for mainScreen_label_eco2_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_eco2_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_eco2_title, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_eco2_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_eco2_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_Prompt
    ui->mainScreen_label_Prompt = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_Prompt, "提示：当前环境舒适");
    lv_label_set_long_mode(ui->mainScreen_label_Prompt, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_Prompt, 75, 128);
    lv_obj_set_size(ui->mainScreen_label_Prompt, 136, 97);

    //Write style for mainScreen_label_Prompt, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_Prompt, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_Prompt, &lv_font_yezi_hei_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_Prompt, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_Prompt, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_Prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_img_role
    ui->mainScreen_img_role = lv_img_create(ui->mainScreen);
    lv_obj_add_flag(ui->mainScreen_img_role, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->mainScreen_img_role, &_img_role_normal_alpha_60x60);
    lv_img_set_pivot(ui->mainScreen_img_role, 50,50);
    lv_img_set_angle(ui->mainScreen_img_role, 0);
    lv_obj_set_pos(ui->mainScreen_img_role, 16, 121);
    lv_obj_set_size(ui->mainScreen_img_role, 60, 60);

    //Write style for mainScreen_img_role, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->mainScreen_img_role, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->mainScreen_img_role, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_img_role, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->mainScreen_img_role, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_label_aqi_val
    ui->mainScreen_label_aqi_val = lv_label_create(ui->mainScreen);
    lv_label_set_text(ui->mainScreen_label_aqi_val, "1");
    lv_label_set_long_mode(ui->mainScreen_label_aqi_val, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->mainScreen_label_aqi_val, 16, 66);
    lv_obj_set_size(ui->mainScreen_label_aqi_val, 129, 22);

    //Write style for mainScreen_label_aqi_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->mainScreen_label_aqi_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->mainScreen_label_aqi_val, &lv_font_yezi_hei_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->mainScreen_label_aqi_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->mainScreen_label_aqi_val, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_aqi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes mainScreen_img_1
    ui->mainScreen_img_1 = lv_img_create(ui->mainScreen);
    lv_obj_add_flag(ui->mainScreen_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->mainScreen_img_1, &_img_boot_logo_alpha_60x60);
    lv_img_set_pivot(ui->mainScreen_img_1, 50,50);
    lv_img_set_angle(ui->mainScreen_img_1, 0);
    lv_obj_set_pos(ui->mainScreen_img_1, 254, 181);
    lv_obj_set_size(ui->mainScreen_img_1, 60, 60);

    //Write style for mainScreen_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->mainScreen_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->mainScreen_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->mainScreen_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->mainScreen_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of mainScreen.


    //Update current screen layout.
    lv_obj_update_layout(ui->mainScreen);

    //Init events for screen.
    events_init_mainScreen(ui);
}
