/* GUI-Guider setup for bootScreen */
#include "lvgl.h"
#include "gui_guider.h"

void setup_scr_bootScreen(lv_ui *ui)
{
    ui->bootScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->bootScreen, 320, 240);
    lv_obj_set_style_bg_color(ui->bootScreen, lv_color_hex(0x0d0d1a), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->bootScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    LV_IMG_DECLARE(_img_bg_main_320x240);
    lv_obj_set_style_bg_img_src(ui->bootScreen, &_img_bg_main_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->bootScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    ui->bootScreen_img_boot_sleep = lv_img_create(ui->bootScreen);
    lv_obj_set_pos(ui->bootScreen_img_boot_sleep, 120, 89);
    lv_obj_set_size(ui->bootScreen_img_boot_sleep, 80, 60);
    LV_IMG_DECLARE(_img_boot_sleep_10_alpha_80x60);
    lv_img_set_src(ui->bootScreen_img_boot_sleep, &_img_boot_sleep_10_alpha_80x60);
    lv_img_set_pivot(ui->bootScreen_img_boot_sleep, 50, 50);
    lv_obj_set_style_img_recolor(ui->bootScreen_img_boot_sleep, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    ui->bootScreen_label_status = lv_label_create(ui->bootScreen);
    lv_obj_set_pos(ui->bootScreen_label_status, 120, 166);
    lv_obj_set_size(ui->bootScreen_label_status, 80, 20);
    lv_label_set_text(ui->bootScreen_label_status, "启动中...");
    lv_label_set_long_mode(ui->bootScreen_label_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_bg_color(ui->bootScreen_label_status, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->bootScreen_label_status, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->bootScreen_label_status, lv_color_hex(0xccccdd), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->bootScreen_label_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->bootScreen_label_status, &lv_font_yezi_hei_10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->bootScreen_label_status, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->bootScreen_label_status, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->bootScreen_label_status, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
}