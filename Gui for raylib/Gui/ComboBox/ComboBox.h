#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>
#include "../Focus/FocusManager.h"

class Multiselect;
class ColorPicker;

class ComboBox : public GuiControl {
private:
    Rectangle m_bounds;
    std::vector<std::string> m_items;
    int m_selectedIndex;
    bool m_isOpen;
    std::function<void(int)> m_onSelectionChange;

    enum ComboBoxState {
        COMBOBOX_STATE_NORMAL,
        COMBOBOX_STATE_HOVER,
        COMBOBOX_STATE_PRESSED
    } m_state;

    float m_dropdownHeight;
    int m_maxVisibleItems;
    float m_scrollOffset;
    bool m_isDraggingScroll;
    float m_scrollbarWidth;

    static ComboBox* s_activeComboBox;

public:
    static bool IsAnyComboBoxActive();
    static void CloseAllComboBoxes();

public:
    ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex = -1, std::function<void(int)> onSelectionChange = nullptr);

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;
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

    Rectangle GetDropdownBounds() const;

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