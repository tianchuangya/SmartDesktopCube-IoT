#include "DataRW.h"
#include "BH1750.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>
#include "DataPool.h"
#include "PM25Predictor.h"
// 光照传感器硬件
extern TwoWire Wire_BH1750;
extern BH1750 bh1750;
extern Adafruit_AHTX0 aht;
extern DFRobot_ENS160_I2C* ens_ptr;

// 根据浓度计算单个污染物的AQI分段值（线性插值）
static int calcAQIForPollutant(float conc, const float bp[][2], int n) {
    if (conc <= bp[0][0]) return bp[0][1];  // 低于最低阈值
    for (int i = 0; i < n - 1; i++) {
        if (conc <= bp[i + 1][0]) {
            // 线性插值: AQI = AQI_low + (AQI_high - AQI_low) / (conc_high - conc_low) * (conc - conc_low)
            float ratio = (conc - bp[i][0]) / (bp[i + 1][0] - bp[i][0]);
            return (int)(bp[i][1] + ratio * (bp[i + 1][1] - bp[i][1]));
        }
    }
    return bp[n - 1][1];  // 超过最高阈值
}

// 根据TVOC/eCO2计算综合AQI (0-500+)，取两者中更差的值
static int calcAQI(float tvoc, float eco2)
{
    // eCO2 断点: {浓度(ppm), AQI值}
    const float eco2_bp[][2] = {
        {400,   0},   // 优
        {600,  50},   // 优 → 良
        {1000,100},   // 良 → 轻度
        {1500,150},   // 轻度 → 中度
        {2000,200},   // 中度 → 重度
        {5000,300},   // 重度 → 严重
        {9999,500}    // 严重上限
    };

    // TVOC 断点: {浓度(ppb), AQI值}
    const float tvoc_bp[][2] = {
        {0,    0},
        {65,  50},
        {220,100},
        {660,150},
        {2200,200},
        {5500,300},
        {9999,500}
    };

    int aqi_eco2 = calcAQIForPollutant(eco2, eco2_bp, 7);
    int aqi_tvoc = calcAQIForPollutant(tvoc, tvoc_bp, 7);
    return (aqi_eco2 > aqi_tvoc) ? aqi_eco2 : aqi_tvoc;
}

// 读取 ENS160 状态（调试用）
static void printENS160Status(void) {
    if (ens_ptr == nullptr) return;
    uint8_t status = ens_ptr->getENS160Status();
    Serial.printf("[ENS160] 状态: 0x%02X", status);
    if (status & 0x01) Serial.print(" | 正常运行");
    else                Serial.print(" | 未就绪");
    if (status & 0x02) Serial.print(" | 错误");
    // 数据有效性: bits 2-3
    uint8_t valid = (status >> 2) & 0x03;
    switch (valid) {
        case 0: Serial.print(" | 数据有效"); break;
        case 1: Serial.print(" | 预热中(1)"); break;
        case 2: Serial.print(" | 初始启动(2)"); break;
        case 3: Serial.print(" | 数据无效"); break;
    }
    Serial.println();
}

