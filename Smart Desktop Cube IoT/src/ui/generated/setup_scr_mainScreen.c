/*
 * Smart Desktop Cube - mainScreen v2
 * 单页显示所有传感器数据 + 小眠角色
 * 320x240 ILI9341 TFT
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "ui/custom/custom.h"

/* ---- 辅助：创建半透明容器 ---- */
static lv_obj_t* mk_cont(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                          lv_coord_t w, lv_coord_t h, lv_color_t bg, lv_opa_t opa,
                          lv_coord_t radius)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_pos(c, x, y);
    lv_obj_set_size(c, w, h);
    lv_obj_set_scrollbar_mode(c, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(c, bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(c, opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(c, radius, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(c, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(c, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(c, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return c;
}

/* ---- 辅助：创建标签 ---- */
static lv_obj_t* mk_label(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                           lv_coord_t w, lv_coord_t h,
                           const char *text, const lv_font_t *font,
                           lv_color_t color, lv_text_align_t align)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_size(l, w, h);
    lv_obj_set_style_text_font(l, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(l, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(l, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(l, align, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(l, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(l, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(l, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(l, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return l;
}

/* ---- 辅助：创建状态小圆点 ---- */
static lv_obj_t* mk_dot(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_color_t color)
{
    lv_obj_t *d = lv_obj_create(parent);
    lv_obj_set_pos(d, x, y);
    lv_obj_set_size(d, 6, 6);
    lv_obj_set_style_bg_color(d, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(d, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(d, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(d, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(d, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(d, LV_OBJ_FLAG_SCROLLABLE);
    return d;
}

void setup_scr_mainScreen(lv_ui *ui)
{
    /* ==================== 屏幕基础 ==================== */
    ui->mainScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui->mainScreen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->mainScreen, LV_SCROLLBAR_MODE_OFF);

    /* 背景图片 */
    lv_obj_set_style_bg_opa(ui->mainScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->mainScreen, &_img_bg_main_320x240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->mainScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 深色遮罩 */
    lv_obj_t *overlay = lv_obj_create(ui->mainScreen);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_size(overlay, 320, 240);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x0C1024), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(overlay, 210, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    /* ==================== 顶部状态栏 (y=0-22) ==================== */
    ui->mainScreen_bar_top = mk_cont(ui->mainScreen, 0, 0, 320, 22,
                                      lv_color_hex(0x000000), 77, 0);

    ui->mainScreen_label_clock = mk_label(ui->mainScreen, 8, 3, 50, 16,
                                           "14:32", &lv_font_montserratMedium_18,
                                           lv_color_hex(0xC0D8FF), LV_TEXT_ALIGN_LEFT);
    lv_obj_add_flag(ui->mainScreen_label_clock, LV_OBJ_FLAG_HIDDEN);  /* 用户不需要时间显示 */

    /* WiFi 状态点（初始红色，连接后变绿） */
    ui->mainScreen_dot_wifi = mk_dot(ui->mainScreen, 240, 8, lv_color_hex(0xF43F5E));
    mk_label(ui->mainScreen, 248, 5, 22, 12, "WiFi", &lv_font_yezi_hei_12,
             lv_color_hex(0x7788AA), LV_TEXT_ALIGN_LEFT);

    /* MQTT 状态点（初始红色） */
    ui->mainScreen_dot_mqtt = mk_dot(ui->mainScreen, 274, 8, lv_color_hex(0xF43F5E));
    mk_label(ui->mainScreen, 282, 5, 26, 12, "MQTT", &lv_font_yezi_hei_12,
             lv_color_hex(0x7788AA), LV_TEXT_ALIGN_LEFT);

    /* 专注指示灯 */
    ui->mainScreen_dot_focus = mk_dot(ui->mainScreen, 308, 8, lv_color_hex(0xA78BFA));
    lv_obj_set_style_bg_opa(ui->mainScreen_dot_focus, 77, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* ==================== 左侧角色列 (x=4-82) ==================== */
    ui->mainScreen_img_role = lv_img_create(ui->mainScreen);
    lv_img_set_src(ui->mainScreen_img_role, &_img_role_normal_alpha_60x60);
    lv_obj_set_pos(ui->mainScreen_img_role, 10, 34);
    lv_obj_set_size(ui->mainScreen_img_role, 60, 60);
    lv_obj_set_style_img_opa(ui->mainScreen_img_role, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->mainScreen_img_role, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 心情标签 */
    lv_obj_t *mood_bg = mk_cont(ui->mainScreen, 6, 100, 70, 14,
                                 lv_color_hex(0x000000), 77, 7);
    ui->mainScreen_label_mood = mk_label(mood_bg, 3, 0, 64, 14, "OK",
                                          &lv_font_yezi_hei_12, lv_color_hex(0x8899BB), LV_TEXT_ALIGN_CENTER);

    /* 角色名 */
    mk_label(ui->mainScreen, 6, 118, 70, 10, "XiaoMian",
             &lv_font_yezi_hei_12, lv_color_hex(0x556688), LV_TEXT_ALIGN_CENTER);

    /* ==================== 右侧数据列 (x=86-316, w=230) ==================== */

    /* --- AQI 卡片：AQI数值 + 颜色圈圈 (y=26-72) --- */
    ui->mainScreen_cont_aqi_hero = mk_cont(ui->mainScreen, 86, 26, 230, 46,
                                            lv_color_hex(0x0F1940), 179, 6);

    ui->mainScreen_label_aqi_big = mk_label(ui->mainScreen, 96, 30, 120, 38,
                                             "AQI 42", &lv_font_yezi_hei_18,
                                             lv_color_hex(0x4ADE80), LV_TEXT_ALIGN_LEFT);

    /* 颜色圈圈：根据AQI值变色（绿→黄→橙→红） */
    ui->mainScreen_label_aqi_level = lv_obj_create(ui->mainScreen);
    lv_obj_set_pos(ui->mainScreen_label_aqi_level, 230, 35);
    lv_obj_set_size(ui->mainScreen_label_aqi_level, 28, 28);
    lv_obj_set_style_radius(ui->mainScreen_label_aqi_level, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->mainScreen_label_aqi_level, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->mainScreen_label_aqi_level, lv_color_hex(0x4ADE80), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->mainScreen_label_aqi_level, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->mainScreen_label_aqi_level, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(ui->mainScreen_label_aqi_level, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->mainScreen_label_aqi_level, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui->mainScreen_label_aqi_level, LV_OBJ_FLAG_SCROLLABLE);

    ui->mainScreen_label_aqi_detail = NULL;  /* 不再需要 */

    /* --- 温度/湿度卡片 (y=76-98) --- */
    ui->mainScreen_cont_temp = mk_cont(ui->mainScreen, 86, 76, 113, 22,
                                        lv_color_hex(0x0F1940), 128, 4);
    ui->mainScreen_label_temp_val = mk_label(ui->mainScreen_cont_temp, 6, 2, 100, 18,
                                              "25.6 C", &lv_font_yezi_hei_15,
                                              lv_color_hex(0xFB923C), LV_TEXT_ALIGN_LEFT);

    ui->mainScreen_cont_humi = mk_cont(ui->mainScreen, 203, 76, 113, 22,
                                        lv_color_hex(0x0F1940), 128, 4);
    ui->mainScreen_label_humi_val = mk_label(ui->mainScreen_cont_humi, 6, 2, 100, 18,
                                              "58 %", &lv_font_yezi_hei_15,
                                              lv_color_hex(0x60A5FA), LV_TEXT_ALIGN_LEFT);

    /* --- 传感器网格 第1行 (y=102-150): Light / CO2 / TVOC --- */
    ui->mainScreen_cell_light = mk_cont(ui->mainScreen, 86, 102, 74, 48,
                                         lv_color_hex(0x0F1940), 115, 4);
    mk_label(ui->mainScreen_cell_light, 4, 2, 66, 10, "Light",
             &lv_font_yezi_hei_12, lv_color_hex(0x556688), LV_TEXT_ALIGN_LEFT);
    ui->mainScreen_label_light_val = mk_label(ui->mainScreen_cell_light, 4, 16, 66, 26,
                                               "320 lx", &lv_font_yezi_hei_15,
                                               lv_color_hex(0xFBBF24), LV_TEXT_ALIGN_LEFT);

    ui->mainScreen_cell_co2 = mk_cont(ui->mainScreen, 164, 102, 74, 48,
                                       lv_color_hex(0x0F1940), 115, 4);
    mk_label(ui->mainScreen_cell_co2, 4, 2, 66, 10, "CO2",
             &lv_font_yezi_hei_12, lv_color_hex(0x556688), LV_TEXT_ALIGN_LEFT);
    ui->mainScreen_label_co2_val = mk_label(ui->mainScreen_cell_co2, 4, 16, 66, 26,
                                             "480 ppm", &lv_font_yezi_hei_15,
                                             lv_color_hex(0xA78BFA), LV_TEXT_ALIGN_LEFT);

    ui->mainScreen_cell_tvoc = mk_cont(ui->mainScreen, 242, 102, 74, 48,
                                        lv_color_hex(0x0F1940), 115, 4);
    mk_label(ui->mainScreen_cell_tvoc, 4, 2, 66, 10, "TVOC",
             &lv_font_yezi_hei_12, lv_color_hex(0x556688), LV_TEXT_ALIGN_LEFT);
    ui->mainScreen_label_tvoc_val = mk_label(ui->mainScreen_cell_tvoc, 4, 16, 66, 26,
                                              "0.12", &lv_font_yezi_hei_15,
                                              lv_color_hex(0xF472B6), LV_TEXT_ALIGN_LEFT);

    /* --- 传感器网格 第2行 (y=154-202): PM2.5 / Human --- */
    ui->mainScreen_cell_pm25 = mk_cont(ui->mainScreen, 86, 154, 113, 48,
                                        lv_color_hex(0x0F1940), 115, 4);
    mk_label(ui->mainScreen_cell_pm25, 4, 2, 105, 10, "PM2.5",
             &lv_font_yezi_hei_12, lv_color_hex(0x556688), LV_TEXT_ALIGN_LEFT);
    ui->mainScreen_label_pm25_val = mk_label(ui->mainScreen_cell_pm25, 4, 16, 105, 26,
                                              "18 ug/m3", &lv_font_yezi_hei_15,
                                              lv_color_hex(0x67E8F9), LV_TEXT_ALIGN_LEFT);

    ui->mainScreen_cell_human = mk_cont(ui->mainScreen, 203, 154, 113, 48,
                                         lv_color_hex(0x0F1940), 115, 4);
    mk_label(ui->mainScreen_cell_human, 4, 2, 105, 10, "Human",
             &lv_font_yezi_hei_12, lv_color_hex(0x556688), LV_TEXT_ALIGN_LEFT);
    ui->mainScreen_label_human_val = mk_label(ui->mainScreen_cell_human, 4, 16, 105, 26,
                                               "Yes", &lv_font_yezi_hei_15,
                                               lv_color_hex(0x4ADE80), LV_TEXT_ALIGN_LEFT);

    /* signal 格已移除 */
    ui->mainScreen_cell_signal      = NULL;
    ui->mainScreen_label_signal_val = NULL;

    /* ==================== 底部栏 (y=218-240) ==================== */
    ui->mainScreen_bar_bottom = mk_cont(ui->mainScreen, 0, 218, 320, 22,
                                         lv_color_hex(0x000000), 77, 0);

    ui->mainScreen_label_fw = mk_label(ui->mainScreen, 8, 222, 50, 14, "v1.0.0",
                                        &lv_font_yezi_hei_12, lv_color_hex(0x445566),
                                        LV_TEXT_ALIGN_LEFT);

    ui->mainScreen_label_auto_brightness = mk_label(ui->mainScreen, 200, 222, 116, 14,
                                                     "Auto Bright ON", &lv_font_yezi_hei_12,
                                                     lv_color_hex(0x667788), LV_TEXT_ALIGN_LEFT);

    /* 右下角 logo 已移除 */
    ui->mainScreen_img_boot_logo = NULL;

    /* WiFi 图标已移除（控制移至 Web 界面） */
    ui->mainScreen_img_wifi = NULL;

    /* ==================== 旧 widget 置 NULL ==================== */
    ui->mainScreen_label_time       = NULL;
    ui->mainScreen_label_summary    = NULL;
    ui->mainScreen_cont_aqi_card    = NULL;
    ui->mainScreen_label_aqi_title  = NULL;
    ui->mainScreen_label_eco2_val   = NULL;
    ui->mainScreen_cont_eco2_card   = NULL;
    ui->mainScreen_label_eco2_title = NULL;
    ui->mainScreen_label_Prompt     = NULL;
    ui->mainScreen_label_aqi_val    = NULL;
    ui->mainScreen_label_char_name  = NULL;

    lv_obj_update_layout(ui->mainScreen);
}
