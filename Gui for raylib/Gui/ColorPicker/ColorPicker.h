#pragma once

#include <raylib.h>
#include <string>
#include <functional>
#include <vector>

class ComboBox;
class Multiselect;

class ColorPicker {
private:
    Rectangle bounds;
    Color currentColor;
    bool isOpen;
    std::function<void(Color)> onColorChange;

    enum ColorPickerState {
        COLORPICKER_STATE_NORMAL,
        COLORPICKER_STATE_HOVER,
        COLORPICKER_STATE_PRESSED
    } state;

    Rectangle satValueArea;
    Rectangle hueSlider;
    Rectangle alphaSlider;
    Rectangle alphaCheckered;

    bool isDraggingSatVal;
    bool isDraggingHue;
    bool isDraggingAlpha;

    float hue;
    float saturation;
    float value;
    float alpha;

    Rectangle previewColor;
    Rectangle inputsArea;
    bool isEditing;
    std::string inputBuffer;
    int editingComponent;

    static ColorPicker* activePicker;

public:
    static bool IsAnyPickerActive();
    static void CloseAllPickers();

public:
    ColorPicker(Rectangle bounds, Color color = RED, std::function<void(Color)> onColorChange = nullptr);

    void Update();
    void Draw();
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle, float, int, Color)> drawRoundedRect, std::function<void(Vector2, float, int, Color)> drawCircle);

    Color GetColor() const;
    void SetColor(Color color);
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetOnColorChange(std::function<void(Color)> callback);

    Color baseColor = {245, 245, 245, 255};
    Color hoverColor = {230, 230, 230, 255};
    Color pressedColor = {200, 200, 200, 255};
    Color borderColor = {120, 120, 120, 255};
    int fontSize = 12;
};
