#include "ComboBox.h"
#include "../Multiselect/Multiselect.h"
#include "../ColorPicker/ColorPicker.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return value;
    return value;
}

ComboBox* ComboBox::s_activeComboBox = nullptr;

bool ComboBox::IsAnyComboBoxActive() {
    return s_activeComboBox != nullptr && s_activeComboBox->m_isOpen;
}

void ComboBox::CloseAllComboBoxes() {
    if (s_activeComboBox != nullptr) {
        s_activeComboBox->m_isOpen = false;
        s_activeComboBox = nullptr;
    }
}

ComboBox::ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex, std::function<void(int)> onSelectionChange)
    : m_bounds(bounds), m_items(items), m_selectedIndex(selectedIndex), m_onSelectionChange(onSelectionChange) {
    m_state = COMBOBOX_STATE_NORMAL;
    m_isOpen = false;
    m_dropdownHeight = 200.0f;
    m_maxVisibleItems = 5;
    m_scrollOffset = 0.0f;
    m_isDraggingScroll = false;
    m_scrollbarWidth = 12.0f;
    FocusManager::Instance().RegisterControl(this);
}

Rectangle ComboBox::GetBounds() const {
    if (m_isOpen) {
        return {
            m_bounds.x,
            m_bounds.y,
            m_bounds.width,
            m_bounds.height + m_dropdownHeight
        };
    }
    return m_bounds;
}

Rectangle ComboBox::GetDropdownBounds() const {
    return {
        m_bounds.x,
        m_bounds.y + m_bounds.height,
        m_bounds.width,
        m_dropdownHeight
    };
}

void ComboBox::Update() {
    if (!m_isEnabled) return;

    if (m_isOpen) {
        Rectangle dropdownBounds = GetDropdownBounds();
        Vector2 mousePos = GetMousePosition();
        bool isOverDropdown = CheckCollisionPointRec(mousePos, dropdownBounds) || CheckCollisionPointRec(mousePos, m_bounds);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, m_bounds)) {
                m_isOpen = false;
                s_activeComboBox = nullptr;
            } else if (!CheckCollisionPointRec(mousePos, dropdownBounds)) {
                m_isOpen = false;
                s_activeComboBox = nullptr;
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
                m_isOpen = false;
                s_activeComboBox = nullptr;
                if (m_onSelectionChange) {
                    m_onSelectionChange(m_selectedIndex);
                }
            }
        }
        return;
    }

    GuiControl* controlAtMouse = FocusManager::Instance().GetControlAtMouse();
    bool isHoveredByFocus = (controlAtMouse == this);

    if (isHoveredByFocus) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = COMBOBOX_STATE_PRESSED;
        } else {
            if (m_state == COMBOBOX_STATE_PRESSED) {
                if (s_activeComboBox != nullptr && s_activeComboBox != this) {
                    s_activeComboBox->m_isOpen = false;
                }
                Multiselect::CloseAllMultiselects();
                ColorPicker::CloseAllPickers();
                m_isOpen = true;
                m_scrollOffset = 0.0f;
                s_activeComboBox = this;
            }
            m_state = COMBOBOX_STATE_HOVER;
        }
    } else {
        m_state = COMBOBOX_STATE_NORMAL;
    }
}

void ComboBox::Draw() {
    Color currentColor = backgroundColor;
    if (m_state == COMBOBOX_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (m_state == COMBOBOX_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    DrawRectangleRec(m_bounds, currentColor);
    DrawRectangleLinesEx(m_bounds, 2, borderColor);

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

void ComboBox::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor, std::function<void(Rectangle, Color)> drawScrollbar) {
    Color currentColor = backgroundColor;
    if (m_state == COMBOBOX_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (m_state == COMBOBOX_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    drawRect(m_bounds, currentColor);
    drawBorder(m_bounds, borderColor, 2.0f);

    if (m_selectedIndex >= 0 && m_selectedIndex < m_items.size()) {
        const char* text = m_items[m_selectedIndex].c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - textSize.y) / 2
        };
        drawText(text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select an option";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), placeholder, fontSize, 1.0f);
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - textSize.y) / 2
        };
        drawText(placeholder, textPos, fontSize, 1.0f, GRAY);
    }

    Vector2 arrowPos = {
        m_bounds.x + m_bounds.width - 25,
        m_bounds.y + m_bounds.height / 2 - 4
    };
    drawLine({arrowPos.x, arrowPos.y}, {arrowPos.x + 8, arrowPos.y + 8}, 1.0f, textColor);
    drawLine({arrowPos.x + 8, arrowPos.y + 8}, {arrowPos.x + 16, arrowPos.y}, 1.0f, textColor);

    if (m_isOpen) {
        Rectangle dropdownBounds = GetDropdownBounds();

        drawRect(dropdownBounds, dropdownColor);
        drawBorder(dropdownBounds, dropdownBorderColor, 1.0f);

        float itemHeight = 30.0f;
        float totalHeight = m_items.size() * itemHeight;
        float maxScroll = totalHeight - m_dropdownHeight;
        if (maxScroll < 0) maxScroll = 0;

        beginScissor(dropdownBounds);

        Vector2 mousePos = GetMousePosition();
        for (size_t i = 0; i < m_items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + m_scrollOffset;
            if (itemY + itemHeight < dropdownBounds.y || itemY > dropdownBounds.y + dropdownBounds.height) continue;

            Rectangle itemRect = { dropdownBounds.x, itemY, dropdownBounds.width, itemHeight };
            Color itemColor = (i == (size_t)m_selectedIndex) ? selectedItemColor : dropdownColor;

            if (CheckCollisionPointRec(mousePos, itemRect) && itemY >= dropdownBounds.y && itemY + itemHeight <= dropdownBounds.y + dropdownBounds.height) {
                itemColor = hoverItemColor;
            }

            drawRect(itemRect, itemColor);

            const char* itemText = m_items[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), itemText, fontSize, 1.0f);
            Vector2 textPos = {
                dropdownBounds.x + 10,
                itemY + (itemHeight - textSize.y) / 2
            };
            drawText(itemText, textPos, fontSize, 1.0f, textColor);
        }

        endScissor();

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
            drawScrollbar(scrollbarBounds, GRAY);
        }
    }
}

void ComboBox::AddItem(const std::string& item) {
    m_items.push_back(item);
}

void ComboBox::RemoveItem(int index) {
    if (index >= 0 && index < m_items.size()) {
        m_items.erase(m_items.begin() + index);
        if (m_selectedIndex >= m_items.size()) {
            m_selectedIndex = m_items.size() - 1;
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

void ComboBox::SetPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void ComboBox::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void ComboBox::SetOnSelectionChange(std::function<void(int)> callback) {
    m_onSelectionChange = callback;
}

void ComboBox::SetMaxVisibleItems(int count) {
    m_maxVisibleItems = count;
    m_dropdownHeight = count * 30.0f;
}