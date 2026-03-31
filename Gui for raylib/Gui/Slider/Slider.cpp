#include "Slider.h"
#include "../Common/GuiTheme.h"
#include "../Common/GuiRenderer.h"
#include "../Common/GuiSkin.h"

Slider::Slider(Rectangle bounds, float minValue, float maxValue, float initialValue, Direction direction, std::function<void(float)> onValueChanged)
    : m_bounds(bounds)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_value(initialValue)
    , m_direction(direction)
    , m_onValueChanged(onValueChanged) {
    m_state = SLIDER_STATE_NORMAL;
    m_thumbSize = 20.0f;
    m_wantsFocus = true; // Slider wants keyboard focus

    auto& theme = GuiTheme::Instance();
    trackColor = theme.slider.track;
    thumbColor = theme.slider.thumb;
    thumbHoverColor = theme.slider.thumbHover;
    thumbPressedColor = theme.slider.thumbPressed;
    textColor = theme.colors.text;

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
        // 垂直方向：底部为最小值，顶部为最大值
        return m_bounds.y + m_thumbSize / 2 + (1.0f - normalizedValue) * trackHeight;
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
        // 垂直方向：底部为最小值，顶部为最大值
        return m_minValue + (1.0f - normalizedPos) * (m_maxValue - m_minValue);
    }
}

bool Slider::IsPointOnThumb(Vector2 point) const {
    return CheckCollisionPointRec(point, GetThumbBounds());
}

void Slider::Update() {
    if (!m_isVisible || !m_isEnabled) return;

    // 拦截被活跃弹出层遮挡时的交互
    if (FocusManager::Instance().HasActiveControl() && !FocusManager::Instance().IsActiveControl(this)) {
        return;
    }

    Vector2 mousePos = GetMousePosition();
    bool isOnThumb = IsPointOnThumb(mousePos);

    if (m_isInputMode) {
        if (!m_isFocused) {
            m_isInputMode = false;
        } else {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') || key == '.' || key == '-') {
                    m_inputText += (char)key;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                if (!m_inputText.empty()) m_inputText.pop_back();
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                m_isInputMode = false;
                try {
                    if (!m_inputText.empty()) {
                        float val = std::stof(m_inputText);
                        SetValue(val);
                        if (m_onValueChanged) m_onValueChanged(m_value);
                    }
                } catch (...) {}
            } else if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(mousePos, m_bounds))) {
                m_isInputMode = false;
            }
            return;
        }
    }

    if (m_isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        m_isInputMode = true;
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.1f", m_value);
        m_inputText = buffer;
        SetFocus(true);
        m_state = SLIDER_STATE_NORMAL;
        return;
    }

    if (m_isFocused) {
        HandleKeyboardInput();
    }

    if (m_state == SLIDER_STATE_DRAGGING) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // 在拖拽过程中，我们通常希望即便鼠标稍微超出 bounds 也能继续滑动
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
    } else if (m_isHovered && isOnThumb) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_state = SLIDER_STATE_DRAGGING;
        } else {
            m_state = SLIDER_STATE_HOVER;
        }
    } else if (m_isHovered) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // 仅对焦，不拖动
            SetFocus(true);
        } else {
            m_state = SLIDER_STATE_NORMAL;
        }
    } else {
        m_state = SLIDER_STATE_NORMAL;
    }
}

