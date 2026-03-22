#include "Slider.h"

Slider::Slider(Rectangle bounds, float minValue, float maxValue, float initialValue, Direction direction, std::function<void(float)> onValueChanged)
    : m_bounds(bounds)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_value(initialValue)
    , m_direction(direction)
    , m_onValueChanged(onValueChanged) {
    m_state = SLIDER_STATE_NORMAL;
    m_thumbSize = 20.0f;
    FocusManager::Instance().RegisterControl(this);
}

Rectangle Slider::GetBounds() const {
    return m_bounds;
}

Rectangle Slider::GetThumbBounds() const {
    if (m_direction == Direction::Horizontal) {
        float thumbX = ValueToPosition(m_value) - m_thumbSize / 2;
        return { thumbX, m_bounds.y, m_thumbSize, m_bounds.height };
    } else {
        float thumbY = ValueToPosition(m_value) - m_thumbSize / 2;
        return { m_bounds.x, thumbY, m_bounds.width, m_thumbSize };
    }
}

float Slider::ValueToPosition(float value) const {
    float clampedValue = (value < m_minValue) ? m_minValue : (value > m_maxValue) ? m_maxValue : value;
    float normalizedValue = (clampedValue - m_minValue) / (m_maxValue - m_minValue);
    if (m_direction == Direction::Horizontal) {
        float trackWidth = m_bounds.width - m_thumbSize;
        return m_bounds.x + m_thumbSize / 2 + normalizedValue * trackWidth;
    } else {
        float trackHeight = m_bounds.height - m_thumbSize;
        return m_bounds.y + m_thumbSize / 2 + normalizedValue * trackHeight;
    }
}

float Slider::PositionToValue(float position) const {
    if (m_direction == Direction::Horizontal) {
        float trackWidth = m_bounds.width - m_thumbSize;
        float normalizedPos = (position - m_bounds.x - m_thumbSize / 2) / trackWidth;
        normalizedPos = (normalizedPos < 0.0f) ? 0.0f : (normalizedPos > 1.0f) ? 1.0f : normalizedPos;
        return m_minValue + normalizedPos * (m_maxValue - m_minValue);
    } else {
        float trackHeight = m_bounds.height - m_thumbSize;
        float normalizedPos = (position - m_bounds.y - m_thumbSize / 2) / trackHeight;
        normalizedPos = (normalizedPos < 0.0f) ? 0.0f : (normalizedPos > 1.0f) ? 1.0f : normalizedPos;
        return m_minValue + normalizedPos * (m_maxValue - m_minValue);
    }
}

bool Slider::IsPointOnThumb(Vector2 point) const {
    return CheckCollisionPointRec(point, GetThumbBounds());
}

void Slider::Update() {
    if (!m_isEnabled) return;

    Vector2 mousePos = GetMousePosition();
    Rectangle thumbBounds = GetThumbBounds();
    bool isOnThumb = IsPointOnThumb(mousePos);
    bool isHovered = CheckCollisionPointRec(mousePos, m_bounds);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GuiControl* controlAtMouse = FocusManager::Instance().GetControlAtMouse();
        if (isHovered && controlAtMouse == this) {
            m_isFocused = true;
            FocusManager::Instance().SetFocusedControl(this);
        } else if (!isHovered) {
            m_isFocused = false;
        }
    }

    if (m_isFocused) {
        HandleKeyboardInput();
    }

    if (m_state == SLIDER_STATE_DRAGGING) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (isHovered) {
                float newValue = PositionToValue(mousePos.x);
                if (m_direction == Direction::Vertical) {
                    newValue = PositionToValue(mousePos.y);
                }
                if (newValue != m_value) {
                    m_value = newValue;
                    if (m_onValueChanged) {
                        m_onValueChanged(m_value);
                    }
                }
            } else {
                m_state = SLIDER_STATE_NORMAL;
            }
        } else {
            m_state = SLIDER_STATE_NORMAL;
        }
    } else if (isOnThumb) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = SLIDER_STATE_DRAGGING;
        } else {
            m_state = SLIDER_STATE_HOVER;
        }
    } else if (isHovered) {
        m_state = SLIDER_STATE_HOVER;
    } else {
        m_state = SLIDER_STATE_NORMAL;
    }
}

