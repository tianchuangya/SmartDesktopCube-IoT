#include "WiFiManager.h"
#include "DataPool.h"
#include <WiFi.h>

void WiFiManager_Connect(void) {
   wl_status_t status_wifi = WiFi.status();

    // 只有断开状态才去重连
    if (status_wifi != WL_CONNECTED) 
    {
        WiFi.disconnect();//清除旧的 WiFi 连接状态
        delay(100);
        WiFi.begin(wifi_config.ssid, wifi_config.pwd);
      
        // 等待结果（最多等 3 秒）
        int retry = 0;
        while (WiFi.status() != WL_CONNECTED && retry < 6) {
            Serial.print("[WiFi] 寻找WiFi: ");
            Serial.print(wifi_config.ssid); 
            int count=0;
            while(count<=retry){
                Serial.print(".");
                count++;
                delay(100);
            }
            Serial.println("");
            retry++;
        }
    }

    // 统一更新WiFi状态
    switch (WiFi.status()) {
        case WL_CONNECTED:
            Serial.println("[WiFi] 连接成功 ✅");
            status.wifi_connected = true;
            break;

        case WL_NO_SSID_AVAIL:
            Serial.println("[WiFi] 连接失败 → 找不到该热点 ❌");
            status.wifi_connected = false;
            break;

        case WL_CONNECT_FAILED:
            Serial.println("[WiFi] 连接失败 → 密码错误 ❌");
            status.wifi_connected = false;
            break;

        case WL_DISCONNECTED:
            Serial.println("[WiFi] 已断开 ❌");
            status.wifi_connected = false;
            break;

        default:
            Serial.println("[WiFi] 状态: 正在尝试连接...");
            status.wifi_connected = false;
            break;
    }
}

// 判断是否连接成功
bool WiFiManager_IsConnected(void) {
    return WiFi.status() == WL_CONNECTED;
}