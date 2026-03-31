# Button (按钮)

基础的交互控件，支持点击事件和多种视觉状态。

## 特性
- **状态管理**：支持 Normal, Hover, Pressed 和 Disabled 状态。
- **自定义事件**：通过 `std::function<void()>` 回调处理点击逻辑。
- **自适应文字**：自动居中渲染按钮文本。
- **皮肤支持**：支持通过 `IGuiSkin` 进行完全自定义渲染。

## 常用接口
- `Button(Rectangle bounds, std::string text, std::function<void()> onClick)`: 构造函数。
- `SetPos(float x, float y)`: 设置位置。
- `SetSize(float width, float height)`: 设置大小。
