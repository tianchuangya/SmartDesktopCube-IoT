import pandas as pd
import numpy as np
import lightgbm as lgb
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
from sklearn.model_selection import KFold
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import warnings
import json
import os

warnings.filterwarnings('ignore')

# ============================================================
# 配置
# ============================================================
DATA_PATH = "data/laboratory.csv"
FEATURE_NAMES = ['tvoc', 'co2', 'temperature', 'humidity']
TARGET_NAMES = ['pm2_5', 'pm1', 'so2', 'o3', 'no2', 'co']
TARGET_LABELS = ['PM2.5', 'PM1', 'SO2', 'O3', 'NO2', 'CO']
OUTPUT_DIR = "multi_output_models"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ============================================================
# 第一步：加载数据
# ============================================================
print("=" * 60)
print("  正在加载数据...")
print("=" * 60)

df = pd.read_csv(DATA_PATH)
print(f"数据量：{len(df)} 条")
print(f"数据列：{list(df.columns)}")

# ============================================================
# 第二步：匹配特征字段和目标字段
# ============================================================
feature_map = {}
for col in df.columns:
    cl = col.lower().strip()
    if cl in FEATURE_NAMES:
        feature_map[cl] = col

target_map = {}
for col in df.columns:
    cl = col.lower().strip()
    for t in TARGET_NAMES:
        if t in cl:
            target_map[t] = col
            break

print(f"\n找到的特征列：{feature_map}")
print(f"找到的目标列：{target_map}")

missing_features = [k for k in FEATURE_NAMES if k not in feature_map]
missing_targets = [k for k in TARGET_NAMES if k not in target_map]
if missing_features:
    print(f"❌ 缺少特征字段：{missing_features}")
if missing_targets:
    print(f"⚠️ 缺少目标字段：{missing_targets}，将跳过")

available_targets = {k: v for k, v in target_map.items() if k in TARGET_NAMES and k not in missing_targets}

# ============================================================
# 第三步：数据清洗
# ============================================================
print("\n" + "=" * 60)
print("  数据清洗")
print("=" * 60)

use_cols = list(feature_map.values()) + list(available_targets.values())
df_clean = df[use_cols].copy()

for col in use_cols:
    df_clean[col] = pd.to_numeric(df_clean[col], errors='coerce')

before = len(df_clean)
df_clean = df_clean.dropna()
# 过滤目标值为负数的行
for t, col in available_targets.items():
    df_clean = df_clean[df_clean[col] >= 0]

print(f"清洗后：{before} → {len(df_clean)} 条")

# ============================================================
# 第四步：训练参数
# ============================================================
feature_cols = list(feature_map.values())
X = df_clean[feature_cols]

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

# ============================================================
# 第五步：逐个目标训练
# ============================================================
results_summary = []

