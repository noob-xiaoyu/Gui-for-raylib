#include "ComboBox.h"
#include "../Common/GuiRenderer.h"
#include "../Common/GuiSkin.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return value;
    return value;
}

ComboBox::ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex, std::function<void(int)> onSelectionChange)
    : DropdownControl(bounds, items), m_selectedIndex(selectedIndex), m_onSelectionChange(onSelectionChange) {
    m_state = COMBOBOX_STATE_NORMAL;
}

void ComboBox::Update() {
    if (!m_isVisible || !m_isEnabled) return;

    if (m_isOpen) {
        Rectangle dropdownBounds = GetDropdownBounds();
        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, m_bounds)) {
                Close();
            } else if (!CheckCollisionPointRec(mousePos, dropdownBounds)) {
                Close();
            }
        }

        float itemHeight = 30.0f;
        float totalHeight = m_items.size() * itemHeight;
        float maxScroll = totalHeight - m_dropdownHeight;
        if (maxScroll < 0) maxScroll = 0;

        if (CheckCollisionPointRec(mousePos, dropdownBounds)) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                m_scrollOffset += wheel * itemHeight * 2;
                if (m_scrollOffset > 0) m_scrollOffset = 0;
                if (m_scrollOffset < -maxScroll) m_scrollOffset = -maxScroll;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, dropdownBounds)) {
            int hoveredIndex = (int)((mousePos.y - dropdownBounds.y - m_scrollOffset) / itemHeight);
            if (hoveredIndex >= 0 && hoveredIndex < m_items.size()) {
                m_selectedIndex = hoveredIndex;
                Close();
                if (m_onSelectionChange) {
                    m_onSelectionChange(m_selectedIndex);
                }
            }
        }
        return;
    }

    if (m_isHovered) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_state = COMBOBOX_STATE_PRESSED;
        }

        if (m_state == COMBOBOX_STATE_PRESSED) {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                FocusManager::Instance().CloseAllActiveControls();
                m_isOpen = true;
                m_scrollOffset = 0.0f;
                FocusManager::Instance().SetActiveControl(this);
                m_state = COMBOBOX_STATE_HOVER;
            }
        } else {
            m_state = COMBOBOX_STATE_HOVER;
        }
    } else {
        m_state = COMBOBOX_STATE_NORMAL;
    }
}

