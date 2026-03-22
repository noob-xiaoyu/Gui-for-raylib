# ComboBox 组件文档

## 概述

ComboBox 是一个基于 raylib 的下拉选择框组件，支持下拉列表、项目选择、滚动条等功能。

## 特性

- **下拉列表**: 点击按钮展开/收起下拉列表
- **项目选择**: 点击项目进行选择
- **滚动条**: 当项目过多时自动显示滚动条
- **自定义渲染**: 支持自定义绘制函数
- **回调函数**: 选择变化时触发回调
- **动态项目管理**: 支持添加、删除、清空项目

## 使用方法

### 基本初始化

```cpp
#include "ComboBox/ComboBox.h"

// 创建选项列表
std::vector<std::string> items = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5"};

// 创建组合框
ComboBox comboBox({100, 100, 200, 40}, items, 0, [](int index) {
    TraceLog(LOG_INFO, "Selected index: %d", index);
});
```

### 在游戏循环中使用

```cpp
while (!WindowShouldClose()) {
    // 更新组合框状态
    comboBox.Update();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // 绘制组合框
    comboBox.Draw();

    EndDrawing();
}
```

### 使用自定义绘制函数

```cpp
comboBox.Draw(
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
```

## API 参考

### 构造函数

```cpp
ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex = -1, std::function<void(int)> onSelectionChange = nullptr);
```

- `bounds`: 组合框的位置和尺寸
- `items`: 选项列表
- `selectedIndex`: 初始选中的索引（-1 表示无选中项）
- `onSelectionChange`: 选择变化时的回调函数

### 公共方法

#### 项目管理

```cpp
void AddItem(const std::string& item);    // 添加项目
void RemoveItem(int index);              // 删除项目
void ClearItems();                       // 清空所有项目
```

#### 选择管理

```cpp
int GetSelectedIndex() const;            // 获取选中索引
std::string GetSelectedItem() const;     // 获取选中项目
void SetSelectedIndex(int index);        // 设置选中索引
```

#### 位置和尺寸

```cpp
void SetPosition(float x, float y);      // 设置位置
void SetSize(float width, float height);  // 设置尺寸
```

#### 其他设置

```cpp
void SetOnSelectionChange(std::function<void(int)> callback);  // 设置选择变化回调
void SetMaxVisibleItems(int count);                          // 设置最大可见项目数
```

#### 更新与绘制

```cpp
void Update();  // 更新状态，处理输入
void Draw();    // 使用默认绘制函数
void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor);  // 使用自定义绘制函数
```

### 公共属性

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `backgroundColor` | Color | WHITE | 背景颜色 |
| `borderColor` | Color | GRAY | 边框颜色 |
| `textColor` | Color | BLACK | 文字颜色 |
| `hoverColor` | Color | LIGHTGRAY | 悬停颜色 |
| `pressedColor` | Color | DARKGRAY | 按下颜色 |
| `dropdownColor` | Color | WHITE | 下拉列表背景颜色 |
| `dropdownBorderColor` | Color | GRAY | 下拉列表边框颜色 |
| `selectedItemColor` | Color | LIGHTGRAY | 选中项目背景颜色 |
| `fontSize` | int | 16 | 字体大小 |

## 依赖

- raylib 库

## 示例

### 基本用法

```cpp
#include "ComboBox/ComboBox.h"

int main() {
    InitWindow(800, 450, "ComboBox Example");
    SetTargetFPS(60);
    
    std::vector<std::string> items = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5"};
    ComboBox comboBox({100, 100, 200, 40}, items, 0);
    
    while (!WindowShouldClose()) {
        comboBox.Update();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        comboBox.Draw();
        
        // 显示选中状态
        DrawText(TextFormat("Selected: %s", comboBox.GetSelectedItem().c_str()), 100, 160, 16, GRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

### 动态添加项目

```cpp
// 添加新项目
comboBox.AddItem("New Option");

// 删除项目
comboBox.RemoveItem(0);

// 清空项目
comboBox.ClearItems();
```

### 自定义外观

```cpp
// 自定义颜色
comboBox.backgroundColor = LIGHTGRAY;
comboBox.borderColor = DARKGRAY;
comboBox.textColor = BLACK;
comboBox.hoverColor = GRAY;
comboBox.pressedColor = DARKGRAY;
comboBox.dropdownColor = LIGHTGRAY;
comboBox.dropdownBorderColor = DARKGRAY;
comboBox.selectedItemColor = GRAY;
comboBox.fontSize = 14;
```
