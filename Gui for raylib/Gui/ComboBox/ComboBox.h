#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>

class Multiselect;
class ColorPicker;

class ComboBox {
private:
    Rectangle bounds;
    std::vector<std::string> items;
    int selectedIndex;
    bool isOpen;
    std::function<void(int)> onSelectionChange;
    
    enum ComboBoxState {
        COMBOBOX_STATE_NORMAL,
        COMBOBOX_STATE_HOVER,
        COMBOBOX_STATE_PRESSED
    } state;
    
    float dropdownHeight;
    int maxVisibleItems;
    float scrollOffset;
    bool isDraggingScroll;
    float scrollbarWidth;

    static ComboBox* activeComboBox;

public:
    static bool IsAnyComboBoxActive();
    static void CloseAllComboBoxes();

public:
    ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex = -1, std::function<void(int)> onSelectionChange = nullptr);
    
    void Update();
    void Draw();
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor, std::function<void(Rectangle, Color)> drawScrollbar);
    
    void AddItem(const std::string& item);
    void RemoveItem(int index);
    void ClearItems();
    
    int GetSelectedIndex() const;
    std::string GetSelectedItem() const;
    void SetSelectedIndex(int index);
    
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetOnSelectionChange(std::function<void(int)> callback);
    void SetMaxVisibleItems(int count);
    
    Color backgroundColor = WHITE;
    Color borderColor = GRAY;
    Color textColor = BLACK;
    Color hoverColor = LIGHTGRAY;
    Color pressedColor = DARKGRAY;
    Color dropdownColor = LIGHTGRAY;
    Color dropdownBorderColor = GRAY;
    Color selectedItemColor = WHITE;
    Color hoverItemColor = WHITE; // 下拉项目悬浮颜色
    int fontSize = 16;
};
