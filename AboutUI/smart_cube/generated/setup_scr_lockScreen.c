/* GUI-Guider setup for lockScreen */
#include "lvgl.h"
#include "gui_guider.h"

void setup_scr_lockScreen(lv_ui *ui)
{
    ui->lockScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->lockScreen, 320, 240);
    lv_obj_set_style_bg_color(ui->lockScreen, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->lockScreen, 229, LV_PART_MAIN|LV_STATE_DEFAULT);

    ui->lockScreen_img_role = lv_img_create(ui->lockScreen);
    lv_obj_set_pos(ui->lockScreen_img_role, 135, 60);
    lv_obj_set_size(ui->lockScreen_img_role, 50, 50);
    LV_IMG_DECLARE(_img_role_alert_alpha_50x50);
    lv_img_set_src(ui->lockScreen_img_role, &_img_role_alert_alpha_50x50);
    lv_img_set_pivot(ui->lockScreen_img_role, 50, 50);
    lv_obj_set_style_img_recolor(ui->lockScreen_img_role, lv_color_hex(0xf44336), LV_PART_MAIN|LV_STATE_DEFAULT);
    ui->lockScreen_label_title = lv_label_create(ui->lockScreen);
    lv_obj_set_pos(ui->lockScreen_label_title, 80, 126);
    lv_obj_set_size(ui->lockScreen_label_title, 160, 26);
    lv_label_set_text(ui->lockScreen_label_title, "设备已锁定");
    lv_label_set_long_mode(ui->lockScreen_label_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_bg_color(ui->lockScreen_label_title, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->lockScreen_label_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->lockScreen_label_title, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->lockScreen_label_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->lockScreen_label_title, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->lockScreen_label_title, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->lockScreen_label_title, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    ui->lockScreen_btn_unlock = lv_btn_create(ui->lockScreen);
    lv_obj_set_pos(ui->lockScreen_btn_unlock, 87, 166);
    lv_obj_set_size(ui->lockScreen_btn_unlock, 150, 34);
    ui->lockScreen_btn_unlock_label = lv_label_create(ui->lockScreen_btn_unlock);
    lv_label_set_long_mode(ui->lockScreen_btn_unlock_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(ui->lockScreen_btn_unlock_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_center(ui->lockScreen_btn_unlock_label);
    lv_obj_set_style_bg_color(ui->lockScreen_btn_unlock, lv_color_hex(0x5c6bc0), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->lockScreen_btn_unlock, 216, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->lockScreen_btn_unlock, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->lockScreen_btn_unlock, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->lockScreen_btn_unlock, &lv_font_yezi_hei_10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->lockScreen_btn_unlock, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->lockScreen_btn_unlock, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->lockScreen_btn_unlock, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
}