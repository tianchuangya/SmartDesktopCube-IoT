#include "ControlPoll.h"
#include "DataPool.h"
#include "focus_mode.h"
#include "LocalIntelligence.h"
#include "secrets.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ==================== 轮询配置 ====================
#define POLL_INTERVAL_MS   5000   // 5 秒轮询一次
#define HTTP_TIMEOUT_MS    8000   // HTTP 请求超时 8 秒

static uint32_t last_poll_ms = 0;

// ==================== 静态 TLS 连接（复用，避免反复分配 ~16KB） ====================
static WiFiClientSecure s_client;
static bool s_client_init = false;

// ==================== 延迟 ACK 队列（ACK 失败时暂存，下次重试） ====================
static bool   s_ack_pending = false;
static char   s_ack_command[32] = {0};
static char   s_ack_result[16]  = {0};

// 确保 TLS 客户端已初始化（只执行一次）
static void ensureClient(void) {
    if (!s_client_init) {
        s_client.setInsecure();
        s_client_init = true;
    }
}

// ==================== 内部：发送 ACK ====================
// 返回 true=成功, false=失败（需延迟重试）
static bool sendAck(const char* command, const char* result)
{
    if (!WiFi.isConnected()) return false;
    if (strlen(security.token) == 0) return false;

    ensureClient();

    // 断开旧连接，复用同一 TLS session slot
    if (s_client.connected()) s_client.stop();

    HTTPClient http;
    char url[256];
    snprintf(url, sizeof(url), "%s/api/v1/control/%s/ack",
             BACKEND_BASE_URL, security.did);

    if (!http.begin(s_client, url)) {
        Serial.println("[ControlPoll] ACK begin failed");
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.setTimeout(HTTP_TIMEOUT_MS);

    static char body[256];  // 静态缓冲区，零堆分配
    snprintf(body, sizeof(body),
        "{\"token\":\"%s\",\"timestamp\":%ld,\"type\":\"control_ack\","
        "\"command\":\"%s\",\"result\":\"%s\"}",
        security.token, (long)time(NULL), command, result);

    int code = http.POST(body);
    http.end();

    if (code == HTTP_CODE_OK || code == HTTP_CODE_CREATED) {
        Serial.printf("[ControlPoll] ACK sent: %s -> %d\n", command, code);
        return true;
    } else {
        Serial.printf("[ControlPoll] ACK failed: code=%d %s\n",
                      code, code > 0 ? "" : http.errorToString(code).c_str());
        return false;
    }
}

// ==================== 内部：执行控制指令 ====================
static void executeCommand(const char* command, const char* value)
{
    bool success = false;

    if (strcmp(command, "focus_mode") == 0) {
        if (strcmp(value, "on") == 0) {
            status.focus_mode = true;
            status.request_focus_screen = true;
            sensorData.focus_duration = 0;
            focusMode_notifyManualEnter();
            success = true;
            Serial.println("[ControlPoll] 远程开启专注模式");
        } else if (strcmp(value, "off") == 0) {
            FocusSession_End();
            status.focus_mode = false;
            sensorData.focus_duration = 0;
            success = true;
            Serial.println("[ControlPoll] 远程关闭专注模式");
        }
    }
    // 未来可扩展更多指令类型
    // else if (strcmp(command, "xxx") == 0) { ... }

    // 尝试发送 ACK，失败则暂存到延迟队列
    if (!sendAck(command, success ? "success" : "failed")) {
        s_ack_pending = true;
        strncpy(s_ack_command, command, sizeof(s_ack_command) - 1);
        s_ack_command[sizeof(s_ack_command) - 1] = '\0';
        strncpy(s_ack_result, success ? "success" : "failed", sizeof(s_ack_result) - 1);
        s_ack_result[sizeof(s_ack_result) - 1] = '\0';
        Serial.println("[ControlPoll] ACK 已暂存，下次轮询重试");
    }
}

// ==================== 公共 API ====================
void ControlPoll_Init(void)
{
    last_poll_ms = 0;
    s_ack_pending = false;
    ensureClient();
    Serial.println("[ControlPoll] HTTP 控制轮询模块已初始化");
}

void ControlPoll_Check(void)
{
    // 时间间隔检查
    uint32_t now = millis();
    if (now - last_poll_ms < POLL_INTERVAL_MS) return;
    last_poll_ms = now;

    // 前置条件：WiFi 已连接 + Token 有效
    if (!WiFi.isConnected()) return;
    if (strlen(security.token) == 0) return;

    // ---- 优先重试上次失败的 ACK ----
    if (s_ack_pending) {
        if (sendAck(s_ack_command, s_ack_result)) {
            s_ack_pending = false;
            Serial.println("[ControlPoll] 延迟 ACK 重试成功");
        }
        // 即使 ACK 重试失败，也继续执行 pull 拉取新指令
    }

    ensureClient();
    if (s_client.connected()) s_client.stop();  // 断开旧连接，准备新请求

    HTTPClient http;

    // 构建 Pull URL
    char url[256];
    snprintf(url, sizeof(url), "%s/api/v1/control/%s/pull",
             BACKEND_BASE_URL, security.did);

    if (!http.begin(s_client, url)) {
        Serial.println("[ControlPoll] HTTP begin failed");
        return;
    }

    // 设置请求头
    char auth_header[128];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", security.token);
    http.addHeader("Authorization", auth_header);
    http.setTimeout(HTTP_TIMEOUT_MS);

    // 发送 GET 请求
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        if (httpCode > 0) {
            Serial.printf("[ControlPoll] Pull failed: HTTP %d\n", httpCode);
        } else {
            Serial.printf("[ControlPoll] Pull error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        return;
    }

    // 解析响应（静态缓冲区，避免 String 堆分配）
    static char payload[1024];
    WiFiClient *stream = http.getStreamPtr();
    size_t len = stream->readBytes(payload, sizeof(payload) - 1);
    payload[len] = '\0';
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("[ControlPoll] JSON parse error: %s\n", err.c_str());
        return;
    }

    // 检查 code == 0
    int code = doc["code"] | -1;
    if (code != 0) {
        Serial.printf("[ControlPoll] Server error: code=%d, msg=%s\n",
                      code, doc["msg"].is<const char*>() ? doc["msg"].as<const char*>() : "");
        return;
    }

    // 检查是否有待执行指令
    bool pending = doc["data"]["pending"] | false;
    if (!pending) return;

    // 读取指令
    const char* command = doc["data"]["command"] | "";
    const char* value = doc["data"]["value"] | "";

    Serial.printf("[ControlPoll] 收到指令: command=%s, value=%s\n", command, value);

    // 执行指令
    if (strlen(command) > 0) {
        executeCommand(command, value);
    }
}
