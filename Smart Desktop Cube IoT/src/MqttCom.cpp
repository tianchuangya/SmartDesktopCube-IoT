#include "MqttCom.h"
#include "DataPool.h"
#include "OTAManager.h"
#include <ArduinoJson.h>
#include "time.h"
#include <WiFi.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// ⚠ MQTT 服务器配置已移至 MqttCom.h，此处不再重复定义

static char topic_status[64];
static char topic_data[64];
static char topic_control[64];
static char topic_ack[64];

// 生成 MAC 地址的 Client ID（格式: cube_AABBCCDDEEFF）
void mqttGenerateClientId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(security.mqtt_client_id, sizeof(security.mqtt_client_id),
             "cube_%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("[MQTT] Client ID: %s\n", security.mqtt_client_id);
}

// 下行消息回调
static void mqttCallback(char* topic, byte* payload, unsigned int len) {
    char buffer[512];
    if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
    memcpy(buffer, payload, len);
    buffer[len] = 0;

    // 打印收到的原始消息
    Serial.printf("[MQTT] 📩 收到消息\n");
    Serial.printf("  Topic: %s\n", topic);
    Serial.printf("  Payload: %s\n", buffer);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, buffer);
    if (err) {
        Serial.printf("[MQTT] ❌ JSON解析失败: %s\n", err.c_str());
        return;
    }

    // ---- 握手响应 ----
    if (doc["type"].is<const char*>() &&
        strcmp(doc["type"], "handshake_ack") == 0) {
        int code = doc["code"] | 0;
        if (code == 200 && doc["token"].is<const char*>()) {
            strncpy(security.token, doc["token"], sizeof(security.token) - 1);
            security.token[sizeof(security.token) - 1] = '\0';
            security.token_ok = true;
            // 计算 token 绝对过期时间（当前时间 + expire_time 秒）
            int expire_sec = doc["expire_time"] | 600;
            security.token_expire_time = time(NULL) + expire_sec;
            Serial.printf("[MQTT] ✅ 握手成功, Token: %s, 过期时间: %ds后\n",
                          security.token, expire_sec);
        } else {
            security.token_ok = false;
            Serial.printf("[MQTT] ❌ 握手失败, code=%d, msg=%s\n",
                          code, doc["msg"].is<const char*>() ? doc["msg"].as<const char*>() : "");
        }
        return;
    }

    // ---- 控制指令 ----
    if (doc["type"].is<const char*>() &&
        strcmp(doc["type"], "control") == 0) {
        const char* command = doc["command"] | "";
        const char* value   = doc["value"] | "";
        Serial.printf("[MQTT] 收到控制指令: command=%s, value=%s\n", command, value);

        bool executed = false;
        const char* result = "unknown_command";

        // ---- 设备锁控制 ----
        if (strcmp(command, "lock") == 0) {
            status.device_lock = (strcmp(value, "on") == 0);
            executed = true;
            result = "success";
            Serial.printf("[MQTT] 设备锁: %s\n", status.device_lock ? "锁定" : "解锁");
        }
        // ---- 专注模式控制 ----
        else if (strcmp(command, "focus") == 0) {
            bool new_focus = (strcmp(value, "on") == 0);
            if (new_focus != status.focus_mode) {
                status.focus_mode = new_focus;
                if (new_focus) {
                    sensorData.focus_duration = 0;
                    status.request_focus_screen = true;
                    Serial.println("[MQTT] 远程开启专注模式");
                } else {
                    Serial.println("[MQTT] 远程关闭专注模式");
                }
            }
            executed = true;
            result = "success";
        }
        // ---- 灯光控制 ----
        else if (strcmp(command, "light") == 0) {
            // TODO: 对接 BluetoothLight 模块
            Serial.printf("[MQTT] 灯光控制: %s（待实现）\n", value);
            executed = true;
            result = "success";
        }

        // 发送 ACK
        mqttSendControlAck(command, value, executed ? result : "failed");
        return;
    }

    // ---- OTA 固件更新响应（不自动更新，等待网页端确认）----
    if (doc["type"].is<const char*>() &&
        strcmp(doc["type"], "ota_update") == 0) {
        /* 只在本轮检测窗口内接受响应；超时/取消后到达的迟到消息直接丢弃，
           防止后端重发导致"请求-响应"死循环 */
        if (status.ota_check_status != 1) {
            Serial.println("[OTA] 收到迟到的 ota_update，已忽略（检测窗口已关闭）");
            return;
        }

        int code = doc["code"] | 0;
        const char* ota_url = doc["url"] | "";
        const char* ota_ver = doc["version"] | "";
        const char* ota_md5 = doc["md5"] | "";

        if (code == 200 && strlen(ota_url) > 0) {
            // 有新版本，存储待确认信息
            Serial.printf("[OTA] 收到新版本通知（等待用户确认）:\n");
            Serial.printf("  URL:     %s\n", ota_url);
            Serial.printf("  Version: %s\n", ota_ver);
            Serial.printf("  MD5:     %s\n", ota_md5);

            strncpy(status.ota_pending_url, ota_url, sizeof(status.ota_pending_url) - 1);
            status.ota_pending_url[sizeof(status.ota_pending_url) - 1] = '\0';
            strncpy(status.ota_pending_version, ota_ver, sizeof(status.ota_pending_version) - 1);
            status.ota_pending_version[sizeof(status.ota_pending_version) - 1] = '\0';
            strncpy(status.ota_pending_md5, ota_md5, sizeof(status.ota_pending_md5) - 1);
            status.ota_pending_md5[sizeof(status.ota_pending_md5) - 1] = '\0';
            status.ota_update_available = true;
            status.ota_check_status = 2;  // update_available

            mqttSendControlAck("ota_update", ota_ver, "pending_confirm");
        } else {
            // code==204 或 url 为空：已是最新版本
            Serial.println("[OTA] 服务器回复：已是最新版本");
            status.ota_check_status = 3;  // latest
        }
        return;
    }
}

