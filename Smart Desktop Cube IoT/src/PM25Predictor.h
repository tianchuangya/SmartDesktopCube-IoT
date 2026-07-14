#ifndef PM25_PREDICTOR_H
#define PM25_PREDICTOR_H

/**
 * PM2.5 预测模块 — 基于 LightGBM 4特征模型
 * 输入：温度(°C) / CO2(ppm) / TVOC(ppb) / 湿度(%)
 * 输出：预测 PM2.5 浓度 (μg/m³)
 */
float pm25Predict(float temperature, float co2, float tvoc, float humidity);

#endif