// 读取所有传感器数据 → 数据池
void DataRead_ReadAll(void)
{
    status.run_seconds = (millis() - status.boot_time) / 1000;

    // ==================== 读取 Bh1750光照 ====================
    if (status.sensor_bh1750) {
        sensorData.light = bh1750.readLightLevel();
    } else {
        sensorData.light = -1;
    }

    // ==================== 读取 AHT20 温湿度 ====================
    if (status.sensor_aht21) {
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);
        sensorData.temp = temp.temperature;
        sensorData.humi = humidity.relative_humidity;
    } else {
        sensorData.temp = -99;
        sensorData.humi = -99;
    }

    // ==================== 读取 ENS160 空气质量 ====================
    static bool ens160_status_printed = false;
    static uint32_t last_compensate = 0;       // 上次温湿度补偿时间
    static int ens160_error_count = 0;         // 连续错误计数
    if (status.sensor_ens160 && ens_ptr != nullptr) {
        // 每 60 秒补偿一次温湿度（太频繁反而影响传感器）
        if (status.sensor_aht21 && millis() - last_compensate > 60000) {
            last_compensate = millis();
            ens_ptr->setTempAndHum(sensorData.temp, sensorData.humi);
        }

        sensorData.tvoc = ens_ptr->getTVOC();
        sensorData.eco2 = ens_ptr->getECO2();

        // 检测传感器是否异常（连续返回 0 或 错误状态）
        uint8_t status_reg = ens_ptr->getENS160Status();
        bool data_invalid = (status_reg & 0x02) ||                          // 错误标志
                           ((status_reg >> 2) & 0x03) == 3 ||               // 数据无效
                           (sensorData.tvoc == 0 && sensorData.eco2 == 0);  // 都为零

        if (data_invalid) {
            ens160_error_count++;
            if (ens160_error_count == 1) {
                Serial.printf("[ENS160] ⚠ 异常! 状态=0x%02X, TVOC=%.0f, eCO2=%.0f\n",
                              status_reg, sensorData.tvoc, sensorData.eco2);
            }
            // 连续 10 次异常（约 10 秒），尝试复位传感器
            if (ens160_error_count >= 10) {
                Serial.println("[ENS160] 🔄 连续异常，尝试复位...");
                ens_ptr->setPWRMode(ENS160_IDLE_MODE);   // 先切到空闲模式
                delay(50);
                ens_ptr->setPWRMode(ENS160_STANDARD_MODE); // 再切回标准模式
                delay(200);
                ens_ptr->setTempAndHum(
                    status.sensor_aht21 ? sensorData.temp : 25.0f,
                    status.sensor_aht21 ? sensorData.humi : 50.0f);
                ens160_error_count = 0;
                ens160_status_printed = false;
            }
            sensorData.aqi = 0;
        } else {
            ens160_error_count = 0;
            sensorData.aqi = calcAQI(sensorData.tvoc, sensorData.eco2);
        }

        // 首次读取时打印状态，诊断预热问题
        if (!ens160_status_printed) {
            printENS160Status();
            if (sensorData.tvoc == 0 && sensorData.eco2 == 0) {
                Serial.println("[ENS160] ⚠ 传感器正在预热中，大约需要 3 分钟，请耐心等待...");
            }
            ens160_status_printed = true;
        }
    } else {
        sensorData.tvoc = -1;
        sensorData.eco2 = -1;
        sensorData.aqi  = 0;
    }

    // ==================== PM2.5 预测（4特征: 温度/CO2/TVOC/湿度）====================
    if (status.sensor_aht21 && status.sensor_ens160 &&
        sensorData.temp >= 0 && sensorData.eco2 >= 0 &&
        sensorData.tvoc >= 0 && sensorData.humi >= 0) {
        sensorData.pm25 = pm25Predict(sensorData.temp, sensorData.eco2,
                                      sensorData.tvoc, sensorData.humi);
    } else {
        sensorData.pm25 = -1;
    }
}



// 输出数据 → 串口
void ALLData_ToSerial(void)
{
    Serial.print("Time:");
    Serial.print(status.run_seconds);
    Serial.println("===== 传感器数据 =====");
    // 光照
    if (status.sensor_bh1750)
        Serial.printf("光照: %.2f lx | ", sensorData.light);
    else
        Serial.print("光照: 离线 | ");

    // 温湿度
    if (status.sensor_aht21)
        Serial.printf("温度: %.1f°C | 湿度: %.1f%% | ", sensorData.temp, sensorData.humi);
    else
        Serial.print("温湿度: 离线 | ");

    // 空气质量
    if (status.sensor_ens160)
        Serial.printf("TVOC: %.0f | eCO2: %.0f | AQI: %d | PM2.5: %.1f\n", sensorData.tvoc, sensorData.eco2, sensorData.aqi, sensorData.pm25);
    else
        Serial.print("空气质量: 离线");

    //wifi连接
    if(status.wifi_connected)
        Serial.printf("wifi名称: %s | wifi密码: %s | ", wifi_config.ssid, wifi_config.pwd);
    else
        Serial.println("WiFi状态: 离线|");

    //专注模式
    if(status.focus_mode)
         Serial.printf("专注模式：启用 |");
    else
        Serial.println("专注模式：关闭 |");
    Serial.println("----------------------------------------");
}
