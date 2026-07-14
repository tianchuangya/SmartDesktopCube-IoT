#include "OTAScreen.h"
#include "OTAManager.h"
#include "ui/generated/gui_guider.h"
#include <stdio.h>

// ==================== OTA 动态 UI 控件 ====================
static lv_obj_t* ota_overlay   = NULL;  // 半透明覆盖层
static lv_obj_t* ota_bar       = NULL;  // 进度条
static lv_obj_t* ota_label_ver = NULL;  // 版本信息
static lv_obj_t* ota_label_warn = NULL; // 警告文字
static lv_obj_t* ota_label_prog = NULL; // 进度百分比
static lv_obj_t* ota_label_stat = NULL; // 状态文字

// ==================== 缓存值（避免无意义重绘）====================
static int  last_percent = -1;
static char last_status[64] = "";

// ==================== 显示 OTA 更新屏幕 ====================
void otaScreenShow(const char* currentVer, const char* newVer) {
    lv_ui* ui = &guider_ui;

    // 如果 lockScreen 还没创建，先创建
    if (!ui->lockScreen) {
        setup_scr_lockScreen(ui);
    }

    // 切换到 lockScreen
    lv_scr_load_anim(ui->lockScreen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);

    // 修改锁定文字为 OTA 提示
    lv_label_set_text(ui->lockScreen_label_title, "");

    // ---- 创建半透明覆盖层 ----
    ota_overlay = lv_obj_create(ui->lockScreen);
    lv_obj_set_size(ota_overlay, 280, 200);
    lv_obj_set_pos(ota_overlay, 20, 20);
    lv_obj_set_style_bg_color(ota_overlay, lv_color_hex(0x1A1A2E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ota_overlay, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ota_overlay, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ota_overlay, lv_color_hex(0x0F3460), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ota_overlay, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ota_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // ---- 版本信息 ----
    ota_label_ver = lv_label_create(ota_overlay);
    char ver_text[64];
    snprintf(ver_text, sizeof(ver_text), "固件更新\n当前: %s  →  新版本: %s", currentVer, newVer);
    lv_label_set_text(ota_label_ver, ver_text);
    lv_obj_set_pos(ota_label_ver, 10, 10);
    lv_obj_set_size(ota_label_ver, 260, 40);
    lv_obj_set_style_text_color(ota_label_ver, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ota_label_ver, &lv_font_yezi_hei_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ota_label_ver, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ---- 进度条 ----
    ota_bar = lv_bar_create(ota_overlay);
    lv_obj_set_size(ota_bar, 240, 20);
    lv_obj_set_pos(ota_bar, 20, 75);
    lv_obj_set_style_bg_color(ota_bar, lv_color_hex(0x333355), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ota_bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ota_bar, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 指示器颜色：蓝色到绿色渐变感
    lv_obj_set_style_bg_color(ota_bar, lv_color_hex(0x00D2FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ota_bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ota_bar, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_bar_set_range(ota_bar, 0, 100);
    lv_bar_set_value(ota_bar, 0, LV_ANIM_OFF);

    // ---- 进度百分比 ----
    ota_label_prog = lv_label_create(ota_overlay);
    lv_label_set_text(ota_label_prog, "0%");
    lv_obj_set_pos(ota_label_prog, 10, 100);
    lv_obj_set_size(ota_label_prog, 260, 30);
    lv_obj_set_style_text_color(ota_label_prog, lv_color_hex(0x00D2FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ota_label_prog, &lv_font_montserratMedium_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ota_label_prog, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ---- 状态文字 ----
    ota_label_stat = lv_label_create(ota_overlay);
    lv_label_set_text(ota_label_stat, "正在连接服务器...");
    lv_obj_set_pos(ota_label_stat, 10, 130);
    lv_obj_set_size(ota_label_stat, 260, 24);
    lv_obj_set_style_text_color(ota_label_stat, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ota_label_stat, &lv_font_yezi_hei_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ota_label_stat, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ---- 警告文字 ----
    ota_label_warn = lv_label_create(ota_overlay);
    lv_label_set_text(ota_label_warn, "⚠ 设备正在更新中\n请勿断电或关机！");
    lv_obj_set_pos(ota_label_warn, 10, 160);
    lv_obj_set_size(ota_label_warn, 260, 36);
    lv_obj_set_style_text_color(ota_label_warn, lv_color_hex(0xFF6B6B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ota_label_warn, &lv_font_yezi_hei_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ota_label_warn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 更新布局，LVGL 任务会在下一个周期自动刷新
    lv_obj_update_layout(ui->lockScreen);

    last_percent = -1;
    last_status[0] = '\0';
}

// ==================== 更新进度条 ====================
void otaScreenUpdateProgress(int percent, const char* status) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    // 只在值变化时更新
    if (percent == last_percent && status && strcmp(status, last_status) == 0) {
        return;
    }
    last_percent = percent;
    if (status) {
        strncpy(last_status, status, sizeof(last_status) - 1);
        last_status[sizeof(last_status) - 1] = '\0';
    }

    if (ota_bar) {
        lv_bar_set_value(ota_bar, percent, LV_ANIM_ON);
    }
    if (ota_label_prog) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", percent);
        lv_label_set_text(ota_label_prog, buf);
    }
    if (ota_label_stat && status) {
        lv_label_set_text(ota_label_stat, status);
    }
}

// ==================== 显示错误 ====================
void otaScreenSetError(const char* error) {
    if (ota_label_stat) {
        lv_label_set_text(ota_label_stat, error);
        lv_obj_set_style_text_color(ota_label_stat, lv_color_hex(0xFF4444), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (ota_label_warn) {
        lv_label_set_text(ota_label_warn, "❌ 更新失败\n请检查网络后重试");
    }
    if (ota_bar) {
        lv_obj_set_style_bg_color(ota_bar, lv_color_hex(0xFF4444), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
}

// ==================== 隐藏 OTA 屏幕 ====================
void otaScreenHide() {
    // 删除动态创建的控件
    if (ota_overlay) {
        lv_obj_del(ota_overlay);
        ota_overlay = NULL;
    }
    ota_bar    = NULL;
    ota_label_ver  = NULL;
    ota_label_warn = NULL;
    ota_label_prog = NULL;
    ota_label_stat = NULL;

    // 恢复 lockScreen 文字
    if (guider_ui.lockScreen_label_title) {
        lv_label_set_text(guider_ui.lockScreen_label_title, "设备已锁定");
    }

    last_percent = -1;
    last_status[0] = '\0';
}
