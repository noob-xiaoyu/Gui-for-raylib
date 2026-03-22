#include "ColorPicker.h"
#include "../ComboBox/ComboBox.h"
#include "../Multiselect/Multiselect.h"
#include <algorithm>
#include <cstdio>

namespace ColorPickerUtils {
    Color HSVToRGB(float h, float s, float v, float a) {
        float r, g, b;
        int i = (int)(h * 6.0f);
        float f = h * 6.0f - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);

        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }

        return Color{
            (unsigned char)(r * 255.0f),
            (unsigned char)(g * 255.0f),
            (unsigned char)(b * 255.0f),
            (unsigned char)(a * 255.0f)
        };
    }

    void RGBToHSV(Color rgb, float& h, float& s, float& v) {
        float r = rgb.r / 255.0f;
        float g = rgb.g / 255.0f;
        float b = rgb.b / 255.0f;

        float maxVal = std::max({r, g, b});
        float minVal = std::min({r, g, b});
        float delta = maxVal - minVal;

        v = maxVal;

        if (maxVal == 0.0f) {
            s = 0.0f;
        } else {
            s = delta / maxVal;
        }

        if (delta == 0.0f) {
            h = 0.0f;
        } else if (maxVal == r) {
            h = (g - b) / delta;
            if (h < 0.0f) h += 6.0f;
        } else if (maxVal == g) {
            h = 2.0f + (b - r) / delta;
        } else {
            h = 4.0f + (r - g) / delta;
        }

        h /= 6.0f;
    }

    Color GetHueColor(float hue) {
        return HSVToRGB(hue, 1.0f, 1.0f, 1.0f);
    }
}

ColorPicker* ColorPicker::s_activePicker = nullptr;

bool ColorPicker::IsAnyPickerActive() {
    return s_activePicker != nullptr;
}

void ColorPicker::CloseAllPickers() {
    if (s_activePicker != nullptr) {
        s_activePicker->m_isOpen = false;
        s_activePicker = nullptr;
    }
}

ColorPicker::ColorPicker(Rectangle bounds, Color color, std::function<void(Color)> onColorChange)
    : m_bounds(bounds), m_currentColor(color), m_onColorChange(onColorChange), m_isOpen(false),
    m_isDraggingSatVal(false), m_isDraggingHue(false), m_isDraggingAlpha(false),
    m_isEditing(false), m_editingComponent(0) {
    m_state = COLORPICKER_STATE_NORMAL;

    ColorPickerUtils::RGBToHSV(color, m_hue, m_saturation, m_value);
    m_alpha = color.a / 255.0f;

    m_previewColor = {bounds.x, bounds.y, bounds.width, bounds.height};
    FocusManager::Instance().RegisterControl(this);
}

Rectangle ColorPicker::GetBounds() const {
    if (m_isOpen) {
        float pickerWidth = 200.0f;
        float pickerHeight = 180.0f;
        float sliderWidth = 15.0f;
        float inputsHeight = 90.0f;
        return {
            m_bounds.x - 5,
            m_bounds.y - 5,
            pickerWidth + sliderWidth + 10,
            pickerHeight + 45 + m_bounds.height + 5
        };
    }
    return m_bounds;
}

