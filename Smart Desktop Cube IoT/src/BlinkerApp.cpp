/*
 * BlinkerApp.cpp — Blinker App 数据发送/接收实现（直连协议，不依赖 Blinker 库）
 *
 * 为什么不用 Blinker 库？
 *   Blinker 库自带旧版 ArduinoJson，与项目使用的 ArduinoJson v7.x 冲突，编译失败。
 *   本模块直接实现 Blinker MQTT 协议：HTTPS 认证 + 独立 PubSubClient 连接。
 *
 * 架构：
 *   - HTTPS 认证握手到 iot.diandeng.tech，获取动态 MQTT 凭证
 *   - 独立 WiFiClient + PubSubClient 连接 Blinker broker（与主 MQTT 互不干扰）
 *   - 在 MQTT 任务（Core 0）中被调用，与主 MQTT 数据上报同步
 *   - 断网时所有操作自动跳过，不影响 WiFi 重连和其他功能
 *
 * 安全设计：
 *   - WiFi 未连接时跳过所有 Blinker 操作
 *   - 认证失败 60s 重试，MQTT 断线 5s 重连，连续 3 次失败重新认证
 *   - 独立的 WiFiClient/PubSubClient 实例，不干扰主 MQTT 连接
 *   - setSocketTimeout(2) 防止看门狗崩溃
 *
 * 用法：
 *   1. BlinkerApp_Init()  — WiFi 就绪后调用一次
 *   2. BlinkerApp_Run()   — 每次循环调用（约 50ms），保持连接 + 处理接收
 *   3. BlinkerApp_SendAll() — 定时调用（2 秒一次），推送全部控件
 *
 * 注意：使用前需在下方填入 BLINKER_AUTH 密钥
 */
#include "BlinkerApp.h"
#include "DataPool.h"
#include "WiFiManager.h"
#include "BluetoothLight.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ==================== 用户配置 ====================
#define BLINKER_AUTH "2fc34ccb327a"

// ==================== Blinker 凭证（认证后填充）====================
static String b_host;
static uint16_t b_port = 0;
static String b_devName;
static String b_iotId;
static String b_iotToken;
static String b_uuid;

// ==================== 独立 MQTT 客户端 ====================
static WiFiClient   b_client;
static PubSubClient b_mqtt(b_client);

// ==================== 主题 ====================
static String b_topic_pub;   // /device/<devName>/s
static String b_topic_sub;   // /device/<devName>/r

// ==================== 状态机 ====================
static bool     b_inited = false;
static bool     b_authed = false;
static uint32_t b_last_auth = 0;
static uint32_t b_last_reconnect = 0;
static int      b_reconnect_fails = 0;

static const uint32_t AUTH_RETRY_MS      = 60000;
static const uint32_t RECONNECT_RETRY_MS = 5000;
static const int      MAX_RECONNECT_FAILS = 3;

// ==================== 认证握手 ====================
static bool b_doAuth()
{
    WiFiClientSecure tls;
    tls.setInsecure();
    tls.setTimeout(10000);

    HTTPClient http;
    String url = String("https://iot.diandeng.tech/api/v1/user/device/diy/auth?authKey=")
                 + BLINKER_AUTH + "&version=1.0&protocol=mqtt";

    if (!http.begin(tls, url)) return false;

    int code = http.GET();
    if (code != 200) { http.end(); return false; }

    String resp = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, resp)) return false;

    if (!doc["detail"].is<JsonObject>()) return false;

    JsonObject d = doc["detail"];
    b_host    = d["host"].as<String>();
    b_port    = d["port"].as<uint16_t>();
    b_devName = d["deviceName"].as<String>();
    b_iotId   = d["iotId"].as<String>();
    b_iotToken = d["iotToken"].as<String>();
    b_uuid    = d["uuid"].as<String>();

    // 去掉 host 中的协议前缀（mqtts:// / mqtt:// 等）
    int proto = b_host.indexOf("://");
    if (proto >= 0) b_host = b_host.substring(proto + 3);
    int colon = b_host.indexOf(':');
    if (colon >= 0) {
        b_port = (uint16_t)b_host.substring(colon + 1).toInt();
        b_host = b_host.substring(0, colon);
    }
    int slash = b_host.indexOf('/');
    if (slash >= 0) b_host = b_host.substring(0, slash);

    b_topic_pub = "/device/" + b_devName + "/s";
    b_topic_sub = "/device/" + b_devName + "/r";

    return true;
}

