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

ColorPicker* ColorPicker::activePicker = nullptr;

bool ColorPicker::IsAnyPickerActive() {
    return activePicker != nullptr;
}

void ColorPicker::CloseAllPickers() {
    if (activePicker != nullptr) {
        activePicker->isOpen = false;
        activePicker = nullptr;
    }
}

ColorPicker::ColorPicker(Rectangle bounds, Color color, std::function<void(Color)> onColorChange)
    : bounds(bounds), currentColor(color), onColorChange(onColorChange), isOpen(false),
    isDraggingSatVal(false), isDraggingHue(false), isDraggingAlpha(false),
    isEditing(false), editingComponent(0) {
    state = COLORPICKER_STATE_NORMAL;

    ColorPickerUtils::RGBToHSV(color, hue, saturation, value);
    alpha = color.a / 255.0f;

    previewColor = {bounds.x, bounds.y, bounds.width, bounds.height};
}

void ColorPicker::Update() {
    Vector2 mousePos = GetMousePosition();

    float pickerWidth = 200.0f;
    float pickerHeight = 180.0f;
    float sliderWidth = 15.0f;
    float pickerX = bounds.x;
    float pickerY = bounds.y + bounds.height + 5;
    Rectangle panelBg = {pickerX - 5, pickerY - 5, pickerWidth + sliderWidth + 10, pickerHeight + 45};

    if (isOpen) {
        if (this != activePicker) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (!CheckCollisionPointRec(mousePos, panelBg) && !CheckCollisionPointRec(mousePos, bounds)) {
                    isOpen = false;
                }
            }
            return;
        }

        satValueArea = {pickerX, pickerY, pickerWidth, pickerHeight};
        hueSlider = {pickerX + pickerWidth + 5, pickerY, sliderWidth, pickerHeight};
        alphaSlider = {pickerX, pickerY + pickerHeight + 5, pickerWidth, 12};

        bool isOverPicker = CheckCollisionPointRec(mousePos, panelBg) ||
                          CheckCollisionPointRec(mousePos, bounds);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isDraggingSatVal = CheckCollisionPointRec(mousePos, satValueArea);
            isDraggingHue = CheckCollisionPointRec(mousePos, hueSlider);
            isDraggingAlpha = CheckCollisionPointRec(mousePos, alphaSlider);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isDraggingSatVal = false;
            isDraggingHue = false;
            isDraggingAlpha = false;
        }

        if (isDraggingSatVal) {
            float localX = (mousePos.x - satValueArea.x) / satValueArea.width;
            float localY = (mousePos.y - satValueArea.y) / satValueArea.height;
            saturation = std::max(0.0f, std::min(1.0f, localX));
            value = std::max(0.0f, std::min(1.0f, 1.0f - localY));
            currentColor = ColorPickerUtils::HSVToRGB(hue, saturation, value, alpha);
            if (onColorChange) {
                onColorChange(currentColor);
            }
        }

        if (isDraggingHue) {
            float localY = (mousePos.y - hueSlider.y) / hueSlider.height;
            hue = std::max(0.0f, std::min(1.0f, localY));
            currentColor = ColorPickerUtils::HSVToRGB(hue, saturation, value, alpha);
            if (onColorChange) {
                onColorChange(currentColor);
            }
        }

        if (isDraggingAlpha) {
            float localX = (mousePos.x - alphaSlider.x) / alphaSlider.width;
            alpha = std::max(0.0f, std::min(1.0f, localX));
            currentColor.a = (unsigned char)(alpha * 255.0f);
            if (onColorChange) {
                onColorChange(currentColor);
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isOverPicker && !CheckCollisionPointRec(mousePos, bounds)) {
            isOpen = false;
            activePicker = nullptr;
        }
        return;
    }

    bool isHovered = CheckCollisionPointRec(mousePos, bounds);

    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state = COLORPICKER_STATE_PRESSED;
        } else {
            state = COLORPICKER_STATE_HOVER;
        }
    } else {
        state = COLORPICKER_STATE_NORMAL;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isHovered) {
        if (activePicker != nullptr && activePicker != this) {
            activePicker->isOpen = false;
        }
        ComboBox::CloseAllComboBoxes();
        Multiselect::CloseAllMultiselects();
        isOpen = true;
        activePicker = this;
    }

    previewColor = {bounds.x, bounds.y, bounds.width, bounds.height};
}

