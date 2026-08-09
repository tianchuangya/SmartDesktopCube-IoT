# Smart Desktop Cube 外壳 v19：中心走线窗口版

## 基础

v19 基于 v18，只修改上下模块区域之间的中间隔板/支撑层。

## 改动

- 将上层模块区与下层 DC/ESP32 区之间的连续中间层拆成分段结构。
- 在中间位置留出走线窗口。
- 下方 DC 电源模块和 ESP32-S3 的线可以从中间窗口向上/向前伸出。
- 两侧和前后仍保留支撑，不整块拆空。

## 保持不变

- 主体外形：约 117 x 138 x 118 mm。
- 整面前压框。
- 右侧滑条。
- 后盖 DC 圆孔、Type-C 矩形孔和通风口。
- 顶部透明窗片。

## 当前输出

- `smart_cube_shell_v19_body_center_wire_opening.stl`
- `smart_cube_shell_v19_front_bezel_full_face.stl`
- `smart_cube_shell_v19_side_simple_slider_vents.stl`
- `smart_cube_shell_v19_back_cover_two_ports_vents.stl`
- `smart_cube_shell_v19_top_clear_lens.stl`
- `smart_cube_shell_v19_center_wire_opening_preview.blend`

## 走线建议

- DC 输出线从左下模块槽出来后，汇到中间窗口。
- ESP32-S3 杜邦线从右下模块槽出来后，汇到中间窗口或右侧线槽。
- 上层传感器线可以通过中间窗口下到 ESP32-S3。
- 屏幕线仍走右侧滑条下方线槽。
