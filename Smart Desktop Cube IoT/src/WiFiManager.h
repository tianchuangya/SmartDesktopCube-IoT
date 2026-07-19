#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

// ==================== ESP32 自建 AP 配置（给 Yeelight 灯带用）====================
// 因手机热点客户端隔离导致 ESP32↔Yeelight 无法通信，
// 改用 ESP32 AP+STA 双模：AP 给灯带连（192.168.4.x，无隔离），STA 连手机热点上 MQTT
// 米家 App 配对灯带时选这个 SSID，密码输入下面的 AP_PASSWORD
#define AP_SSID       "CubeLight"
#define AP_PASSWORD   "cube2026"   // 8 位以上，WPA2 最小长度

bool WiFiManager_IsConnected(void);
void WiFiManager_Connect(void);
bool WiFiManager_IsTimeSynced(void);  // NTP 时间是否已同步

#endif