void ComboBox::Draw() {
    if (!m_isVisible) return;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        std::string currentText = (m_selectedIndex >= 0 && m_selectedIndex < (int)m_items.size()) ? m_items[m_selectedIndex] : "Select...";
        m_skin->DrawComboBox(ctx, currentText, m_isOpen);

        if (m_isOpen) {
            Rectangle dropdownBounds = GetDropdownBounds();
            m_skin->DrawDropdownBackground(ctx, dropdownBounds);

            float itemHeight = 30.0f;
            float totalHeight = m_items.size() * itemHeight;
            float maxScroll = totalHeight - m_dropdownHeight;
            if (maxScroll < 0) maxScroll = 0;

            BeginScissorMode((int)dropdownBounds.x, (int)dropdownBounds.y, (int)dropdownBounds.width, (int)dropdownBounds.height);
            Vector2 mousePos = GetMousePosition();
            for (size_t i = 0; i < m_items.size(); i++) {
                float itemY = dropdownBounds.y + i * itemHeight + m_scrollOffset;
                if (itemY + itemHeight < dropdownBounds.y || itemY > dropdownBounds.y + dropdownBounds.height) continue;

                Rectangle itemRect = { dropdownBounds.x, itemY, dropdownBounds.width, itemHeight };
                bool isHovered = CheckCollisionPointRec(mousePos, itemRect) && itemY >= dropdownBounds.y && itemY + itemHeight <= dropdownBounds.y + dropdownBounds.height;
                bool isSelected = (i == (size_t)m_selectedIndex);
                
                m_skin->DrawDropdownItem(ctx, itemRect, m_items[i].c_str(), isHovered, isSelected);
            }
            EndScissorMode();
            
            // Draw simple scrollbar if needed (or could add to skin)
            if (totalHeight > m_dropdownHeight) {
                float scrollbarHeight = (m_dropdownHeight / totalHeight) * m_dropdownHeight;
                if (scrollbarHeight < 20) scrollbarHeight = 20;
                float scrollbarY = dropdownBounds.y + (-m_scrollOffset / maxScroll) * (m_dropdownHeight - scrollbarHeight);
                Rectangle scrollbarBounds = {
                    dropdownBounds.x + dropdownBounds.width - m_scrollbarWidth,
                    scrollbarY,
                    m_scrollbarWidth,
                    scrollbarHeight
                };
                DrawRectangleRec(scrollbarBounds, { 100, 100, 100, 100 });
            }
        }
        return;
    }

    Color currentColor = backgroundColor;
    if (!m_isEnabled) currentColor = GuiTheme::Instance().colors.disabled;
    else if (m_state == COMBOBOX_STATE_HOVER) currentColor = hoverColor;
    else if (m_state == COMBOBOX_STATE_PRESSED) currentColor = pressedColor;

    GuiRenderer::DrawRect(m_bounds, currentColor, true, borderColor);

    if (m_selectedIndex >= 0 && m_selectedIndex < m_items.size()) {
        const char* text = m_items[m_selectedIndex].c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select an option";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), placeholder, fontSize, 1.0f);
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), placeholder, textPos, fontSize, 1.0f, GRAY);
    }

    Vector2 arrowPos = {
        m_bounds.x + m_bounds.width - 25,
        m_bounds.y + m_bounds.height / 2 - 4
    };
    DrawTriangle(
        { arrowPos.x, arrowPos.y },
        { arrowPos.x + 8, arrowPos.y + 8 },
        { arrowPos.x + 16, arrowPos.y },
        textColor
    );

    if (m_isOpen) {
        Rectangle dropdownBounds = GetDropdownBounds();

        DrawRectangleRec(dropdownBounds, dropdownColor);
        DrawRectangleLinesEx(dropdownBounds, 1, dropdownBorderColor);

        float itemHeight = 30.0f;
        float totalHeight = m_items.size() * itemHeight;
        float maxScroll = totalHeight - m_dropdownHeight;
        if (maxScroll < 0) maxScroll = 0;

        BeginScissorMode((int)dropdownBounds.x, (int)dropdownBounds.y, (int)dropdownBounds.width, (int)dropdownBounds.height);

        Vector2 mousePos = GetMousePosition();
        for (size_t i = 0; i < m_items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + m_scrollOffset;
            if (itemY + itemHeight < dropdownBounds.y || itemY > dropdownBounds.y + dropdownBounds.height) continue;

            Rectangle itemRect = { dropdownBounds.x, itemY, dropdownBounds.width, itemHeight };
            Color itemColor = (i == (size_t)m_selectedIndex) ? selectedItemColor : dropdownColor;

            if (CheckCollisionPointRec(mousePos, itemRect) && itemY >= dropdownBounds.y && itemY + itemHeight <= dropdownBounds.y + dropdownBounds.height) {
                itemColor = hoverItemColor;
            }

            DrawRectangleRec(itemRect, itemColor);

            const char* itemText = m_items[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), itemText, fontSize, 1.0f);
            Vector2 textPos = {
                dropdownBounds.x + 10,
                itemY + (itemHeight - textSize.y) / 2
            };
            DrawTextEx(GetFontDefault(), itemText, textPos, fontSize, 1.0f, textColor);
        }

        EndScissorMode();

        if (totalHeight > m_dropdownHeight) {
            float scrollbarHeight = (m_dropdownHeight / totalHeight) * m_dropdownHeight;
            if (scrollbarHeight < 20) scrollbarHeight = 20;
            float scrollbarY = dropdownBounds.y + (-m_scrollOffset / maxScroll) * (m_dropdownHeight - scrollbarHeight);
            Rectangle scrollbarBounds = {
                dropdownBounds.x + dropdownBounds.width - m_scrollbarWidth,
                scrollbarY,
                m_scrollbarWidth,
                scrollbarHeight
            };
            DrawRectangleRec(scrollbarBounds, GRAY);
        }
    }
}

void ComboBox::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
}

int ComboBox::GetSelectedIndex() const {
    return m_selectedIndex;
}

std::string ComboBox::GetSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < m_items.size()) {
        return m_items[m_selectedIndex];
    }
    return "";
}

void ComboBox::SetSelectedIndex(int index) {
    if (index >= -1 && index < m_items.size()) {
        m_selectedIndex = index;
    }
}

void ComboBox::SetOnSelectionChange(std::function<void(int)> callback) {
    m_onSelectionChange = callback;
}