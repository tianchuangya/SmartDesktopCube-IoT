#include "OTAManager.h"
#include "DataPool.h"
#include <HTTPClient.h>
#include <Update.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// ==================== 内部状态 ====================
static OtaProgress ota = {
    .state = OTA_IDLE,
    .progress = 0,
    .bytes_downloaded = 0,
    .total_bytes = 0,
    .new_version = {0},
    .error_msg = {0}
};

static SemaphoreHandle_t otaMutex = NULL;      // 保护 ota 结构体
static SemaphoreHandle_t otaTriggerSem = NULL;  // 触发信号量
static char trigger_url[256] = "";
static char trigger_version[32] = "";
static char trigger_md5[33] = "";

// ==================== 版本号比较 ====================
// 简单语义版本比较：1.2.3 或 v1.2.3 → (1,2,3)，按位比较
static int compareVersion(const char* v1, const char* v2) {
    int major1 = 0, minor1 = 0, patch1 = 0;
    int major2 = 0, minor2 = 0, patch2 = 0;
    // 跳过可能的 'v' 前缀
    if (v1 && v1[0] == 'v') v1++;
    if (v2 && v2[0] == 'v') v2++;
    sscanf(v1, "%d.%d.%d", &major1, &minor1, &patch1);
    sscanf(v2, "%d.%d.%d", &major2, &minor2, &patch2);

    if (major1 != major2) return major1 - major2;
    if (minor1 != minor2) return minor1 - minor2;
    return patch1 - patch2;
}

// ==================== 初始化 ====================
void otaInit() {
    otaMutex = xSemaphoreCreateMutex();
    otaTriggerSem = xSemaphoreCreateBinary();
}

// ==================== 触发 OTA ====================
bool otaTrigger(const char* url, const char* version, const char* md5) {
    if (otaIsInProgress()) {
        Serial.println("[OTA] ⚠ OTA 已在进行中，忽略重复触发");
        return false;
    }

    // 版本号检查：新版本必须大于当前版本
    if (version && strlen(version) > 0) {
        if (compareVersion(version, FW_VERSION) <= 0) {
            Serial.printf("[OTA] ⚠ 新版本 %s <= 当前版本 %s，跳过更新\n", version, FW_VERSION);
            return false;
        }
    }

    // 保存触发参数
    strncpy(trigger_url, url, sizeof(trigger_url) - 1);
    trigger_url[sizeof(trigger_url) - 1] = '\0';
    strncpy(trigger_version, version, sizeof(trigger_version) - 1);
    trigger_version[sizeof(trigger_version) - 1] = '\0';
    if (md5) {
        strncpy(trigger_md5, md5, sizeof(trigger_md5) - 1);
        trigger_md5[sizeof(trigger_md5) - 1] = '\0';
    } else {
        trigger_md5[0] = '\0';
    }

    // 更新状态
    xSemaphoreTake(otaMutex, portMAX_DELAY);
    ota.state = OTA_DOWNLOADING;
    ota.progress = 0;
    ota.bytes_downloaded = 0;
    ota.total_bytes = 0;
    strncpy(ota.new_version, version, sizeof(ota.new_version) - 1);
    ota.error_msg[0] = '\0';
    xSemaphoreGive(otaMutex);

    // 发送信号量唤醒 OTA 任务
    xSemaphoreGive(otaTriggerSem);

    Serial.printf("[OTA] 触发更新: %s → %s\nurl=%s\n", FW_VERSION, version, url);
    return true;
}

// ==================== 获取进度 ====================
OtaProgress otaGetProgress() {
    OtaProgress copy;
    xSemaphoreTake(otaMutex, portMAX_DELAY);
    copy = ota;
    xSemaphoreGive(otaMutex);
    return copy;
}

// ==================== 是否进行中 ====================
bool otaIsInProgress() {
    xSemaphoreTake(otaMutex, portMAX_DELAY);
    bool in_progress = (ota.state == OTA_DOWNLOADING ||
                        ota.state == OTA_VERIFYING ||
                        ota.state == OTA_WRITING);
    xSemaphoreGive(otaMutex);
    return in_progress;
}

// ==================== 设置错误 ====================
static void otaSetError(const char* msg) {
    xSemaphoreTake(otaMutex, portMAX_DELAY);
    ota.state = OTA_ERROR;
    strncpy(ota.error_msg, msg, sizeof(ota.error_msg) - 1);
    ota.error_msg[sizeof(ota.error_msg) - 1] = '\0';
    xSemaphoreGive(otaMutex);
    Serial.printf("[OTA] ❌ 错误: %s\n", msg);
}

// ==================== 辅助：更新共享状态（供 LVGL 任务读取）====================
static void otaSetStatusText(const char* text) {
    strncpy(status.ota_status_text, text, sizeof(status.ota_status_text) - 1);
    status.ota_status_text[sizeof(status.ota_status_text) - 1] = '\0';
}

