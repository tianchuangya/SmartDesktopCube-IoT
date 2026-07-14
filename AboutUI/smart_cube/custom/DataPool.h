/*
 * Stub DataPool.h for LVGL simulator.
 * Provides the sensorData and status structs expected by custom.c.
 */
#ifndef DATAPOOL_H
#define DATAPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct {
    float temp;
    float humi;
    float tvoc;
    float eco2;
    float light;
    float pm25;
    int   aqi;
    float human_duration;
} SensorData;

typedef struct {
    bool wifi_connected;
    bool mqtt_connected;
    bool is_human_exist;
    int  screen_brightness;
    bool focus_mode;
    bool device_lock;
} DeviceStatus;

extern SensorData sensorData;
extern DeviceStatus status;

#ifdef __cplusplus
}
#endif
#endif