// ==================== 接收回调 ====================
static void b_callback(char* topic, byte* payload, unsigned int len)
{
    char buf[256];
    if (len >= sizeof(buf)) return;
    memcpy(buf, payload, len);
    buf[len] = '\0';

    JsonDocument doc;
    if (deserializeJson(doc, buf)) return;

    JsonObject data = doc["data"].as<JsonObject>();
    if (!data) return;

    for (JsonPair kv : data) {
        String key = kv.key().c_str();

        if (key == "bnt-light") {
            String v = kv.value().as<String>();
            if (v == "tap" || v == "press") {
                if (blLight.brightness > 0) BL_TurnOff();
                else BL_TurnOn();
            }
        }
        else if (key == "ran-liangdu") {
            int val = kv.value().as<int>();
            if (val < 0) val = 0;
            if (val > 100) val = 100;
            BL_SetBrightness((uint8_t)val);
        }
        else if (key == "ran-sewen") {
            int val = kv.value().as<int>();
            if (val < 0) val = 0;
            if (val > 100) val = 100;
            BL_SetColorTemp((uint8_t)val);
        }
    }
}

// ==================== MQTT 连接 ====================
static bool b_connectMqtt()
{
    if (b_host.length() == 0) return false;

    b_mqtt.setServer(b_host.c_str(), b_port);
    b_mqtt.setCallback(b_callback);
    b_mqtt.setBufferSize(512);
    b_mqtt.setSocketTimeout(2);

    if (b_mqtt.connect(b_devName.c_str(), b_iotId.c_str(), b_iotToken.c_str())) {
        b_mqtt.subscribe(b_topic_sub.c_str());
        return true;
    }
    return false;
}

// ==================== 发送辅助函数 ====================
static void b_pubVal(const char* key, float val)
{
    if (!b_mqtt.connected()) return;
    JsonDocument doc;
    JsonObject data = doc["data"].to<JsonObject>();
    data[key].to<JsonObject>()["val"] = val;
    doc["fromDevice"] = b_devName;
    doc["toDevice"]   = b_uuid;
    doc["deviceType"] = "OwnApp";
    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    b_mqtt.publish(b_topic_pub.c_str(), buf);
}

static void b_pubSwi(const char* key, const char* swi)
{
    if (!b_mqtt.connected()) return;
    JsonDocument doc;
    JsonObject data = doc["data"].to<JsonObject>();
    data[key].to<JsonObject>()["swi"] = swi;
    doc["fromDevice"] = b_devName;
    doc["toDevice"]   = b_uuid;
    doc["deviceType"] = "OwnApp";
    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    b_mqtt.publish(b_topic_pub.c_str(), buf);
}

// ==================== 微信通知（HTTP API）====================
// Blinker 微信推送走 HTTP POST 到 iot.diandeng.tech/api/v1/user/device/wxMsg/
// JSON body: {"deviceName":"...","key":"...","title":"...","state":"...","msg":"...","receivers":""}
static void b_sendWechat(const char* title, const char* state, const char* msg)
{
    if (!WiFiManager_IsConnected()) return;
    if (b_devName.length() == 0) return;  // 未认证

    WiFiClientSecure tls;
    tls.setInsecure();
    tls.setTimeout(10000);

    HTTPClient http;
    String url = "https://iot.diandeng.tech/api/v1/user/device/wxMsg/";

    if (!http.begin(tls, url)) return;

    http.addHeader("Content-Type", "application/json;charset=utf-8");

    // 构造 JSON body
    String body = "{\"deviceName\":\"";
    body += b_devName;
    body += "\",\"key\":\"";
    body += BLINKER_AUTH;
    body += "\",\"title\":\"";
    body += title;
    body += "\",\"state\":\"";
    body += state;
    body += "\",\"msg\":\"";
    body += msg;
    body += "\",\"receivers\":\"\"}";

    http.POST(body);
    http.end();
}