void Slider::Draw() {
    if (!m_isVisible) return;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        m_skin->DrawSlider(ctx, m_value, m_minValue, m_maxValue, m_direction == Direction::Vertical);
        return;
    }

    Color currentThumbColor = thumbColor;
    if (!m_isEnabled) currentThumbColor = GuiTheme::Instance().colors.disabled;
    else if (m_state == SLIDER_STATE_DRAGGING) currentThumbColor = thumbPressedColor;
    else if (m_state == SLIDER_STATE_HOVER || m_isFocused) currentThumbColor = thumbHoverColor;

    GuiRenderer::DrawRect(m_bounds, trackColor, true, GuiTheme::Instance().colors.border);

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

    const char* displayStr;
    char valueText[32];
    if (m_isInputMode) {
        snprintf(valueText, sizeof(valueText), "%s_", m_inputText.c_str());
        displayStr = valueText;
    } else {
        snprintf(valueText, sizeof(valueText), "%.1f", m_value);
        displayStr = valueText;
    }

    Vector2 textSize = MeasureTextEx(GetFontDefault(), displayStr, 16, 1.0f);
    Vector2 textPos;
    if (m_direction == Direction::Horizontal) {
        textPos = { m_bounds.x + (m_bounds.width - textSize.x) / 2, m_bounds.y + m_bounds.height + 5 };
    } else {
        textPos = { m_bounds.x + m_bounds.width + 5, m_bounds.y + (m_bounds.height - textSize.y) / 2 };
    }
    DrawTextEx(GetFontDefault(), displayStr, textPos, 16, 1.0f, textColor);
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
    if (focus) {
        FocusManager::Instance().SetFocusedControl(this);
    } else if (FocusManager::Instance().GetFocusedControl() == this) {
        FocusManager::Instance().ClearFocusedControl();
    }
}

void Slider::HandleKeyboardInput() {
    if (!m_isFocused) return;

    // 默认单次按下增加或减少 1.0f
    float step = 1.0f;
    
    // 如果按下了 Shift 键，则步长变为 0.1f
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        step = 0.1f;
    }

    // 处理鼠标滚轮输入
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0.0f) {
        float newValue = m_value + wheelMove * step;
        if (newValue < m_minValue) newValue = m_minValue;
        if (newValue > m_maxValue) newValue = m_maxValue;
        if (newValue != m_value) {
            m_value = newValue;
            if (m_onValueChanged) m_onValueChanged(m_value);
        }
    }

    if (m_direction == Direction::Horizontal) {
        if (IsKeyPressed(KEY_HOME)) {
            if (m_value != m_minValue) {
                m_value = m_minValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_END)) {
            if (m_value != m_maxValue) {
                m_value = m_maxValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
            float newValue = m_value - step;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
            float newValue = m_value + step;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    } else {
        if (IsKeyPressed(KEY_HOME)) {
            if (m_value != m_maxValue) {
                m_value = m_maxValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_END)) {
            if (m_value != m_minValue) {
                m_value = m_minValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
            float newValue = m_value + step;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
            float newValue = m_value - step;
            if (newValue < m_minValue) newValue = m_minValue;
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
    m_wantsFocus = true; // IntSlider needs keyboard focus

    auto& theme = GuiTheme::Instance();
    trackColor = theme.slider.track;
    thumbColor = theme.slider.thumb;
    thumbHoverColor = theme.slider.thumbHover;
    thumbPressedColor = theme.slider.thumbPressed;
    textColor = theme.colors.text;

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
        // 垂直方向：底部为最小值，顶部为最大值
        return m_bounds.y + m_thumbSize / 2 + (1.0f - normalizedValue) * trackHeight;
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
        // 垂直方向：底部为最小值，顶部为最大值
        return static_cast<float>(m_minValue) + (1.0f - normalizedPos) * (static_cast<float>(m_maxValue) - static_cast<float>(m_minValue));
    }
}

bool IntSlider::IsPointOnThumb(Vector2 point) const {
    return CheckCollisionPointRec(point, GetThumbBounds());
}

void IntSlider::Update() {
    if (!m_isVisible || !m_isEnabled) return;

    // 拦截被活跃弹出层遮挡时的交互
    if (FocusManager::Instance().HasActiveControl() && !FocusManager::Instance().IsActiveControl(this)) {
        return;
    }

    Vector2 mousePos = GetMousePosition();
    bool isOnThumb = IsPointOnThumb(mousePos);

    if (m_isInputMode) {
        if (!m_isFocused) {
            m_isInputMode = false;
        } else {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') || key == '-') {
                    m_inputText += (char)key;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                if (!m_inputText.empty()) m_inputText.pop_back();
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                m_isInputMode = false;
                try {
                    if (!m_inputText.empty()) {
                        int val = std::stoi(m_inputText);
                        SetValue(val);
                        if (m_onValueChanged) m_onValueChanged(m_value);
                    }
                } catch (...) {}
            } else if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(mousePos, m_bounds))) {
                m_isInputMode = false;
            }
            return;
        }
    }

    if (m_isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        m_isInputMode = true;
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d", m_value);
        m_inputText = buffer;
        SetFocus(true);
        m_state = INTSLIDER_STATE_NORMAL;
        return;
    }

    if (m_isFocused) {
        HandleKeyboardInput();
    }

    if (m_state == INTSLIDER_STATE_DRAGGING) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
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
    } else if (m_isHovered && isOnThumb) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_state = INTSLIDER_STATE_DRAGGING;
        } else {
            m_state = INTSLIDER_STATE_HOVER;
        }
    } else if (m_isHovered) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // 仅对焦，不拖动
            SetFocus(true);
        } else {
            m_state = INTSLIDER_STATE_NORMAL;
        }
    } else {
        m_state = INTSLIDER_STATE_NORMAL;
    }
}

