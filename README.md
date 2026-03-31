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

### 1. 焦点与交互管理 (FocusManager)
- **全局调度**：统一管理所有控件的焦点切换、悬停检测和活跃状态。
- **鼠标捕获 (Mouse Capture)**：按下鼠标时自动锁定操作控件，消除滑动时的悬停穿透，确保拖拽体验丝滑。
- **活跃控件屏蔽**：当弹出层（如 ComboBox）展开时，自动拦截底层控件的输入，防止操作穿透。

### 2. 主题与样式系统 (GuiTheme)
- **集中化配置**：通过 `GuiTheme` 单例统一定义全局颜色、字体和间距。
- **一键换肤**：修改主题配置即可实现全界面风格的同步更新。

### 3. 皮肤系统 (IGuiSkin)
- **解耦渲染**：引入 `IGuiSkin` 接口，将 UI 逻辑与绘图代码彻底分离。
- **深度定制**：开发者可以实现自己的皮肤类，为特定控件或全局替换视觉风格（如圆角、渐变、阴影）。

### 4. 健壮的组件库
- **文本处理**：`TextBox` 支持撤销/重做、光标定位、多行滚动及 IME 输入。
- **架构复用**：`ComboBox` 与 `Multiselect` 共享 `DropdownControl` 基类逻辑。

## 项目结构

```
Gui for raylib/
├── Gui/
│   ├── Common/           # 核心基类 (DropdownControl), 主题 (GuiTheme), 渲染助手 (GuiRenderer)
│   ├── Button/           # 按钮组件
│   ├── CheckBox/         # 复选框组件
│   ├── ColorPicker/      # 颜色选择器
│   ├── ComboBox/         # 下拉选择框
│   ├── Focus/            # 焦点管理系统 (FocusManager)
│   ├── Multiselect/      # 多选框
│   ├── Slider/           # 滑块组件 (Slider & IntSlider)
│   └── TextBox/          # 文本输入框
├── gui introduce/        # 详细的组件介绍文档
├── Package/              # 平台相关代码 (Windows IME 等)
├── include/              # raylib 头文件和库
└── main/                 # 示例程序
```

## 使用方法

### 基本示例

```cpp
#include "Gui/Button/Button.h"
#include "Gui/Focus/FocusManager.h"

int main() {
    InitWindow(800, 600, "GUI Test");
    
    Button button({100, 100, 150, 50}, "Click Me", []() {
        TraceLog(LOG_INFO, "Clicked!");
    });

    while (!WindowShouldClose()) {
        FocusManager::Instance().Update(); // 必须首先更新管理器
        button.Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        button.Draw();
        EndDrawing();
    }
    return 0;
}
```

### 皮肤定制示例 (IGuiSkin)

```cpp
class MySkin : public IGuiSkin {
public:
    void DrawButton(const PaintContext& ctx, const char* text) override {
        Color c = (ctx.state == ControlState::HOVER) ? SKYBLUE : GRAY;
        DrawRectangleRounded(ctx.bounds, 0.2f, 8, c);
        DrawText(text, ctx.bounds.x + 10, ctx.bounds.y + 15, 20, WHITE);
    }
    // ... 实现其他组件的绘制方法
};

// 应用皮肤
MySkin customSkin;
button.SetSkin(&customSkin);
```

## 编译要求

- Visual Studio 2022 或更高版本
- raylib 库（已包含在项目中）
- C++17 或更高版本

## 运行示例

编译后运行 `Gui for raylib.exe` 可以看到所有组件的演示界面。

## 许可证

MIT License