void ColorPicker::Draw() {
    DrawRectangleRec(previewColor, currentColor);
    DrawRectangleLinesEx(previewColor, 1, borderColor);

    if (isOpen) {
        float pickerWidth = 200.0f;
        float pickerHeight = 180.0f;
        float sliderWidth = 15.0f;
        float inputsHeight = 90.0f;

        float pickerX = bounds.x;
        float pickerY = bounds.y + bounds.height + 5;

        Rectangle panelBg = {pickerX - 5, pickerY - 5, pickerWidth + sliderWidth + 10, pickerHeight + 45};
        DrawRectangleRec(panelBg, {255, 255, 255, 255});
        DrawRectangleLinesEx(panelBg, 1, borderColor);

        Color hueColor = ColorPickerUtils::HSVToRGB(hue, 1.0f, 1.0f, 1.0f);
        for (int y = 0; y < (int)pickerHeight; y++) {
            float v = 1.0f - (float)y / pickerHeight;
            for (int x = 0; x < (int)pickerWidth; x++) {
                float s = (float)x / pickerWidth;
                Color pixelColor = ColorPickerUtils::HSVToRGB(hue, s, v, 1.0f);
                DrawPixel(pickerX + x, pickerY + y, pixelColor);
            }
        }
        DrawRectangleLinesEx(satValueArea, 1, borderColor);

        float satValX = pickerX + saturation * pickerWidth;
        float satValY = pickerY + (1.0f - value) * pickerHeight;
        DrawCircleLines((int)satValX, (int)satValY, 5, WHITE);
        DrawCircleLines((int)satValX, (int)satValY, 6, BLACK);

        for (int y = 0; y < (int)pickerHeight; y++) {
            float h = (float)y / pickerHeight;
            DrawLineEx({pickerX + pickerWidth + 5, pickerY + y}, {pickerX + pickerWidth + 5 + sliderWidth, pickerY + y}, 1, ColorPickerUtils::GetHueColor(h));
        }
        DrawRectangleLinesEx(hueSlider, 1, borderColor);

        float hueY = pickerY + hue * pickerHeight;
        DrawRectangleLinesEx({pickerX + pickerWidth + 5 - 1, hueY - 2, sliderWidth + 2, 5}, 1, WHITE);
        DrawRectangleLinesEx({pickerX + pickerWidth + 5 - 2, hueY - 3, sliderWidth + 4, 7}, 1, BLACK);

        for (int x = 0; x < (int)pickerWidth; x++) {
            float a = (float)x / pickerWidth;
            unsigned char alphaVal = (unsigned char)(a * 255.0f);
            DrawLineEx({pickerX + x, pickerY + pickerHeight + 5 + 1}, {pickerX + x, pickerY + pickerHeight + 5 + 11}, 1,
                {currentColor.r, currentColor.g, currentColor.b, alphaVal});
        }
        DrawRectangleLinesEx(alphaSlider, 1, borderColor);

        float alphaX = pickerX + alpha * pickerWidth;
        DrawRectangleLinesEx({alphaX - 2, pickerY + pickerHeight + 5, 4, 13}, 1, WHITE);
        DrawRectangleLinesEx({alphaX - 3, pickerY + pickerHeight + 5 - 1, 6, 15}, 1, BLACK);

        char hexBuffer[16];
        snprintf(hexBuffer, sizeof(hexBuffer), "#%02X%02X%02X", currentColor.r, currentColor.g, currentColor.b);
        DrawText(hexBuffer, pickerX + pickerWidth - 60, pickerY + pickerHeight + 20, 10, BLACK);

        char rgbBuffer[32];
        snprintf(rgbBuffer, sizeof(rgbBuffer), "R:%d G:%d B:%d", currentColor.r, currentColor.g, currentColor.b);
        DrawText(rgbBuffer, pickerX, pickerY + pickerHeight + 20, 10, BLACK);
    }
}

void ColorPicker::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle, float, int, Color)> drawRoundedRect, std::function<void(Vector2, float, int, Color)> drawCircle) {
    drawRect(previewColor, currentColor);
    drawBorder(previewColor, borderColor, 1.0f);

    if (isOpen) {
        float pickerWidth = 200.0f;
        float pickerHeight = 180.0f;
        float sliderWidth = 15.0f;

        float pickerX = bounds.x;
        float pickerY = bounds.y + bounds.height + 5;

        satValueArea = {pickerX, pickerY, pickerWidth, pickerHeight};
        hueSlider = {pickerX + pickerWidth + 5, pickerY, sliderWidth, pickerHeight};
        alphaSlider = {pickerX, pickerY + pickerHeight + 5, pickerWidth, 12};

        for (int y = 0; y < (int)pickerHeight; y++) {
            float v = 1.0f - (float)y / pickerHeight;
            for (int x = 0; x < (int)pickerWidth; x++) {
                float s = (float)x / pickerWidth;
                Color pixelColor = ColorPickerUtils::HSVToRGB(hue, s, v, 1.0f);
                drawRect({(float)(pickerX + x), (float)(pickerY + y), 1.0f, 1.0f}, pixelColor);
            }
        }
        drawBorder(satValueArea, borderColor, 1.0f);

        for (int y = 0; y < (int)pickerHeight; y++) {
            float h = (float)y / pickerHeight;
            drawLine({pickerX + pickerWidth + 5, pickerY + y}, {pickerX + pickerWidth + 5 + sliderWidth, pickerY + y}, 1, ColorPickerUtils::GetHueColor(h));
        }
        drawBorder(hueSlider, borderColor, 1.0f);

        for (int x = 0; x < (int)pickerWidth; x++) {
            float a = (float)x / pickerWidth;
            unsigned char alphaVal = (unsigned char)(a * 255.0f);
            drawLine({pickerX + x, pickerY + pickerHeight + 5 + 1}, {pickerX + x, pickerY + pickerHeight + 5 + 11}, 1,
                {currentColor.r, currentColor.g, currentColor.b, alphaVal});
        }
        drawBorder(alphaSlider, borderColor, 1.0f);

        char hexBuffer[16];
        snprintf(hexBuffer, sizeof(hexBuffer), "#%02X%02X%02X", currentColor.r, currentColor.g, currentColor.b);
        drawText(hexBuffer, {pickerX + pickerWidth - 60, (float)(pickerY + pickerHeight + 20)}, 10, 1.0f, BLACK);
    }
}

Color ColorPicker::GetColor() const {
    return currentColor;
}

void ColorPicker::SetColor(Color color) {
    currentColor = color;
    ColorPickerUtils::RGBToHSV(color, hue, saturation, value);
    alpha = color.a / 255.0f;
}

void ColorPicker::SetPosition(float x, float y) {
    bounds.x = x;
    bounds.y = y;
}

void ColorPicker::SetSize(float width, float height) {
    bounds.width = width;
    bounds.height = height;
}

void ColorPicker::SetOnColorChange(std::function<void(Color)> callback) {
    onColorChange = callback;
}