void IntSlider::Draw() {
    if (!m_isVisible) return;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        m_skin->DrawIntSlider(ctx, m_value, m_minValue, m_maxValue, m_direction == Direction::Vertical);
        return;
    }

    Color currentThumbColor = thumbColor;
    if (!m_isEnabled) currentThumbColor = GuiTheme::Instance().colors.disabled;
    else if (m_state == INTSLIDER_STATE_DRAGGING) currentThumbColor = thumbPressedColor;
    else if (m_state == INTSLIDER_STATE_HOVER || m_isFocused) currentThumbColor = thumbHoverColor;

    GuiRenderer::DrawRect(m_bounds, trackColor, true, GuiTheme::Instance().colors.border);

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

    const char* displayStr;
    char valueText[32];
    if (m_isInputMode) {
        snprintf(valueText, sizeof(valueText), "%s_", m_inputText.c_str());
        displayStr = valueText;
    } else {
        snprintf(valueText, sizeof(valueText), "%d", m_value);
        displayStr = valueText;
    }

    Vector2 textSize = MeasureTextEx(GetFontDefault(), displayStr, 16, 1.0f);
    Vector2 textPos;
    if (m_direction == Direction::Horizontal) {
        textPos = { m_bounds.x + (m_bounds.width - textSize.x) / 2, m_bounds.y + m_bounds.height + 5 };
    } else {
        textPos = { m_bounds.x + m_bounds.width + 5, m_bounds.y + (m_bounds.height - textSize.y) / 2 };
    }
    DrawTextEx(GetFontDefault(), displayStr, textPos, 16, 1.0f, textColor);
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
    if (focus) {
        FocusManager::Instance().SetFocusedControl(this);
    } else if (FocusManager::Instance().GetFocusedControl() == this) {
        FocusManager::Instance().ClearFocusedControl();
    }
}

void IntSlider::HandleKeyboardInput() {
    if (!m_isFocused) return;

    int step = 1; // 默认单次按下增加或减少 1

    // 处理鼠标滚轮输入
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0.0f) {
        int newValue = m_value + static_cast<int>(wheelMove > 0 ? step : -step);
        if (newValue < m_minValue) newValue = m_minValue;
        if (newValue > m_maxValue) newValue = m_maxValue;
        if (newValue != m_value) {
            m_value = newValue;
            if (m_onValueChanged) m_onValueChanged(m_value);
        }
    }

    if (m_direction == Direction::Horizontal) {
        if (IsKeyPressed(KEY_HOME)) {
            if (m_value != m_minValue) {
                m_value = m_minValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_END)) {
            if (m_value != m_maxValue) {
                m_value = m_maxValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
            int newValue = m_value - step;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
            int newValue = m_value + step;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    } else {
        if (IsKeyPressed(KEY_HOME)) {
            if (m_value != m_maxValue) {
                m_value = m_maxValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_END)) {
            if (m_value != m_minValue) {
                m_value = m_minValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
            int newValue = m_value + step;
            if (newValue > m_maxValue) newValue = m_maxValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
            int newValue = m_value - step;
            if (newValue < m_minValue) newValue = m_minValue;
            if (newValue != m_value) {
                m_value = newValue;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
    }
}