// ==================== 报警阈值 ====================
#define ALARM_AQI_HIGH      300     // AQI≥300 严重污染
#define ALARM_TEMP_HIGH     45.0f   // 温度≥45°C 疑似火灾
#define ALARM_TEMP_LOW       5.0f   // 温度≤5°C 过冷
#define ALARM_HUMI_HIGH     85.0f   // 湿度≥85% 过湿
#define ALARM_HUMI_LOW      20.0f   // 湿度≤20% 过干
#define ALARM_PM25_HIGH    150.0f   // PM2.5≥150 重度污染
#define ALARM_ECO2_HIGH   2000.0f   // eCO2≥2000ppm 重度污染
#define ALARM_TVOC_HIGH   2200.0f   // TVOC≥2200ppb 重度污染

#define ALARM_COOLDOWN_MS  120000   // 报警冷却 2 分钟（Blinker 服务端限制 60s）
#define ALARM_COUNT 8

typedef enum {
    ALARM_AQI = 0,
    ALARM_TEMP_HI,
    ALARM_TEMP_LO,
    ALARM_HUMI_HI,
    ALARM_HUMI_LO,
    ALARM_PM25,
    ALARM_ECO2,
    ALARM_TVOC
} AlarmType_t;

static uint32_t alarm_last_sent[ALARM_COUNT] = {0};

static bool alarm_can_send(AlarmType_t t) {
    uint32_t now = millis();
    if (alarm_last_sent[t] == 0) return true;
    return (now - alarm_last_sent[t] >= ALARM_COOLDOWN_MS);
}

static void alarm_send(AlarmType_t t, const char* title, const char* state, const char* msg)
{
    if (!alarm_can_send(t)) return;
    alarm_last_sent[t] = millis();
    b_sendWechat(title, state, msg);
}

// ==================== 公共 API ====================

void BlinkerApp_Init()
{
    if (b_inited) return;
    b_inited = true;
}

void BlinkerApp_Run()
{
    if (!WiFiManager_IsConnected()) return;

    if (!b_authed) {
        if (millis() - b_last_auth < AUTH_RETRY_MS) return;
        b_last_auth = millis();
        b_authed = b_doAuth();
        if (b_authed) b_connectMqtt();
        return;
    }

    if (!b_mqtt.connected()) {
        if (millis() - b_last_reconnect < RECONNECT_RETRY_MS) return;
        b_last_reconnect = millis();
        if (b_connectMqtt()) {
            b_reconnect_fails = 0;
        } else {
            b_reconnect_fails++;
            if (b_reconnect_fails >= MAX_RECONNECT_FAILS) {
                b_authed = false;
                b_reconnect_fails = 0;
            }
        }
        return;
    }

    b_mqtt.loop();
}

// ==================== 各控件发送函数 ====================

void BlinkerApp_SendLight()       { b_pubSwi("bnt-light", blLight.brightness > 0 ? "on" : "off"); }
void BlinkerApp_SendBrightness()  { b_pubVal("ran-liangdu", blLight.brightness); }
void BlinkerApp_SendColorTemp()   { b_pubVal("ran-sewen", blLight.color_temperature); }
void BlinkerApp_SendTemp()        { b_pubVal("num-wendu", sensorData.temp); }
void BlinkerApp_SendHumi()        { b_pubVal("num-shidu", sensorData.humi); }
void BlinkerApp_SendECO2()        { b_pubVal("num-eco2", sensorData.eco2); }
void BlinkerApp_SendPM25()        { b_pubVal("num-pm25", sensorData.pm25); }
void BlinkerApp_SendLightLux()    { b_pubVal("num-lx", sensorData.light); }
void BlinkerApp_SendTVOC()        { b_pubVal("num-ppb", sensorData.tvoc); }

