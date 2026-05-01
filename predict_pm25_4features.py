"""
PM2.5 预测 - 使用 TVOC + CO2 + 温度 + 湿度
"""

import lightgbm as lgb
import pandas as pd

# 1. 加载模型
model = lgb.Booster(model_file='pm25_predictor_4features.txt')

# 2. 填入你的传感器数据
new_data = pd.DataFrame([{
    'tvoc': 165.774,         # TVOC 传感器读数
    'co2': 758.319,          # CO2 传感器读数
    'temperature': 20.894, # 温度传感器读数
    'humidity': 53.242,      # 湿度传感器读数
}])

# 3. 预测
pm25_pred = model.predict(new_data)
print(f"TVOC={new_data['tvoc'].values[0]}, "
      f"CO2={new_data['co2'].values[0]}, "
      f"温度={new_data['temperature'].values[0]}, "
      f"湿度={new_data['humidity'].values[0]}")
print(f"预测 PM2.5: {pm25_pred[0]:.1f} μg/m³")
