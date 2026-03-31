#include "Multiselect.h"
#include "../Common/GuiRenderer.h"
#include "../Common/GuiSkin.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return value;
    return value;
}

Multiselect::Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange)
    : DropdownControl(bounds, items), m_onSelectionChange(onSelectionChange) {
    m_state = MULTISELECT_STATE_NORMAL;
    m_selectedItems.resize(items.size(), false);
}

void Multiselect::Update() {
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
                m_selectedItems[hoveredIndex] = !m_selectedItems[hoveredIndex];

                if (m_onSelectionChange) {
                    m_onSelectionChange(GetSelectedIndices());
                }
            }
        }
        return;
    }

    if (m_isHovered) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_state = MULTISELECT_STATE_PRESSED;
        }

        if (m_state == MULTISELECT_STATE_PRESSED) {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                FocusManager::Instance().CloseAllActiveControls();
                m_isOpen = true;
                m_scrollOffset = 0.0f;
                FocusManager::Instance().SetActiveControl(this);
                m_state = MULTISELECT_STATE_HOVER;
            }
        } else {
            m_state = MULTISELECT_STATE_HOVER;
        }
    } else {
        m_state = MULTISELECT_STATE_NORMAL;
    }
}

void Multiselect::Draw() {
    if (!m_isVisible) return;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        std::vector<std::string> selected = GetSelectedItems();
        std::string displayText = selected.empty() ? "Select options" : "";
        for (size_t i = 0; i < selected.size(); i++) {
            if (i > 0) displayText += ", ";
            displayText += selected[i];
        }
        if (displayText.length() > 20) displayText = displayText.substr(0, 17) + "...";
        
        m_skin->DrawComboBox(ctx, displayText, m_isOpen);

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
                bool isSelected = m_selectedItems[i];
                
                m_skin->DrawDropdownItem(ctx, itemRect, m_items[i].c_str(), isHovered, isSelected);
            }
            EndScissorMode();
            
            if (totalHeight > m_dropdownHeight) {
                float scrollbarHeight = (m_dropdownHeight / totalHeight) * m_dropdownHeight;
                if (scrollbarHeight < 20) scrollbarHeight = 20;
                float scrollbarY = dropdownBounds.y + (-m_scrollOffset / (totalHeight - m_dropdownHeight)) * (m_dropdownHeight - scrollbarHeight);
                Rectangle scrollbarBounds = { dropdownBounds.x + dropdownBounds.width - m_scrollbarWidth, scrollbarY, m_scrollbarWidth, scrollbarHeight };
                DrawRectangleRec(scrollbarBounds, { 100, 100, 100, 100 });
            }
        }
        return;
    }

    Color currentColor = backgroundColor;
    if (!m_isEnabled) currentColor = GuiTheme::Instance().colors.disabled;
    else if (m_state == MULTISELECT_STATE_HOVER) currentColor = hoverColor;
    else if (m_state == MULTISELECT_STATE_PRESSED) currentColor = pressedColor;

    GuiRenderer::DrawRect(m_bounds, currentColor, true, borderColor);

    std::vector<std::string> selected = GetSelectedItems();
    if (!selected.empty()) {
        std::string displayText = "";
        for (size_t i = 0; i < selected.size(); i++) {
            if (i > 0) displayText += ", ";
            displayText += selected[i];
        }
        if (displayText.length() > 20) {
            displayText = displayText.substr(0, 17) + "...";
        }

        const char* text = displayText.c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select options";
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
        DrawRectangleLinesEx(dropdownBounds, 2, dropdownBorderColor);

        float itemHeight = 30.0f;
        float totalHeight = m_items.size() * itemHeight;

        BeginScissorMode(
            (int)dropdownBounds.x,
            (int)dropdownBounds.y,
            (int)dropdownBounds.width,
            (int)dropdownBounds.height
        );

        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < m_items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + m_scrollOffset;
            Rectangle itemBounds = {
                dropdownBounds.x,
                itemY,
                dropdownBounds.width - (totalHeight > m_dropdownHeight ? m_scrollbarWidth : 0),
                itemHeight
            };

            if (m_selectedItems[i]) {
                DrawRectangleRec(itemBounds, selectedItemColor);
            } else if (CheckCollisionPointRec(mousePos, itemBounds)) {
                DrawRectangleRec(itemBounds, hoverItemColor);
            }

            Rectangle checkboxBounds = {
                itemBounds.x + 8,
                itemY + (itemHeight - 16) / 2,
                16,
                16
            };
            DrawRectangleRec(checkboxBounds, WHITE);
            DrawRectangleLinesEx(checkboxBounds, 2, borderColor);

            if (m_selectedItems[i]) {
                DrawLineEx(
                    { checkboxBounds.x + 4, checkboxBounds.y + 8 },
                    { checkboxBounds.x + 8, checkboxBounds.y + 12 },
                    2.0f,
                    textColor
                );
                DrawLineEx(
                    { checkboxBounds.x + 8, checkboxBounds.y + 12 },
                    { checkboxBounds.x + 12, checkboxBounds.y + 4 },
                    2.0f,
                    textColor
                );
            }

            const char* text = m_items[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
            Vector2 textPos = {
                itemBounds.x + 32,
                itemY + (itemHeight - textSize.y) / 2
            };
            DrawTextEx(GetFontDefault(), text, textPos, fontSize, 1.0f, textColor);
        }

        if (totalHeight > m_dropdownHeight) {
            float scrollbarHeight = (m_dropdownHeight / totalHeight) * m_dropdownHeight;
            if (scrollbarHeight < 20) scrollbarHeight = 20;
            float maxScroll = totalHeight - m_dropdownHeight;
            float scrollPercent = -m_scrollOffset / maxScroll;
            float scrollbarY = dropdownBounds.y + scrollPercent * (m_dropdownHeight - scrollbarHeight);

            Rectangle scrollbarBounds = {
                dropdownBounds.x + dropdownBounds.width - m_scrollbarWidth + 2,
                scrollbarY,
                m_scrollbarWidth - 4,
                scrollbarHeight
            };

            DrawRectangleRec(scrollbarBounds, GRAY);
        }

        EndScissorMode();
    }
}

