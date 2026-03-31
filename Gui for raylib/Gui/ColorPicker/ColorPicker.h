#pragma once

#include <raylib.h>
#include <string>
#include <functional>
#include <vector>
#include "../Focus/FocusManager.h"

class ColorPicker : public GuiControl {
private:
    Rectangle m_bounds;
    Color m_currentColor;
    bool m_isOpen;
    std::function<void(Color)> m_onColorChange;

    enum ColorPickerState {
        COLORPICKER_STATE_NORMAL,
        COLORPICKER_STATE_HOVER,
        COLORPICKER_STATE_PRESSED
    } m_state;

    Rectangle m_satValueArea;
    Rectangle m_hueSlider;
    Rectangle m_alphaSlider;
    Rectangle m_alphaCheckered;

    bool m_isDraggingSatVal;
    bool m_isDraggingHue;
    bool m_isDraggingAlpha;

    float m_hue;
    float m_saturation;
    float m_value;
    float m_alpha;

    Rectangle m_previewColor;
    Rectangle m_inputsArea;
    bool m_isEditing;
    std::string m_inputBuffer;
    int m_editingComponent;

public:
    ColorPicker(Rectangle bounds, Color color = RED, std::function<void(Color)> onColorChange = nullptr);

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle, float, int, Color)> drawRoundedRect, std::function<void(Vector2, float, int, Color)> drawCircle);

    Color GetColor() const;
    void SetColor(Color color);
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetOnColorChange(std::function<void(Color)> callback);

    void Close();

    Color baseColor = {245, 245, 245, 255};
    Color hoverColor = {230, 230, 230, 255};
    Color pressedColor = {200, 200, 200, 255};
    Color borderColor = {120, 120, 120, 255};
    int fontSize = 12;
};