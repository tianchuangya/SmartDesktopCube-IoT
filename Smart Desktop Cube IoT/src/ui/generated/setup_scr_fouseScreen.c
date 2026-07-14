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



void setup_scr_fouseScreen(lv_ui *ui)
{
    //Write codes fouseScreen
    ui->fouseScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->fouseScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->fouseScreen, LV_SCROLLBAR_MODE_OFF);

    //Write style for fouseScreen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->fouseScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->fouseScreen, &_img_bg_focus_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->fouseScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->fouseScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes fouseScreen_btn_1
    ui->fouseScreen_btn_1 = lv_btn_create(ui->fouseScreen);
    ui->fouseScreen_btn_1_label = lv_label_create(ui->fouseScreen_btn_1);
    lv_label_set_text(ui->fouseScreen_btn_1_label, "返回");
    lv_label_set_long_mode(ui->fouseScreen_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->fouseScreen_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->fouseScreen_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->fouseScreen_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->fouseScreen_btn_1, 10, 11);
    lv_obj_set_size(ui->fouseScreen_btn_1, 65, 26);

    //Write style for fouseScreen_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->fouseScreen_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->fouseScreen_btn_1, lv_color_hex(0xb7d1e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->fouseScreen_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->fouseScreen_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->fouseScreen_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->fouseScreen_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->fouseScreen_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->fouseScreen_btn_1, &lv_font_yezi_hei_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->fouseScreen_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->fouseScreen_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes fouseScreen_label_focus_timer
    ui->fouseScreen_label_focus_timer = lv_label_create(ui->fouseScreen);
    lv_label_set_text(ui->fouseScreen_label_focus_timer, "00:00");
    lv_label_set_long_mode(ui->fouseScreen_label_focus_timer, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->fouseScreen_label_focus_timer, 93, 87);
    lv_obj_set_size(ui->fouseScreen_label_focus_timer, 124, 42);

    //Write style for fouseScreen_label_focus_timer, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->fouseScreen_label_focus_timer, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->fouseScreen_label_focus_timer, &lv_font_montserratMedium_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->fouseScreen_label_focus_timer, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->fouseScreen_label_focus_timer, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->fouseScreen_label_focus_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes fouseScreen_label_light_info
    ui->fouseScreen_label_light_info = lv_label_create(ui->fouseScreen);
    lv_label_set_text(ui->fouseScreen_label_light_info, "台灯色温: 3000K 暖光");
    lv_label_set_long_mode(ui->fouseScreen_label_light_info, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->fouseScreen_label_light_info, 10, 138);
    lv_obj_set_size(ui->fouseScreen_label_light_info, 173, 22);

    //Write style for fouseScreen_label_light_info, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->fouseScreen_label_light_info, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->fouseScreen_label_light_info, &lv_font_yezi_hei_14, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->fouseScreen_label_light_info, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->fouseScreen_label_light_info, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->fouseScreen_label_light_info, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes fouseScreen_label_status
    ui->fouseScreen_label_status = lv_label_create(ui->fouseScreen);
    lv_label_set_text(ui->fouseScreen_label_status, "安静陪伴中...");
    lv_label_set_long_mode(ui->fouseScreen_label_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->fouseScreen_label_status, 83, 164);
    lv_obj_set_size(ui->fouseScreen_label_status, 100, 16);

    //Write style for fouseScreen_label_status, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->fouseScreen_label_status, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->fouseScreen_label_status, &lv_font_yezi_hei_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->fouseScreen_label_status, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->fouseScreen_label_status, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->fouseScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes fouseScreen_img_role_focus
    ui->fouseScreen_img_role_focus = lv_img_create(ui->fouseScreen);
    lv_obj_add_flag(ui->fouseScreen_img_role_focus, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->fouseScreen_img_role_focus, &_img_role_focus_alpha_60x60);
    lv_img_set_pivot(ui->fouseScreen_img_role_focus, 50,50);
    lv_img_set_angle(ui->fouseScreen_img_role_focus, 0);
    lv_obj_set_pos(ui->fouseScreen_img_role_focus, 10, 160);
    lv_obj_set_size(ui->fouseScreen_img_role_focus, 60, 60);

    //Write style for fouseScreen_img_role_focus, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->fouseScreen_img_role_focus, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->fouseScreen_img_role_focus, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->fouseScreen_img_role_focus, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->fouseScreen_img_role_focus, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	    //Write codes fouseScreen_label_1
	    ui->fouseScreen_label_1 = lv_label_create(ui->fouseScreen);
	    lv_label_set_text(ui->fouseScreen_label_1, "专注模式");
	    lv_label_set_long_mode(ui->fouseScreen_label_1, LV_LABEL_LONG_WRAP);
	    lv_obj_set_pos(ui->fouseScreen_label_1, 226, 7);
	    lv_obj_set_size(ui->fouseScreen_label_1, 100, 32);

	    //Write style for fouseScreen_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	    lv_obj_set_style_border_width(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_radius(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_text_color(ui->fouseScreen_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_text_font(ui->fouseScreen_label_1, &lv_font_yezi_hei_20, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_text_opa(ui->fouseScreen_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_text_letter_space(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_text_line_space(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_text_align(ui->fouseScreen_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_bg_opa(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_pad_top(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_pad_right(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_pad_bottom(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_pad_left(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	    lv_obj_set_style_shadow_width(ui->fouseScreen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of fouseScreen.


    //Update current screen layout.
    lv_obj_update_layout(ui->fouseScreen);

    // events_init_fouseScreen 已在 events_init() 统一调用，此处删除避免双重注册
}
