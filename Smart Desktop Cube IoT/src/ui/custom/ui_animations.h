/*
 * ui_animations.h — 屏幕过渡动画 + 控件逐级显示
 *
 * 用法：
 *   1. 切屏时调用 ui_anim_switch_screen() 代替 lv_scr_load_anim(NONE)
 *   2. 在 LV_EVENT_SCREEN_LOADED 回调里调用 ui_anim_reveal_screen()
 *   3. 控件会按顺序淡入 + 从下方滑入，形成"逐级加载"效果
 */
#ifndef __UI_ANIMATIONS_H_
#define __UI_ANIMATIONS_H_

#include "lvgl.h"
#include "../generated/gui_guider.h"

/* 带淡入过渡的屏幕切换（替代 LV_SCR_LOAD_ANIM_NONE） */
void ui_anim_switch_screen(lv_obj_t *new_scr);

/* 屏幕加载完成后逐级显示控件（在 LV_EVENT_SCREEN_LOADED 中调用） */
void ui_anim_reveal_screen(lv_ui *ui, lv_obj_t *screen);

/* 通用：淡入 + 从下方滑入 */
void ui_anim_fade_slide_in(lv_obj_t *obj, int16_t offset_y,
                           uint16_t delay_ms, uint16_t duration_ms);

/* 通用：仅淡入 */
void ui_anim_fade_in(lv_obj_t *obj, uint16_t delay_ms, uint16_t duration_ms);

#endif /* __UI_ANIMATIONS_H_ */
