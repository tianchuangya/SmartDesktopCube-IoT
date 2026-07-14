/*
 * ESP32-S3 双色温灯光控制器 (BLE GATT Server)
 * ============================================
 * 支持 ESP32-S3 / ESP32-C3，用多余的板子即可
 * 功能：接收 BLE 指令，用两路 PWM 驱动暖白 + 冷白 LED 灯带
 *
 * 硬件接线（非常简单！）：
 *   ESP32 GPIO5     →  MOS管模块1 信号脚  →  暖白LED灯带
 *   ESP32 GPIO6     →  MOS管模块2 信号脚  →  冷白LED灯带
 *   ESP32 GND       →  MOS管模块 GND
 *   MOS管模块 VCC   →  灯带电源正极（5V/12V）
 *   LED灯带正极     →  灯带电源正极
 *   灯带电源负极    →  MOS管模块输出端
 *
 * BLE 服务（自定义）：
 *   Service UUID:  12345678-1234-1234-1234-123456789abc
 *   Char1 UUID:    12345678-1234-1234-1234-123456789abd  (读写: 亮度 0-100)
 *   Char2 UUID:    12345678-1234-1234-1234-123456789abe  (读写: 色温 0-100)
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ==================== UUID 定义 ====================
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHAR_BRIGHTNESS_UUID "12345678-1234-1234-1234-123456789abd"
#define CHAR_COLORTEMP_UUID  "12345678-1234-1234-1234-123456789abe"

// ==================== PWM 引脚（适用于 ESP32-S3 / ESP32-C3）====================
#define PWM_WARM_PIN   5    // 暖白 LED → MOS管1
#define PWM_COLD_PIN   6    // 冷白 LED → MOS管2
#define PWM_FREQ       5000 // PWM 频率 5kHz（无频闪）
#define PWM_RESOLUTION 8    // 8位分辨率 (0-255)

// ==================== 当前状态 ====================
uint8_t currentBrightness = 80;  // 默认亮度 80%
uint8_t currentColorTemp  = 50;  // 默认色温 50 (约4500K中性光)
// 上次非零亮度（关灯后恢复用）
uint8_t lastBrightness = 80;

// ==================== PWM 输出 ====================
void updatePWM() {
  // 根据色温计算暖白和冷白的混合比例
  // colorTemp = 0   → 100%暖白, 0%冷白 (约2700K)
  // colorTemp = 50  → 50%暖白, 50%冷白 (约4500K)
  // colorTemp = 100 → 0%暖白, 100%冷白 (约6500K)

  float warmRatio = (100.0 - currentColorTemp) / 100.0;
  float coldRatio = currentColorTemp / 100.0;

  int warmPWM = (int)(currentBrightness * warmRatio * 255 / 100);
  int coldPWM = (int)(currentBrightness * coldRatio * 255 / 100);

  ledcWrite(0, warmPWM);  // 通道0 → GPIO3
  ledcWrite(1, coldPWM);  // 通道1 → GPIO4

  Serial.printf("[LED] 亮度=%d%% 色温=%d → 暖白PWM=%d 冷白PWM=%d\n",
                currentBrightness, currentColorTemp, warmPWM, coldPWM);
}

// ==================== BLE 回调 ====================

// 亮度写入回调
class BrightnessCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    uint8_t val = pChar->getValue()[0];
    if (val <= 100) {
      currentBrightness = val;
      if (val > 0) lastBrightness = val;
      updatePWM();
    }
    Serial.printf("[BLE] 收到亮度指令: %d%%\n", val);
  }

  void onRead(BLECharacteristic *pChar) {
    pChar->setValue(&currentBrightness, 1);
  }
};

// 色温写入回调
class ColorTempCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    uint8_t val = pChar->getValue()[0];
    if (val <= 100) {
      currentColorTemp = val;
      updatePWM();
    }
    Serial.printf("[BLE] 收到色温指令: %d\n", val);
  }

  void onRead(BLECharacteristic *pChar) {
    pChar->setValue(&currentColorTemp, 1);
  }
};

// 连接回调
class ServerConnCallback : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("[BLE] ✓ 客户端已连接");
  }

  void onDisconnect(BLEServer *pServer) {
    Serial.println("[BLE] 客户端已断开，继续广播等待连接...");
    // 重新开始广播，等待下一次连接
    BLEDevice::startAdvertising();
  }
};

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n====================================");
  Serial.println("  双色温灯光控制器 BLE Server");
  Serial.println("  ESP32-C3 启动中...");
  Serial.println("====================================");

  // 初始化 PWM
  ledcSetup(0, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_WARM_PIN, 0);
  ledcAttachPin(PWM_COLD_PIN, 1);
  // 默认输出
  updatePWM();

  // 初始化 BLE
  BLEDevice::init("SmartCube-Light");  // BLE 设备名

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerConnCallback());

  // 创建服务
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 创建亮度特征 (可读可写)
  BLECharacteristic *pBrightness = pService->createCharacteristic(
      CHAR_BRIGHTNESS_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pBrightness->setCallbacks(new BrightnessCallback());
  pBrightness->setValue(&currentBrightness, 1);
  pBrightness->addDescriptor(new BLE2902());  // 支持 CCCD

  // 创建色温特征 (可读可写)
  BLECharacteristic *pColorTemp = pService->createCharacteristic(
      CHAR_COLORTEMP_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pColorTemp->setCallbacks(new ColorTempCallback());
  pColorTemp->setValue(&currentColorTemp, 1);
  pColorTemp->addDescriptor(new BLE2902());

  // 启动服务
  pService->start();

  // 开始广播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] 服务已启动，正在广播...");
  Serial.println("[BLE] 设备名: SmartCube-Light");
  Serial.print(  "[BLE] Service UUID: "); Serial.println(SERVICE_UUID);
  Serial.println("====================================\n");
}

void loop() {
  delay(1000);  // 主循环什么都不用做，BLE 和 PWM 都是异步的
}
