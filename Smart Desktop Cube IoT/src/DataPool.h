#ifndef DATAPOOL_H
#define DATAPOOL_H
#include <stdint.h>

// ==================== 固件版本号（手动递增）====================
#define FW_VERSION "v1.0.0"
// 传感器数据结构体
typedef struct {
  float temp;        // 温度
  float humi;        // 湿度
  float tvoc;        // 总挥发性有机物
  float eco2;        // 等效二氧化碳
  float light;       // 光照强度
  float pm25;        // PM2.5浓度
  int aqi;           // 空气质量指数
  int human_duration;     // 人体存在持续时间
  uint32_t focus_duration; // 专注模式持续秒数
} SensorData;

// WiFi 配置
typedef struct {
    char ssid[64];     // WiFi 名称
    char pwd[64];      // WiFi 密码
} WiFiConfig_t;

// 专注模式配置
typedef struct {
    uint16_t radar_max_distance;    // 雷达最大探测距离（cm），默认150
    uint8_t  radar_sensitivity;     // 雷达灵敏度（1-10），默认5
    uint32_t leave_timeout_ms;      // 离开超时（毫秒），超过此时间无人认为退出专注，默认30000
    uint32_t auto_enter_delay_ms;   // 自动进入专注延时（毫秒），持续有人超过此时间自动进入，默认5000
    uint16_t human_distance;        // 检测到的人体距离（cm），无人时为0
    bool is_human_exist;            // 是否有人
    bool     auto_enter_enabled;    // 是否启用自动进入专注模式
    bool     auto_exit_enabled;     // 是否启用自动退出专注模式
    uint32_t focus_cooldown_ms;     // 手动退出后冷却时间（毫秒），冷却期内禁止自动进入，默认30000
    uint32_t last_manual_exit_ms;   // 上次手动退出专注的时间戳（毫秒），0=不在冷却期
} FocusConfig_t;

// 传感器历史记录（用于趋势图和预测）
#define HISTORY_SIZE 60  // 60 个点 × 5 分钟间隔 = 5 小时历史
typedef struct {
    float temp[HISTORY_SIZE];    // 温度历史
    float humi[HISTORY_SIZE];    // 湿度历史
    float light[HISTORY_SIZE];   // 光照历史
    float eco2[HISTORY_SIZE];    // CO2 历史
    int history_index;           // 当前写入位置（环形缓冲）
    int history_count;           // 已记录点数（最多 HISTORY_SIZE）
} SensorHistory;

// 设备状态结构体
typedef struct {
  bool wifi_connected;  // WiFi连接状态
  bool mqtt_connected;  // MQTT连接状态
  bool time_synced;   // NTP时间已同步（断网后仍保持true，time()继续可用）
  bool is_human_exist;  // 人体存在检测
  int screen_brightness;// 屏幕亮度
  bool sensor_bh1750;    // 光照传感器
  bool sensor_aht21;     // 温湿度传感器
  bool sensor_ens160;    // 空气质量传感器
  uint32_t boot_time;    // 开机时间戳（毫秒）
  uint32_t run_seconds;  // 已运行秒数
  bool focus_mode;            // 专注模式
  bool device_lock;           // 设备锁
  bool request_focus_screen;  // 自动进入专注时，请求UI切换到fouseScreen
  bool on_main_screen;        // 当前是否在mainScreen（UI层每帧更新）
  bool ota_in_progress;       // OTA 是否正在进行
  int  ota_progress;          // OTA 进度 0-100
  char ota_new_version[32];   // OTA 新固件版本号
  char ota_status_text[64];   // OTA 状态描述文字
  bool ota_check_requested;   // 用户手动请求检查更新
  uint8_t ota_check_status;   // 0=idle 1=checking 2=update_available 3=latest 4=failed
  uint32_t ota_check_time;    // 发起检测的时间戳(millis)
  bool ota_update_available;  // 后端返回了新版本，等待用户确认
  char ota_pending_url[256];  // 待确认的固件下载地址
  char ota_pending_version[32]; // 待确认的新版本号
  char ota_pending_md5[33];   // 待确认的 MD5
  // ---- 本地智能决策 ----
  bool air_quality_alert;     // 空气质量告警（CO2>1000 或 TVOC>0.5）
  bool temp_comfort_alert;    // 温度舒适度告警（<18°C 或 >28°C）
  bool auto_brightness_enabled; // 是否启用自动亮度调节
  bool silent_mode;           // 免打扰模式（夜间不告警）
  // ---- Web→TFT 弹窗通知 ----
  char pending_toast[48];     // 待显示的弹窗文字
  volatile bool toast_pending; // 是否有待显示弹窗
  // ---- 启动模块加载 ----
  bool modules_ready;         // 所有模块初始化完成
  char boot_status_text[32];  // 启动状态文字（供LVGL任务显示）
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
  char did[32];            // 设备唯一标识（JSON消息中使用）
  char mqtt_client_id[32]; // MQTT Client ID（MAC地址生成，如 cube_AABBCCDDEEFF）
  char token[64];          // 认证令牌
  bool token_ok;           // 令牌是否有效
  uint32_t token_expire_time; // token 过期时间戳（Unix时间，秒），0=未设置
} SecurityData;

