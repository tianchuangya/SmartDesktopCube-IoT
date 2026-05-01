"""
=====================================================
  室内 PM2.5 预测 - LightGBM 训练脚本
  数据集：2023 Indoor Air Quality Dataset Germany
=====================================================
使用方法：
  1. 将下载的数据集 CSV 文件放到与本脚本同一目录
  2. 修改下方 DATA_PATH 为你的文件路径
  3. 运行：python train_pm25.py
"""

import pandas as pd
import numpy as np
import lightgbm as lgb
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
import matplotlib
matplotlib.use('Agg')  # 无界面模式
import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings('ignore')

# ============================================================
# 【第一步】配置 - 请修改为你的实际文件路径
# ============================================================
DATA_PATH = "data/laboratory.csv"  # ← 改成你的文件名

# ============================================================
# 【第二步】加载数据
# ============================================================
print("=" * 60)
print("  正在加载数据...")
print("=" * 60)

df = pd.read_csv(DATA_PATH)
print(f"数据量：{len(df)} 条")
print(f"字段数：{len(df.columns)} 个")
print(f"\n所有字段：{df.columns.tolist()}")
print(f"\n前5行数据：")
print(df.head())
print(f"\n数据类型：")
print(df.dtypes)
print(f"\n基本统计：")
print(df.describe())

# ============================================================
# 【第三步】数据预处理
# ============================================================
print("\n" + "=" * 60)
print("  数据预处理")
print("=" * 60)

# 3.1 检查目标字段是否存在
target_col = None
for col in df.columns:
    if 'pm2_5' in col.lower() or 'pm25' in col.lower():
        target_col = col
        break

if target_col is None:
    print("❌ 未找到 PM2.5 字段！请检查数据集。可用字段：")
    print(df.columns.tolist())
    exit(1)

print(f"✅ 找到预测目标字段：{target_col}")

# 3.2 处理时间字段
time_col = None
for col in df.columns:
    if 'time' in col.lower() or 'date' in col.lower():
        time_col = col
        break

if time_col:
    df[time_col] = pd.to_datetime(df[time_col], errors='coerce')
    df['hour'] = df[time_col].dt.hour
    df['month'] = df[time_col].dt.month
    df['dayofweek'] = df[time_col].dt.dayofweek
    df['is_weekend'] = df['dayofweek'].isin([5, 6]).astype(int)
    df['is_night'] = df['hour'].isin([22, 23, 0, 1, 2, 3, 4, 5]).astype(int)
    print(f"✅ 从时间字段 [{time_col}] 提取了时间特征")

# 3.3 选择特征列（排除目标、时间、系统类字段）
exclude_cols = [target_col]
if time_col:
    exclude_cols.append(time_col)

# 排除系统/诊断类字段
system_cols = ['performance', 'health', 'typos', 'measuretime', 'dhch', 'dc02dat']
for sc in system_cols:
    if sc in df.columns:
        exclude_cols.append(sc)

feature_cols = [c for c in df.columns if c not in exclude_cols]
print(f"\n使用的特征列（{len(feature_cols)} 个）：")
for i, col in enumerate(feature_cols):
    print(f"  {i+1}. {col}")

# 3.4 处理缺失值
print(f"\n缺失值统计：")
missing = df[feature_cols + [target_col]].isnull().sum()
missing = missing[missing > 0]
if len(missing) > 0:
    print(missing)
    # 用中位数填充数值型缺失值
    for col in feature_cols + [target_col]:
        if df[col].isnull().sum() > 0:
            df[col] = pd.to_numeric(df[col], errors='coerce')
            median_val = df[col].median()
            df[col] = df[col].fillna(median_val)
            print(f"  → {col} 用中位数 {median_val} 填充")
else:
    print("  无缺失值 ✅")

# 3.5 处理异常值（替换为 NaN 再填充）
for col in feature_cols + [target_col]:
    df[col] = pd.to_numeric(df[col], errors='coerce')

# 删除目标值为空或异常的行
df = df.dropna(subset=[target_col])
# 去除目标值为负数的行（PM2.5 不可能为负）
df = df[df[target_col] >= 0]
print(f"\n清洗后数据量：{len(df)} 条")

# ============================================================
# 【第四步】构造交叉特征
# ============================================================
print("\n" + "=" * 60)
print("  构造交叉特征")
print("=" * 60)

# 温湿度交叉
if 'temperature' in df.columns and 'humidity' in df.columns:
    df['temp_x_humidity'] = df['temperature'] * df['humidity']
    print("  ✅ 新增特征：温度 × 湿度")

# TVOC 与 CO2 的比值
if 'tvoc' in df.columns and 'co2' in df.columns:
    df['tvoc_co2_ratio'] = df['tvoc'] / (df['co2'] + 1)
    print("  ✅ 新增特征：TVOC / CO2 比值")

