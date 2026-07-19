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



void setup_scr_bootScreen(lv_ui *ui)
{
    //Write codes bootScreen
    ui->bootScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->bootScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->bootScreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for bootScreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_color(ui->bootScreen, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->bootScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes bootScreen_img_boot_sleep
    ui->bootScreen_img_boot_sleep = lv_img_create(ui->bootScreen);
    lv_obj_add_flag(ui->bootScreen_img_boot_sleep, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->bootScreen_img_boot_sleep, &_img_boot_sleep_22_alpha_168x155);
    lv_img_set_pivot(ui->bootScreen_img_boot_sleep, 50,50);
    lv_img_set_angle(ui->bootScreen_img_boot_sleep, 0);
    lv_obj_set_pos(ui->bootScreen_img_boot_sleep, 72, 22);
    lv_obj_set_size(ui->bootScreen_img_boot_sleep, 168, 155);

    //Write style for bootScreen_img_boot_sleep, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->bootScreen_img_boot_sleep, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->bootScreen_img_boot_sleep, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->bootScreen_img_boot_sleep, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->bootScreen_img_boot_sleep, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes bootScreen_label_status
    ui->bootScreen_label_status = lv_label_create(ui->bootScreen);
    lv_label_set_text(ui->bootScreen_label_status, "启动中...");
    lv_label_set_long_mode(ui->bootScreen_label_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->bootScreen_label_status, 107, 183);
    lv_obj_set_size(ui->bootScreen_label_status, 100, 32);

    //Write style for bootScreen_label_status, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->bootScreen_label_status, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->bootScreen_label_status, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->bootScreen_label_status, 174, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->bootScreen_label_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of bootScreen.


    //Update current screen layout.
    lv_obj_update_layout(ui->bootScreen);

}
