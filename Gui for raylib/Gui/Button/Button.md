# Button 组件

基于 raylib 的按钮组件，支持内置绘制和自定义绘制函数。

## 文件位置

```
Gui/Button/
├── Button.h    # 头文件
├── Button.cpp  # 实现文件
└── Button.md   # 文档
```

## 构造函数

```cpp
Button(Rectangle bounds, std::string text, std::function<void()> onClick)
```

| 参数 | 类型 | 说明 |
|------|------|------|
| bounds | Rectangle | 按钮位置和大小 |
| text | std::string | 按钮显示文字 |
| onClick | std::function<void()> | 点击回调函数 |

## 公共方法

### 位置控制

```cpp
void SetPos(float x, float y);      // 设置坐标
void SetPos(Vector2 pos);           // 使用 Vector2 设置坐标
```

### 大小控制

```cpp
void SetSize(float width, float height); // 设置按钮大小
void SetSize(Vector2 size); // 设置按钮大小
```

### 状态更新

```cpp
void Update();  // 更新按钮状态（每帧调用）
```

### 绘制方法

```cpp
void Draw();    // 使用默认绘制函数绘制按钮
void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(const char*, Vector2, float, float, Color)> drawText);  // 使用自定义绘制函数
```

### 状态查询

```cpp
Vector2 GetPos() const;             // 获取当前位置
Vector2 GetSize() const;            // 获取按钮大小
Rectangle GetBtn() const;           // 获取按钮位置和大小
std::string GetText() const;        // 获取按钮文字
int GetFontSize() const;            // 获取字体大小
bool IsHovered() const;             // 是否悬停
bool IsPressed() const;             // 是否按下
```

## 公共属性

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| baseColor | Color | GRAY | 正常状态颜色 |
| hoverColor | Color | LIGHTGRAY | 悬停状态颜色 |
| pressedColor | Color | DARKGRAY | 按下状态颜色 |
| textColor | Color | WHITE | 文字颜色 |
| fontSize | int | 20 | 字体大小 |

## 使用示例

### 基本用法

```cpp
#include "Gui/Button/Button.h"

int main() {
    InitWindow(800, 600, "Button Demo");

    Button btn({300, 250, 120, 40}, "Click Me", []() {
        TraceLog(LOG_INFO, "Button clicked!");
    });

    while (!WindowShouldClose()) {
        btn.Update();

        BeginDrawing();
        ClearBackground(BLACK);

        // 使用默认绘制
        btn.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

### 使用自定义绘制函数

```cpp
#include "Gui/Button/Button.h"

int main() {
    InitWindow(800, 600, "Custom Button Demo");

    Button btn({300, 250, 120, 40}, "Click Me", []() {
        TraceLog(LOG_INFO, "Button clicked!");
    });

    while (!WindowShouldClose()) {
        btn.Update();

        BeginDrawing();
        ClearBackground(BLACK);

        // 使用自定义绘制函数
        btn.Draw(
            [](Rectangle rect, Color color) {
                // 绘制圆角按钮
                DrawRectangleRounded(rect, 0.3f, 8, color);
                DrawRectangleRoundedLines(rect, 0.3f, 8, 2, DARKGRAY);
            },
            [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                // 绘制带阴影的文字
                DrawTextEx(GetFontDefault(), text, {pos.x + 1, pos.y + 1}, fontSize, spacing, {0, 0, 0, 100});
                DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
            }
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## 状态说明

| 状态 | 触发条件 |
|------|----------|
| NORMAL | 鼠标未悬停 |
| HOVER | 鼠标悬停但未按下 |
| PRESSED | 鼠标按下 |

## 注意事项

- 每个 Button 实例有独立的状态，互不干扰
- Update() 需要在主循环中每帧调用
- 提供两种绘制方式：默认绘制和自定义绘制
- 回调函数使用 lambda 表达式或 std::function
- 自定义绘制函数可以实现更复杂的视觉效果，如圆角、阴影等