void Slider::Draw() {
    Color currentThumbColor = thumbColor;
    if (m_state == SLIDER_STATE_HOVER) {
        currentThumbColor = thumbHoverColor;
    } else if (m_state == SLIDER_STATE_DRAGGING) {
        currentThumbColor = thumbPressedColor;
    }

    DrawRectangleRec(m_bounds, trackColor);

    Rectangle thumbBounds = GetThumbBounds();
    if (m_direction == Direction::Horizontal) {
        Rectangle filledPart = { m_bounds.x, m_bounds.y, thumbBounds.x + m_thumbSize / 2 - m_bounds.x, m_bounds.height };
        if (filledPart.width > 0) {
            DrawRectangleRec(filledPart, currentThumbColor);
        }
    } else {
        Rectangle filledPart = { m_bounds.x, thumbBounds.y + m_thumbSize / 2, m_bounds.width, m_bounds.y + m_bounds.height - (thumbBounds.y + m_thumbSize / 2) };
        if (filledPart.height > 0) {
            DrawRectangleRec(filledPart, currentThumbColor);
        }
    }

    DrawRectangleRec(thumbBounds, currentThumbColor);
    DrawRectangleLinesEx(thumbBounds, 2, textColor);

    char valueText[32];
    snprintf(valueText, sizeof(valueText), "%.1f", m_value);
    Vector2 textSize = MeasureTextEx(GetFontDefault(), valueText, 16, 1.0f);
    Vector2 textPos;
    if (m_direction == Direction::Horizontal) {
        textPos = { m_bounds.x + (m_bounds.width - textSize.x) / 2, m_bounds.y + m_bounds.height + 5 };
    } else {
        textPos = { m_bounds.x + m_bounds.width + 5, m_bounds.y + (m_bounds.height - textSize.y) / 2 };
    }
    DrawTextEx(GetFontDefault(), valueText, textPos, 16, 1.0f, textColor);
}

void Slider::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText) {
    Color currentThumbColor = thumbColor;
    if (m_state == SLIDER_STATE_HOVER) {
        currentThumbColor = thumbHoverColor;
    } else if (m_state == SLIDER_STATE_DRAGGING) {
        currentThumbColor = thumbPressedColor;
    }

    drawRect(m_bounds, trackColor);

    Rectangle thumbBounds = GetThumbBounds();
    drawRect(thumbBounds, currentThumbColor);
    drawBorder(thumbBounds, textColor, 2.0f);

    char valueText[32];
    snprintf(valueText, sizeof(valueText), "%.1f", m_value);
    Vector2 textPos;
    if (m_direction == Direction::Horizontal) {
        float textX = m_bounds.x + (m_bounds.width - MeasureText(valueText, 16)) / 2;
        float textY = m_bounds.y + m_bounds.height + 5;
        textPos = { textX, textY };
    } else {
        float textX = m_bounds.x + m_bounds.width + 5;
        float textY = m_bounds.y + (m_bounds.height - 16) / 2;
        textPos = { textX, textY };
    }
    drawText(valueText, textPos, 16, 1.0f, textColor);
}

void Slider::SetValue(float value) {
    m_value = (value < m_minValue) ? m_minValue : (value > m_maxValue) ? m_maxValue : value;
}

float Slider::GetValue() const {
    return m_value;
}

void Slider::SetMinValue(float minValue) {
    m_minValue = minValue;
    if (m_value < m_minValue) m_value = m_minValue;
}

void Slider::SetMaxValue(float maxValue) {
    m_maxValue = maxValue;
    if (m_value > m_maxValue) m_value = m_maxValue;
}

void Slider::SetFocus(bool focus) {
    m_isFocused = focus;
    if (focus) {
        FocusManager::Instance().SetFocusedControl(this);
    }
}

bool Slider::IsFocused() const {
    return m_isFocused;
}

void Slider::HandleKeyboardInput() {
    if (!m_isFocused) return;

    float step = (m_maxValue - m_minValue) / 20.0f;
    if (step < 0.1f) step = 0.1f;

    if (m_direction == Direction::Horizontal) {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_HOME)) {
            float newValue = m_value - step * GetFrameTime() * 10.0f;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_END)) {
            float newValue = m_value + step * GetFrameTime() * 10.0f;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    } else {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_HOME)) {
            float newValue = m_value - step * GetFrameTime() * 10.0f;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_END)) {
            float newValue = m_value + step * GetFrameTime() * 10.0f;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    }
}

