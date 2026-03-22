# Multiselect 组件

## 概述

Multiselect 是一个基于 raylib 的多选下拉框组件，允许用户从列表中选择多个选项。

## 特性

- 支持多选功能
- 下拉列表显示，带有滚动条支持
- 复选框可视化选中状态
- 支持自定义渲染函数
- 选中状态变更回调
- 响应式鼠标事件处理

## 类定义

```cpp
class Multiselect {
private:
    Rectangle bounds;
    std::vector<std::string> items;
    std::vector<bool> selectedItems;
    bool isOpen;
    std::function<void(const std::vector<int>&)> onSelectionChange;
    
    enum MultiselectState {
        MULTISELECT_STATE_NORMAL,
        MULTISELECT_STATE_HOVER,
        MULTISELECT_STATE_PRESSED
    } state;
    
    float dropdownHeight;
    int maxVisibleItems;
    float scrollOffset;
    bool isDraggingScroll;
    float scrollbarWidth;
    
public:
    Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange = nullptr);
    
    void Update();
    void Draw();
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor);
    
    void AddItem(const std::string& item);
    void RemoveItem(int index);
    void ClearItems();
    
    std::vector<int> GetSelectedIndices() const;
    std::vector<std::string> GetSelectedItems() const;
    void SetSelectedIndices(const std::vector<int>& indices);
    void SetSelectedItems(const std::vector<std::string>& items);
    
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback);
    void SetMaxVisibleItems(int count);
    
    Color backgroundColor = WHITE;
    Color borderColor = GRAY;
    Color textColor = BLACK;
    Color hoverColor = LIGHTGRAY;
    Color pressedColor = DARKGRAY;
    Color dropdownColor = WHITE;
    Color dropdownBorderColor = GRAY;
    Color selectedItemColor = LIGHTGRAY;
    Color hoverItemColor = LIGHTGRAY;
    int fontSize = 16;
};
```

## 构造函数

```cpp
Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange = nullptr);
```

- `bounds`：组件的位置和大小
- `items`：选项列表
- `onSelectionChange`：选中状态变更回调函数

## 方法

### Update

```cpp
void Update();
```

处理用户输入和状态更新。

### Draw

```cpp
void Draw();
```

使用默认渲染函数绘制组件。

### Draw (自定义渲染)

```cpp
void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor);
```

使用自定义渲染函数绘制组件。

### AddItem

```cpp
void AddItem(const std::string& item);
```

添加一个选项。

### RemoveItem

```cpp
void RemoveItem(int index);
```

移除指定索引的选项。

### ClearItems

```cpp
void ClearItems();
```

清空所有选项。

### GetSelectedIndices

```cpp
std::vector<int> GetSelectedIndices() const;
```

获取选中的选项索引。

### GetSelectedItems

```cpp
std::vector<std::string> GetSelectedItems() const;
```

获取选中的选项文本。

### SetSelectedIndices

```cpp
void SetSelectedIndices(const std::vector<int>& indices);
```

设置选中的选项索引。

### SetSelectedItems

```cpp
void SetSelectedItems(const std::vector<std::string>& items);
```

设置选中的选项文本。

### SetPosition

```cpp
void SetPosition(float x, float y);
```

设置组件位置。

### SetSize

```cpp
void SetSize(float width, float height);
```

设置组件大小。

### SetOnSelectionChange

```cpp
void SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback);
```

设置选中状态变更回调函数。

### SetMaxVisibleItems

```cpp
void SetMaxVisibleItems(int count);
```

设置下拉列表最大可见项目数。

## 属性

- `backgroundColor`：背景颜色
- `borderColor`：边框颜色
- `textColor`：文本颜色
- `hoverColor`：悬停颜色
- `pressedColor`：按下颜色
- `dropdownColor`：下拉列表背景颜色
- `dropdownBorderColor`：下拉列表边框颜色
- `selectedItemColor`：选中项目背景颜色
- `hoverItemColor`：下拉列表项目悬停颜色
- `fontSize`：字体大小

## 使用示例

```cpp
#include "Gui/Multiselect/Multiselect.h"

// 创建选项列表
std::vector<std::string> items = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5"};

// 创建 Multiselect 组件
Multiselect multiselect({100, 100, 200, 40}, items, [](const std::vector<int>& indices) {
    // 处理选中状态变更
    std::cout << "Selected indices: ";
    for (int index : indices) {
        std::cout << index << " ";
    }
    std::cout << std::endl;
});

// 在游戏主循环中
while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // 更新和绘制组件
    multiselect.Update();
    multiselect.Draw();
    
    EndDrawing();
}
```

## 自定义渲染示例

```cpp
// 自定义渲染函数
multiselect.Draw(
    [](Rectangle rect, Color color) {
        // 自定义矩形绘制
        DrawRectangleRec(rect, color);
    },
    [](Vector2 start, Vector2 end, float thickness, Color color) {
        // 自定义线条绘制
        DrawLineEx(start, end, thickness, color);
    },
    [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
        // 自定义文本绘制
        DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
    },
    [](Rectangle rect) {
        // 自定义裁剪开始
        BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
    },
    []() {
        // 自定义裁剪结束
        EndScissorMode();
    }
);
```

## 注意事项

- 确保在主循环中调用 `Update()` 和 `Draw()` 方法
- 可以通过修改颜色属性来自定义组件外观
- 当选项数量较多时，下拉列表会自动显示滚动条
- 点击下拉列表外部会关闭下拉列表