void Multiselect::AddItem(const std::string& item) {
    m_items.push_back(item);
    m_selectedItems.push_back(false);
}

void Multiselect::RemoveItem(int index) {
    if (index >= 0 && index < m_items.size()) {
        m_items.erase(m_items.begin() + index);
        m_selectedItems.erase(m_selectedItems.begin() + index);
    }
}

void Multiselect::ClearItems() {
    m_items.clear();
    m_selectedItems.clear();
}

std::vector<int> Multiselect::GetSelectedIndices() const {
    std::vector<int> indices;
    for (int i = 0; i < m_selectedItems.size(); i++) {
        if (m_selectedItems[i]) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<std::string> Multiselect::GetSelectedItems() const {
    std::vector<std::string> selected;
    for (int i = 0; i < m_items.size(); i++) {
        if (m_selectedItems[i]) {
            selected.push_back(m_items[i]);
        }
    }
    return selected;
}

void Multiselect::SetSelectedIndices(const std::vector<int>& indices) {
    for (int i = 0; i < m_selectedItems.size(); i++) {
        m_selectedItems[i] = false;
    }

    for (int index : indices) {
        if (index >= 0 && index < m_selectedItems.size()) {
            m_selectedItems[index] = true;
        }
    }

    if (m_onSelectionChange) {
        m_onSelectionChange(GetSelectedIndices());
    }
}

void Multiselect::SetSelectedItems(const std::vector<std::string>& selectedItems) {
    for (int i = 0; i < m_selectedItems.size(); i++) {
        m_selectedItems[i] = false;
    }

    for (const std::string& item : selectedItems) {
        for (int i = 0; i < m_items.size(); i++) {
            if (m_items[i] == item) {
                m_selectedItems[i] = true;
                break;
            }
        }
    }

    if (m_onSelectionChange) {
        m_onSelectionChange(GetSelectedIndices());
    }
}

void Multiselect::SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback) {
    m_onSelectionChange = callback;
}