void ColorPicker::Update() {
    if (!m_isEnabled) return;

    Vector2 mousePos = GetMousePosition();

    float pickerWidth = 200.0f;
    float pickerHeight = 180.0f;
    float sliderWidth = 15.0f;
    float pickerX = m_bounds.x;
    float pickerY = m_bounds.y + m_bounds.height + 5;
    Rectangle panelBg = {pickerX - 5, pickerY - 5, pickerWidth + sliderWidth + 15, pickerHeight + 45};

    if (m_isOpen) {
        if (this != s_activePicker) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (!CheckCollisionPointRec(mousePos, panelBg) && !CheckCollisionPointRec(mousePos, m_bounds)) {
                    m_isOpen = false;
                }
            }
            return;
        }

        m_satValueArea = {pickerX, pickerY, pickerWidth, pickerHeight};
        m_hueSlider = {pickerX + pickerWidth + 5, pickerY, sliderWidth, pickerHeight};
        m_alphaSlider = {pickerX, pickerY + pickerHeight + 5, pickerWidth, 12};

        bool isOverPicker = CheckCollisionPointRec(mousePos, panelBg) ||
                          CheckCollisionPointRec(mousePos, m_bounds);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_isDraggingSatVal = CheckCollisionPointRec(mousePos, m_satValueArea);
            m_isDraggingHue = CheckCollisionPointRec(mousePos, m_hueSlider);
            m_isDraggingAlpha = CheckCollisionPointRec(mousePos, m_alphaSlider);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            m_isDraggingSatVal = false;
            m_isDraggingHue = false;
            m_isDraggingAlpha = false;
        }

        if (m_isDraggingSatVal) {
            float localX = (mousePos.x - m_satValueArea.x) / m_satValueArea.width;
            float localY = (mousePos.y - m_satValueArea.y) / m_satValueArea.height;
            m_saturation = std::max(0.0f, std::min(1.0f, localX));
            m_value = std::max(0.0f, std::min(1.0f, 1.0f - localY));
            m_currentColor = ColorPickerUtils::HSVToRGB(m_hue, m_saturation, m_value, m_alpha);
            if (m_onColorChange) {
                m_onColorChange(m_currentColor);
            }
        }

        if (m_isDraggingHue) {
            float localY = (mousePos.y - m_hueSlider.y) / m_hueSlider.height;
            m_hue = std::max(0.0f, std::min(1.0f, localY));
            m_currentColor = ColorPickerUtils::HSVToRGB(m_hue, m_saturation, m_value, m_alpha);
            if (m_onColorChange) {
                m_onColorChange(m_currentColor);
            }
        }

        if (m_isDraggingAlpha) {
            float localX = (mousePos.x - m_alphaSlider.x) / m_alphaSlider.width;
            m_alpha = std::max(0.0f, std::min(1.0f, localX));
            m_currentColor.a = (unsigned char)(m_alpha * 255.0f);
            if (m_onColorChange) {
                m_onColorChange(m_currentColor);
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isOverPicker && !CheckCollisionPointRec(mousePos, m_bounds)) {
            m_isOpen = false;
            s_activePicker = nullptr;
        }
        return;
    }

    GuiControl* controlAtMouse = FocusManager::Instance().GetControlAtMouse();
    bool isHoveredByFocus = (controlAtMouse == this);
    bool isHovered = CheckCollisionPointRec(mousePos, m_bounds) || isHoveredByFocus;

    if (isHoveredByFocus) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = COLORPICKER_STATE_PRESSED;
        } else {
            m_state = COLORPICKER_STATE_HOVER;
        }
    } else {
        m_state = COLORPICKER_STATE_NORMAL;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isHoveredByFocus) {
        if (s_activePicker != nullptr && s_activePicker != this) {
            s_activePicker->m_isOpen = false;
        }
        ComboBox::CloseAllComboBoxes();
        Multiselect::CloseAllMultiselects();
        m_isOpen = true;
        s_activePicker = this;
    }

    m_previewColor = {m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height};
}

void ColorPicker::Draw() {
    DrawRectangleRec(m_previewColor, m_currentColor);
    DrawRectangleLinesEx(m_previewColor, 1, borderColor);

    if (m_isOpen) {
        float pickerWidth = 200.0f;
        float pickerHeight = 180.0f;
        float sliderWidth = 15.0f;

        float pickerX = m_bounds.x;
        float pickerY = m_bounds.y + m_bounds.height + 5;

        Rectangle panelBg = {pickerX - 5, pickerY - 5, pickerWidth + sliderWidth + 15, pickerHeight + 45};
        DrawRectangleRec(panelBg, {255, 255, 255, 255});
        DrawRectangleLinesEx(panelBg, 1, borderColor);

        Color hueColor = ColorPickerUtils::HSVToRGB(m_hue, 1.0f, 1.0f, 1.0f);
        for (int y = 0; y < (int)pickerHeight; y++) {
            float v = 1.0f - (float)y / pickerHeight;
            for (int x = 0; x < (int)pickerWidth; x++) {
                float s = (float)x / pickerWidth;
                Color pixelColor = ColorPickerUtils::HSVToRGB(m_hue, s, v, 1.0f);
                DrawPixel(pickerX + x, pickerY + y, pixelColor);
            }
        }
        DrawRectangleLinesEx(m_satValueArea, 1, borderColor);

        float satValX = pickerX + m_saturation * pickerWidth;
        float satValY = pickerY + (1.0f - m_value) * pickerHeight;
        DrawCircleLines((int)satValX, (int)satValY, 5, WHITE);
        DrawCircleLines((int)satValX, (int)satValY, 6, BLACK);

        for (int y = 0; y < (int)pickerHeight; y++) {
            float h = (float)y / pickerHeight;
            DrawLineEx({pickerX + pickerWidth + 5, pickerY + y}, {pickerX + pickerWidth + 5 + sliderWidth, pickerY + y}, 1, ColorPickerUtils::GetHueColor(h));
        }
        DrawRectangleLinesEx(m_hueSlider, 1, borderColor);

        float hueY = pickerY + m_hue * pickerHeight;
        DrawRectangleLinesEx({pickerX + pickerWidth + 5 - 1, hueY - 2, sliderWidth + 2, 5}, 1, WHITE);
        DrawRectangleLinesEx({pickerX + pickerWidth + 5 - 2, hueY - 3, sliderWidth + 4, 7}, 1, BLACK);

        for (int x = 0; x < (int)pickerWidth; x++) {
            float a = (float)x / pickerWidth;
            unsigned char alphaVal = (unsigned char)(a * 255.0f);
            DrawLineEx({pickerX + x, pickerY + pickerHeight + 5 + 1}, {pickerX + x, pickerY + pickerHeight + 5 + 11}, 1,
                {m_currentColor.r, m_currentColor.g, m_currentColor.b, alphaVal});
        }
        DrawRectangleLinesEx(m_alphaSlider, 1, borderColor);

        float alphaX = pickerX + m_alpha * pickerWidth;
        DrawRectangleLinesEx({alphaX - 2, pickerY + pickerHeight + 5, 4, 13}, 1, WHITE);
        DrawRectangleLinesEx({alphaX - 3, pickerY + pickerHeight + 5 - 1, 6, 15}, 1, BLACK);

        char hexBuffer[16];
        snprintf(hexBuffer, sizeof(hexBuffer), "#%02X%02X%02X", m_currentColor.r, m_currentColor.g, m_currentColor.b);
        DrawText(hexBuffer, pickerX + pickerWidth - 60, pickerY + pickerHeight + 20, 10, BLACK);

        char rgbBuffer[32];
        snprintf(rgbBuffer, sizeof(rgbBuffer), "R:%d G:%d B:%d", m_currentColor.r, m_currentColor.g, m_currentColor.b);
        DrawText(rgbBuffer, pickerX, pickerY + pickerHeight + 20, 10, BLACK);
    }
}

