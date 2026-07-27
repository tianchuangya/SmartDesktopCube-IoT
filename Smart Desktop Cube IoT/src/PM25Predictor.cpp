#include <Arduino.h>  // PROGMEM, pgm_read_* 等
#include "PM25Predictor.h"
#include "pm25_model_data.h"
#include <math.h>     // isnan, isinf

/**
 * 遍历一棵 LightGBM 决策树，返回该树的预测值
 *
 * 重要：LightGBM 模型文件中 leaf_value 已经包含了 learning rate，
 *       预测时直接求和即可，不要再乘以 shrinkage！
 *
 * 子节点索引约定：
 *   child >= 0 → 内部节点，直接 0-based 索引（0=root, 1~29=其他节点）
 *   child < 0  → 叶子节点，-1=leaf[0], -2=leaf[1], ...
 */
static float predictTree(int treeIdx, const float features[4])
{
    int node = 0;  // 从根节点（索引 0）开始
    while (true) {
        uint8_t feat    = pgm_read_byte(&pm25_features[treeIdx][node]);
        float threshold = pgm_read_float(&pm25_thresholds[treeIdx][node]);

        int8_t child;
        if (features[feat] <= threshold) {
            child = (int8_t)pgm_read_byte(&pm25_left[treeIdx][node]);
        } else {
            child = (int8_t)pgm_read_byte(&pm25_right[treeIdx][node]);
        }

        if (child < 0) {
            // 叶子：-1 → leaf[0], -2 → leaf[1], ...
            int leafIdx = -child - 1;
            return pgm_read_float(&pm25_leaves[treeIdx][leafIdx]);
        } else {
            // 内部节点：直接 0-based 索引
            node = child;
        }
    }
}

float pm25Predict(float temperature, float co2, float tvoc, float humidity)
{
    // 输入校验：传感器故障时可能传入 NaN/Inf
    if (isnan(temperature) || isinf(temperature) ||
        isnan(co2)         || isinf(co2)         ||
        isnan(tvoc)        || isinf(tvoc)        ||
        isnan(humidity)    || isinf(humidity)) {
        return -1.0f;  // 无效预测
    }

    float features[4] = { temperature, co2, tvoc, humidity };
    float sum = 0.0f;

    for (int t = 0; t < PM25_NUM_TREES; t++) {
        sum += predictTree(t, features);
    }

    // 输出钳位：PM2.5 不应为负值
    if (sum < 0.0f) sum = 0.0f;

    return sum;
}
