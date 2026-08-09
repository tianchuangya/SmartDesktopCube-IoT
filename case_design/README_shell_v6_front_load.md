# Smart Desktop Cube 外壳 v6：前装屏幕版本

v6 修正 v5 的主要风险：屏幕可能从后面装不进去。

设计变化：
- 主体前面做大开口，屏幕 PCB 从前面放入。
- 另做一个独立前压框 `smart_cube_shell_v6_front_screen_bezel.stl`。
- 压框覆盖大开口边缘，只露出屏幕可视窗口。
- 这样即使屏幕 PCB、排针、排线略大，也不需要从后方穿过内部结构硬塞。

推荐组合：
- 主体：`smart_cube_shell_v6_front_load_body.stl`
- 前压框：`smart_cube_shell_v6_front_screen_bezel.stl`
- 后盖：`smart_cube_shell_v4_back_cover_open_ports.stl`

建议装配顺序：
1. 屏幕从前面放入主体大开口。
2. 用前压框压住屏幕 PCB 边缘，M2 螺丝或小自攻固定。
3. ESP32、传感器、雷达从后面放入并走线。
4. 最后装后盖。

仍需实测：屏幕 PCB 实际长宽、触控排线方向、屏幕可视区位置。如果你给尺寸，v7 可以做精确压框。
