#include <raylib.h>
#include "Gui/Button/Button.h"
#include "Gui/CheckBox/CheckBox.h"
#include "Gui/ColorPicker/ColorPicker.h"
#include "Gui/ComboBox/ComboBox.h"
#include "Gui/Multiselect/Multiselect.h"
#include "Gui/Slider/Slider.h"
#include "Gui/TextBox/TextBox.h"
#include "Gui/Focus/FocusManager.h"

int main() {
    const int screenWidth = 1000;
    const int screenHeight = 700;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "GUI Components Test");

    SetTargetFPS(240);

    Button button({50, 50, 150, 45}, "Click Me", []() {
        TraceLog(LOG_INFO, "Button clicked!");
    });

    bool featureEnabled = false;
    CheckBox checkBox({50, 110, 20, 20}, "Enable Feature", featureEnabled, [&](bool checked) {
        featureEnabled = checked;
        TraceLog(LOG_INFO, "CheckBox state: %s", checked ? "checked" : "unchecked");
    });

    TextBox textBox({50, 160, 300, 150});
    textBox.SetText("Enter text here...");

    std::vector<std::string> comboItems = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5", "Option 6", "Option 7", "Option 8"};
    ComboBox comboBox({400, 50, 200, 40}, comboItems, 0, [](int index) {
        TraceLog(LOG_INFO, "ComboBox selected: %d", index);
    });

    std::vector<std::string> multiItems = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5", "Item 6", "Item 7", "Item 8"};
    Multiselect multiselect({400, 110, 200, 40}, multiItems, [](const std::vector<int>& indices) {
        TraceLog(LOG_INFO, "Multiselect selected indices:");
        for (int index : indices) {
            TraceLog(LOG_INFO, "%d", index);
        }
    });

    ColorPicker colorPicker({650, 50, 60, 40}, RED, [](Color color) {
        char hex[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", color.r, color.g, color.b);
        TraceLog(LOG_INFO, "ColorPicker: %s", hex);
    });

    float sliderValue = 50.0f;
    Slider horizontalSlider({400, 280, 250, 30}, 0.0f, 100.0f, sliderValue, Slider::Direction::Horizontal, [&](float value) {
        sliderValue = value;
        TraceLog(LOG_INFO, "Horizontal Slider: %.1f", value);
    });

    float verticalSliderValue = 75.0f;
    Slider verticalSlider({900, 50, 30, 200}, 0.0f, 100.0f, verticalSliderValue, Slider::Direction::Vertical, [&](float value) {
        verticalSliderValue = value;
        TraceLog(LOG_INFO, "Vertical Slider: %.1f", value);
    });

    int intSliderValue = 50;
    IntSlider horizontalIntSlider({400, 360, 250, 30}, 0, 100, intSliderValue, IntSlider::Direction::Horizontal, [&](int value) {
        intSliderValue = value;
        TraceLog(LOG_INFO, "Horizontal IntSlider: %d", value);
    });

    int intVerticalSliderValue = 25;
    IntSlider verticalIntSlider({900, 280, 30, 200}, 0, 100, intVerticalSliderValue, IntSlider::Direction::Vertical, [&](int value) {
        intVerticalSliderValue = value;
        TraceLog(LOG_INFO, "Vertical IntSlider: %d", value);
    });

    bool useCustomRender = false;
    CheckBox customRenderCheckBox({50, 620, 20, 20}, "Use Custom Render", useCustomRender, [&](bool checked) {
        useCustomRender = checked;
    });

    while (!WindowShouldClose()) {
        FocusManager::Instance().Update();

        colorPicker.Update();

        if (ColorPicker::IsAnyPickerActive()) {
            ComboBox::CloseAllComboBoxes();
            Multiselect::CloseAllMultiselects();
        }

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

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("GUI Components Test", 350, 15, 24, DARKGRAY);
        DrawFPS(900, 15);

        if (useCustomRender) {
            button.Draw(
                [](Rectangle rect, Color color) {
                    DrawRectangleRounded(rect, 0.3f, 8, color);
                },
                [](Rectangle rect, Color color, float thickness) {
                    DrawRectangleLinesEx(rect, thickness, color);
                },
                [](const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
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
            button.Draw();
            checkBox.Draw();
            textBox.Draw(LIGHTGRAY, BLACK);
            colorPicker.Draw();
        }

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
        } else {
            for (size_t i = 0; i < selectedItems.size(); i++) {
                if (i > 0) selectedText += ", ";
                selectedText += selectedItems[i];
            }
        }
        DrawText(selectedText.c_str(), 400, 160, 14, GRAY);
        DrawText(TextFormat("Render mode: %s", useCustomRender ? "Custom" : "Default"), 50, 640, 14, GRAY);

        customRenderCheckBox.Draw();
        horizontalSlider.Draw();
        verticalSlider.Draw();
        horizontalIntSlider.Draw();
        verticalIntSlider.Draw();

        if (featureEnabled) {
            DrawText("Feature is ENABLED", 50, 145, 16, GREEN);
        } else {
            DrawText("Feature is DISABLED", 50, 145, 16, RED);
        }

        if (FocusManager::Instance().HasFocus(&comboBox) || FocusManager::Instance().HasFocus(&multiselect)) {
        } else {
        }

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