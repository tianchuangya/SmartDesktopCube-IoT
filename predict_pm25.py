"""
PM2.5 预测脚本
使用训练好的 LightGBM 模型进行预测
"""

import lightgbm as lgb
import pandas as pd
import json

# 1. 加载模型和特征列名
model = lgb.Booster(model_file='pm25_predictor.txt')
with open('feature_columns.json') as f:
    feature_cols = json.load(f)

print(f"模型需要 {len(feature_cols)} 个特征：")
for i, col in enumerate(feature_cols):
    print(f"  {i+1}. {col}")

# 2. 填入你的传感器数据（所有字段都必须有值）
new_data = pd.DataFrame([{
    'TypPS': 0,                    # 传感器类型标识（默认0）
    'oxygen': 20.9,                # 氧气浓度
    'pm10': 45,                    # PM10
    'cnt0_5': 800,                 # 0.5μm 颗粒计数
    'co': 0.5,                     # 一氧化碳
    'temperature': 22.5,           # 温度
    'co2': 450,                    # 二氧化碳
    'so2': 5,                      # 二氧化硫
    'no2': 20,                     # 二氧化氮
    'cnt5': 100,                   # 5μm 颗粒计数
    'timestamp': 0,                # 时间戳（已提取为其他特征，填0即可）
    'pm1': 15,                     # PM1
    'cnt1': 1000,                  # 1μm 颗粒计数
    'dewpt': 15,                   # 露点温度
    'tvoc': 120,                   # TVOC
    'pressure': 1013,              # 气压
    'cnt10': 50,                   # 10μm 颗粒计数
    'dCO2dt': 0,                   # CO2 变化率
    'sound_max': 55,               # 最大噪音
    'temperature_o2': 22.5,        # 温度传感器2
    'cnt2_5': 500,                 # 2.5μm 颗粒计数
    'o3': 30,                      # 臭氧
    'humidity': 65,                # 相对湿度
    'dHdt': 0,                     # 湿度变化率
    'humidity_abs': 10,            # 绝对湿度
    'sound': 40,                   # 噪音
    'cnt0_3': 2000,                # 0.3μm 颗粒计数
    'hour': 14,                    # 当前小时（0-23）
    'month': 4,                    # 当前月份（1-12）
    'dayofweek': 1,                # 星期几（0=周一, 6=周日）
    'is_weekend': 0,               # 是否周末（0=否, 1=是）
    'is_night': 0,                 # 是否夜间（0=否, 1=是）
    'temp_x_humidity': 22.5 * 65,  # 温度 × 湿度（自动计算的交叉特征）
    'tvoc_co2_ratio': 120 / (450 + 1),  # TVOC / CO2（自动计算的交叉特征）
}])

# 3. 确保特征顺序一致
new_data = new_data[feature_cols]

# 4. 预测
pm25_pred = model.predict(new_data)
print(f"\n预测 PM2.5: {pm25_pred[0]:.1f} μg/m³")
