#include "Multiselect.h"
#include "../ComboBox/ComboBox.h"
#include "../ColorPicker/ColorPicker.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Multiselect* Multiselect::activeMultiselect = nullptr;

bool Multiselect::IsAnyMultiselectActive() {
    return activeMultiselect != nullptr && activeMultiselect->isOpen;
}

void Multiselect::CloseAllMultiselects() {
    if (activeMultiselect != nullptr) {
        activeMultiselect->isOpen = false;
        activeMultiselect = nullptr;
    }
}

Multiselect::Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange)
    : bounds(bounds), items(items), onSelectionChange(onSelectionChange) {
    state = MULTISELECT_STATE_NORMAL;
    isOpen = false;
    dropdownHeight = 200.0f;
    maxVisibleItems = 5;
    scrollOffset = 0.0f;
    isDraggingScroll = false;
    scrollbarWidth = 12.0f;

    selectedItems.resize(items.size(), false);
}

void Multiselect::Update() {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);

    if (isOpen) {
        Rectangle dropdownBounds = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            dropdownHeight
        };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, bounds)) {
                isOpen = false;
                activeMultiselect = nullptr;
            } else if (!CheckCollisionPointRec(mousePos, dropdownBounds)) {
                isOpen = false;
                activeMultiselect = nullptr;
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
                selectedItems[hoveredIndex] = !selectedItems[hoveredIndex];

                if (onSelectionChange) {
                    onSelectionChange(GetSelectedIndices());
                }
            }
        }
        return;
    }

    if (isHovered) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state = MULTISELECT_STATE_PRESSED;
        } else {
            if (state == MULTISELECT_STATE_PRESSED) {
                if (activeMultiselect != nullptr && activeMultiselect != this) {
                    activeMultiselect->isOpen = false;
                }
                ComboBox::CloseAllComboBoxes();
                ColorPicker::CloseAllPickers();
                isOpen = true;
                scrollOffset = 0.0f;
                activeMultiselect = this;
            }
            state = MULTISELECT_STATE_HOVER;
        }
    } else {
        state = MULTISELECT_STATE_NORMAL;
    }
}