void ColorPicker::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle, float, int, Color)> drawRoundedRect, std::function<void(Vector2, float, int, Color)> drawCircle) {
    drawRect(m_previewColor, m_currentColor);
    drawBorder(m_previewColor, borderColor, 1.0f);

    if (m_isOpen) {
        float pickerWidth = 200.0f;
        float pickerHeight = 180.0f;
        float sliderWidth = 15.0f;

        float pickerX = m_bounds.x;
        float pickerY = m_bounds.y + m_bounds.height + 5;

        m_satValueArea = {pickerX, pickerY, pickerWidth, pickerHeight};
        m_hueSlider = {pickerX + pickerWidth + 5, pickerY, sliderWidth, pickerHeight};
        m_alphaSlider = {pickerX, pickerY + pickerHeight + 5, pickerWidth, 12};

        for (int y = 0; y < (int)pickerHeight; y++) {
            float v = 1.0f - (float)y / pickerHeight;
            for (int x = 0; x < (int)pickerWidth; x++) {
                float s = (float)x / pickerWidth;
                Color pixelColor = ColorPickerUtils::HSVToRGB(m_hue, s, v, 1.0f);
                drawRect({(float)(pickerX + x), (float)(pickerY + y), 1.0f, 1.0f}, pixelColor);
            }
        }
        drawBorder(m_satValueArea, borderColor, 1.0f);

        for (int y = 0; y < (int)pickerHeight; y++) {
            float h = (float)y / pickerHeight;
            drawLine({pickerX + pickerWidth + 5, pickerY + y}, {pickerX + pickerWidth + 5 + sliderWidth, pickerY + y}, 1, ColorPickerUtils::GetHueColor(h));
        }
        drawBorder(m_hueSlider, borderColor, 1.0f);

        for (int x = 0; x < (int)pickerWidth; x++) {
            float a = (float)x / pickerWidth;
            unsigned char alphaVal = (unsigned char)(a * 255.0f);
            drawLine({pickerX + x, pickerY + pickerHeight + 5 + 1}, {pickerX + x, pickerY + pickerHeight + 5 + 11}, 1,
                {m_currentColor.r, m_currentColor.g, m_currentColor.b, alphaVal});
        }
        drawBorder(m_alphaSlider, borderColor, 1.0f);

        char hexBuffer[16];
        snprintf(hexBuffer, sizeof(hexBuffer), "#%02X%02X%02X", m_currentColor.r, m_currentColor.g, m_currentColor.b);
        drawText(hexBuffer, {pickerX + pickerWidth - 60, (float)(pickerY + pickerHeight + 20)}, 10, 1.0f, BLACK);
    }
}

Color ColorPicker::GetColor() const {
    return m_currentColor;
}

void ColorPicker::SetColor(Color color) {
    m_currentColor = color;
    ColorPickerUtils::RGBToHSV(color, m_hue, m_saturation, m_value);
    m_alpha = color.a / 255.0f;
}

void ColorPicker::SetPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void ColorPicker::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void ColorPicker::SetOnColorChange(std::function<void(Color)> callback) {
    m_onColorChange = callback;
}