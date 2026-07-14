#include "BluetoothLight.h"
#include <NimBLEDevice.h>

// ==================== 静态/全局变量 ====================

BluetoothLightState blLight = {
    .connected = false,
    .brightness = 80,
    .color_temperature = 50,
    .target_brightness = 80,
    .target_color_temp = 50,
    .pending_update = false,
    .connection_attempts = 0,
    .last_fail_time = 0,
    .last_connect_failed = false
};

// 上次非零亮度（用于关灯后恢复）
static uint8_t lastBrightness = 80;

// BLE 连接对象
static NimBLEClient*  pClient  = nullptr;
static NimBLERemoteCharacteristic* pCharBrightness = nullptr;
static NimBLERemoteCharacteristic* pCharColorTemp  = nullptr;

// 任务句柄
static TaskHandle_t blTaskHandle = nullptr;

// 扫描到目标设备的地址
static NimBLEAddress targetAddr;

// ==================== 内部实现 ====================

// 色温数值(0-100) 转 开尔文温度
uint16_t BL_ColorTempToKelvin(uint8_t ct) {
    // 0 -> 2700K, 100 -> 6500K
    return 2700 + (uint16_t)ct * 38;  // (6500-2700)/100 = 38
}

// 扫描回调：查找名为 "SmartCube-Light" 的设备
class LightScanCallbacks : public NimBLEScanCallbacks {
    bool found = false;
    NimBLEAddress foundAddr;

public:
    void onResult(const NimBLEAdvertisedDevice* device) {
        if (device->haveName() && device->getName() == BLE_LIGHT_DEVICE_NAME) {
            found = true;
            foundAddr = device->getAddress();
            Serial.printf("[BLE Scan] ✓ 发现目标设备: %s (%s), RSSI=%d\n",
                          device->getName().c_str(),
                          device->getAddress().toString().c_str(),
                          device->getRSSI());
        }
    }

    bool isFound() { return found; }
    NimBLEAddress getAddr() { return foundAddr; }
};

// 扫描寻找灯光控制器
static bool BL_ScanForDevice() {
    Serial.println("[BLE Scan] 开始扫描灯光控制器...");

    NimBLEScan* pScan = NimBLEDevice::getScan();
    LightScanCallbacks* callbacks = new LightScanCallbacks();
    pScan->setScanCallbacks(callbacks, false);
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(80);

    NimBLEScanResults results = pScan->getResults(5);  // 扫描5秒

    if (callbacks->isFound()) {
        targetAddr = callbacks->getAddr();
        delete callbacks;
        Serial.printf("[BLE Scan] ✓ 成功找到设备，地址: %s\n", targetAddr.toString().c_str());
        return true;
    }

    delete callbacks;
    Serial.println("[BLE Scan] ✗ 未找到灯光控制器 (SmartCube-Light)");
    return false;
}

// 实际发送BLE指令
static bool BL_SendCommand(uint8_t brightness, uint8_t color_temp) {
    if (!blLight.connected) {
        return false;
    }

    bool ok = true;

    // 写入亮度
    if (pCharBrightness) {
        uint8_t val = brightness;
        if (!pCharBrightness->writeValue(&val, 1, false)) {
            Serial.println("[BLE Light] ✗ 写入亮度失败!");
            ok = false;
        }
    } else {
        ok = false;
    }

    // 写入色温
    if (pCharColorTemp) {
        uint8_t val = color_temp;
        if (!pCharColorTemp->writeValue(&val, 1, false)) {
            Serial.println("[BLE Light] ✗ 写入色温失败!");
            ok = false;
        }
    } else {
        ok = false;
    }

    if (ok) {
        blLight.brightness = brightness;
        blLight.color_temperature = color_temp;
        blLight.pending_update = false;
        Serial.printf("[BLE Light] ✓ 已发送: 亮度=%d%%, 色温=%d (%dK)\n",
                      brightness, color_temp, BL_ColorTempToKelvin(color_temp));
    }

    return ok;
}

