#include <raylib.h>
#include <cstdio>
#include <vector>
#include <string>
#include "Gui/Button/Button.h"
#include "Gui/CheckBox/CheckBox.h"
#include "Gui/ColorPicker/ColorPicker.h"
#include "Gui/ComboBox/ComboBox.h"
#include "Gui/Multiselect/Multiselect.h"
#include "Gui/Slider/Slider.h"
#include "Gui/TextBox/TextBox.h"
#include "Gui/Focus/FocusManager.h"
#include "Gui/Common/GuiSkin.h"
#include "Gui/Common/GuiSkins.h"

// 这是一个使用 raylib 和自定义 GUI 库的示例程序，展示了各种 GUI 组件的使用和皮肤切换功能。
class SimpleCustomSkin : public IGuiSkin {
public:
    // 实现 IGuiSkin 接口，提供简单的自定义绘制逻辑
    std::string GetSkinName() const override { return "SimpleSkin"; }
    // 这里只是示例，实际可以根据需要设计更复杂的外观
    // 例如，按钮绘制为蓝色圆角矩形，文本居中显示
    void DrawButton(const PaintContext& ctx, const char* text) override {
        Color color = (ctx.state == ControlState::PRESSED) ? DARKGRAY : (ctx.state == ControlState::HOVER) ? LIGHTGRAY : GRAY;
        DrawRectangleRounded(ctx.bounds, 0.3f, 8, color);
        DrawRectangleLinesEx(ctx.bounds, 1, ctx.isFocused ? BLUE : BLACK);
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
        DrawTextEx(GetFontDefault(), text, { ctx.bounds.x + (ctx.bounds.width - textSize.x) / 2, ctx.bounds.y + (ctx.bounds.height - textSize.y) / 2 }, 20, 1, WHITE);
    }
    // 复选框绘制为白色背景的圆角矩形，选中时显示蓝色对勾
    void DrawCheckBox(const PaintContext& ctx, bool checked, const char* label) override {
        Color color = (ctx.state == ControlState::PRESSED) ? GRAY : (ctx.state == ControlState::HOVER) ? LIGHTGRAY : WHITE;
        DrawRectangleRounded(ctx.bounds, 0.2f, 4, color);
        DrawRectangleLinesEx(ctx.bounds, 1, BLACK);
        if (checked) {
            DrawLineEx({ ctx.bounds.x + 4, ctx.bounds.y + 10 }, { ctx.bounds.x + 8, ctx.bounds.y + 16 }, 2.0f, BLUE);
            DrawLineEx({ ctx.bounds.x + 8, ctx.bounds.y + 16 }, { ctx.bounds.x + 16, ctx.bounds.y + 4 }, 2.0f, BLUE);
        }
        if (label && label[0] != '\0') {
            DrawText(label, (int)(ctx.bounds.x + ctx.bounds.width + 10), (int)(ctx.bounds.y + 2), 16, BLACK);
        }
    }
    // 滑块绘制为灰色轨道和红色滑块，垂直和水平都支持
    void DrawSlider(const PaintContext& ctx, float value, float min, float max, bool isVertical) override {
        DrawRectangleRec(ctx.bounds, LIGHTGRAY);
        DrawRectangleLinesEx(ctx.bounds, 1, BLACK);
        if (isVertical) {
            float thumbY = ctx.bounds.y + (1.0f - (value - min) / (max - min)) * ctx.bounds.height;
            DrawRectangleRec({ ctx.bounds.x - 5, thumbY - 5, ctx.bounds.width + 10, 10 }, RED);
        } else {
            float thumbX = ctx.bounds.x + ((value - min) / (max - min)) * ctx.bounds.width;
            DrawRectangleRec({ thumbX - 5, ctx.bounds.y - 5, 10, ctx.bounds.height + 10 }, RED);
        }
    }
    // 整数滑块绘制与浮点滑块类似，但显示整数值
    void DrawIntSlider(const PaintContext& ctx, int value, int min, int max, bool isVertical) override {
        DrawSlider(ctx, (float)value, (float)min, (float)max, isVertical);
    }
    // 文本框绘制为白色背景的圆角矩形，边框根据焦点状态改变颜色
    void DrawTextBox(const PaintContext& ctx, const std::string& text, int cursorIndex, int selectionAnchor) override {
        DrawRectangleRounded(ctx.bounds, 0.1f, 8, LIGHTGRAY);
        DrawRectangleLinesEx(ctx.bounds, 1, ctx.isFocused ? BLUE : BLACK);
        // 内部文本由 TextBox 自己的逻辑绘制
    }
    // 下拉框和颜色选择器的背景绘制为白色矩形，边框为黑色
    void DrawComboBox(const PaintContext& ctx, const std::string& currentItem, bool isOpen) override {
        DrawRectangleRec(ctx.bounds, WHITE);
        DrawRectangleLinesEx(ctx.bounds, 1, BLACK);
        DrawText(currentItem.c_str(), (int)ctx.bounds.x + 5, (int)ctx.bounds.y + 10, 20, BLACK);
    }
    // 颜色选择器面板绘制为白色背景，边框为黑色，当前颜色显示在面板内
    void DrawColorPicker(const PaintContext& ctx, Color color, bool isOpen) override {
        DrawRectangleRounded(ctx.bounds, 0.1f, 4, color);
        DrawRectangleLinesEx(ctx.bounds, 1, BLACK);
    }
    // 下拉菜单背景绘制为浅灰色，边框为黑色
    void DrawDropdownBackground(const PaintContext& ctx, const Rectangle& bounds) override {
        DrawRectangleRec(bounds, LIGHTGRAY);
        DrawRectangleLinesEx(bounds, 1, BLACK);
    }
    // 下拉菜单项绘制为白色背景，选中时为蓝色，悬停时为浅灰色
    void DrawDropdownItem(const PaintContext& ctx, const Rectangle& bounds, const char* text, bool isHovered, bool isSelected) override {
        if (isSelected) DrawRectangleRec(bounds, GRAY);
        else if (isHovered) DrawRectangleRec(bounds, { 200, 200, 200, 255 });
        DrawText(text, (int)bounds.x + 5, (int)bounds.y + 5, 20, BLACK);
    }
    // 颜色选择器面板绘制为白色背景，边框为黑色，当前颜色显示在面板内
    void DrawColorPickerPanel(const PaintContext& ctx, const Rectangle& bounds, Color current, float hue, float sat, float val) override {
        DrawRectangleRec(bounds, WHITE);
        DrawRectangleLinesEx(bounds, 1, BLACK);
    }
};

