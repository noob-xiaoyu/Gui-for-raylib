#include "CheckBox.h"

CheckBox::CheckBox(Rectangle bounds, std::string label, bool checked, std::function<void(bool)> onCheckChange)
    : m_bounds(bounds), m_label(label), m_checked(checked), m_onCheckChange(onCheckChange) {
    m_state = CHECKBOX_STATE_NORMAL;
    FocusManager::Instance().RegisterControl(this);
}

Rectangle CheckBox::GetBounds() const {
    float labelWidth = 0;
    if (!m_label.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), m_label.c_str(), 16.0f, 1.0f);
        labelWidth = textSize.x + 10.0f;
    }
    return { m_bounds.x, m_bounds.y, m_bounds.width + labelWidth, m_bounds.height };
}

void CheckBox::Update() {
    if (!m_isEnabled) return;

    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, m_bounds);

    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = CHECKBOX_STATE_PRESSED;
        } else {
            if (m_state == CHECKBOX_STATE_PRESSED) {
                m_checked = !m_checked;
                if (m_onCheckChange) {
                    m_onCheckChange(m_checked);
                }
            }
            m_state = CHECKBOX_STATE_HOVER;
        }
    } else {
        m_state = CHECKBOX_STATE_NORMAL;
    }
}

void CheckBox::Draw() {
    Color bgColor = fillColor;
    if (m_state == CHECKBOX_STATE_HOVER) {
        bgColor = hoverColor;
    } else if (m_state == CHECKBOX_STATE_PRESSED) {
        bgColor = pressedColor;
    }

    DrawRectangleRec(m_bounds, bgColor);
    DrawRectangleLinesEx(m_bounds, 2, borderColor);

    if (m_checked) {
        float size = fmin(m_bounds.width, m_bounds.height) * 0.6f;
        float offsetX = m_bounds.x + (m_bounds.width - size) / 2;
        float offsetY = m_bounds.y + (m_bounds.height - size) / 2;

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

    if (!m_label.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), m_label.c_str(), 16.0f, 1.0f);
        float textX = m_bounds.x + m_bounds.width + 10.0f;
        float textY = m_bounds.y + (m_bounds.height - textSize.y) / 2;
        DrawTextEx(GetFontDefault(), m_label.c_str(), { textX, textY }, 16.0f, 1.0f, textColor);
    }
}

void CheckBox::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText) {
    Color bgColor = fillColor;
    if (m_state == CHECKBOX_STATE_HOVER) {
        bgColor = hoverColor;
    } else if (m_state == CHECKBOX_STATE_PRESSED) {
        bgColor = pressedColor;
    }

    drawRect(m_bounds, bgColor);

    Vector2 topLeft = { m_bounds.x, m_bounds.y };
    Vector2 topRight = { m_bounds.x + m_bounds.width, m_bounds.y };
    Vector2 bottomLeft = { m_bounds.x, m_bounds.y + m_bounds.height };
    Vector2 bottomRight = { m_bounds.x + m_bounds.width, m_bounds.y + m_bounds.height };

    drawLine(topLeft, topRight, 2.0f, borderColor);
    drawLine(topRight, bottomRight, 2.0f, borderColor);
    drawLine(bottomRight, bottomLeft, 2.0f, borderColor);
    drawLine(bottomLeft, topLeft, 2.0f, borderColor);

    if (m_checked) {
        float size = fmin(m_bounds.width, m_bounds.height) * 0.6f;
        float offsetX = m_bounds.x + (m_bounds.width - size) / 2;
        float offsetY = m_bounds.y + (m_bounds.height - size) / 2;

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

    if (!m_label.empty()) {
        float textX = m_bounds.x + m_bounds.width + 10.0f;
        float textY = m_bounds.y + (m_bounds.height - 16.0f) / 2;
        drawText(m_label.c_str(), { textX, textY }, 16.0f, 1.0f, textColor);
    }
}

bool CheckBox::IsChecked() const {
    return m_checked;
}

void CheckBox::SetChecked(bool checked) {
    this->m_checked = checked;
}

void CheckBox::SetLabel(const std::string& label) {
    this->m_label = label;
}

void CheckBox::SetPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void CheckBox::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void CheckBox::SetOnCheckChange(std::function<void(bool)> callback) {
    m_onCheckChange = callback;
}