# 更新特征列
feature_cols = [c for c in df.columns if c not in exclude_cols]
print(f"\n最终特征数：{len(feature_cols)} 个")

# ============================================================
# 【第五步】划分训练集和测试集
# ============================================================
print("\n" + "=" * 60)
print("  划分数据集")
print("=" * 60)

X = df[feature_cols]
y = df[target_col]

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

print(f"训练集：{len(X_train)} 条")
print(f"测试集：{len(X_test)} 条")

# ============================================================
# 【第六步】训练 LightGBM 模型
# ============================================================
print("\n" + "=" * 60)
print("  开始训练 LightGBM 模型...")
print("=" * 60)

train_data = lgb.Dataset(X_train, label=y_train)
test_data = lgb.Dataset(X_test, label=y_test, reference=train_data)

params = {
    'objective': 'regression',
    'metric': 'rmse',
    'learning_rate': 0.05,
    'num_leaves': 63,
    'max_depth': 8,
    'min_child_samples': 20,
    'feature_fraction': 0.8,
    'bagging_fraction': 0.8,
    'bagging_freq': 5,
    'reg_alpha': 0.1,       # L1 正则化
    'reg_lambda': 0.1,      # L2 正则化
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
# 【第七步】模型评估
# ============================================================
print("\n" + "=" * 60)
print("  模型评估结果")
print("=" * 60)

y_pred = model.predict(X_test)

rmse = np.sqrt(mean_squared_error(y_test, y_pred))
mae = mean_absolute_error(y_test, y_pred)
r2 = r2_score(y_test, y_pred)
mape = np.mean(np.abs((y_test - y_pred) / (y_test + 1))) * 100  # 避免 y=0

print(f"  RMSE（均方根误差）  : {rmse:.2f}")
print(f"  MAE（平均绝对误差）  : {mae:.2f}")
print(f"  R²（决定系数）      : {r2:.4f}")
print(f"  MAPE（平均百分比误差）: {mape:.2f}%")

if r2 > 0.8:
    print("\n  🎉 模型效果很好！R² > 0.8")
elif r2 > 0.6:
    print("\n  👍 模型效果不错，R² > 0.6，可以继续优化")
elif r2 > 0.4:
    print("\n  🤔 模型效果一般，建议增加特征或调整参数")
else:
    print("\n  ⚠️ 模型效果较差，需要检查数据质量或特征选择")

# ============================================================
# 【第八步】特征重要性分析
# ============================================================
print("\n" + "=" * 60)
print("  特征重要性 Top 15")
print("=" * 60)

importance = model.feature_importance(importance_type='gain')
feat_imp = sorted(zip(feature_cols, importance), key=lambda x: -x[1])

print(f"{'排名':<4} {'特征名':<25} {'重要性':<15}")
print("-" * 44)
for i, (name, imp) in enumerate(feat_imp[:15]):
    print(f"{i+1:<4} {name:<25} {imp:<15.1f}")

# 绘制特征重要性图
plt.figure(figsize=(10, 8))
top_features = feat_imp[:15]
names = [x[0] for x in top_features][::-1]
values = [x[1] for x in top_features][::-1]
plt.barh(names, values, color='steelblue')
plt.xlabel('Feature Importance (Gain)')
plt.title('LightGBM 特征重要性 Top 15')
plt.tight_layout()
plt.savefig('feature_importance.png', dpi=150)
print("\n  📊 特征重要性图已保存：feature_importance.png")

# ============================================================
# 【第九步】预测值 vs 真实值散点图
# ============================================================
plt.figure(figsize=(8, 8))
plt.scatter(y_test, y_pred, alpha=0.3, s=10, color='steelblue')
max_val = max(y_test.max(), y_pred.max())
plt.plot([0, max_val], [0, max_val], 'r--', linewidth=2, label='理想预测线')
plt.xlabel('真实 PM2.5')
plt.ylabel('预测 PM2.5')
plt.title(f'预测值 vs 真实值 (R²={r2:.4f})')
plt.legend()
plt.tight_layout()
plt.savefig('prediction_scatter.png', dpi=150)
print("  📊 预测散点图已保存：prediction_scatter.png")

# ============================================================
# 【第十步】保存模型
# ============================================================
model.save_model('pm25_predictor.txt')
print("\n  💾 模型已保存：pm25_predictor.txt")

# 保存特征列名（部署时需要）
import json
with open('feature_columns.json', 'w') as f:
    json.dump(feature_cols, f)
print("  💾 特征列名已保存：feature_columns.json")

print("\n" + "=" * 60)
print("  全部完成！")
print("=" * 60)
print("""
下一步你可以：
  1. 查看 feature_importance.png 了解哪些特征最重要
  2. 查看 prediction_scatter.png 了解预测效果
  3. 用 pm25_predictor.txt 部署模型进行实际预测
""")
