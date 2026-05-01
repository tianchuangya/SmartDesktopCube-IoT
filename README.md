# PM2.5 预测模型文档

## 模型概述

这是一个基于LightGBM的PM2.5浓度预测模型，使用4个环境特征进行训练。模型采用回归任务，能够根据温度、CO2、TVOC和湿度等环境参数预测PM2.5浓度。

## 模型基本信息

- **模型类型**: LightGBM (梯度提升决策树)
- **任务类型**: 回归 (regression)
- **特征数量**: 4个
- **类别数量**: 1 (单输出回归)
- **模型版本**: v4
- **训练方法**: 5折交叉验证 + 全量数据训练

## 特征信息

模型使用以下4个环境特征进行预测：

1. **temperature** (温度) - 范围: 18.333°C 到 24.614°C
2. **co2** (二氧化碳浓度) - 范围: 424.946 ppm 到 908.562 ppm
3. **tvoc** (总挥发性有机化合物) - 范围: 0 ppb 到 4568.4 ppb
4. **humidity** (湿度) - 范围: 26.759% 到 66.856%

## 训练过程

### 数据准备
- **数据源**: `data/laboratory.csv`
- **目标字段**: 自动识别包含"pm2_5"或"pm25"的列
- **特征字段**: 自动匹配tvoc、co2、temperature、humidity
- **数据清洗**: 数值转换、缺失值处理、负值过滤

### 模型训练
- **交叉验证**: 5折交叉验证评估模型性能
- **训练参数**:
  - 学习率: 0.05
  - 叶子节点数: 31
  - 最大深度: 6
  - 最小子样本数: 20
  - 早停轮数: 50
- **评估指标**: R²、RMSE、MAE

### 训练结果
基于5折交叉验证的平均性能：
- **R²**: 0.xxxx ± 0.xxxx (具体数值见训练输出)
- **RMSE**: xx.xx ± xx.xx
- **MAE**: xx.xx ± xx.xx

## 模型结构

### 决策树数量
模型包含多个决策树，每个树的大小如下（部分展示）：
- 树大小序列: 2687, 2776, 2786, 2782, 2792, ...

### 树结构示例
每个决策树包含以下关键信息：
- **num_leaves**: 叶子节点数量
- **split_feature**: 分裂特征索引
- **split_gain**: 分裂增益
- **threshold**: 分裂阈值
- **leaf_value**: 叶子节点预测值
- **leaf_weight**: 叶子节点权重

## 模型参数

- **shrinkage**: 学习率 (第一棵树为1，后续树为0.05)
- **num_class**: 1 (单类别回归)
- **num_tree_per_iteration**: 1
- **max_feature_idx**: 3 (最大特征索引)

## 使用说明

### Python环境要求
```bash
pip install pandas numpy lightgbm scikit-learn matplotlib
```

### Python加载模型进行预测
```python
import lightgbm as lgb

# 加载模型
model = lgb.Booster(model_file='pm25_predictor_4features_fold.txt')

# 准备输入数据 (顺序: temperature, co2, tvoc, humidity)
input_data = [[20.5, 600, 1000, 50.0]]  # 示例数据

# 预测PM2.5
prediction = model.predict(input_data)
print(f"预测PM2.5浓度: {prediction[0]:.2f} μg/m³")
```

### C++环境要求
#### 方法1: 使用LightGBM C++ API
1. 下载并编译LightGBM C++库
```bash
git clone --recursive https://github.com/microsoft/LightGBM.git
cd LightGBM
mkdir build && cd build
cmake ..
make -j4
```

2. 在项目中包含头文件
```cpp
#include <LightGBM/c_api.h>
```

#### 方法2: 使用预编译库
下载预编译的LightGBM库文件，包含：
- `lib_lightgbm.a` (静态库)
- `lightgbm.dll` (Windows动态库)
- 头文件

