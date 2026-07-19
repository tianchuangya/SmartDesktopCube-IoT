#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

// Phase 1: 显示初始化（LVGL + 启动画面，快速）
void Device_InitDisplay(void);

// Phase 2: 模块加载（传感器 + 雷达，每个5次重试）
void Device_InitModules(void);

// 兼容旧调用（依次执行 Phase1 + Phase2）
void Device_InitAll(void);

#endif