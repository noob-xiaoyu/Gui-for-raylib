# CheckBox (复选框)

用于切换布尔状态（开/关）的控件。

## 特性
- **标签支持**：支持在框体右侧显示描述文本，点击文本区域同样可触发切换。
- **自动同步**：通过引用或回调函数同步外部布尔变量。
- **皮肤支持**：内置勾选框绘制逻辑，亦可使用 `IGuiSkin` 自定义外观。

## 常用接口
- `CheckBox(Rectangle bounds, std::string label, bool checked, std::function<void(bool)> onCheckChange)`: 构造函数。
- `SetChecked(bool checked)`: 手动设置勾选状态。
