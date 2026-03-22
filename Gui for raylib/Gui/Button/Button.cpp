#include "Button.h"

Button::Button(Rectangle bounds, std::string text, std::function<void()> onClick)
    : m_bounds(bounds), m_text(text), m_onClick(onClick) {
    m_state = BUTTON_STATE_NORMAL;
    FocusManager::Instance().RegisterControl(this);
}

void Button::SetPos(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void Button::SetPos(Vector2 pos) {
    m_bounds.x = pos.x;
    m_bounds.y = pos.y;
}

void Button::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void Button::SetSize(Vector2 size) {
    m_bounds.width = size.x;
    m_bounds.height = size.y;
}

Rectangle Button::GetBounds() const {
    return m_bounds;
}

Vector2 Button::GetPos() const {
    return { m_bounds.x, m_bounds.y };
}

Vector2 Button::GetSize() const {
    return { m_bounds.width, m_bounds.height };
}

void Button::Update() {
    if (!m_isEnabled) return;

    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, m_bounds);

    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = BUTTON_STATE_PRESSED;
        } else {
            if (m_state == BUTTON_STATE_PRESSED && m_onClick) {
                m_onClick();
            }
            m_state = BUTTON_STATE_HOVER;
        }
    } else {
        m_state = BUTTON_STATE_NORMAL;
    }
}

void Button::Draw() {
    Color currentColor = baseColor;
    if (m_state == BUTTON_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (m_state == BUTTON_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    DrawRectangleRec(m_bounds, currentColor);

    if (!m_text.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), m_text.c_str(), fontSize, 1.0f);
        Vector2 textPos = {
            m_bounds.x + (m_bounds.width - textSize.x) / 2,
            m_bounds.y + (m_bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), m_text.c_str(), textPos, fontSize, 1.0f, textColor);
    }
}

void Button::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText) {
    Color currentColor = baseColor;
    if (m_state == BUTTON_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (m_state == BUTTON_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    drawRect(m_bounds, currentColor);
    drawBorder(m_bounds, textColor, 2.0f);

    if (!m_text.empty()) {
        float textX = m_bounds.x + (m_bounds.width - MeasureText(m_text.c_str(), fontSize)) / 2;
        float textY = m_bounds.y + (m_bounds.height - fontSize) / 2;
        drawText(m_text.c_str(), { textX, textY }, fontSize, 1.0f, textColor);
    }
}

std::string Button::GetText() const {
    return m_text;
}

int Button::GetFontSize() const {
    return fontSize;
}

bool Button::IsHovered() const {
    return m_state == BUTTON_STATE_HOVER;
}

bool Button::IsPressed() const {
    return m_state == BUTTON_STATE_PRESSED;
}