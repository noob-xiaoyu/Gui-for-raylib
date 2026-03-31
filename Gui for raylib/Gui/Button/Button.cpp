#include "Button.h"
#include "../Common/GuiTheme.h"
#include "../Common/GuiRenderer.h"
#include "../Common/GuiSkin.h"

Button::Button(Rectangle bounds, std::string text, std::function<void()> onClick)
    : m_bounds(bounds), m_text(text), m_onClick(onClick) {
    m_state = BUTTON_STATE_NORMAL;
    
    // Set default colors from theme
    auto& theme = GuiTheme::Instance();
    baseColor = theme.colors.background;
    hoverColor = theme.colors.hover;
    pressedColor = theme.colors.pressed;
    textColor = theme.colors.text;
    fontSize = theme.typography.fontSize;

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
    if (!m_isVisible || !m_isEnabled) return;

    if (m_isHovered) {
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
    if (!m_isVisible) return;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        m_skin->DrawButton(ctx, m_text.c_str());
        return;
    }

    Color currentColor = baseColor;
    if (!m_isEnabled) currentColor = GuiTheme::Instance().colors.disabled;
    else if (m_state == BUTTON_STATE_HOVER) currentColor = hoverColor;
    else if (m_state == BUTTON_STATE_PRESSED) currentColor = pressedColor;

    GuiRenderer::DrawRect(m_bounds, currentColor, true, GuiTheme::Instance().colors.border);
    GuiRenderer::DrawCenteredText(m_bounds, m_text.c_str(), fontSize, textColor);
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