// ==================== 总发送函数 ====================

void BlinkerApp_SendAll()
{
    if (!WiFiManager_IsConnected()) return;
    if (!b_mqtt.connected()) return;

    JsonDocument doc;
    JsonObject data = doc["data"].to<JsonObject>();

    data["bnt-light"].to<JsonObject>()["swi"] = (blLight.brightness > 0) ? "on" : "off";
    data["ran-liangdu"].to<JsonObject>()["val"] = blLight.brightness;
    data["ran-sewen"].to<JsonObject>()["val"] = blLight.color_temperature;
    data["num-wendu"].to<JsonObject>()["val"] = sensorData.temp;
    data["num-shidu"].to<JsonObject>()["val"] = sensorData.humi;
    data["num-eco2"].to<JsonObject>()["val"] = sensorData.eco2;
    data["num-pm25"].to<JsonObject>()["val"] = sensorData.pm25;
    data["num-lx"].to<JsonObject>()["val"] = sensorData.light;
    data["num-ppb"].to<JsonObject>()["val"] = sensorData.tvoc;

    doc["fromDevice"] = b_devName;
    doc["toDevice"]   = b_uuid;
    doc["deviceType"] = "OwnApp";

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    b_mqtt.publish(b_topic_pub.c_str(), buf);
}

// ==================== 微信报警通知 ====================

void BlinkerApp_SendWechat(const char* title, const char* state, const char* message)
{
    b_sendWechat(title, state, message);
}

void BlinkerApp_CheckAlarms()
{
    if (!WiFiManager_IsConnected()) return;
    if (b_devName.length() == 0) return;  // 未认证不检查

    // AQI 严重污染
    if (sensorData.aqi >= ALARM_AQI_HIGH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "AQI值过高(%d)，请注意室内通风", sensorData.aqi);
        alarm_send(ALARM_AQI, "空气报警", "严重", msg);
    }

    // 温度过高（疑似火灾）
    if (sensorData.temp >= ALARM_TEMP_HIGH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "室内温度过高，目前%.1f°C，疑似火灾！", sensorData.temp);
        alarm_send(ALARM_TEMP_HI, "温度报警", "紧急", msg);
    }

    // 温度过低
    if (sensorData.temp >= 0 && sensorData.temp <= ALARM_TEMP_LOW) {
        char msg[64];
        snprintf(msg, sizeof(msg), "室内温度过低，目前%.1f°C", sensorData.temp);
        alarm_send(ALARM_TEMP_LO, "温度报警", "提醒", msg);
    }

    // 湿度过高
    if (sensorData.humi >= ALARM_HUMI_HIGH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "室内湿度过高，当前%.1f%%，注意防潮", sensorData.humi);
        alarm_send(ALARM_HUMI_HI, "湿度报警", "提醒", msg);
    }

    // 湿度过低
    if (sensorData.humi >= 0 && sensorData.humi <= ALARM_HUMI_LOW) {
        char msg[64];
        snprintf(msg, sizeof(msg), "室内湿度过低，当前%.1f%%，注意保湿", sensorData.humi);
        alarm_send(ALARM_HUMI_LO, "湿度报警", "提醒", msg);
    }

    // PM2.5 超标
    if (sensorData.pm25 >= ALARM_PM25_HIGH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "PM2.5浓度超标，当前%.1f μg/m³，请通风或佩戴口罩", sensorData.pm25);
        alarm_send(ALARM_PM25, "空气报警", "严重", msg);
    }

    // eCO2 超标
    if (sensorData.eco2 >= ALARM_ECO2_HIGH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "CO2浓度过高，当前%.0f ppm，请开窗通风", sensorData.eco2);
        alarm_send(ALARM_ECO2, "空气报警", "严重", msg);
    }

    // TVOC 超标
    if (sensorData.tvoc >= ALARM_TVOC_HIGH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "TVOC浓度过高，当前%.0f ppb，注意室内空气", sensorData.tvoc);
        alarm_send(ALARM_TVOC, "空气报警", "严重", msg);
    }
}
