```c
tft.fillScreen(TFT_BLACK);          // 全屏涂黑
tft.fillRect(x,y,w,h,颜色);         // 画矩形（按钮、背景）
tft.drawRect(x,y,w,h,颜色);         // 画空心矩形
tft.setTextColor(颜色);             // 文字颜色
tft.drawString("文字",x,y,字号);    // 写字
tft.drawCentreString("文字",x,y,字号); // 居中文字
```

在线一键转换图片 → 代码

[Rinky-Dink Electronics](http://www.rinkydinkelectronics.com/t_imageconverter565.php)

使用步骤：

1. 上传你的图片（LOGO / 图标 / 背景）
2. 选择 **Arduino → TFT_eSPI 格式**
3. 点击 **Generate**
4. 复制生成的数组
5. 放进代码里用 `tft.pushImage()` 显示

[在线图片大小修改器，图片尺寸修改，格式转换【免费】](https://www.sojson.com/image/change.html)


