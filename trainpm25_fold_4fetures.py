import pandas as pd
import numpy as np
import lightgbm as lgb
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
from sklearn.model_selection import KFold
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

params = {
    'objective': 'regression',
    'metric': 'rmse',
    'learning_rate': 0.05,
    'num_leaves': 31,
    'max_depth': 6,
    'min_child_samples': 20,
    'verbose': -1
}
kf = KFold(n_splits=5, shuffle=True, random_state=42)
rmse_scores = []
mae_scores = []
r2_scores = []

all_y_true = []
all_y_pred = []
all_importances = np.zeros(len(feature_cols))

for fold, (train_index, test_index) in enumerate(kf.split(X)):
    X_train_fold = X.iloc[train_index]
    X_test_fold = X.iloc[test_index]
    y_train_fold = y.iloc[train_index]
    y_test_fold = y.iloc[test_index]
    train_data = lgb.Dataset(X_train_fold, label=y_train_fold)
    test_data = lgb.Dataset(X_test_fold, label=y_test_fold, reference=train_data)

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
    y_pred = model.predict(X_test_fold)
    rmse_scores.append(np.sqrt(mean_squared_error(y_test_fold, y_pred)))
    mae_scores.append(mean_absolute_error(y_test_fold, y_pred))
    r2_scores.append(r2_score(y_test_fold, y_pred))
    print(f"第 {fold + 1} 折: R²={r2_scores[-1]:.4f}, RMSE={rmse_scores[-1]:.2f}")

    all_y_true.extend(y_test_fold.values.tolist())
    all_y_pred.extend(y_pred.tolist())
    all_importances += model.feature_importance(importance_type='gain')
all_importances /= kf.n_splits

print(f"\n{'='*50}")
print(f"  5折交叉验证结果：")
print(f"  R²   : {np.mean(r2_scores):.4f} ± {np.std(r2_scores):.4f}")
print(f"  RMSE : {np.mean(rmse_scores):.2f} ± {np.std(rmse_scores):.2f}")
print(f"  MAE  : {np.mean(mae_scores):.2f} ± {np.std(mae_scores):.2f}")
print(f"{'='*50}")

print("\n特征重要性：")

for name, imp in sorted(zip(feature_cols, all_importances), key=lambda x: -x[1]):
    print(f"  {name}: {imp:.1f}")

final_train_data = lgb.Dataset(X, label=y)
final_model = lgb.train(
    params,
    final_train_data,
    num_boost_round=1000
)

# ============================================================
# 第八步：图表
# ============================================================

# 预测散点图
plt.figure(figsize=(8, 8))
plt.scatter(all_y_true, all_y_pred, alpha=0.3, s=10, color='steelblue')
max_val = max(max(all_y_true), max(all_y_pred))
plt.plot([0, max_val], [0, max_val], 'r--', linewidth=2, label='理想预测线')
plt.xlabel('真实 PM2.5')
plt.ylabel('预测 PM2.5')
avg_r2 = np.mean(r2_scores)
plt.title(f'TVOC+CO2+temperature+humidity → PM2.5 (R²={avg_r2:.4f})')
plt.legend()
plt.tight_layout()
plt.savefig('prediction_scatter_4features.png', dpi=150)
print("\n📊 散点图已保存：prediction_scatter_4features.png")

plt.figure(figsize=(10, 6))
sorted_idx = np.argsort(all_importances)
plt.barh(range(len(sorted_idx)), all_importances[sorted_idx])
plt.yticks(range(len(sorted_idx)), [feature_cols[i] for i in sorted_idx])
plt.xlabel('Gain')
plt.title('Feature Importance (Average of 5 Folds)')
plt.tight_layout()
plt.savefig('feature_importance.png', dpi=150)
print("📊 特征重要性图已保存：feature_importance_fold.png")

final_model.save_model('pm25_predictor_4features_fold.txt')
print("\n💾 最终模型已保存：pm25_predictor_4features_fold.txt")
print("\n" + "=" * 60)
print("  全部完成！")
print("=" * 60)
