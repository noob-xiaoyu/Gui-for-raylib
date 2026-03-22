#include "CheckBox.h"

CheckBox::CheckBox(Rectangle bounds, std::string label, bool checked, std::function<void(bool)> onCheckChange)
    : bounds(bounds), label(label), checked(checked), onCheckChange(onCheckChange) {
    state = CHECKBOX_STATE_NORMAL;
}

void CheckBox::Update() {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);
    
    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state = CHECKBOX_STATE_PRESSED;
        } else {
            if (state == CHECKBOX_STATE_PRESSED) {
                checked = !checked;
                if (onCheckChange) {
                    onCheckChange(checked);
                }
            }
            state = CHECKBOX_STATE_HOVER;
        }
    } else {
        state = CHECKBOX_STATE_NORMAL;
    }
}

void CheckBox::Draw() {
    // 绘制背景
    Color bgColor = fillColor;
    if (state == CHECKBOX_STATE_HOVER) {
        bgColor = hoverColor;
    } else if (state == CHECKBOX_STATE_PRESSED) {
        bgColor = pressedColor;
    }
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 2, borderColor);
    
    // 绘制勾选标记
    if (checked) {
        float size = fmin(bounds.width, bounds.height) * 0.6f;
        float offsetX = bounds.x + (bounds.width - size) / 2;
        float offsetY = bounds.y + (bounds.height - size) / 2;
        
        // 绘制对勾
        DrawLineEx(
            { offsetX + size * 0.2f, offsetY + size * 0.5f },
            { offsetX + size * 0.4f, offsetY + size * 0.8f },
            2.0f, checkColor
        );
        DrawLineEx(
            { offsetX + size * 0.4f, offsetY + size * 0.8f },
            { offsetX + size * 0.8f, offsetY + size * 0.2f },
            2.0f, checkColor
        );
    }
    
    // 绘制标签
    if (!label.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), label.c_str(), 16.0f, 1.0f);
        float textX = bounds.x + bounds.width + 10.0f;
        float textY = bounds.y + (bounds.height - textSize.y) / 2;
        DrawTextEx(GetFontDefault(), label.c_str(), { textX, textY }, 16.0f, 1.0f, textColor);
    }
}

void CheckBox::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText) {
    // 绘制背景
    Color bgColor = fillColor;
    if (state == CHECKBOX_STATE_HOVER) {
        bgColor = hoverColor;
    } else if (state == CHECKBOX_STATE_PRESSED) {
        bgColor = pressedColor;
    }
    
    // 绘制矩形和边框
    drawRect(bounds, bgColor);
    
    // 绘制边框（使用 drawLine 绘制四条边）
    Vector2 topLeft = { bounds.x, bounds.y };
    Vector2 topRight = { bounds.x + bounds.width, bounds.y };
    Vector2 bottomLeft = { bounds.x, bounds.y + bounds.height };
    Vector2 bottomRight = { bounds.x + bounds.width, bounds.y + bounds.height };
    
    drawLine(topLeft, topRight, 2.0f, borderColor);
    drawLine(topRight, bottomRight, 2.0f, borderColor);
    drawLine(bottomRight, bottomLeft, 2.0f, borderColor);
    drawLine(bottomLeft, topLeft, 2.0f, borderColor);
    
    // 绘制勾选标记
    if (checked) {
        float size = fmin(bounds.width, bounds.height) * 0.6f;
        float offsetX = bounds.x + (bounds.width - size) / 2;
        float offsetY = bounds.y + (bounds.height - size) / 2;
        
        // 绘制对勾
        drawLine(
            { offsetX + size * 0.2f, offsetY + size * 0.5f },
            { offsetX + size * 0.4f, offsetY + size * 0.8f },
            2.0f, checkColor
        );
        drawLine(
            { offsetX + size * 0.4f, offsetY + size * 0.8f },
            { offsetX + size * 0.8f, offsetY + size * 0.2f },
            2.0f, checkColor
        );
    }
    
    // 绘制标签
    if (!label.empty()) {
        float textX = bounds.x + bounds.width + 10.0f;
        float textY = bounds.y + (bounds.height - 16.0f) / 2; // 假设字体大小为16
        drawText(label.c_str(), { textX, textY }, 16.0f, 1.0f, textColor);
    }
}

bool CheckBox::IsChecked() const {
    return checked;
}

void CheckBox::SetChecked(bool checked) {
    this->checked = checked;
}

void CheckBox::SetLabel(const std::string& label) {
    this->label = label;
}

void CheckBox::SetPosition(float x, float y) {
    bounds.x = x;
    bounds.y = y;
}

void CheckBox::SetSize(float width, float height) {
    bounds.width = width;
    bounds.height = height;
}

void CheckBox::SetOnCheckChange(std::function<void(bool)> callback) {
    onCheckChange = callback;
}
