#include "MqttCom.h"
#include "DataPool.h"
#include <ArduinoJson.h>
#include "time.h"
#include <WiFi.h>
WiFiClient wifiClient; 
PubSubClient mqtt(wifiClient);

#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT   1883

static char topic_status[64];
static char topic_data[64];
static char topic_control[64];

// 下行消息回调
static void mqttCallback(char* topic, byte* payload, unsigned int len) {
    char buffer[512];
    memcpy(buffer, payload, len);
    buffer[len] = 0;

    JsonDocument doc;
    if (deserializeJson(doc, buffer)) return;

    if (doc["lock"].is<bool>()) {
    status.device_lock = doc["lock"].as<bool>();
    }
    if (doc["focus"].is<bool>()) {
        bool new_focus = doc["focus"].as<bool>();
        status.focus_mode = new_focus;
        if (new_focus) {
            sensorData.focus_duration = 0;
        } else {
            sensorData.focus_duration = 0;
        }
}
}

void mqttInit() {
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);

    snprintf(topic_status, sizeof(topic_status), "cube2026/device/%s/status", security.did);
    snprintf(topic_data, sizeof(topic_data), "cube2026/device/%s/data", security.did);
    snprintf(topic_control, sizeof(topic_control), "cube2026/server/%s/control", security.did);
}

bool mqttConnect(uint32_t timeout_ms) {
    if (mqtt.connected()) return true;

    const uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (mqtt.connect(security.did)) {
            mqtt.subscribe(topic_control);
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
    doc["version"] = "1.0";

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
    doc["status"]["sensor_normal"] = true;
    doc["status"]["focus_mode"] = status.focus_mode;

    char json[512];
    serializeJson(doc, json);
    mqttPublish(topic_status, json);
}

void mqttSendDataReport() {
    JsonDocument doc;
    doc["device_id"] = security.did;
    doc["token"] = security.token;
    doc["timestamp"] = time(NULL);
    doc["type"] = "data_report";

    doc["data"]["data"]["temperature"] = sensorData.temp;
    doc["data"]["data"]["humidity"] = sensorData.humi;
    doc["data"]["data"]["illuminance"] = sensorData.light;
    doc["data"]["data"]["aqi"] = sensorData.aqi;
    doc["data"]["data"]["tvoc"] = sensorData.tvoc;
    doc["data"]["data"]["eco2"] = sensorData.eco2;
    doc["data"]["data"]["mold_risk"] = 0;
    doc["data"]["data"]["gas"] = 0;
    doc["data"]["data"]["version"] = "1.0";

    doc["data"]["status"]["wifi_connected"] = status.wifi_connected;
    doc["data"]["status"]["mqtt_connected"] = mqttIsConnected();
    doc["data"]["status"]["screen_normal"] = true;
    doc["data"]["status"]["sensor_normal"] = true;
    doc["data"]["status"]["focus_mode"] = status.focus_mode;

    char json[1024];
    serializeJson(doc, json);
    mqttPublish(topic_data, json);
}