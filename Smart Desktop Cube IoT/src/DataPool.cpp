#include "DataPool.h"

// 全局数据池初始化
SensorData sensorData = {
  .temp = 0.0f,
  .humi = 0.0f,
  .tvoc = 0.0f,
  .eco2 = 0.0f,
  .light = 0.0f,
  .pm25 = 0.0f,
  .aqi = 0,
  .human_duration = 0
};

// WiFi 默认配置
WiFiConfig_t wifi_config = {
  "tianchuangya",
  "qzl610929"
};

DeviceStatus status = {
  .wifi_connected = false,
  .mqtt_connected = false,
  .is_human_exist = false,
  .screen_brightness = 100,
  .sensor_bh1750 = false,
  .sensor_aht21 = false,
  .sensor_ens160 = false,
  .boot_time = 0,
  .run_seconds = 0
};

CommandData cmd = {
  "",          // speak_text
  0,           // player_cmd
  0,           // aircon_mode
  25           // target_temp
};

SecurityData security = {
    "ESP32-CUBE-001",  // did
    "",                // token
    false              // token_ok
};