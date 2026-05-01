"""
=====================================================
  室内 PM2.5 预测 - 使用 TVOC + CO2 + 温度 + 湿度
  LightGBM 训练脚本
=====================================================
"""

import pandas as pd
import numpy as np
import lightgbm as lgb
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings('ignore')

# ============================================================
# 【配置】请修改为你的实际文件路径
# ============================================================
DATA_PATH = "data/laboratory.csv"  # ← 改成你的文件名

# ============================================================
# 第一步：加载数据
# ============================================================
print("=" * 60)
print("  正在加载数据...")
print("=" * 60)

df = pd.read_csv(DATA_PATH)
print(f"数据量：{len(df)} 条")

# ============================================================
# 第二步：找到目标字段和特征字段
# ============================================================
target_col = None
for col in df.columns:
    if 'pm2_5' in col.lower() or 'pm25' in col.lower():
        target_col = col
        break

if target_col is None:
    print("❌ 未找到 PM2.5 字段！")
    exit(1)

# 自动匹配特征字段
feature_map = {}
for col in df.columns:
    cl = col.lower()
    if cl == 'tvoc':
        feature_map['tvoc'] = col
    elif cl == 'co2':
        feature_map['co2'] = col
    elif cl == 'temperature':
        feature_map['temperature'] = col
    elif cl == 'humidity':
        feature_map['humidity'] = col

print(f"✅ 预测目标：{target_col}")
for key, col in feature_map.items():
    print(f"✅ 特征：{key} → {col}")

missing = [k for k in ['tvoc', 'co2', 'temperature', 'humidity'] if k not in feature_map]
if missing:
    print(f"❌ 缺少字段：{missing}")
    exit(1)

# ============================================================
# 第三步：数据清洗
# ============================================================
print("\n" + "=" * 60)
print("  数据清洗")
print("=" * 60)

use_cols = list(feature_map.values()) + [target_col]
df = df[use_cols].copy()

for col in use_cols:
    df[col] = pd.to_numeric(df[col], errors='coerce')

before = len(df)
df = df.dropna()
df = df[df[target_col] >= 0]
print(f"清洗后：{before} → {len(df)} 条")

# ============================================================
# 第四步：划分数据集
# ============================================================
feature_cols = list(feature_map.values())
X = df[feature_cols]
y = df[target_col]

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

print(f"训练集：{len(X_train)} 条，测试集：{len(X_test)} 条")

# ============================================================
# 第五步：训练 LightGBM
# ============================================================
print("\n" + "=" * 60)
print("  训练 LightGBM（TVOC + CO2 + 温度 + 湿度 → PM2.5）")
print("=" * 60)

train_data = lgb.Dataset(X_train, label=y_train)
test_data = lgb.Dataset(X_test, label=y_test, reference=train_data)

params = {
    'objective': 'regression',
    'metric': 'rmse',
    'learning_rate': 0.05,
    'num_leaves': 31,
    'max_depth': 6,
    'min_child_samples': 20,
    'verbose': -1
}

model = lgb.train(
    params,
    train_data,
    num_boost_round=1000,
    valid_sets=[train_data, test_data],
    valid_names=['train', 'valid'],
    callbacks=[
        lgb.early_stopping(stopping_rounds=50),
        lgb.log_evaluation(100)
    ]
)

print("\n✅ 训练完成！")

# ============================================================
# 第六步：评估
# ============================================================
print("\n" + "=" * 60)
print("  模型评估结果")
print("=" * 60)

y_pred = model.predict(X_test)

rmse = np.sqrt(mean_squared_error(y_test, y_pred))
mae = mean_absolute_error(y_test, y_pred)
r2 = r2_score(y_test, y_pred)
mape = np.mean(np.abs((y_test - y_pred) / (y_test + 1))) * 100

print(f"  RMSE : {rmse:.2f}")
print(f"  MAE  : {mae:.2f}")
print(f"  R²   : {r2:.4f}")
print(f"  MAPE : {mape:.2f}%")

if r2 > 0.8:
    print("\n  🎉 效果很好！")
elif r2 > 0.5:
    print("\n  👍 效果不错")
else:
    print("\n  🤔 效果一般，可尝试增加更多特征")

# ============================================================
# 第七步：特征重要性
# ============================================================
print("\n特征重要性：")
importance = model.feature_importance(importance_type='gain')
for name, imp in sorted(zip(feature_cols, importance), key=lambda x: -x[1]):
    print(f"  {name}: {imp:.1f}")

# ============================================================
# 第八步：图表
# ============================================================

# 预测散点图
plt.figure(figsize=(8, 8))
plt.scatter(y_test, y_pred, alpha=0.3, s=10, color='steelblue')
max_val = max(y_test.max(), y_pred.max())
plt.plot([0, max_val], [0, max_val], 'r--', linewidth=2, label='理想预测线')
plt.xlabel('真实 PM2.5')
plt.ylabel('预测 PM2.5')
plt.title(f'TVOC+CO2+温度+湿度 → PM2.5 (R²={r2:.4f})')
plt.legend()
plt.tight_layout()
plt.savefig('prediction_scatter_4features.png', dpi=150)
print("\n📊 散点图已保存：prediction_scatter_4features.png")

# ============================================================
# 第九步：保存模型
# ============================================================
model.save_model('pm25_predictor_4features.txt')
print("💾 模型已保存：pm25_predictor_4features.txt")

print("\n" + "=" * 60)
print("  全部完成！")
print("=" * 60)
