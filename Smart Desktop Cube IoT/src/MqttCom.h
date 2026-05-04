#ifndef MQTT_COM_H
#define MQTT_COM_H
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>  
// 全局客户端对象
extern WiFiClient wifiClient;
extern PubSubClient mqtt;

// 初始化（设置服务器/回调，不连接）
void mqttInit();

// 带超时的连接（阻塞最多 timeout_ms 毫秒，内部使用 vTaskDelay 让出 CPU）
bool mqttConnect(uint32_t timeout_ms = 3000);

// 断开
void mqttDisconnect();

// 订阅
bool mqttSubscribe(const char* topic);

// 发布
bool mqttPublish(const char* topic, const char* payload);

// 状态查询
bool mqttIsConnected();

// 运行循环（处理接收，无阻塞）
void mqttLoop();

// 握手、心跳、数据上报
bool mqttSendHandshake();
void mqttSendHeartbeat();
void mqttSendDataReport();

#endif