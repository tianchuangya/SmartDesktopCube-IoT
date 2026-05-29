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



void setup_scr_lockScreen(lv_ui *ui)
{
    //Write codes lockScreen
    ui->lockScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->lockScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->lockScreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for lockScreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->lockScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->lockScreen, &_img_bg_report_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->lockScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->lockScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes lockScreen_img_1
    ui->lockScreen_img_1 = lv_img_create(ui->lockScreen);
    lv_obj_add_flag(ui->lockScreen_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->lockScreen_img_1, &_img_role_alert_alpha_100x100);
    lv_img_set_pivot(ui->lockScreen_img_1, 50,50);
    lv_img_set_angle(ui->lockScreen_img_1, 0);
    lv_obj_set_pos(ui->lockScreen_img_1, 110, 70);
    lv_obj_set_size(ui->lockScreen_img_1, 100, 100);

    //Write style for lockScreen_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->lockScreen_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->lockScreen_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->lockScreen_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->lockScreen_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes lockScreen_label_title
    ui->lockScreen_label_title = lv_label_create(ui->lockScreen);
    lv_label_set_text(ui->lockScreen_label_title, "设备已锁定");
    lv_label_set_long_mode(ui->lockScreen_label_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->lockScreen_label_title, 110, 185);
    lv_obj_set_size(ui->lockScreen_label_title, 100, 32);

    //Write style for lockScreen_label_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->lockScreen_label_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->lockScreen_label_title, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->lockScreen_label_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->lockScreen_label_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of lockScreen.


    //Update current screen layout.
    lv_obj_update_layout(ui->lockScreen);

}
