# 当前模型基线：回退到 v12

## 状态

v13 不作为当前推荐版本。原因：

- v13 的屏幕滑板件过大，视觉上形成了额外的前侧/侧边壳子。
- 它偏离了“主体侧边开口 + 屏幕插入”的简洁方向。
- 后续不应继续沿 v13 的大滑板外壳方案修改。

当前基线回退到 v12，并复制为 `current_rollback_v12` 文件组。

## 当前文件

- `smart_cube_shell_current_body_rollback_v12.stl`
- `smart_cube_shell_current_screen_side_cover_rollback_v12.stl`
- `smart_cube_shell_current_back_cover_rollback_v12.stl`
- `smart_cube_shell_current_top_clear_lens_rollback_v12.stl`

## 后续修改方向

下一版应基于 v12 思路继续改：

- 前面保持平整矩形屏幕窗口。
- 不再增加第二个前侧壳子。
- 侧边只做一个可拆小盖或锁止片，不能变成大外壳。
- 真正的屏幕插槽应做在主体内部，表现为连续的上下导轨。
- 显示屏从侧边插入主体内部导轨，侧边小盖只负责防止退出和保护杜邦线。
- 屏幕背面 SD 卡槽区域继续留空，不做中部硬顶。
