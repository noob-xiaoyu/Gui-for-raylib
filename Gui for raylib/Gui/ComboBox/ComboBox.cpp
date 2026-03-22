#include "ComboBox.h"
#include "../Multiselect/Multiselect.h"
#include "../ColorPicker/ColorPicker.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

ComboBox* ComboBox::activeComboBox = nullptr;

bool ComboBox::IsAnyComboBoxActive() {
    return activeComboBox != nullptr && activeComboBox->isOpen;
}

void ComboBox::CloseAllComboBoxes() {
    if (activeComboBox != nullptr) {
        activeComboBox->isOpen = false;
        activeComboBox = nullptr;
    }
}

ComboBox::ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex, std::function<void(int)> onSelectionChange)
    : bounds(bounds), items(items), selectedIndex(selectedIndex), onSelectionChange(onSelectionChange) {
    state = COMBOBOX_STATE_NORMAL;
    isOpen = false;
    dropdownHeight = 200.0f;
    maxVisibleItems = 5;
    scrollOffset = 0.0f;
    isDraggingScroll = false;
    scrollbarWidth = 12.0f;
}

void ComboBox::Update() {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);

    if (isOpen) {
        Rectangle dropdownBounds = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            dropdownHeight
        };

        bool isOverDropdown = CheckCollisionPointRec(mousePos, dropdownBounds) || CheckCollisionPointRec(mousePos, bounds);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, bounds)) {
                isOpen = false;
                activeComboBox = nullptr;
            } else if (!CheckCollisionPointRec(mousePos, dropdownBounds)) {
                isOpen = false;
                activeComboBox = nullptr;
            }
        }

        float itemHeight = 30.0f;
        float totalHeight = items.size() * itemHeight;
        float maxScroll = totalHeight - dropdownHeight;
        if (maxScroll < 0) maxScroll = 0;

        if (CheckCollisionPointRec(mousePos, dropdownBounds)) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                scrollOffset += wheel * itemHeight * 2;
                if (scrollOffset > 0) scrollOffset = 0;
                if (scrollOffset < -maxScroll) scrollOffset = -maxScroll;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, dropdownBounds)) {
            int hoveredIndex = (int)((mousePos.y - dropdownBounds.y - scrollOffset) / itemHeight);
            if (hoveredIndex >= 0 && hoveredIndex < items.size()) {
                selectedIndex = hoveredIndex;
                isOpen = false;
                activeComboBox = nullptr;
                if (onSelectionChange) {
                    onSelectionChange(selectedIndex);
                }
            }
        }
        return;
    }

    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state = COMBOBOX_STATE_PRESSED;
        } else {
            if (state == COMBOBOX_STATE_PRESSED) {
                if (activeComboBox != nullptr && activeComboBox != this) {
                    activeComboBox->isOpen = false;
                }
                Multiselect::CloseAllMultiselects();
                ColorPicker::CloseAllPickers();
                isOpen = true;
                scrollOffset = 0.0f;
                activeComboBox = this;
            }
            state = COMBOBOX_STATE_HOVER;
        }
    } else {
        state = COMBOBOX_STATE_NORMAL;
    }
}

void ComboBox::Draw() {
    Color currentColor = backgroundColor;
    if (state == COMBOBOX_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (state == COMBOBOX_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    DrawRectangleRec(bounds, currentColor);
    DrawRectangleLinesEx(bounds, 2, borderColor);

    if (selectedIndex >= 0 && selectedIndex < items.size()) {
        const char* text = items[selectedIndex].c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select an option";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), placeholder, fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), placeholder, textPos, fontSize, 1.0f, GRAY);
    }

    Vector2 arrowPos = {
        bounds.x + bounds.width - 25,
        bounds.y + bounds.height / 2 - 4
    };
    DrawTriangle(
        { arrowPos.x, arrowPos.y },
        { arrowPos.x + 8, arrowPos.y + 8 },
        { arrowPos.x + 16, arrowPos.y },
        textColor
    );

    if (isOpen) {
        Rectangle dropdownBounds = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            dropdownHeight
        };

        DrawRectangleRec(dropdownBounds, dropdownColor);
        DrawRectangleLinesEx(dropdownBounds, 1, dropdownBorderColor);

        float itemHeight = 30.0f;
        float totalHeight = items.size() * itemHeight;
        float maxScroll = totalHeight - dropdownHeight;
        if (maxScroll < 0) maxScroll = 0;

        BeginScissorMode((int)dropdownBounds.x, (int)dropdownBounds.y, (int)dropdownBounds.width, (int)dropdownBounds.height);

        for (size_t i = 0; i < items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + scrollOffset;
            if (itemY + itemHeight < dropdownBounds.y || itemY > dropdownBounds.y + dropdownBounds.height) continue;

            Rectangle itemRect = { dropdownBounds.x, itemY, dropdownBounds.width, itemHeight };
            Color itemColor = (i == (size_t)selectedIndex) ? selectedItemColor : dropdownColor;

            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, itemRect) && itemY >= dropdownBounds.y && itemY + itemHeight <= dropdownBounds.y + dropdownBounds.height) {
                itemColor = hoverItemColor;
            }

            DrawRectangleRec(itemRect, itemColor);

            const char* itemText = items[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), itemText, fontSize, 1.0f);
            Vector2 textPos = {
                dropdownBounds.x + 10,
                itemY + (itemHeight - textSize.y) / 2
            };
            DrawTextEx(GetFontDefault(), itemText, textPos, fontSize, 1.0f, textColor);
        }

        EndScissorMode();

        if (totalHeight > dropdownHeight) {
            float scrollbarHeight = (dropdownHeight / totalHeight) * dropdownHeight;
            if (scrollbarHeight < 20) scrollbarHeight = 20;
            float scrollbarY = dropdownBounds.y + (-scrollOffset / maxScroll) * (dropdownHeight - scrollbarHeight);
            Rectangle scrollbarBounds = {
                dropdownBounds.x + dropdownBounds.width - scrollbarWidth,
                scrollbarY,
                scrollbarWidth,
                scrollbarHeight
            };
            DrawRectangleRec(scrollbarBounds, GRAY);
        }
    }
}