void Multiselect::Draw() {
    // 绘制主按钮
    Color currentColor = backgroundColor;
    if (state == MULTISELECT_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (state == MULTISELECT_STATE_PRESSED) {
        currentColor = pressedColor;
    }
    
    DrawRectangleRec(bounds, currentColor);
    DrawRectangleLinesEx(bounds, 2, borderColor);
    
    // 绘制选中项
    std::vector<std::string> selected = GetSelectedItems();
    if (!selected.empty()) {
        std::string displayText = "";
        for (size_t i = 0; i < selected.size(); i++) {
            if (i > 0) displayText += ", ";
            displayText += selected[i];
        }
        // 如果文本过长，截断并添加省略号
        if (displayText.length() > 20) {
            displayText = displayText.substr(0, 17) + "...";
        }
        
        const char* text = displayText.c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select options";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), placeholder, fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), placeholder, textPos, fontSize, 1.0f, GRAY);
    }
    
    // 绘制下拉箭头
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
    
    // 绘制下拉列表
    if (isOpen) {
        Rectangle dropdownBounds = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            dropdownHeight
        };
        
        DrawRectangleRec(dropdownBounds, dropdownColor);
        DrawRectangleLinesEx(dropdownBounds, 2, dropdownBorderColor);
        
        // 绘制项目
        float itemHeight = 30.0f;
        float totalHeight = items.size() * itemHeight;
        
        // 开始裁剪
        BeginScissorMode(
            (int)dropdownBounds.x,
            (int)dropdownBounds.y,
            (int)dropdownBounds.width,
            (int)dropdownBounds.height
        );
        
        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + scrollOffset;
            Rectangle itemBounds = {
                dropdownBounds.x,
                itemY,
                dropdownBounds.width - (totalHeight > dropdownHeight ? scrollbarWidth : 0),
                itemHeight
            };
            
            // 绘制项目背景
            if (selectedItems[i]) {
                DrawRectangleRec(itemBounds, selectedItemColor);
            } else if (CheckCollisionPointRec(mousePos, itemBounds)) {
                DrawRectangleRec(itemBounds, hoverItemColor);
            }
            
            // 绘制复选框
            Rectangle checkboxBounds = {
                itemBounds.x + 8,
                itemY + (itemHeight - 16) / 2,
                16,
                16
            };
            DrawRectangleRec(checkboxBounds, WHITE);
            DrawRectangleLinesEx(checkboxBounds, 2, borderColor);
            
            // 如果选中，绘制勾选
            if (selectedItems[i]) {
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
            
            // 绘制项目文本
            const char* text = items[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
            Vector2 textPos = {
                itemBounds.x + 32,
                itemY + (itemHeight - textSize.y) / 2
            };
            DrawTextEx(GetFontDefault(), text, textPos, fontSize, 1.0f, textColor);
        }
        
        // 绘制滚动条
        if (totalHeight > dropdownHeight) {
            float scrollbarHeight = (dropdownHeight / totalHeight) * dropdownHeight;
            if (scrollbarHeight < 20) scrollbarHeight = 20;
            float maxScroll = totalHeight - dropdownHeight;
            float scrollPercent = -scrollOffset / maxScroll;
            float scrollbarY = dropdownBounds.y + scrollPercent * (dropdownHeight - scrollbarHeight);
            
            Rectangle scrollbarBounds = {
                dropdownBounds.x + dropdownBounds.width - scrollbarWidth + 2,
                scrollbarY,
                scrollbarWidth - 4,
                scrollbarHeight
            };
            
            DrawRectangleRec(scrollbarBounds, GRAY);
        }
        
        EndScissorMode();
    }
}

void Multiselect::Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor, std::function<void(Rectangle, Color)> drawScrollbar) {
    // 绘制主按钮
    Color currentColor = backgroundColor;
    if (state == MULTISELECT_STATE_HOVER) {
        currentColor = hoverColor;
    } else if (state == MULTISELECT_STATE_PRESSED) {
        currentColor = pressedColor;
    }
    
    drawRect(bounds, currentColor);
    
    // 绘制边框
    drawBorder(bounds, borderColor, 2.0f);
    
    // 绘制选中项
    std::vector<std::string> selected = GetSelectedItems();
    if (!selected.empty()) {
        std::string displayText = "";
        for (size_t i = 0; i < selected.size(); i++) {
            if (i > 0) displayText += ", ";
            displayText += selected[i];
        }
        // 如果文本过长，截断并添加省略号
        if (displayText.length() > 20) {
            displayText = displayText.substr(0, 17) + "...";
        }
        
        const char* text = displayText.c_str();
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - fontSize) / 2
        };
        drawText(text, textPos, fontSize, 1.0f, textColor);
    } else {
        const char* placeholder = "Select options";
        Vector2 textPos = {
            bounds.x + 10,
            bounds.y + (bounds.height - fontSize) / 2
        };
        drawText(placeholder, textPos, fontSize, 1.0f, GRAY);
    }
    
    // 绘制下拉箭头
    Vector2 arrowPos = {
        bounds.x + bounds.width - 25,
        bounds.y + bounds.height / 2 - 4
    };
    // 简化的箭头绘制
    drawLine({ arrowPos.x, arrowPos.y }, { arrowPos.x + 8, arrowPos.y + 8 }, 2.0f, textColor);
    drawLine({ arrowPos.x + 8, arrowPos.y + 8 }, { arrowPos.x + 16, arrowPos.y }, 2.0f, textColor);
    
    // 绘制下拉列表
    if (isOpen) {
        Rectangle dropdownBounds = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            dropdownHeight
        };
        
        drawRect(dropdownBounds, dropdownColor);
        
        // 绘制边框
        drawBorder(dropdownBounds, dropdownBorderColor, 2.0f);
        
        // 绘制项目
        float itemHeight = 30.0f;
        float totalHeight = items.size() * itemHeight;
        
        // 开始裁剪
        beginScissor(dropdownBounds);
        
        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < items.size(); i++) {
            float itemY = dropdownBounds.y + i * itemHeight + scrollOffset;
            Rectangle itemBounds = {
                dropdownBounds.x,
                itemY,
                dropdownBounds.width - (totalHeight > dropdownHeight ? scrollbarWidth : 0),
                itemHeight
            };
            
            // 绘制项目背景
            if (selectedItems[i]) {
                drawRect(itemBounds, selectedItemColor);
            } else if (CheckCollisionPointRec(mousePos, itemBounds)) {
                drawRect(itemBounds, hoverItemColor);
            }
            
            // 绘制复选框
            Rectangle checkboxBounds = {
                itemBounds.x + 8,
                itemY + (itemHeight - 16) / 2,
                16,
                16
            };
            drawRect(checkboxBounds, WHITE);
            
            // 绘制复选框边框
            Vector2 checkboxTopLeft = { checkboxBounds.x, checkboxBounds.y };
            Vector2 checkboxTopRight = { checkboxBounds.x + checkboxBounds.width, checkboxBounds.y };
            Vector2 checkboxBottomLeft = { checkboxBounds.x, checkboxBounds.y + checkboxBounds.height };
            Vector2 checkboxBottomRight = { checkboxBounds.x + checkboxBounds.width, checkboxBounds.y + checkboxBounds.height };
            drawLine(checkboxTopLeft, checkboxTopRight, 2.0f, borderColor);
            drawLine(checkboxTopRight, checkboxBottomRight, 2.0f, borderColor);
            drawLine(checkboxBottomRight, checkboxBottomLeft, 2.0f, borderColor);
            drawLine(checkboxBottomLeft, checkboxTopLeft, 2.0f, borderColor);
            
            // 如果选中，绘制勾选
            if (selectedItems[i]) {
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
            
            // 绘制项目文本
            const char* text = items[i].c_str();
            Vector2 textPos = {
                itemBounds.x + 32,
                itemY + (itemHeight - fontSize) / 2
            };
            drawText(text, textPos, fontSize, 1.0f, textColor);
        }
        
        // 绘制滚动条
        if (totalHeight > dropdownHeight) {
            float scrollbarHeight = (dropdownHeight / totalHeight) * dropdownHeight;
            if (scrollbarHeight < 20) scrollbarHeight = 20;
            float maxScroll = totalHeight - dropdownHeight;
            float scrollPercent = -scrollOffset / maxScroll;
            float scrollbarY = dropdownBounds.y + scrollPercent * (dropdownHeight - scrollbarHeight);
            
            Rectangle scrollbarBounds = {
                dropdownBounds.x + dropdownBounds.width - scrollbarWidth + 2,
                scrollbarY,
                scrollbarWidth - 4,
                scrollbarHeight
            };
            
            drawScrollbar(scrollbarBounds, GRAY);
        }
        
        endScissor();
    }
}

