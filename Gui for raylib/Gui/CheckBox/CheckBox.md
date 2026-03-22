# CheckBox 组件文档

## 概述

CheckBox 是一个基于 raylib 的复选框组件，支持点击切换选中状态、悬停效果、标签显示等功能。

## 特性

- **点击切换**: 点击复选框可切换选中/未选中状态
- **悬停效果**: 鼠标悬停时显示不同的视觉效果
- **标签支持**: 可添加文本标签
- **回调函数**: 状态变化时触发回调
- **自定义颜色**: 支持自定义各种颜色属性

## 使用方法

### 基本初始化

```cpp
#include "CheckBox/CheckBox.h"

// 创建复选框
Rectangle bounds = { 100, 100, 20, 20 };
CheckBox checkBox(bounds, "Enable Feature", false);

// 添加状态变化回调
checkBox.SetOnCheckChange([](bool checked) {
    if (checked) {
        // 处理选中状态
    } else {
        // 处理未选中状态
    }
});
```

### 在游戏循环中使用

```cpp
while (!WindowShouldClose()) {
    // 更新复选框状态
    checkBox.Update();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // 绘制复选框
    checkBox.Draw();

    EndDrawing();
}
```

### 使用自定义绘制函数

```cpp
// 使用 raylib 的绘制函数
checkBox.Draw(
    [](Rectangle rect, Color color) {
        DrawRectangleRec(rect, color);
    },
    [](Vector2 start, Vector2 end, float thickness, Color color) {
        DrawLineEx(start, end, thickness, color);
    },
    [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
        DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
    }
);
```

### 获取和设置状态

```cpp
// 获取当前状态
bool isChecked = checkBox.IsChecked();

// 设置状态
checkBox.SetChecked(true);
```

## API 参考

### 构造函数

```cpp
CheckBox(Rectangle bounds, std::string label = "", bool checked = false, std::function<void(bool)> onCheckChange = nullptr);
```

- `bounds`: 复选框的位置和尺寸
- `label`: 显示的标签文本
- `checked`: 初始选中状态
- `onCheckChange`: 状态变化时的回调函数

### 公共方法

#### 更新与绘制

```cpp
void Update();  // 更新状态，处理输入
void Draw();    // 绘制复选框和标签
void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText);  // 使用自定义绘制函数
```

#### 设置与获取

```cpp
bool IsChecked() const;  // 获取当前选中状态
void SetChecked(bool checked);  // 设置选中状态
void SetLabel(const std::string& label);  // 设置标签文本
void SetPosition(float x, float y);  // 设置位置
void SetSize(float width, float height);  // 设置尺寸
void SetOnCheckChange(std::function<void(bool)> callback);  // 设置状态变化回调
```

### 颜色属性

| 属性 | 默认值 | 描述 |
|------|--------|------|
| `borderColor` | DARKGRAY | 边框颜色 |
| `fillColor` | WHITE | 背景填充颜色 |
| `checkColor` | BLUE | 勾选标记颜色 |
| `textColor` | BLACK | 标签文本颜色 |
| `hoverColor` | LIGHTGRAY | 鼠标悬停时的背景颜色 |
| `pressedColor` | GRAY | 鼠标按下时的背景颜色 |

## 依赖

- raylib 库

## 示例

### 基本用法

```cpp
#include "CheckBox/CheckBox.h"

int main() {
    InitWindow(800, 450, "CheckBox Example");
    SetTargetFPS(60);
    
    // 创建多个复选框
    CheckBox option1({ 100, 100, 20, 20 }, "Option 1");
    CheckBox option2({ 100, 140, 20, 20 }, "Option 2", true);
    
    while (!WindowShouldClose()) {
        option1.Update();
        option2.Update();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        option1.Draw();
        option2.Draw();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

### 自定义绘制函数示例

```cpp
#include "CheckBox/CheckBox.h"

int main() {
    InitWindow(800, 450, "Custom Draw CheckBox Example");
    SetTargetFPS(60);
    
    CheckBox checkBox({ 100, 100, 20, 20 }, "Custom Draw");
    
    while (!WindowShouldClose()) {
        checkBox.Update();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // 使用自定义绘制函数
        checkBox.Draw(
            [](Rectangle rect, Color color) {
                // 自定义矩形绘制（添加圆角效果）
                DrawRectangleRounded(rect, 0.2f, 4, color);
            },
            [](Vector2 start, Vector2 end, float thickness, Color color) {
                // 自定义线条绘制
                DrawLineEx(start, end, thickness, color);
            },
            [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                // 自定义文本绘制
                DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
            }
        );
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

### 自定义颜色

```cpp
CheckBox checkBox({ 100, 100, 20, 20 }, "Custom Color");
checkBox.borderColor = RED;
checkBox.checkColor = GREEN;
checkBox.fillColor = YELLOW;
checkBox.hoverColor = ORANGE;
```
