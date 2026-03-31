#include "CheckBox.h"
#include "../Common/GuiTheme.h"
#include "../Common/GuiRenderer.h"
#include "../Common/GuiSkin.h"

CheckBox::CheckBox(Rectangle bounds, std::string label, bool checked, std::function<void(bool)> onCheckChange)
    : m_bounds(bounds), m_label(label), m_checked(checked), m_onCheckChange(onCheckChange) {
    m_state = CHECKBOX_STATE_NORMAL;
    
    auto& theme = GuiTheme::Instance();
    fillColor = theme.colors.background;
    hoverColor = theme.colors.hover;
    pressedColor = theme.colors.pressed;
    borderColor = theme.colors.border;
    checkColor = theme.colors.text;
    textColor = theme.colors.text;

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
    if (!m_isVisible || !m_isEnabled) return;

    if (m_isHovered) {
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
    if (!m_isVisible) return;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        m_skin->DrawCheckBox(ctx, m_checked, m_label.c_str());
        return;
    }

    Color bgColor = fillColor;
    if (!m_isEnabled) bgColor = GuiTheme::Instance().colors.disabled;
    else if (m_state == CHECKBOX_STATE_HOVER) bgColor = hoverColor;
    else if (m_state == CHECKBOX_STATE_PRESSED) bgColor = pressedColor;

    GuiRenderer::DrawRect(m_bounds, bgColor, true, borderColor);

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
        GuiRenderer::DrawLeftAlignedText(
            {m_bounds.x + m_bounds.width + 10.0f, m_bounds.y, 200, m_bounds.height},
            m_label.c_str(), 16, 0, textColor
        );
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