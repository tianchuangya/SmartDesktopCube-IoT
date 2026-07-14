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



void setup_scr_aircreen(lv_ui *ui)
{
    //Write codes aircreen
    ui->aircreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->aircreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->aircreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for aircreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->aircreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->aircreen, &_img_bg_main_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->aircreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->aircreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_label_eco2_val
    ui->aircreen_label_eco2_val = lv_label_create(ui->aircreen);
    lv_label_set_text(ui->aircreen_label_eco2_val, "450");
    lv_label_set_long_mode(ui->aircreen_label_eco2_val, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->aircreen_label_eco2_val, 162, 66);
    lv_obj_set_size(ui->aircreen_label_eco2_val, 138, 30);

    //Write style for aircreen_label_eco2_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->aircreen_label_eco2_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->aircreen_label_eco2_val, &lv_font_yezi_hei_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->aircreen_label_eco2_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->aircreen_label_eco2_val, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_label_eco2_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_label_eco2_title
    ui->aircreen_label_eco2_title = lv_label_create(ui->aircreen);
    lv_label_set_text(ui->aircreen_label_eco2_title, "eCO2 ppm");
    lv_label_set_long_mode(ui->aircreen_label_eco2_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->aircreen_label_eco2_title, 189, 93);
    lv_obj_set_size(ui->aircreen_label_eco2_title, 100, 32);

    //Write style for aircreen_label_eco2_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->aircreen_label_eco2_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->aircreen_label_eco2_title, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->aircreen_label_eco2_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->aircreen_label_eco2_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_label_eco2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_cont_2
    ui->aircreen_cont_2 = lv_obj_create(ui->aircreen);
    lv_obj_set_pos(ui->aircreen_cont_2, 16, 58);
    lv_obj_set_size(ui->aircreen_cont_2, 138, 55);
    lv_obj_set_scrollbar_mode(ui->aircreen_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for aircreen_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_cont_2, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_cont_2, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->aircreen_cont_2, lv_color_hex(0x00eeff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->aircreen_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_cont_1
    ui->aircreen_cont_1 = lv_obj_create(ui->aircreen);
    lv_obj_set_pos(ui->aircreen_cont_1, 166, 58);
    lv_obj_set_size(ui->aircreen_cont_1, 138, 55);
    lv_obj_set_scrollbar_mode(ui->aircreen_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for aircreen_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_cont_1, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_cont_1, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->aircreen_cont_1, lv_color_hex(0x00eeff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->aircreen_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_label_tvoc_title
    ui->aircreen_label_tvoc_title = lv_label_create(ui->aircreen);
    lv_label_set_text(ui->aircreen_label_tvoc_title, "TVOC ppb");
    lv_label_set_long_mode(ui->aircreen_label_tvoc_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->aircreen_label_tvoc_title, 32, 89);
    lv_obj_set_size(ui->aircreen_label_tvoc_title, 100, 32);

    //Write style for aircreen_label_tvoc_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->aircreen_label_tvoc_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->aircreen_label_tvoc_title, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->aircreen_label_tvoc_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->aircreen_label_tvoc_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_label_tvoc_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_label_tvoc_val
    ui->aircreen_label_tvoc_val = lv_label_create(ui->aircreen);
    lv_label_set_text(ui->aircreen_label_tvoc_val, "120");
    lv_label_set_long_mode(ui->aircreen_label_tvoc_val, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->aircreen_label_tvoc_val, 16, 66);
    lv_obj_set_size(ui->aircreen_label_tvoc_val, 129, 22);

    //Write style for aircreen_label_tvoc_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->aircreen_label_tvoc_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->aircreen_label_tvoc_val, &lv_font_yezi_hei_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->aircreen_label_tvoc_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->aircreen_label_tvoc_val, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_label_tvoc_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_label_title
    ui->aircreen_label_title = lv_label_create(ui->aircreen);
    lv_label_set_text(ui->aircreen_label_title, "空气质量详情");
    lv_label_set_long_mode(ui->aircreen_label_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->aircreen_label_title, 217, 8);
    lv_obj_set_size(ui->aircreen_label_title, 100, 32);

    //Write style for aircreen_label_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->aircreen_label_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->aircreen_label_title, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->aircreen_label_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->aircreen_label_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_img_role2
    ui->aircreen_img_role2 = lv_img_create(ui->aircreen);
    lv_obj_add_flag(ui->aircreen_img_role2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->aircreen_img_role2, &_img_role_normal_alpha_60x60);
    lv_img_set_pivot(ui->aircreen_img_role2, 50,50);
    lv_img_set_angle(ui->aircreen_img_role2, 0);
    lv_obj_set_pos(ui->aircreen_img_role2, 15, 121);
    lv_obj_set_size(ui->aircreen_img_role2, 60, 60);

    //Write style for aircreen_img_role2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->aircreen_img_role2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->aircreen_img_role2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_img_role2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->aircreen_img_role2, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes aircreen_label_ai_tip
    ui->aircreen_label_ai_tip = lv_label_create(ui->aircreen);
    lv_label_set_text(ui->aircreen_label_ai_tip, "小慕建议：多开窗通风");
    lv_label_set_long_mode(ui->aircreen_label_ai_tip, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->aircreen_label_ai_tip, 82, 125);
    lv_obj_set_size(ui->aircreen_label_ai_tip, 216, 102);

    //Write style for aircreen_label_ai_tip, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->aircreen_label_ai_tip, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->aircreen_label_ai_tip, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->aircreen_label_ai_tip, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->aircreen_label_ai_tip, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->aircreen_label_ai_tip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of aircreen.


    //Update current screen layout.
    lv_obj_update_layout(ui->aircreen);

    // events_init_aircreen 已在 events_init() 统一调用，此处删除避免双重注册
}