// ==================== OTA 任务 ====================
// 注意：此任务不直接调用任何 LVGL 函数！
// 所有 UI 更新通过 status.ota_* 共享字段，由 custom.c 的 LVGL 定时器渲染。
void otaTaskFunc(void* pvParameters) {
    while (1) {
        // 等待触发信号
        if (xSemaphoreTake(otaTriggerSem, pdMS_TO_TICKS(500)) != pdTRUE) {
            continue;
        }

        Serial.println("[OTA] ========== OTA 更新开始 ==========");
        Serial.printf("[OTA] 当前版本: %s\n", FW_VERSION);

        // ---- 1. 设置共享状态，LVGL 任务会自动显示 OTA 屏幕 ----
        status.ota_in_progress = true;
        status.ota_progress = 0;
        strncpy(status.ota_new_version, trigger_version, sizeof(status.ota_new_version) - 1);
        status.ota_new_version[sizeof(status.ota_new_version) - 1] = '\0';
        otaSetStatusText("Connecting...");

        // ---- 2. HTTP 下载 + 流式写入 Flash ----
        HTTPClient http;
        http.setTimeout(30000);

        Serial.printf("[OTA] 正在连接: %s\n", trigger_url);
        bool connected = http.begin(trigger_url);

        if (!connected) {
            otaSetError("HTTP 连接失败");
            otaSetStatusText("Connect failed!");
            vTaskDelay(pdMS_TO_TICKS(5000));
            status.ota_in_progress = false;
            continue;
        }

        int httpCode = http.GET();
        if (httpCode != 200) {
            char buf[64];
            snprintf(buf, sizeof(buf), "HTTP error: %d", httpCode);
            otaSetError(buf);
            otaSetStatusText(buf);
            http.end();
            vTaskDelay(pdMS_TO_TICKS(5000));
            status.ota_in_progress = false;
            continue;
        }

        // 获取固件大小
        size_t totalSize = http.getSize();
        if (totalSize <= 0) {
            totalSize = 0x400000;  // 默认 4MB 最大值
        }

        Serial.printf("[OTA] 固件大小: %u 字节 (%.2f MB)\n", totalSize, totalSize / 1048576.0f);

        xSemaphoreTake(otaMutex, portMAX_DELAY);
        ota.total_bytes = totalSize;
        ota.state = OTA_DOWNLOADING;
        xSemaphoreGive(otaMutex);

        // ---- 3. 准备 Update（擦除 OTA 分区）----
        otaSetStatusText("Preparing flash...");
        if (!Update.begin(totalSize, U_FLASH)) {
            otaSetError("Flash 分区擦除失败");
            otaSetStatusText("Flash error!");
            http.end();
            vTaskDelay(pdMS_TO_TICKS(5000));
            status.ota_in_progress = false;
            continue;
        }

        // 设置期望 MD5
        if (strlen(trigger_md5) > 0) {
            Update.setMD5(trigger_md5);
            Serial.printf("[OTA] 期望 MD5: %s\n", trigger_md5);
        }

        // ---- 4. 流式下载 + 写入 ----
        WiFiClient* stream = http.getStreamPtr();
        uint8_t buffer[4096];
        size_t downloaded = 0;
        int lastProgress = -1;
        bool downloadOk = true;

        xSemaphoreTake(otaMutex, portMAX_DELAY);
        ota.state = OTA_WRITING;
        xSemaphoreGive(otaMutex);

        while (http.connected() && downloaded < totalSize) {
            size_t available = stream->available();
            if (available == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            size_t toRead = (available > sizeof(buffer)) ? sizeof(buffer) : available;
            if (toRead > (totalSize - downloaded)) {
                toRead = totalSize - downloaded;
            }

            int bytesRead = stream->readBytes(buffer, toRead);
            if (bytesRead <= 0) {
                if (downloaded >= totalSize) break;
                downloadOk = false;
                break;
            }

            // 分块写入 Flash
            size_t written = Update.write(buffer, bytesRead);
            if (written != (size_t)bytesRead) {
                char buf[64];
                snprintf(buf, sizeof(buf), "Flash write fail @%u", downloaded);
                otaSetError(buf);
                otaSetStatusText("Write failed!");
                downloadOk = false;
                break;
            }

            downloaded += bytesRead;

            // 更新共享进度
            int pct = (totalSize > 0) ? (int)(downloaded * 100 / totalSize) : 0;
            xSemaphoreTake(otaMutex, portMAX_DELAY);
            ota.bytes_downloaded = downloaded;
            ota.progress = pct;
            xSemaphoreGive(otaMutex);

            // 更新 status 共享字段（LVGL 任务读取）
            if (pct != lastProgress) {
                lastProgress = pct;
                status.ota_progress = pct;
                char st[64];
                snprintf(st, sizeof(st), "Downloading... %d%%", pct);
                otaSetStatusText(st);
                Serial.printf("[OTA] 进度: %d%% (%u/%u)\n", pct, downloaded, totalSize);
            }

            // 让出 CPU
            vTaskDelay(pdMS_TO_TICKS(5));
        }

        http.end();

        if (!downloadOk) {
            Update.abort();
            vTaskDelay(pdMS_TO_TICKS(5000));
            status.ota_in_progress = false;
            continue;
        }

        // ---- 5. 校验 & 完成 ----
        xSemaphoreTake(otaMutex, portMAX_DELAY);
        ota.state = OTA_VERIFYING;
        ota.progress = 100;
        xSemaphoreGive(otaMutex);

        status.ota_progress = 100;
        otaSetStatusText("Verifying...");
        Serial.println("[OTA] 固件下载完成，正在校验...");

        if (!Update.end()) {
            char buf[64];
            snprintf(buf, sizeof(buf), "Verify fail: %s", Update.errorString());
            otaSetError(buf);
            otaSetStatusText("Verify failed!");
            Serial.printf("[OTA] ❌ %s\n", buf);
            vTaskDelay(pdMS_TO_TICKS(5000));
            status.ota_in_progress = false;
            continue;
        }

        // ---- 6. 成功！准备重启 ----
        xSemaphoreTake(otaMutex, portMAX_DELAY);
        ota.state = OTA_SUCCESS;
        xSemaphoreGive(otaMutex);

        status.ota_progress = 100;
        otaSetStatusText("Done! Rebooting...");
        Serial.println("[OTA] ✅ 固件更新成功！3秒后重启...");
        vTaskDelay(pdMS_TO_TICKS(3000));

        ESP.restart();
    }
}
