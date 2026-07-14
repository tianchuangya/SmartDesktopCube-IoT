#ifndef BLUETOOTH_LIGHT_H
#define BLUETOOTH_LIGHT_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==================== BLE 灯具参数 ====================
// 目标设备广播名（ESP32-C3 灯光控制器使用的名称）
#define BLE_LIGHT_DEVICE_NAME "SmartCube-Light"

// 服务UUID（与 ESP32-C3 端完全一致）
#define BLE_SRV_UUID            "12345678-1234-1234-1234-123456789abc"
// 亮度特征UUID
#define BLE_CHAR_BRIGHTNESS     "12345678-1234-1234-1234-123456789abd"
// 色温特征UUID
#define BLE_CHAR_COLORTEMP      "12345678-1234-1234-1234-123456789abe"

// ==================== 色温预设模板 ====================
typedef enum {
    LIGHT_PRESET_WARM  = 0,   // 暖光：亮度100%, 色温~3000K
    LIGHT_PRESET_COOL  = 1,   // 冷光：亮度80%,  色温~6500K
    LIGHT_PRESET_WHITE = 2    // 白光：亮度80%,  色温~4500K
} LightPreset_t;

// ==================== 灯具状态 ====================
typedef struct {
    bool     connected;           // BLE是否已连接
    uint8_t  brightness;          // 当前亮度 0-100
    uint8_t  color_temperature;   // 当前色温 0-100 (映射到2700K-6500K)
    uint8_t  target_brightness;   // 目标亮度
    uint8_t  target_color_temp;   // 目标色温
    bool     pending_update;      // 是否有待发送的更新
    uint8_t  connection_attempts; // 连续连接失败次数
    uint32_t last_fail_time;      // 上次连接失败的时间戳(ms)
    bool     last_connect_failed; // 最近一次连接尝试是否失败（UI可读取以弹出提示）
} BluetoothLightState;

extern BluetoothLightState blLight;

// ==================== API ====================

// 初始化BLE并启动连接任务（在main中调用一次）
void BL_Init();

// 设置亮度 (0-100)，非阻塞，通过任务异步发送
void BL_SetBrightness(uint8_t brightness);

// 设置色温 (0-100)，非阻塞，通过任务异步发送
// 0 = 暖色(~2700K), 50 = 中性(~4500K), 100 = 冷色(~6500K)
void BL_SetColorTemp(uint8_t color_temp);

// 同时设置亮度和色温
void BL_SetLight(uint8_t brightness, uint8_t color_temp);

// 一键预设模板
void BL_PresetWarm();   // 暖光：最亮+低色温
void BL_PresetCool();   // 冷光：高亮+高色温
void BL_PresetWhite();  // 白光：中亮+中色温

// 开关灯（通过设置亮度0实现）
void BL_TurnOff();
void BL_TurnOn();       // 恢复之前的亮度

// 获取当前连接状态
bool BL_IsConnected();

// 获取最近一次连接是否失败（读取后自动清除标记，供UI轮询弹窗）
bool BL_ConsumeConnectFailedFlag();

// 获取状态描述文字（用于UI显示）
const char* BL_GetStatusMsg();

// 获取色温对应的开尔文值（用于UI显示）
uint16_t BL_ColorTempToKelvin(uint8_t ct);

#ifdef __cplusplus
}
#endif

#endif