for target_key, target_col in available_targets.items():
    print(f"\n{'='*60}")
    print(f"  🎯 训练目标：{target_key.upper()} ({target_col})")
    print(f"{'='*60}")

    y = df_clean[target_col]

    # 检查目标是否有足够方差
    if y.std() < 1e-6:
        print(f"  ⚠️ {target_key} 方差极小（std={y.std():.6f}），跳过")
        continue

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
                lgb.log_evaluation(0)  # 静默
            ]
        )

        y_pred = model.predict(X_test_fold)
        rmse_scores.append(np.sqrt(mean_squared_error(y_test_fold, y_pred)))
        mae_scores.append(mean_absolute_error(y_test_fold, y_pred))
        r2_scores.append(r2_score(y_test_fold, y_pred))

        all_y_true.extend(y_test_fold.values.tolist())
        all_y_pred.extend(y_pred.tolist())
        all_importances += model.feature_importance(importance_type='gain')

    all_importances /= kf.n_splits

    avg_r2 = np.mean(r2_scores)
    avg_rmse = np.mean(rmse_scores)
    avg_mae = np.mean(mae_scores)

    print(f"  R²   : {avg_r2:.4f} ± {np.std(r2_scores):.4f}")
    print(f"  RMSE : {avg_rmse:.4f} ± {np.std(rmse_scores):.4f}")
    print(f"  MAE  : {avg_mae:.4f} ± {np.std(mae_scores):.4f}")

    # 特征重要性
    print(f"  特征重要性：")
    for name, imp in sorted(zip(feature_cols, all_importances), key=lambda x: -x[1]):
        print(f"    {name}: {imp:.1f}")

    # 保存结果
    result = {
        'target': target_key,
        'r2': round(avg_r2, 4),
        'r2_std': round(np.std(r2_scores), 4),
        'rmse': round(avg_rmse, 4),
        'mae': round(avg_mae, 4),
        'feature_importance': {name: round(float(imp), 2) for name, imp in zip(feature_cols, all_importances)}
    }
    results_summary.append(result)

    # 散点图
    fig, ax = plt.subplots(figsize=(8, 8))
    ax.scatter(all_y_true, all_y_pred, alpha=0.3, s=10, color='steelblue')
    max_val = max(max(all_y_true), max(all_y_pred))
    ax.plot([0, max_val], [0, max_val], 'r--', linewidth=2, label='理想预测线')
    ax.set_xlabel(f'真实 {target_key.upper()}')
    ax.set_ylabel(f'预测 {target_key.upper()}')
    ax.set_title(f'TVOC+CO2+Temp+Humi → {target_key.upper()} (R²={avg_r2:.4f})')
    ax.legend()
    plt.tight_layout()
    scatter_path = os.path.join(OUTPUT_DIR, f'scatter_{target_key}.png')
    plt.savefig(scatter_path, dpi=150)
    plt.close()
    print(f"  📊 散点图已保存：{scatter_path}")

    # 训练最终模型并保存
    final_train_data = lgb.Dataset(X, label=y)
    final_model = lgb.train(params, final_train_data, num_boost_round=1000)
    model_path = os.path.join(OUTPUT_DIR, f'{target_key}_predictor.txt')
    final_model.save_model(model_path)
    print(f"  💾 模型已保存：{model_path}")

# ============================================================
# 第六步：汇总结果
# ============================================================
print(f"\n{'='*60}")
print(f"  📋 汇总结果")
print(f"{'='*60}")
print(f"{'目标':<10} {'R²':<12} {'RMSE':<12} {'MAE':<12} {'评价'}")
print(f"{'-'*60}")

for r in results_summary:
    r2 = r['r2']
    if r2 >= 0.7:
        grade = '✅ 优秀'
    elif r2 >= 0.4:
        grade = '⚠️ 一般'
    elif r2 >= 0.1:
        grade = '❌ 较差'
    else:
        grade = '❌ 不可用'
    print(f"{r['target'].upper():<10} {r2:.4f}±{r['r2_std']:.4f}  {r['rmse']:<12.4f} {r['mae']:<12.4f} {grade}")

print(f"{'='*60}")

# 保存汇总JSON
summary_path = os.path.join(OUTPUT_DIR, 'results_summary.json')
with open(summary_path, 'w', encoding='utf-8') as f:
    json.dump(results_summary, f, ensure_ascii=False, indent=2)
print(f"\n💾 汇总结果已保存：{summary_path}")

# 综合对比柱状图
fig, axes = plt.subplots(1, 3, figsize=(18, 6))

targets = [r['target'].upper() for r in results_summary]
r2_vals = [r['r2'] for r in results_summary]
rmse_vals = [r['rmse'] for r in results_summary]
mae_vals = [r['mae'] for r in results_summary]

colors = ['#2ecc71' if v >= 0.7 else '#f39c12' if v >= 0.4 else '#e74c3c' for v in r2_vals]

axes[0].bar(targets, r2_vals, color=colors)
axes[0].set_title('R² Score')
axes[0].set_ylabel('R²')
axes[0].axhline(y=0.7, color='green', linestyle='--', alpha=0.5, label='优秀阈值')
axes[0].axhline(y=0.4, color='orange', linestyle='--', alpha=0.5, label='一般阈值')
axes[0].legend()

axes[1].bar(targets, rmse_vals, color='steelblue')
axes[1].set_title('RMSE')
axes[1].set_ylabel('RMSE')

axes[2].bar(targets, mae_vals, color='steelblue')
axes[2].set_title('MAE')
axes[2].set_ylabel('MAE')

plt.suptitle('多目标预测效果对比 (输入: TVOC, CO2, Temperature, Humidity)', fontsize=14)
plt.tight_layout()
comparison_path = os.path.join(OUTPUT_DIR, 'comparison.png')
plt.savefig(comparison_path, dpi=150)
plt.close()
print(f"📊 对比图已保存：{comparison_path}")

print("\n" + "=" * 60)
print("  全部完成！")
print("=" * 60)
