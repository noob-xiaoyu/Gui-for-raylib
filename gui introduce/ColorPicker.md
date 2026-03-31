# ColorPicker (颜色选择器)

直观的 HSV 颜色编辑控件。

## 特性
- **弹出式设计**：点击颜色预览块后展开完整的 HSV 调色盘。
- **全参数调节**：包含饱和度/明度选择区、色相滑块以及 Alpha 透明度滑块。
- **颜色空间**：内部自动处理 RGB 与 HSV 颜色空间的转换。
- **皮肤支持**：支持自定义预览块和弹出面板的外观。

## 常用接口
- `ColorPicker(Rectangle bounds, Color initialColor, ...)`: 构造函数。
- `SetColor(Color color)`: 强制设置当前颜色。