void ComboBox::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor, std::function<void(Rectangle, Color)> drawScrollbar) {
    Color currentColor = backgroundColor;
    if (state == COMBOBOX_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (state == COMBOBOX_STATE_PRESSED) {
        currentColor = pressedColor;
    }

    drawRect(bounds, currentColor);
    drawBorder(bounds, borderColor, 2.0f);

    if (selectedIndex >= 0 && selectedIndex < items.size()) {
        const char* text = items[selectedIndex].c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        drawText(text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select an option";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), placeholder, fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        drawText(placeholder, textPos, fontSize, 1.0f, GRAY);
    }

    Vector2 arrowPos = {
        bounds.x + bounds.width - 25,
        bounds.y + bounds.height / 2 - 4
    };
    drawLine({arrowPos.x, arrowPos.y}, {arrowPos.x + 8, arrowPos.y + 8}, 1.0f, textColor);
    drawLine({arrowPos.x + 8, arrowPos.y + 8}, {arrowPos.x + 16, arrowPos.y}, 1.0f, textColor);

    if (isOpen) {
        Rectangle dropdownBounds = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            dropdownHeight
        };

        drawRect(dropdownBounds, dropdownColor);
        drawBorder(dropdownBounds, dropdownBorderColor, 1.0f);

        float itemHeight = 30.0f;
        float totalHeight = items.size() * itemHeight;
        float maxScroll = totalHeight - dropdownHeight;
        if (maxScroll < 0) maxScroll = 0;

        beginScissor(dropdownBounds);

        for (size_t i = 0; i < items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + scrollOffset;
            if (itemY + itemHeight < dropdownBounds.y || itemY > dropdownBounds.y + dropdownBounds.height) continue;

            Rectangle itemRect = { dropdownBounds.x, itemY, dropdownBounds.width, itemHeight };
            Color itemColor = (i == (size_t)selectedIndex) ? selectedItemColor : dropdownColor;

            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, itemRect) && itemY >= dropdownBounds.y && itemY + itemHeight <= dropdownBounds.y + dropdownBounds.height) {
                itemColor = hoverItemColor;
            }

            drawRect(itemRect, itemColor);

            const char* itemText = items[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), itemText, fontSize, 1.0f);
            Vector2 textPos = {
                dropdownBounds.x + 10,
                itemY + (itemHeight - textSize.y) / 2
            };
            drawText(itemText, textPos, fontSize, 1.0f, textColor);
        }

        endScissor();

        if (totalHeight > dropdownHeight) {
            float scrollbarHeight = (dropdownHeight / totalHeight) * dropdownHeight;
            if (scrollbarHeight < 20) scrollbarHeight = 20;
            float scrollbarY = dropdownBounds.y + (-scrollOffset / maxScroll) * (dropdownHeight - scrollbarHeight);
            Rectangle scrollbarBounds = {
                dropdownBounds.x + dropdownBounds.width - scrollbarWidth,
                scrollbarY,
                scrollbarWidth,
                scrollbarHeight
            };
            drawScrollbar(scrollbarBounds, GRAY);
        }
    }
}

void ComboBox::AddItem(const std::string& item) {
    items.push_back(item);
}

void ComboBox::RemoveItem(int index) {
    if (index >= 0 && index < items.size()) {
        items.erase(items.begin() + index);
        if (selectedIndex >= items.size()) {
            selectedIndex = items.size() - 1;
        }
    }
}

void ComboBox::ClearItems() {
    items.clear();
    selectedIndex = -1;
}

int ComboBox::GetSelectedIndex() const {
    return selectedIndex;
}

std::string ComboBox::GetSelectedItem() const {
    if (selectedIndex >= 0 && selectedIndex < items.size()) {
        return items[selectedIndex];
    }
    return "";
}

void ComboBox::SetSelectedIndex(int index) {
    if (index >= -1 && index < items.size()) {
        selectedIndex = index;
    }
}

void ComboBox::SetPosition(float x, float y) {
    bounds.x = x;
    bounds.y = y;
}

void ComboBox::SetSize(float width, float height) {
    bounds.width = width;
    bounds.height = height;
}

void ComboBox::SetOnSelectionChange(std::function<void(int)> callback) {
    onSelectionChange = callback;
}

void ComboBox::SetMaxVisibleItems(int count) {
    maxVisibleItems = count;
    dropdownHeight = count * 30.0f;
}
