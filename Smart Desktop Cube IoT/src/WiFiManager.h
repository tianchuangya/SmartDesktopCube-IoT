#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <Arduino.h>

// ==================== ESP32 自建 AP 配置 ====================
// AP+STA 双模：AP 给手机/灯带连（192.168.4.x），STA 连目标 WiFi 上 MQTT
#define AP_SSID       "CubeLight"
#define AP_PASSWORD   "cube2026"   // 8 位以上，WPA2 最小长度

// ==================== 公共 API ====================
bool WiFiManager_IsConnected(void);
void WiFiManager_Connect(void);
bool WiFiManager_IsTimeSynced(void);

// WiFi 扫描（返回 JSON 字符串，调用者负责释放）
String WiFiManager_Scan(void);

// 连接指定 WiFi（保存到 NVS，重启后自动连接）
bool WiFiManager_ConnectTo(const char* ssid, const char* password);

// 获取当前连接的 SSID
const char* WiFiManager_GetSSID(void);

// AP 客户端数量（用于检测是否有设备连入）
uint8_t WiFiManager_GetAPClientCount(void);

// Captive Portal DNS 服务（在 WebServerTask 中调用）
void WiFiManager_DNSLoop(void);

#endif
