#include "Multiselect.h"
#include "../ComboBox/ComboBox.h"
#include "../ColorPicker/ColorPicker.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return value;
    return value;
}

Multiselect* Multiselect::s_activeMultiselect = nullptr;

bool Multiselect::IsAnyMultiselectActive() {
    return s_activeMultiselect != nullptr && s_activeMultiselect->m_isOpen;
}

void Multiselect::CloseAllMultiselects() {
    if (s_activeMultiselect != nullptr) {
        s_activeMultiselect->m_isOpen = false;
        s_activeMultiselect = nullptr;
    }
}

Multiselect::Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange)
    : m_bounds(bounds), m_items(items), m_onSelectionChange(onSelectionChange) {
    m_state = MULTISELECT_STATE_NORMAL;
    m_isOpen = false;
    m_dropdownHeight = 200.0f;
    m_maxVisibleItems = 5;
    m_scrollOffset = 0.0f;
    m_isDraggingScroll = false;
    m_scrollbarWidth = 12.0f;

    m_selectedItems.resize(items.size(), false);
    FocusManager::Instance().RegisterControl(this);
}

Rectangle Multiselect::GetBounds() const {
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

Rectangle Multiselect::GetDropdownBounds() const {
    return {
        m_bounds.x,
        m_bounds.y + m_bounds.height,
        m_bounds.width,
        m_dropdownHeight
    };
}

void Multiselect::Update() {
    if (!m_isEnabled) return;

    if (m_isOpen) {
        Rectangle dropdownBounds = GetDropdownBounds();
        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, m_bounds)) {
                m_isOpen = false;
                s_activeMultiselect = nullptr;
            } else if (!CheckCollisionPointRec(mousePos, dropdownBounds)) {
                m_isOpen = false;
                s_activeMultiselect = nullptr;
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

    GuiControl* controlAtMouse = FocusManager::Instance().GetControlAtMouse();
    bool isHoveredByFocus = (controlAtMouse == this);

    if (isHoveredByFocus) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_state = MULTISELECT_STATE_PRESSED;
        } else {
            if (m_state == MULTISELECT_STATE_PRESSED) {
                if (s_activeMultiselect != nullptr && s_activeMultiselect != this) {
                    s_activeMultiselect->m_isOpen = false;
                }
                ComboBox::CloseAllComboBoxes();
                ColorPicker::CloseAllPickers();
                m_isOpen = true;
                m_scrollOffset = 0.0f;
                s_activeMultiselect = this;
            }
            m_state = MULTISELECT_STATE_HOVER;
        }
    } else {
        m_state = MULTISELECT_STATE_NORMAL;
    }
}

void Multiselect::Draw() {
    Color currentColor = backgroundColor;
    if (m_state == MULTISELECT_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (m_state == MULTISELECT_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    DrawRectangleRec(m_bounds, currentColor);
    DrawRectangleLinesEx(m_bounds, 2, borderColor);

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

void Multiselect::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor, std::function<void(Rectangle, Color)> drawScrollbar) {
    Color currentColor = backgroundColor;
    if (m_state == MULTISELECT_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (m_state == MULTISELECT_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    drawRect(m_bounds, currentColor);
    drawBorder(m_bounds, borderColor, 2.0f);

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
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - fontSize) / 2
        };
        drawText(text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select options";
        Vector2 textPos = {
            m_bounds.x + 10,
            m_bounds.y + (m_bounds.height - fontSize) / 2
        };
        drawText(placeholder, textPos, fontSize, 1.0f, GRAY);
    }

    Vector2 arrowPos = {
        m_bounds.x + m_bounds.width - 25,
        m_bounds.y + m_bounds.height / 2 - 4
    };
    drawLine({ arrowPos.x, arrowPos.y }, { arrowPos.x + 8, arrowPos.y + 8 }, 2.0f, textColor);
    drawLine({ arrowPos.x + 8, arrowPos.y + 8 }, { arrowPos.x + 16, arrowPos.y }, 2.0f, textColor);

    if (m_isOpen) {
        Rectangle dropdownBounds = GetDropdownBounds();

        drawRect(dropdownBounds, dropdownColor);
        drawBorder(dropdownBounds, dropdownBorderColor, 2.0f);

        float itemHeight = 30.0f;
        float totalHeight = m_items.size() * itemHeight;

        beginScissor(dropdownBounds);

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
                drawRect(itemBounds, selectedItemColor);
            } else if (CheckCollisionPointRec(mousePos, itemBounds)) {
                drawRect(itemBounds, hoverItemColor);
            }

            Rectangle checkboxBounds = {
                itemBounds.x + 8,
                itemY + (itemHeight - 16) / 2,
                16,
                16
            };
            drawRect(checkboxBounds, WHITE);

            Vector2 checkboxTopLeft = { checkboxBounds.x, checkboxBounds.y };
            Vector2 checkboxTopRight = { checkboxBounds.x + checkboxBounds.width, checkboxBounds.y };
            Vector2 checkboxBottomLeft = { checkboxBounds.x, checkboxBounds.y + checkboxBounds.height };
            Vector2 checkboxBottomRight = { checkboxBounds.x + checkboxBounds.width, checkboxBounds.y + checkboxBounds.height };
            drawLine(checkboxTopLeft, checkboxTopRight, 2.0f, borderColor);
            drawLine(checkboxTopRight, checkboxBottomRight, 2.0f, borderColor);
            drawLine(checkboxBottomRight, checkboxBottomLeft, 2.0f, borderColor);
            drawLine(checkboxBottomLeft, checkboxTopLeft, 2.0f, borderColor);

            if (m_selectedItems[i]) {
                drawLine(
                    { checkboxBounds.x + 4, checkboxBounds.y + 8 },
                    { checkboxBounds.x + 8, checkboxBounds.y + 12 },
                    2.0f,
                    textColor
                );
                drawLine(
                    { checkboxBounds.x + 8, checkboxBounds.y + 12 },
                    { checkboxBounds.x + 12, checkboxBounds.y + 4 },
                    2.0f,
                    textColor
                );
            }

            const char* text = m_items[i].c_str();
            Vector2 textPos = {
                itemBounds.x + 32,
                itemY + (itemHeight - fontSize) / 2
            };
            drawText(text, textPos, fontSize, 1.0f, textColor);
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

            drawScrollbar(scrollbarBounds, GRAY);
        }

        endScissor();
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

void Multiselect::SetPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void Multiselect::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void Multiselect::SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback) {
    m_onSelectionChange = callback;
}

void Multiselect::SetMaxVisibleItems(int count) {
    m_maxVisibleItems = count;
    m_dropdownHeight = m_maxVisibleItems * 30.0f;
}