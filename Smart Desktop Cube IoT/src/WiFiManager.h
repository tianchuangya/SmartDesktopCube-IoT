#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

bool WiFiManager_IsConnected(void);
void WiFiManager_Connect(void);
bool WiFiManager_IsTimeSynced(void);  // NTP 时间是否已同步

#endif