IntSlider::IntSlider(Rectangle bounds, int minValue, int maxValue, int initialValue, Direction direction, std::function<void(int)> onValueChanged)
    : m_bounds(bounds)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_value(initialValue)
    , m_direction(direction)
    , m_onValueChanged(onValueChanged) {
    m_state = INTSLIDER_STATE_NORMAL;
    m_thumbSize = 20.0f;
    FocusManager::Instance().RegisterControl(this);
}

Rectangle IntSlider::GetBounds() const {
    return m_bounds;
}

Rectangle IntSlider::GetThumbBounds() const {
    if (m_direction == Direction::Horizontal) {
        float thumbX = ValueToPosition(static_cast<float>(m_value)) - m_thumbSize / 2;
        return { thumbX, m_bounds.y, m_thumbSize, m_bounds.height };
    } else {
        float thumbY = ValueToPosition(static_cast<float>(m_value)) - m_thumbSize / 2;
        return { m_bounds.x, thumbY, m_bounds.width, m_thumbSize };
    }
}

float IntSlider::ValueToPosition(float value) const {
    float clampedValue = (value < static_cast<float>(m_minValue)) ? static_cast<float>(m_minValue) : (value > static_cast<float>(m_maxValue)) ? static_cast<float>(m_maxValue) : value;
    float normalizedValue = (clampedValue - static_cast<float>(m_minValue)) / (static_cast<float>(m_maxValue) - static_cast<float>(m_minValue));
    if (m_direction == Direction::Horizontal) {
        float trackWidth = m_bounds.width - m_thumbSize;
        return m_bounds.x + m_thumbSize / 2 + normalizedValue * trackWidth;
    } else {
        float trackHeight = m_bounds.height - m_thumbSize;
        return m_bounds.y + m_thumbSize / 2 + normalizedValue * trackHeight;
    }
}

float IntSlider::PositionToValue(float position) const {
    if (m_direction == Direction::Horizontal) {
        float trackWidth = m_bounds.width - m_thumbSize;
        float normalizedPos = (position - m_bounds.x - m_thumbSize / 2) / trackWidth;
        normalizedPos = (normalizedPos < 0.0f) ? 0.0f : (normalizedPos > 1.0f) ? 1.0f : normalizedPos;
        return static_cast<float>(m_minValue) + normalizedPos * (static_cast<float>(m_maxValue) - static_cast<float>(m_minValue));
    } else {
        float trackHeight = m_bounds.height - m_thumbSize;
        float normalizedPos = (position - m_bounds.y - m_thumbSize / 2) / trackHeight;
        normalizedPos = (normalizedPos < 0.0f) ? 0.0f : (normalizedPos > 1.0f) ? 1.0f : normalizedPos;
        return static_cast<float>(m_minValue) + normalizedPos * (static_cast<float>(m_maxValue) - static_cast<float>(m_minValue));
    }
}

bool IntSlider::IsPointOnThumb(Vector2 point) const {
    return CheckCollisionPointRec(point, GetThumbBounds());
}

void IntSlider::Update() {
    if (!m_isEnabled) return;

    Vector2 mousePos = GetMousePosition();
    Rectangle thumbBounds = GetThumbBounds();
    bool isOnThumb = IsPointOnThumb(mousePos);
    bool isHovered = CheckCollisionPointRec(mousePos, m_bounds);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GuiControl* controlAtMouse = FocusManager::Instance().GetControlAtMouse();
        if (isHovered && controlAtMouse == this) {
            m_isFocused = true;
            FocusManager::Instance().SetFocusedControl(this);
        } else if (!isHovered) {
            m_isFocused = false;
        }
    }

    if (m_isFocused) {
        HandleKeyboardInput();
    }

    if (m_state == INTSLIDER_STATE_DRAGGING) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (isHovered) {
                float newValue = PositionToValue(mousePos.x);
                if (m_direction == Direction::Vertical) {
                    newValue = PositionToValue(mousePos.y);
                }
                int intNewValue = static_cast<int>(newValue + 0.5f);
                if (intNewValue != m_value) {
                    m_value = intNewValue;
                    if (m_onValueChanged) {
                        m_onValueChanged(m_value);
                    }
                }
            } else {
                m_state = INTSLIDER_STATE_NORMAL;
            }
        } else {
            m_state = INTSLIDER_STATE_NORMAL;
        }
    } else if (isOnThumb) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = INTSLIDER_STATE_DRAGGING;
        } else {
            m_state = INTSLIDER_STATE_HOVER;
        }
    } else if (isHovered) {
        m_state = INTSLIDER_STATE_HOVER;
    } else {
        m_state = INTSLIDER_STATE_NORMAL;
    }
}

