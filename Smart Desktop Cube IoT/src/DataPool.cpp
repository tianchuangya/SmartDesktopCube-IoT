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

FocusConfig_t focusConfig = {
    .radar_max_distance   = 60,    // 1.5米
    .radar_sensitivity    = 5,      // 中等灵敏度
    .leave_timeout_ms     = 5000,   // 5秒
    .auto_enter_delay_ms  = 5000,   // 5秒
    .human_distance       = 0,       // 检测到的人体距离（cm），无人时为0
    .is_human_exist       =false,   // 是否有人
    .auto_enter_enabled   = true,   // 默认启用自动进入
    .auto_exit_enabled    = true    // 默认启用自动退出
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
  .run_seconds = 0,
  .focus_mode=false,
  .device_lock=false
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

ImagePool_t img = {

    //角色状态图片
    .role = {
        .normal    = nullptr,  // 正常微笑
        .remind    = nullptr,  // 提醒
        .alert     = nullptr,  // 告警
        .focus     = nullptr,  // 专注
        .celebrate = nullptr,  // 点赞
        .guide     = nullptr   // 菜单向导
    },

    //AQI 5 级背景
    .aqi = {
        .aqi_1 = nullptr,  // 优
        .aqi_2 = nullptr,  // 良
        .aqi_3 = nullptr,  // 中
        .aqi_4 = nullptr,  // 差
        .aqi_5 = nullptr   // 劣
    },

    //开机启动动画
    .boot = {
        .sleep   = nullptr,  // 睡觉
        .wake_1  = nullptr,  // 苏醒1
        .wake_2  = nullptr,  // 苏醒2
        .welcome = nullptr,  // 欢迎
        .logo    = nullptr   // LOGO
    },

    //界面背景
    .bg = {
        .main    = nullptr,  // 主界面
        .focus   = nullptr,  // 专注模式
        .menu    = nullptr,  // 菜单
        .report  = nullptr,  // 报告
        .setting = nullptr   // 设置
    },

    //UI 菜单/选项卡
    .ui = {
        .tab_air = nullptr,  // 空气质量
        .tab_env = nullptr,  // 环境
        .tab_time= nullptr,  // 计时
        .tab_set = nullptr,  // 设置
        .btn_back= nullptr,  // 返回
        .btn_ok  = nullptr   // 确认
    },

    //专注模式图片
    .focus = {
        .focus_start   = nullptr,  // 开始专注
        .focus_running = nullptr,  // 专注中
        .focus_end     = nullptr   // 结束
    }
};