//角色状态图片（60×60）
typedef struct {
    const uint8_t* normal;      // 正常：微笑
    const uint8_t* remind;      // 普通提醒：担忧/问号
    const uint8_t* alert;       // 严重告警：紧张/警告
    const uint8_t* focus;       // 专注模式：安静陪伴
    const uint8_t* celebrate;   // 完成/恢复：点赞
    const uint8_t* guide;       // 菜单向导：指引
} RoleImg_t;

// AQI 等级图片（60*60）
typedef struct {
    const uint8_t* aqi_1;  // 优
    const uint8_t* aqi_2;  // 良
    const uint8_t* aqi_3;  // 中
    const uint8_t* aqi_4;  // 差
    const uint8_t* aqi_5;  // 劣
} AqiBgImg_t;


//开机启动动画（320×240）
typedef struct {
    const uint8_t* sleep;    // 睡觉
    const uint8_t* wake_1;   // 苏醒1
    const uint8_t* wake_2;   // 苏醒2
    const uint8_t* welcome;  // 欢迎
    const uint8_t* logo;     // LOGO
} BootImg_t;


//主界面背景（320×240）
typedef struct {
    const uint8_t* main;     // 主界面
    const uint8_t* focus;    // 专注模式
    const uint8_t* menu;     // 菜单界面
    const uint8_t* report;   // 专注报告
    const uint8_t* setting;  // 设置界面
} BgImg_t;


//菜单/选项卡图片（UI控件）
typedef struct {
    const uint8_t* tab_air;      // 空气质量选项卡
    const uint8_t* tab_env;      // 环境数据选项卡
    const uint8_t* tab_time;     // 计时选项卡
    const uint8_t* tab_set;      // 设置选项卡
    const uint8_t* btn_back;     // 返回按钮
    const uint8_t* btn_ok;       // 确认按钮
} UIControlImg_t;


//专注模式相关图片
typedef struct {
    const uint8_t* focus_start;   // 开始专注
    const uint8_t* focus_running; // 专注中
    const uint8_t* focus_end;     // 结束专注
} FocusImg_t;

//总图片池
typedef struct {
    RoleImg_t       role;    // 角色
    AqiBgImg_t      aqi;     // AQI等级背景
    BootImg_t       boot;    // 开机动画
    BgImg_t         bg;      // 背景
    UIControlImg_t  ui;      // UI控件
    FocusImg_t      focus;   // 专注模式
} ImagePool_t;


// 全局数据池实例声明（外部可访问）
extern SensorData sensorData;
extern WiFiConfig_t  wifi_config;
extern DeviceStatus status;
extern CommandData cmd;
extern SecurityData security;
extern ImagePool_t    img; 
extern FocusConfig_t focusConfig;
extern SensorHistory sensorHistory;
#endif 