int main() {
    const int screenWidth = 1000;
    const int screenHeight = 700;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "GUI Components Test");

    SetTargetFPS(240);

    // 创建一个离屏渲染目标，用于绘制 GUI
    RenderTexture2D guiTarget = LoadRenderTexture(screenWidth, screenHeight);

    // 创建 GUI 组件实例
        // 每个组件都绑定一个回调函数，用于在交互时输出日志信息，方便测试和验证功能
        // 例如，按钮点击时输出 "Button clicked!"，复选框状态改变时输出当前状态，文本框内容改变时输出当前文本，等等。
        Button button({50, 50, 150, 45}, "Click Me", []() {
            TraceLog(LOG_INFO, "Button clicked!");
        });

        // 复选框示例，初始状态为未选中，点击时切换状态并输出当前状态
        bool featureEnabled = false;
        CheckBox checkBox({50, 110, 20, 20}, "Enable Feature", featureEnabled, [&](bool checked) {
            featureEnabled = checked;
            TraceLog(LOG_INFO, "CheckBox state: %s", checked ? "checked" : "unchecked");
        });

        // 文本框示例，初始文本为 "Enter text here..."，用户输入时输出当前文本内容
        TextBox textBox({50, 160, 300, 150});
        textBox.SetText("Enter text here...");

        // 下拉框示例，提供多个选项，用户选择时输出当前选中项的索引和文本
        std::vector<std::string> comboItems = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5", "Option 6", "Option 7", "Option 8"};
        ComboBox comboBox({400, 50, 200, 40}, comboItems, 0, [](int index) {
            TraceLog(LOG_INFO, "ComboBox selected: %d", index);
        });

        // 多选框示例，提供多个选项，用户选择时输出当前选中项的索引列表和文本列表
        std::vector<std::string> multiItems = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5", "Item 6", "Item 7", "Item 8"};
        Multiselect multiselect({400, 110, 200, 40}, multiItems, [](const std::vector<int>& indices) {
            TraceLog(LOG_INFO, "Multiselect selected indices:");
            for (int index : indices) {
                TraceLog(LOG_INFO, "%d", index);
            }
        });

        // 颜色选择器示例，初始颜色为红色，用户选择时输出当前颜色的 RGB 值
        ColorPicker colorPicker({650, 50, 60, 40}, RED, [](Color color) {
            char hex[16];
            snprintf(hex, sizeof(hex), "#%02X%02X%02X", color.r, color.g, color.b);
            TraceLog(LOG_INFO, "ColorPicker: %s", hex);
        });

        // 滑块示例，提供一个水平滑块和一个垂直滑块，用户调整时输出当前滑块的值
        // 水平滑块示例，初始值为 50，用户调整时输出当前值
        float sliderValue = 50.0f;
        Slider horizontalSlider({400, 280, 250, 30}, 0.0f, 100.0f, sliderValue, Slider::Direction::Horizontal, [&](float value) {
            sliderValue = value;
            TraceLog(LOG_INFO, "Horizontal Slider: %.1f", value);
        });

        // 垂直滑块示例，初始值为 75，用户调整时输出当前值
        float verticalSliderValue = 75.0f;
        Slider verticalSlider({900, 50, 30, 200}, 0.0f, 100.0f, verticalSliderValue, Slider::Direction::Vertical, [&](float value) {
            verticalSliderValue = value;
            TraceLog(LOG_INFO, "Vertical Slider: %.1f", value);
        });

        // 整数滑块示例，提供一个水平整数滑块和一个垂直整数滑块，用户调整时输出当前值
        // 水平整数滑块示例，初始值为 50，用户调整时输出当前值
        int intSliderValue = 50;
        IntSlider horizontalIntSlider({400, 360, 250, 30}, 0, 100, intSliderValue, IntSlider::Direction::Horizontal, [&](int value) {
            intSliderValue = value;
            TraceLog(LOG_INFO, "Horizontal IntSlider: %d", value);
        });

        // 垂直整数滑块示例，初始值为 25，用户调整时输出当前值
        int intVerticalSliderValue = 25;
        IntSlider verticalIntSlider({900, 280, 30, 200}, 0, 100, intVerticalSliderValue, IntSlider::Direction::Vertical, [&](int value) {
            intVerticalSliderValue = value;
            TraceLog(LOG_INFO, "Vertical IntSlider: %d", value);
        });

        // 自定义渲染开关，允许用户切换是否使用自定义皮肤进行渲染，方便测试不同皮肤的效果
        bool useCustomRender = false;
        CheckBox customRenderCheckBox({50, 620, 20, 20}, "Use Custom Skin", useCustomRender, [&](bool checked) {
            useCustomRender = checked;
        });
    //-----

    // 皮肤实例
    DarkSkin darkSkin;              // 原有的暗色皮肤
    LightSkin lightSkin;            // 原有的内置皮肤
    SimpleCustomSkin customSkin;    // 原有的简单皮肤

    while (!WindowShouldClose()) {
        FocusManager::Instance().Update(); // 更新焦点管理器状态，处理输入和交互逻辑

        // 根据开关切换皮肤
        IGuiSkin* activeSkin = useCustomRender ? &darkSkin : nullptr;

        // 将所有组件的皮肤设置为当前选定的皮肤（如果 activeSkin 为 nullptr，则组件将使用默认绘制逻辑）
        button.SetSkin(activeSkin);
        checkBox.SetSkin(activeSkin);
        textBox.SetSkin(activeSkin);
        comboBox.SetSkin(activeSkin);
        multiselect.SetSkin(activeSkin);
        colorPicker.SetSkin(activeSkin);
        horizontalSlider.SetSkin(activeSkin);
        verticalSlider.SetSkin(activeSkin);
        horizontalIntSlider.SetSkin(activeSkin);
        verticalIntSlider.SetSkin(activeSkin);
        customRenderCheckBox.SetSkin(activeSkin);

        // 更新组件状态，处理用户输入和交互
        colorPicker.Update();
        multiselect.Update();
        comboBox.Update();
        button.Update();
        checkBox.Update();
        textBox.Update();
        customRenderCheckBox.Update();
        horizontalSlider.Update();
        verticalSlider.Update();
        horizontalIntSlider.Update();
        verticalIntSlider.Update();

        // --- 开始离屏渲染 GUI 到纹理 ---
        BeginTextureMode(guiTarget);
            ClearBackground(BLANK);

            // 绘制调试信息和组件状态文本，帮助验证组件的交互和状态变化是否正确
            DrawText("GUI Components Test", 350, 15, 24, DARKGRAY);
            DrawText(TextFormat("Button state: %s", button.IsPressed() ? "Pressed" : button.IsHovered() ? "Hovered" : "Normal"), 50, 340, 14, GRAY);
            DrawText(TextFormat("CheckBox state: %s", checkBox.IsChecked() ? "Checked" : "Unchecked"), 50, 360, 14, GRAY);
            DrawText(TextFormat("ComboBox selected: %s", comboBox.GetSelectedItem().c_str()), 400, 180, 14, GRAY);
            Color pickerColor = colorPicker.GetColor();
            char hexColor[16];
            snprintf(hexColor, sizeof(hexColor), "#%02X%02X%02X", pickerColor.r, pickerColor.g, pickerColor.b);
            DrawText(TextFormat("ColorPicker: %s", hexColor), 650, 100, 14, GRAY);
            DrawText(TextFormat("TextBox text length: %d", textBox.GetText().length()), 50, 520, 14, GRAY);
            DrawText(TextFormat("Horizontal Float Slider: %.1f", sliderValue), 400, 260, 14, GRAY);
            DrawText(TextFormat("Vertical Float Slider: %.1f", verticalSliderValue), 850, 35, 14, GRAY);
            DrawText(TextFormat("Horizontal Int Slider: %d", intSliderValue), 400, 340, 14, GRAY);
            DrawText(TextFormat("Vertical Int Slider: %d", intVerticalSliderValue), 850, 260, 14, GRAY);

            std::vector<std::string> selectedItems = multiselect.GetSelectedItems();
            std::string selectedText = "Multiselect selected: ";
            if (selectedItems.empty()) {
                selectedText += "None";
            }
            else {
                for (size_t i = 0; i < selectedItems.size(); i++) {
                    if (i > 0) selectedText += ", ";
                    selectedText += selectedItems[i];
                }
            }
            DrawText(selectedText.c_str(), 400, 160, 14, GRAY);
            DrawText(TextFormat("Render mode: %s", useCustomRender ? "Custom" : "Default"), 50, 640, 14, GRAY);

            if (featureEnabled) {
                DrawText("Feature is ENABLED", 50, 145, 16, GREEN);
            }
            else {
                DrawText("Feature is DISABLED", 50, 145, 16, RED);
            }

            // 绘制所有组件到纹理
            button.Draw();
            checkBox.Draw();
            textBox.Draw();
            colorPicker.Draw();
            customRenderCheckBox.Draw();
            horizontalSlider.Draw();
            verticalSlider.Draw();
            horizontalIntSlider.Draw();
            verticalIntSlider.Draw();
            multiselect.Draw();
            comboBox.Draw();
        EndTextureMode();

        BeginDrawing();
            ClearBackground(RAYWHITE);

            // 绘制 GUI 纹理到屏幕
            DrawTextureRec(guiTarget.texture, Rectangle{ 0, 0, (float)guiTarget.texture.width, (float)-guiTarget.texture.height }, Vector2{ 0, 0 }, WHITE);
            DrawFPS(900, 15);
        EndDrawing();
    }
    // 释放资源
    UnloadRenderTexture(guiTarget);
    return 0;
}