void Multiselect::AddItem(const std::string& item) {
    items.push_back(item);
    selectedItems.push_back(false);
}

void Multiselect::RemoveItem(int index) {
    if (index >= 0 && index < items.size()) {
        items.erase(items.begin() + index);
        selectedItems.erase(selectedItems.begin() + index);
    }
}

void Multiselect::ClearItems() {
    items.clear();
    selectedItems.clear();
}

std::vector<int> Multiselect::GetSelectedIndices() const {
    std::vector<int> indices;
    for (int i = 0; i < selectedItems.size(); i++) {
        if (selectedItems[i]) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<std::string> Multiselect::GetSelectedItems() const {
    std::vector<std::string> selected;
    for (int i = 0; i < items.size(); i++) {
        if (selectedItems[i]) {
            selected.push_back(items[i]);
        }
    }
    return selected;
}

void Multiselect::SetSelectedIndices(const std::vector<int>& indices) {
    // 重置所有选中状态
    for (int i = 0; i < selectedItems.size(); i++) {
        selectedItems[i] = false;
    }
    
    // 设置指定的选中状态
    for (int index : indices) {
        if (index >= 0 && index < selectedItems.size()) {
            selectedItems[index] = true;
        }
    }
    
    // 触发选择变更回调
    if (onSelectionChange) {
        onSelectionChange(GetSelectedIndices());
    }
}

void Multiselect::SetSelectedItems(const std::vector<std::string>& selectedItems) {
    // 重置所有选中状态
    for (int i = 0; i < this->selectedItems.size(); i++) {
        this->selectedItems[i] = false;
    }
    
    // 设置指定的选中状态
    for (const std::string& item : selectedItems) {
        for (int i = 0; i < items.size(); i++) {
            if (items[i] == item) {
                this->selectedItems[i] = true;
                break;
            }
        }
    }
    
    // 触发选择变更回调
    if (onSelectionChange) {
        onSelectionChange(GetSelectedIndices());
    }
}

void Multiselect::SetPosition(float x, float y) {
    bounds.x = x;
    bounds.y = y;
}

void Multiselect::SetSize(float width, float height) {
    bounds.width = width;
    bounds.height = height;
}

void Multiselect::SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback) {
    onSelectionChange = callback;
}

void Multiselect::SetMaxVisibleItems(int count) {
    maxVisibleItems = count;
    dropdownHeight = maxVisibleItems * 30.0f;
}