void IntSlider::Draw() {
    Color currentThumbColor = thumbColor;
    if (m_state == INTSLIDER_STATE_HOVER) {
        currentThumbColor = thumbHoverColor;
    } else if (m_state == INTSLIDER_STATE_DRAGGING) {
        currentThumbColor = thumbPressedColor;
    }

    DrawRectangleRec(m_bounds, trackColor);

    Rectangle thumbBounds = GetThumbBounds();
    if (m_direction == Direction::Horizontal) {
        Rectangle filledPart = { m_bounds.x, m_bounds.y, thumbBounds.x + m_thumbSize / 2 - m_bounds.x, m_bounds.height };
        if (filledPart.width > 0) {
            DrawRectangleRec(filledPart, currentThumbColor);
        }
    } else {
        Rectangle filledPart = { m_bounds.x, thumbBounds.y + m_thumbSize / 2, m_bounds.width, m_bounds.y + m_bounds.height - (thumbBounds.y + m_thumbSize / 2) };
        if (filledPart.height > 0) {
            DrawRectangleRec(filledPart, currentThumbColor);
        }
    }

    DrawRectangleRec(thumbBounds, currentThumbColor);
    DrawRectangleLinesEx(thumbBounds, 2, textColor);

    char valueText[32];
    snprintf(valueText, sizeof(valueText), "%d", m_value);
    Vector2 textSize = MeasureTextEx(GetFontDefault(), valueText, 16, 1.0f);
    Vector2 textPos;
    if (m_direction == Direction::Horizontal) {
        textPos = { m_bounds.x + (m_bounds.width - textSize.x) / 2, m_bounds.y + m_bounds.height + 5 };
    } else {
        textPos = { m_bounds.x + m_bounds.width + 5, m_bounds.y + (m_bounds.height - textSize.y) / 2 };
    }
    DrawTextEx(GetFontDefault(), valueText, textPos, 16, 1.0f, textColor);
}

void IntSlider::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText) {
    Color currentThumbColor = thumbColor;
    if (m_state == INTSLIDER_STATE_HOVER) {
        currentThumbColor = thumbHoverColor;
    } else if (m_state == INTSLIDER_STATE_DRAGGING) {
        currentThumbColor = thumbPressedColor;
    }

    drawRect(m_bounds, trackColor);

    Rectangle thumbBounds = GetThumbBounds();
    drawRect(thumbBounds, currentThumbColor);
    drawBorder(thumbBounds, textColor, 2.0f);

    char valueText[32];
    snprintf(valueText, sizeof(valueText), "%d", m_value);
    Vector2 textPos;
    if (m_direction == Direction::Horizontal) {
        float textX = m_bounds.x + (m_bounds.width - MeasureText(valueText, 16)) / 2;
        float textY = m_bounds.y + m_bounds.height + 5;
        textPos = { textX, textY };
    } else {
        float textX = m_bounds.x + m_bounds.width + 5;
        float textY = m_bounds.y + (m_bounds.height - 16) / 2;
        textPos = { textX, textY };
    }
    drawText(valueText, textPos, 16, 1.0f, textColor);
}

void IntSlider::SetValue(int value) {
    m_value = (value < m_minValue) ? m_minValue : (value > m_maxValue) ? m_maxValue : value;
}

int IntSlider::GetValue() const {
    return m_value;
}

void IntSlider::SetMinValue(int minValue) {
    m_minValue = minValue;
    if (m_value < m_minValue) m_value = m_minValue;
}

void IntSlider::SetMaxValue(int maxValue) {
    m_maxValue = maxValue;
    if (m_value > m_maxValue) m_value = m_maxValue;
}

void IntSlider::SetFocus(bool focus) {
    m_isFocused = focus;
    if (focus) {
        FocusManager::Instance().SetFocusedControl(this);
    }
}

bool IntSlider::IsFocused() const {
    return m_isFocused;
}

void IntSlider::HandleKeyboardInput() {
    if (!m_isFocused) return;

    int step = 1;
    if ((m_maxValue - m_minValue) > 100) step = 5;
    if ((m_maxValue - m_minValue) > 500) step = 10;

    if (m_direction == Direction::Horizontal) {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_HOME)) {
            int newValue = m_value - step;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_END)) {
            int newValue = m_value + step;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    } else {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_HOME)) {
            int newValue = m_value - step;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_END)) {
            int newValue = m_value + step;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    }
}