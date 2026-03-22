#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>

class ComboBox;
class ColorPicker;

class Multiselect {
private:
    Rectangle bounds;
    std::vector<std::string> items;
    std::vector<bool> selectedItems;
    bool isOpen;
    std::function<void(const std::vector<int>&)> onSelectionChange;
    
    enum MultiselectState {
        MULTISELECT_STATE_NORMAL,
        MULTISELECT_STATE_HOVER,
        MULTISELECT_STATE_PRESSED
    } state;
    
    float dropdownHeight;
    int maxVisibleItems;
    float scrollOffset;
    bool isDraggingScroll;
    float scrollbarWidth;

    static Multiselect* activeMultiselect;

public:
    static bool IsAnyMultiselectActive();
    static void CloseAllMultiselects();

public:
    Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange = nullptr);
    
    void Update();
    void Draw();
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor, std::function<void(Rectangle, Color)> drawScrollbar);
    
    void AddItem(const std::string& item);
    void RemoveItem(int index);
    void ClearItems();
    
    std::vector<int> GetSelectedIndices() const;
    std::vector<std::string> GetSelectedItems() const;
    void SetSelectedIndices(const std::vector<int>& indices);
    void SetSelectedItems(const std::vector<std::string>& items);
    
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback);
    void SetMaxVisibleItems(int count);
    
    Color backgroundColor = WHITE;
    Color borderColor = GRAY;
    Color textColor = BLACK;
    Color hoverColor = LIGHTGRAY;
    Color pressedColor = DARKGRAY;
    Color dropdownColor = LIGHTGRAY;
    Color dropdownBorderColor = GRAY;
    Color selectedItemColor = WHITE;
    Color hoverItemColor = WHITE;
    int fontSize = 16;
};