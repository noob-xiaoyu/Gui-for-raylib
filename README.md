# Gui for raylib

基于 raylib 的 C++ GUI 组件库，提供常用的界面控件和焦点管理系统。

## 项目概述

这是一个轻量级的即时模式 GUI 库，专为 raylib 框架设计。所有组件都支持焦点管理、鼠标悬停检测和自定义渲染。

## 支持的组件

### 基础组件
- **Button** - 按钮组件，支持点击事件和自定义渲染
- **CheckBox** - 复选框，支持选中/取消选中状态
- **TextBox** - 文本输入框，支持多行文本、滚动条、IME 输入

### 选择组件
- **ComboBox** - 下拉选择框，支持多选和滚动
- **Multiselect** - 多选框，支持多项选择
- **ColorPicker** - 颜色选择器，支持 HSV 颜色模型

### 滑块组件
- **Slider** - 浮点数滑块，支持水平和垂直方向
- **IntSlider** - 整数滑块，支持水平和垂直方向

## 特性

### 焦点管理系统
- 统一的 `FocusManager` 单例管理所有控件的焦点
- 支持控件注册/注销
- 自动处理鼠标悬停状态
- 键盘输入支持（Slider 支持方向键调整值）

### 自定义渲染
大多数组件支持通过回调函数自定义渲染：
- 背景绘制
- 边框绘制
- 文本绘制
- 特殊效果（如圆角矩形、裁剪区域）

### 鼠标交互
- 悬停检测
- 点击事件处理
- 拖动支持
- 防止误触（拖动时离开控件区域会自动取消）

## 项目结构

```
Gui for raylib/
├── Gui/
│   ├── Button/           # 按钮组件
│   ├── CheckBox/         # 复选框组件
│   ├── ColorPicker/      # 颜色选择器
│   ├── ComboBox/         # 下拉选择框
│   ├── Focus/            # 焦点管理系统
│   ├── Multiselect/      # 多选框
│   ├── Slider/           # 滑块组件 (Slider & IntSlider)
│   └── TextBox/          # 文本输入框
├── Package/              # 平台相关代码
├── include/              # raylib 头文件和库
└── main/                 # 示例程序
```

## 使用方法

### 基本示例

```cpp
#include "Gui/Button/Button.h"
#include "Gui/Slider/Slider.h"
#include "Gui/Focus/FocusManager.h"

int main() {
    InitWindow(800, 600, "GUI Test");
    SetTargetFPS(60);

    Button button({100, 100, 150, 50}, "Click Me", []() {
        TraceLog(LOG_INFO, "Button clicked!");
    });

    Slider slider({100, 200, 200, 30}, 0.0f, 100.0f, 50.0f, 
                  Slider::Direction::Horizontal, [](float value) {
        TraceLog(LOG_INFO, "Slider: %.1f", value);
    });

    while (!WindowShouldClose()) {
        FocusManager::Instance().Update();
        button.Update();
        slider.Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        button.Draw();
        slider.Draw();
        EndDrawing();
    }

    return 0;
}
```

### 自定义渲染示例

```cpp
button.Draw(
    [](Rectangle rect, Color color) {
        DrawRectangleRounded(rect, 0.3f, 8, color);
    },
    [](Rectangle rect, Color color, float thickness) {
        DrawRectangleLinesEx(rect, thickness, color);
    },
    [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
        DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
    }
);
```

## 编译要求

- Visual Studio 2022 或更高版本
- raylib 库（已包含在项目中）
- C++17 或更高版本

## 运行示例

编译后运行 `Gui for raylib.exe` 可以看到所有组件的演示界面。

## 许可证

MIT License
