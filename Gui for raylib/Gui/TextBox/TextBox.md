# TextBox 组件文档

## 概述

TextBox 是一个基于 raylib 的功能完整的文本输入组件，支持 Unicode 文本输入、撤销/重做、文本选择、滚动条等功能。

## 特性

- **Unicode 支持**: 完整的 UTF-8 编码支持，包括多字节字符
- **撤销/重做**: 支持最多 100 步的撤销和重做操作
- **文本选择**: 支持鼠标拖拽选择文本
- **自动缩进**: 按下回车后自动复制上一行的缩进
- **剪贴板支持**: 支持复制、粘贴、剪切操作
- **滚动条**: 支持鼠标滚轮和拖拽滚动
- **IME 支持**: Windows 平台下支持输入法跟随光标
- **光标闪烁**: 可配置的光标闪烁动画

## 使用方法

### 基本初始化

```cpp
#include "TextBox/TextBox.h"

// 创建文本框
Rectangle bounds = { 100, 100, 300, 200 };
TextBox textBox(bounds);

// 设置字体（可选）
textBox.SetFont(GetFontDefault(), 20.0f, 1.0f, 24.0f);

// 设置初始文本（可选）
textBox.SetText("Hello, World!");
```

### 在游戏循环中使用

```cpp
while (!WindowShouldClose()) {
    // 更新文本框状态
    textBox.Update();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // 绘制文本框
    textBox.Draw(LIGHTGRAY, BLACK);

    EndDrawing();
}
```

### 获取文本内容

```cpp
std::string content = textBox.GetText();
```

## 键盘快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + C` | 复制选中文本 |
| `Ctrl + V` | 粘贴剪贴板内容 |
| `Ctrl + X` | 剪切选中文本 |
| `Ctrl + A` | 全选所有文本 |
| `Ctrl + Z` | 撤销 |
| `Ctrl + Y` | 重做 |
| `Ctrl + Shift + Z` | 重做 |
| `Enter` | 插入换行并自动缩进 |
| `Backspace` | 删除光标前字符 |
| `Delete` | 删除光标后字符 |
| `← / →` | 光标左右移动 |
| `↑ / ↓` | 光标上下移动 |

## API 参考

### 构造函数

```cpp
TextBox(Rectangle bounds);
```

创建文本框并指定边界区域。

### 公共方法

#### 设置与获取

```cpp
void SetText(const std::string& text);  // 设置文本内容
std::string GetText() const;             // 获取文本内容
void SetFocus(bool focus);               // 设置焦点状态
bool IsFocused() const;                  // 获取焦点状态

void SetSize(float width, float height);    // 设置尺寸
void SetPosition(float x, float y);          // 设置位置

// 设置字体配置
// font: raylib 字体对象
// fontSize: 字体大小
// spacing: 水平字符间距
// lineHeight: 行高（若为0，默认等于fontSize）
void SetFont(Font font, float fontSize, float spacing, float lineHeight = 0.0f);
```

#### 更新与绘制

```cpp
void Update();  // 更新状态，处理输入
void Draw(Color bgcolor, Color textcolor);  // 绘制文本框
void Draw(Color bgcolor, Color textcolor, std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor);  // 使用自定义绘制函数
```

### 内部方法（私有）

- `HandleInput()`: 处理键盘输入
- `HandleMouse()`: 处理鼠标交互
- `HandleScroll()`: 处理滚动条交互
- `InsertCodepoint()`: 插入 Unicode 码点
- `InsertText()`: 插入文本
- `Backspace()`: 删除光标前字符
- `Delete()`: 删除光标后字符
- `DeleteSelection()`: 删除选中文本
- `Copy()`: 复制选中文本
- `Paste()`: 粘贴剪贴板内容
- `Cut()`: 剪切选中文本
- `PerformAutoIndent()`: 执行自动缩进
- `SaveUndoState()`: 保存撤销状态
- `Undo()`: 撤销操作
- `Redo()`: 重做操作

## 依赖

- raylib 库
- Windows IME 支持（仅 Windows 平台）

## 注意事项

1. **UTF-8 编码**: 源文件应保存为 UTF-8 编码
2. **焦点管理**: 点击文本框内部获得焦点，点击外部失去焦点
3. **滚动条**: 仅在文本内容超出可视区域时显示
4. **IME 位置**: Windows 平台下 IME 窗口会跟随光标位置

## 示例

### 基本用法

```cpp
#include "TextBox/TextBox.h"

int main() {
    InitWindow(800, 450, "TextBox Example");
    SetTargetFPS(60);
    
    TextBox textBox({ 100, 100, 300, 200 });
    
    while (!WindowShouldClose()) {
        textBox.Update();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        textBox.Draw(LIGHTGRAY, BLACK);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

### 使用自定义绘制函数

```cpp
#include "TextBox/TextBox.h"

int main() {
    InitWindow(800, 450, "Custom TextBox Example");
    SetTargetFPS(60);
    
    TextBox textBox({ 100, 100, 300, 200 });
    
    while (!WindowShouldClose()) {
        textBox.Update();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // 使用自定义绘制函数
        textBox.Draw(
            LIGHTGRAY, BLACK,
            [](Rectangle rect, Color color) {
                // 绘制圆角背景
                DrawRectangleRounded(rect, 0.1f, 8, color);
            },
            [](Vector2 start, Vector2 end, float thickness, Color color) {
                // 绘制线条
                DrawLineEx(start, end, thickness, color);
            },
            [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                // 绘制文本
                DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
            },
            [](Rectangle rect) {
                // 开始裁剪
                BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
            },
            []() {
                // 结束裁剪
                EndScissorMode();
            }
        );
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```
