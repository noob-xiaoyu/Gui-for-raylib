#include "Button.h"

Button::Button(Rectangle bounds, std::string text, std::function<void()> onClick)
    : bounds(bounds), text(text), onClick(onClick) {
    state = BUTTON_STATE_NORMAL;
}

void Button::SetPos(float x, float y) {
    bounds.x = x;
    bounds.y = y;
}

void Button::SetPos(Vector2 pos) {
    bounds.x = pos.x;
    bounds.y = pos.y;
}

void Button::SetSize(float width, float height) {
    bounds.width = width;
    bounds.height = height;
}

void Button::SetSize(Vector2 size) {
    bounds.width = size.x;
    bounds.height = size.y;
}

Vector2 Button::GetPos() const {
    return { bounds.x, bounds.y };
}

Vector2 Button::GetSize() const {
    return { bounds.width, bounds.height };
}

void Button::Update() {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);

    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state = BUTTON_STATE_PRESSED;
        } else {
            if (state == BUTTON_STATE_PRESSED && onClick) {
                onClick();
            }
            state = BUTTON_STATE_HOVER;
        }
    } else {
        state = BUTTON_STATE_NORMAL;
    }
}

void Button::Draw() {
    // 确定当前颜色
    Color currentColor = baseColor;
    if (state == BUTTON_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (state == BUTTON_STATE_PRESSED) {
        currentColor = pressedColor;
    }
    
    // 绘制按钮背景
    DrawRectangleRec(bounds, currentColor);
    
    // 绘制按钮文本
    if (!text.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + (bounds.width - textSize.x) / 2,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text.c_str(), textPos, fontSize, 1.0f, textColor);
    }
}

void Button::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText) {
    // 确定当前颜色
    Color currentColor = baseColor;
    if (state == BUTTON_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (state == BUTTON_STATE_PRESSED) {
        currentColor = pressedColor;
    }
    
    // 绘制按钮背景
    drawRect(bounds, currentColor);
    
    // 绘制按钮边框
    drawBorder(bounds, textColor, 2.0f);
    
    // 绘制按钮文本
    if (!text.empty()) {
        // 假设使用默认字体，计算文本位置
        float textX = bounds.x + (bounds.width - MeasureText(text.c_str(), fontSize)) / 2;
        float textY = bounds.y + (bounds.height - fontSize) / 2;
        drawText(text.c_str(), { textX, textY }, fontSize, 1.0f, textColor);
    }
}

Rectangle Button::GetBtn() const {
    return bounds;
}

std::string Button::GetText() const {
    return text;
}

int Button::GetFontSize() const {
    return fontSize;
}

bool Button::IsHovered() const {
    return state == BUTTON_STATE_HOVER;
}

bool Button::IsPressed() const {
    return state == BUTTON_STATE_PRESSED;
}