// 连接到灯具
static bool BL_DoConnect() {
    if (blLight.connected) return true;

    // 第一步：扫描设备
    if (!BL_ScanForDevice()) {
        Serial.println("[BLE Light] ✗ 扫描失败!");
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    Serial.printf("[BLE Light] 正在连接 %s ...\n", targetAddr.toString().c_str());

    // 第二步：连接
    if (!pClient) {
        pClient = NimBLEDevice::createClient();
        pClient->setConnectTimeout(10);  // 10秒超时
    }

    if (!pClient->connect(targetAddr)) {
        Serial.println("[BLE Light] ✗ 连接失败!");
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    Serial.println("[BLE Light] ✓ 已连接，正在发现服务...");

    // 第三步：获取服务
    NimBLERemoteService* pSrv = pClient->getService(BLE_SRV_UUID);
    if (!pSrv) {
        Serial.println("[BLE Light] ✗ 未找到目标服务!");
        pClient->disconnect();
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    // 第四步：获取亮度特征
    pCharBrightness = pSrv->getCharacteristic(BLE_CHAR_BRIGHTNESS);
    if (!pCharBrightness) {
        Serial.println("[BLE Light] ✗ 未找到亮度特征!");
        pClient->disconnect();
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    // 第五步：获取色温特征
    pCharColorTemp = pSrv->getCharacteristic(BLE_CHAR_COLORTEMP);
    if (!pCharColorTemp) {
        Serial.println("[BLE Light] ✗ 未找到色温特征!");
        pClient->disconnect();
        blLight.last_connect_failed = true;
        blLight.last_fail_time = millis();
        blLight.connection_attempts++;
        return false;
    }

    blLight.connected = true;
    blLight.connection_attempts = 0;       // 连接成功，重置失败计数
    blLight.last_connect_failed = false;
    Serial.println("[BLE Light] ✓ 连接并发现服务完成!");
    return true;
}

// 断开连接
static void BL_DoDisconnect() {
    if (pClient && pClient->isConnected()) {
        pClient->disconnect();
    }
    pCharBrightness = nullptr;
    pCharColorTemp  = nullptr;
    blLight.connected = false;
    blLight.pending_update = false;
    Serial.println("[BLE Light] 已断开连接");
}

// ==================== BLE 管理任务 ====================
static void BL_Task(void* pvParameters) {
    uint32_t lastConnectAttempt = 0;
    const uint32_t reconnectInterval = 5000;  // 连接失败后5秒重试

    while (1) {
        uint32_t now = millis();

        // 保持连接 & 自动重连
        if (!blLight.connected) {
            if (now - lastConnectAttempt > reconnectInterval) {
                lastConnectAttempt = now;
                BL_DoConnect();
            }
        } else {
            // 已连接：检查是否有待发送的更新
            if (blLight.pending_update) {
                if (BL_SendCommand(blLight.target_brightness, blLight.target_color_temp)) {
                    // 发送成功
                } else {
                    // 发送失败，标记断开
                    Serial.println("[BLE Light] 发送失败，连接可能已断开");
                    BL_DoDisconnect();
                }
            }

            // 检查连接是否仍然有效
            if (pClient && !pClient->isConnected()) {
                Serial.println("[BLE Light] 检测到连接断开");
                BL_DoDisconnect();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));  // 200ms周期检查
    }
}

// ==================== 公开API ====================

void BL_Init() {
    Serial.println("[BLE Light] 初始化BLE...");
    NimBLEDevice::init("SmartCube-Client");

    // 创建BLE管理任务（运行在核心1，优先级2）
    xTaskCreatePinnedToCore(
        BL_Task,
        "BLE_Light",
        8192,
        NULL,
        2,
        &blTaskHandle,
        1  // 核心1（应用层）
    );

    Serial.println("[BLE Light] 初始化完成，后台任务已启动");
}

void BL_SetBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    blLight.target_brightness = brightness;
    blLight.pending_update = true;
    if (brightness > 0) lastBrightness = brightness;
}

void BL_SetColorTemp(uint8_t color_temp) {
    if (color_temp > 100) color_temp = 100;
    blLight.target_color_temp = color_temp;
    blLight.pending_update = true;
}

void BL_SetLight(uint8_t brightness, uint8_t color_temp) {
    if (brightness > 100) brightness = 100;
    if (color_temp > 100) color_temp = 100;
    blLight.target_brightness = brightness;
    blLight.target_color_temp = color_temp;
    blLight.pending_update = true;
    if (brightness > 0) lastBrightness = brightness;

    if (!blLight.connected) {
        Serial.println("[BLE Light] ⚠ 警告: 灯具未连接，指令将在连接成功后自动发送");
    }
}

// ==================== 一键预设模板 ====================

void BL_PresetWarm() {
    // 暖光：亮度100%，色温≈3000K (ct=8)
    BL_SetLight(100, 8);
    Serial.println("[BLE Light] 预设：暖光模式 (100%, ~3000K)");
}

void BL_PresetCool() {
    // 冷光：亮度80%，色温≈6500K (ct=100)
    BL_SetLight(80, 100);
    Serial.println("[BLE Light] 预设：冷光模式 (80%, ~6500K)");
}

void BL_PresetWhite() {
    // 白光：亮度80%，色温≈4500K (ct=47)
    BL_SetLight(80, 47);
    Serial.println("[BLE Light] 预设：白光模式 (80%, ~4500K)");
}

void BL_TurnOff() {
    if (blLight.brightness > 0) {
        lastBrightness = blLight.brightness;
    }
    BL_SetBrightness(0);
    Serial.println("[BLE Light] 关灯");
}

void BL_TurnOn() {
    BL_SetBrightness(lastBrightness > 0 ? lastBrightness : 80);
    Serial.printf("[BLE Light] 开灯 (亮度=%d%%)\n", lastBrightness);
}

bool BL_IsConnected() {
    return blLight.connected;
}

// 读取并清除"连接失败"标记（UI 轮询用）
bool BL_ConsumeConnectFailedFlag() {
    bool failed = blLight.last_connect_failed;
    blLight.last_connect_failed = false;  // 消费后清零，避免重复弹窗
    return failed;
}

const char* BL_GetStatusMsg() {
    if (blLight.connected) {
        return "已连接";
    }
    if (blLight.connection_attempts > 0) {
        return "连接失败";
    }
    return "未连接";
}