### C++加载模型进行预测
```cpp
#include <iostream>
#include <vector>
#include "LightGBM/c_api.h"

int main() {
    // 初始化模型句柄
    BoosterHandle booster;
    int ret;
    
    // 加载模型文件
    ret = LGBM_BoosterCreateFromModelfile(
        "pm25_predictor_4features_fold.txt", 
        &booster
    );
    
    if (ret != 0) {
        std::cerr << "加载模型失败" << std::endl;
        return -1;
    }
    
    // 准备输入数据 (顺序: temperature, co2, tvoc, humidity)
    std::vector<double> input_data = {20.5, 600.0, 1000.0, 50.0};
    
    // 预测
    double prediction;
    int64_t out_len;
    
    ret = LGBM_BoosterPredictForMatSingleRow(
        booster,
        input_data.data(),
        C_API_DTYPE_FLOAT64,
        4,  // 特征数量
        1,  // 行数
        1,  // 是否为行优先
        C_API_PREDICT_NORMAL,
        0,  // 起始迭代
        -1, // 结束迭代
        "", // 参数
        &out_len,
        &prediction
    );
    
    if (ret == 0) {
        std::cout << "预测PM2.5浓度: " << prediction << " μg/m³" << std::endl;
    } else {
        std::cerr << "预测失败" << std::endl;
    }
    
    // 释放资源
    LGBM_BoosterFree(booster);
    return 0;
}
```

### C++编译命令
```bash
# 使用g++编译
g++ -std=c++11 pm25_predictor.cpp -I/path/to/lightgbm/include -L/path/to/lightgbm/lib -l_lightgbm -o pm25_predictor

# 运行
./pm25_predictor
```

### 嵌入式系统简化版本
对于嵌入式设备，可以使用简化的C接口：
```c
#include <stdio.h>
#include "lightgbm_c_api.h"

int main() {
    BoosterHandle booster;
    double features[4] = {20.5, 600.0, 1000.0, 50.0};
    double result;
    
    // 加载模型
    if (LGBM_BoosterCreateFromModelfile("model.txt", &booster) == 0) {
        // 预测
        LGBM_BoosterPredictForMatSingleRow(
            booster, features, C_API_DTYPE_FLOAT64, 
            4, 1, 1, C_API_PREDICT_NORMAL, 0, -1, "", NULL, &result
        );
        printf("PM2.5: %.2f μg/m³\n", result);
        LGBM_BoosterFree(booster);
    }
    
    return 0;
}
```

### 预测范围
模型在训练数据范围内的预测效果最佳：
- 温度: 18.333°C - 24.614°C
- CO2: 424.946 ppm - 908.562 ppm
- TVOC: 0 ppb - 4568.4 ppb
- 湿度: 26.759% - 66.856%

### 重新训练模型
如需重新训练，运行训练脚本：
```bash
python trainpm25_fold_4fetures.py
```

## 文件说明

### 主要文件
- `pm25_predictor_4features_fold.txt` - 训练好的LightGBM模型
- `trainpm25_fold_4fetures.py` - 训练脚本
- `prediction_scatter_4features.png` - 预测散点图
- `feature_importance_fold.png` - 特征重要性图

### 训练脚本功能
- 自动数据加载和特征识别
- 5折交叉验证评估
- 模型训练和保存
- 可视化结果生成

## 注意事项

1. 输入特征应在训练数据范围内，超出范围可能影响预测准确性
2. 确保数据格式正确，特征顺序为: temperature, co2, tvoc, humidity
3. 模型采用LightGBM格式，需要使用LightGBM库进行加载和预测
4. 训练数据应包含完整的4个特征和PM2.5目标值

## 技术细节

- **目标函数**: regression
- **特征索引**: 从0开始 (0:温度, 1:CO2, 2:TVOC, 3:湿度)
- **决策类型**: 2 (表示标准的二分决策)
- **特征重要性**: 基于增益(gain)计算

## 维护信息

- **创建时间**: 模型文件生成时间未知
- **文件路径**: `/Users/sensen/Desktop/model/pm25_predictor_4features_fold.txt`
- **训练脚本**: `/Users/sensen/Desktop/model/trainpm25_fold_4fetures.py`
- **文件大小**: 包含多个决策树的详细结构

---
*文档最后更新: 2026-05-01*
