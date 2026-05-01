#ifndef DATAPOOL_H
#define DATAPOOL_H
#include <stdint.h>
// 传感器数据结构体
typedef struct {
  float temp;        // 温度
  float humi;        // 湿度
  float tvoc;        // 总挥发性有机物
  float eco2;        // 等效二氧化碳
  float light;       // 光照强度
  float pm25;        // PM2.5浓度
  int aqi;           // 空气质量指数
  int human_duration;// 人体存在持续时间
} SensorData;

// 设备状态结构体
typedef struct {
  bool wifi_connected;  // WiFi连接状态
  bool mqtt_connected;  // MQTT连接状态
  bool is_human_exist;  // 人体存在检测
  int screen_brightness;// 屏幕亮度
  bool sensor_bh1750;    // 光照传感器
  bool sensor_aht21;     // 温湿度传感器
  bool sensor_ens160;    // 空气质量传感器
  uint32_t boot_time;        // 开机时间戳（毫秒）
  uint32_t run_seconds;      // 已运行秒数
} DeviceStatus;

// 指令数据结构体
typedef struct {
  char speak_text[128]; // 语音播报文本
  int player_cmd;       // 播放器指令
  int aircon_mode;      // 空调模式
  int target_temp;      // 目标温度
} CommandData;

// 安全认证数据结构体
typedef struct {
  char did[32];     // 设备唯一标识
  char token[64];   // 认证令牌
  bool token_ok;    // 令牌是否有效
} SecurityData;

// 全局数据池实例声明（外部可访问）
extern SensorData sensorData;
extern DeviceStatus status;
extern CommandData cmd;
extern SecurityData security;

#endif // DATAPOOL_H