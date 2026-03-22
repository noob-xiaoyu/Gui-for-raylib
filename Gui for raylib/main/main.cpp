#include <raylib.h>
#include "Gui/Button/Button.h"
#include "Gui/CheckBox/CheckBox.h"
#include "Gui/ColorPicker/ColorPicker.h"
#include "Gui/ComboBox/ComboBox.h"
#include "Gui/Multiselect/Multiselect.h"
#include "Gui/TextBox/TextBox.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "GUI Components Test");

    SetTargetFPS(240);

    // 创建 Button
    Button button({100, 100, 150, 50}, "Click Me", []() {
        TraceLog(LOG_INFO, "Button clicked!");
    });

    // 创建 CheckBox
    bool featureEnabled = false;
    CheckBox checkBox({100, 200, 20, 20}, "Enable Feature", featureEnabled, [&](bool checked) {
        featureEnabled = checked;
        TraceLog(LOG_INFO, "CheckBox state: %s", checked ? "checked" : "unchecked");
    });

    // 创建 TextBox
    TextBox textBox({100, 300, 300, 200});
    textBox.SetText("Enter text here...");

    // 创建 ComboBox
    std::vector<std::string> comboItems = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5", "Option 6", "Option 7", "Option 8"};
    ComboBox comboBox({450, 300, 200, 40}, comboItems, 0, [](int index) {
        TraceLog(LOG_INFO, "ComboBox selected: %d", index);
    });

    // 创建 Multiselect
    std::vector<std::string> multiItems = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5", "Item 6", "Item 7", "Item 8"};
    Multiselect multiselect({450, 400, 200, 40}, multiItems, [](const std::vector<int>& indices) {
        TraceLog(LOG_INFO, "Multiselect selected indices:");
        for (int index : indices) {
            TraceLog(LOG_INFO, "%d", index);
        }
    });

    // 创建 ColorPicker
    ColorPicker colorPicker({800, 400, 60, 40}, RED, [](Color color) {
        char hex[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", color.r, color.g, color.b);
        TraceLog(LOG_INFO, "ColorPicker: %s", hex);
    });

    // 切换自定义渲染模式
    bool useCustomRender = false;
    CheckBox customRenderCheckBox({450, 100, 20, 20}, "Use Custom Render", useCustomRender, [&](bool checked) {
        useCustomRender = checked;
    });

    while (!WindowShouldClose()) {
        // 更新所有组件
        colorPicker.Update();

        if (ColorPicker::IsAnyPickerActive()) {
            button.Update();
            checkBox.Update();
        } else {
            multiselect.Update();
            comboBox.Update();
        }
        textBox.Update();
        customRenderCheckBox.Update();

        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        // 绘制标题
        DrawText("GUI Components Test", 300, 20, 24, DARKGRAY);
		DrawFPS(700, 20);

        // 绘制组件
        if (useCustomRender) {
            // 使用自定义渲染函数
            button.Draw(
                [](Rectangle rect, Color color) {
                    // 绘制圆角按钮
                    DrawRectangleRounded(rect, 0.3f, 8, color);
                },
                [](Rectangle rect, Color color, float thickness) {
                    // 绘制边框
                    DrawRectangleLinesEx(rect, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                    // 绘制带阴影的文字
                    DrawTextEx(GetFontDefault(), text, {pos.x + 1, pos.y + 1}, fontSize, spacing, {0, 0, 0, 100});
                    DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
                }
            );

            checkBox.Draw(
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.2f, 4, color);
                },
                [](Vector2 start, Vector2 end, float thickness, Color color) {
                    DrawLineEx(start, end, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                    DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
                }
            );

            textBox.Draw(
                LIGHTGRAY, BLACK,
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.1f, 8, color);
                },
                [](Vector2 start, Vector2 end, float thickness, Color color) {
                    DrawLineEx(start, end, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                    DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
                },
                [](Rectangle rect) {
                    BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
                },
                []() {
                    EndScissorMode();
                }
            );

            colorPicker.Draw(
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.1f, 4, color);
                },
                [](Rectangle rect, Color color, float thickness) {
                    DrawRectangleLinesEx(rect, thickness, color);
                },
                [](Vector2 start, Vector2 end, float thickness, Color color) {
                    DrawLineEx(start, end, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                    DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
                },
                [](Rectangle rect, float roundness, int segments, Color color) {
                    DrawRectangleRounded(rect, roundness, segments, color);
                },
                [](Vector2 center, float radius, int segments, Color color) {
                    DrawCircleV(center, radius, color);
                }
            );
        } else {
            // 使用默认渲染
            button.Draw();
            checkBox.Draw();
            textBox.Draw(LIGHTGRAY, BLACK);
            colorPicker.Draw();
        }

        // 绘制自定义渲染选项
        customRenderCheckBox.Draw();
        
        // 显示状态信息
        DrawText(TextFormat("Button state: %s", button.IsPressed() ? "Pressed" : button.IsHovered() ? "Hovered" : "Normal"), 100, 520, 14, GRAY);
        DrawText(TextFormat("CheckBox state: %s", checkBox.IsChecked() ? "Checked" : "Unchecked"), 100, 540, 14, GRAY);
        DrawText(TextFormat("ComboBox selected: %s", comboBox.GetSelectedItem().c_str()), 100, 560, 14, GRAY);
        Color pickerColor = colorPicker.GetColor();
        char hexColor[16];
        snprintf(hexColor, sizeof(hexColor), "#%02X%02X%02X", pickerColor.r, pickerColor.g, pickerColor.b);
        DrawText(TextFormat("ColorPicker: %s", hexColor), 100, 580, 14, GRAY);
        DrawText(TextFormat("TextBox text length: %d", textBox.GetText().length()), 450, 520, 14, GRAY);
        std::vector<std::string> selectedItems = multiselect.GetSelectedItems();
        std::string selectedText = "Multiselect selected: ";
        if (selectedItems.empty()) {
            selectedText += "None";
        } else {
            for (size_t i = 0; i < selectedItems.size(); i++) {
                if (i > 0) selectedText += ", ";
                selectedText += selectedItems[i];
            }
        }
        DrawText(selectedText.c_str(), 450, 540, 14, GRAY);
        DrawText(TextFormat("Render mode: %s", useCustomRender ? "Custom" : "Default"), 450, 130, 14, GRAY);
        
        // 显示功能状态
        if (featureEnabled) {
            DrawText("Feature is ENABLED", 450, 205, 16, GREEN);
        } else {
            DrawText("Feature is DISABLED", 450, 205, 16, RED);
        }
        
        // 最后绘制ComboBox和Multiselect，确保下拉列表在最上层
        if (useCustomRender) {
            multiselect.Draw(
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.1f, 8, color);
                },
                [](Rectangle rect, Color color, float thickness) {
                    DrawRectangleLinesEx(rect, thickness, color);
                },
                [](Vector2 start, Vector2 end, float thickness, Color color) {
                    DrawLineEx(start, end, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                    DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
                },
                [](Rectangle rect) {
                    BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
                },
                []() {
                    EndScissorMode();
                },
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.5f, 4, color);
                }
            );
            comboBox.Draw(
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.1f, 8, color);
                },
                [](Rectangle rect, Color color, float thickness) {
                    DrawRectangleLinesEx(rect, thickness, color);
                },
                [](Vector2 start, Vector2 end, float thickness, Color color) {
                    DrawLineEx(start, end, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
                    DrawTextEx(GetFontDefault(), text, pos, fontSize, spacing, color);
                },
                [](Rectangle rect) {
                    BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
                },
                []() {
                    EndScissorMode();
                },
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.5f, 4, color);
                }
            );
        } else {
            multiselect.Draw();
            comboBox.Draw();
        }

        EndDrawing();
    }

    return 0;
}