void mqttInit() {
    mqttGenerateClientId();  // 先基于 MAC 地址生成 Client ID
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setBufferSize(512); // 默认256不够，控制指令/OTA消息可能超
    mqtt.setCallback(mqttCallback);
    mqtt.setSocketTimeout(2); // 默认15s阻塞致看门狗复位，限2秒

    snprintf(topic_status, sizeof(topic_status), "cube2026/device/%s/status", security.did);
    snprintf(topic_data, sizeof(topic_data), "cube2026/device/%s/data", security.did);
    snprintf(topic_control, sizeof(topic_control), "cube2026/server/%s/control", security.did);
    snprintf(topic_ack, sizeof(topic_ack), "cube2026/device/%s/ack", security.did);
}

bool mqttConnect(uint32_t timeout_ms) {
    if (mqtt.connected()) return true;

    // 关键修复：强制关闭旧 socket，防止异常断连后 WiFiClient 卡在 CLOSE_WAIT
    // 不清理的话，服务器崩溃/内网穿透断掉后永远连不上
    mqtt.disconnect();
    wifiClient.stop();
    delay(50);  // 等 socket 完全释放

    const uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (mqtt.connect(security.mqtt_client_id)) {  // 使用 MAC-based Client ID
            bool sub1 = mqtt.subscribe(topic_status);   // 接收 handshake_ack、heartbeat_ack
            bool sub2 = mqtt.subscribe(topic_data);     // 接收 data_report_ack
            bool sub3 = mqtt.subscribe(topic_control);  // 接收控制指令
            Serial.printf("[MQTT] 订阅结果: status=%d, data=%d, control=%d\n", sub1, sub2, sub3);

            // 等 broker 处理 SUBACK，避免握手 PUBLISH 先于订阅生效
            mqtt.loop();
            delay(300);
            mqtt.loop();

            Serial.println("[MQTT] 连接成功 ✅");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // 超时退出后直接打印失败并返回,不要再尝试连接
    Serial.printf("[MQTT] 连接超时,状态码: %d\n", mqtt.state());
    return false;
}

void mqttDisconnect() {
    mqtt.disconnect();
    wifiClient.stop();   // 确保底层 TCP socket 释放
}

bool mqttSubscribe(const char* topic) {
    return mqtt.subscribe(topic);
}

bool mqttPublish(const char* topic, const char* payload) {
    return mqtt.publish(topic, payload);
}

bool mqttIsConnected() {
    return mqtt.connected();
}

void mqttLoop() {
    mqtt.loop();
}

bool mqttSendHandshake() {
    JsonDocument doc;
    doc["device_id"] = security.did;
    doc["timestamp"] = time(NULL);
    doc["type"] = "handshake";
    doc["chip_model"] = "ESP32-S3";
    doc["version"] = FW_VERSION;  // 上报当前固件版本

    char json[512];
    serializeJson(doc, json);
    return mqttPublish(topic_status, json);
}

void mqttSendHeartbeat() {
    JsonDocument doc;
    doc["device_id"] = security.did;
    doc["token"] = security.token;
    doc["timestamp"] = time(NULL);
    doc["type"] = "heartbeat";

    doc["status"]["wifi_connected"] = status.wifi_connected;
    doc["status"]["mqtt_connected"] = mqttIsConnected();
    doc["status"]["screen_normal"] = true;
    doc["status"]["sensor_normal"] = (status.sensor_bh1750 && status.sensor_aht21 && status.sensor_ens160);
    doc["status"]["focus_mode"] = status.focus_mode;

    char json[512];
    serializeJson(doc, json);
    mqttPublish(topic_status, json);
}

void mqttSendVersionCheck() {
    JsonDocument doc;
    doc["device_id"] = security.did;
    doc["token"] = security.token;
    doc["timestamp"] = time(NULL);
    doc["type"] = "version_check";
    doc["current_version"] = FW_VERSION;

    char json[256];
    serializeJson(doc, json);
    mqttPublish(topic_status, json);
    Serial.println("[OTA] 版本检查请求已发送 📡");
}

void mqttSendDataReport() {
    JsonDocument doc;
    doc["device_id"] = security.did;
    doc["token"] = security.token;
    doc["timestamp"] = time(NULL);
    doc["type"] = "data_report";

    doc["data"]["temperature"] = sensorData.temp;
    doc["data"]["humidity"] = sensorData.humi;
    doc["data"]["illuminance"] = sensorData.light;
    doc["data"]["aqi"] = sensorData.aqi;
    doc["data"]["tvoc"] = sensorData.tvoc;
    doc["data"]["eco2"] = sensorData.eco2;
    doc["data"]["pm25"] = sensorData.pm25;  // AI 预测 PM2.5
    doc["data"]["mold_risk"] = 0;
    doc["data"]["gas"] = 0;
    doc["data"]["wifi_rssi"] = WiFi.RSSI();
    doc["data"]["version"] = FW_VERSION;

    doc["status"]["focus_mode"] = status.focus_mode;

    char json[1024];
    serializeJson(doc, json);
    mqttPublish(topic_data, json);
}

void mqttSendControlAck(const char* command, const char* value, const char* result) {
    JsonDocument doc;
    doc["device_id"] = security.did;
    doc["token"] = security.token;
    doc["timestamp"] = time(NULL);
    doc["type"] = "control_ack";
    doc["command"] = command;
    doc["value"] = value;
    doc["result"] = result;

    char json[512];
    serializeJson(doc, json);
    mqttPublish(topic_ack, json);
    Serial.printf("[MQTT] 控制ACK已发送: command=%s, result=%s\n", command, result);
}