/*
 * BlinkerApp.h — Blinker App 数据发送/接收模块
 *
 * 功能：
 *   - 将传感器数据推送到 Blinker App 的 Number 控件
 *   - 接收 App 端 Button/Slider 指令控制灯光
 *   - 断网时自动跳过，不影响 WiFi 重连和其他功能
 *
 * 用法：
 *   1. 在 BlinkerApp_Init() 之前确保 WiFi 已连接
 *   2. 在主循环中调用 BlinkerApp_Run()（约 50ms 一次）
 *   3. 在 MQTT 数据上报时调用 BlinkerApp_SendAll()
 *
 * 注意：使用前需在 BlinkerApp.cpp 顶部填入 BLINKER_AUTH 密钥
 */
#ifndef __BLINKER_APP_H_
#define __BLINKER_APP_H_

#include <Arduino.h>

// 初始化 Blinker（WiFi 已连接后调用，仅初始化一次）
void BlinkerApp_Init();

// 主循环处理（在 MQTT 任务中每 50ms 调用一次）
void BlinkerApp_Run();

// 发送所有数据到 Blinker App（与 MQTT 数据上报同步调用）
void BlinkerApp_SendAll();

// === 各控件单独发送函数 ===
void BlinkerApp_SendLight();       // bnt-light: 灯开关状态
void BlinkerApp_SendBrightness();  // ran-liangdu: 亮度值
void BlinkerApp_SendColorTemp();   // ran-sewen: 色温值
void BlinkerApp_SendTemp();        // num-wendu: 温度
void BlinkerApp_SendHumi();        // num-shidu: 湿度
void BlinkerApp_SendECO2();        // num-eco2: eCO2
void BlinkerApp_SendPM25();        // num-pm2.5: PM2.5
void BlinkerApp_SendLightLux();    // num-lx: 光照
void BlinkerApp_SendTVOC();        // num-ppb: TVOC

#endif /* __BLINKER_APP_H_ */
