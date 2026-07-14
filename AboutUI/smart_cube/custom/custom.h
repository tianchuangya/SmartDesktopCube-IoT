/*
 * Copyright 2023 NXP
 * NXP Proprietary.
 */

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void custom_init(lv_ui *ui);
void ui_update_timer_cb(lv_timer_t *t);
void focus_count_timer_cb(lv_timer_t *t);
void reset_focus_timer(void);
void stop_focus_timer(void);
void toggle_presence(void);
void set_night_mode(bool enabled);

#ifdef __cplusplus
}
#endif
#endif
