/*
 * Copyright 2026 NXP
 * NXP Proprietary.
 */

#ifndef EVENTS_INIT_H_
#define EVENTS_INIT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "gui_guider.h"

void events_init(lv_ui *ui);
void events_init_bootScreen(lv_ui *ui);
void events_init_mainScreen(lv_ui *ui);
void events_init_airScreen(lv_ui *ui);
void events_init_envScreen(lv_ui *ui);
void events_init_focusScreen(lv_ui *ui);
void events_init_settingsScreen(lv_ui *ui);
void events_init_lockScreen(lv_ui *ui);

#ifdef __cplusplus
}
#endif
#endif
