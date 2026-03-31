#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>
#include "../Focus/FocusManager.h"
#include "GuiTheme.h"

class DropdownControl : public GuiControl {
protected:
    Rectangle m_bounds;
    std::vector<std::string> m_items;
    bool m_isOpen = false;
    
    float m_dropdownHeight = 200.0f;
    int m_maxVisibleItems = 5;
    float m_scrollOffset = 0.0f;
    bool m_isDraggingScroll = false;
    float m_scrollbarWidth = 12.0f;

public:
    DropdownControl(Rectangle bounds, std::vector<std::string> items)
        : m_bounds(bounds), m_items(items) {
        
        auto& theme = GuiTheme::Instance();
        backgroundColor = theme.colors.background;
        borderColor = theme.colors.border;
        textColor = theme.colors.text;
        hoverColor = theme.colors.hover;
        pressedColor = theme.colors.pressed;
        dropdownColor = theme.dropdown.background;
        dropdownBorderColor = theme.dropdown.border;
        selectedItemColor = theme.dropdown.itemSelected;
        hoverItemColor = theme.dropdown.itemHover;
        fontSize = theme.typography.fontSize;

        FocusManager::Instance().RegisterControl(this);
    }

    Rectangle GetBounds() const override {
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

    Rectangle GetDropdownBounds() const {
        return {
            m_bounds.x,
            m_bounds.y + m_bounds.height,
            m_bounds.width,
            m_dropdownHeight
        };
    }

    void SetPosition(float x, float y) {
        m_bounds.x = x;
        m_bounds.y = y;
    }

    void SetSize(float width, float height) {
        m_bounds.width = width;
        m_bounds.height = height;
    }

    void SetMaxVisibleItems(int count) {
        m_maxVisibleItems = count;
        m_dropdownHeight = count * 30.0f; // Default item height
    }

    virtual void AddItem(const std::string& item) { m_items.push_back(item); }
    virtual void RemoveItem(int index) { if (index >= 0 && index < (int)m_items.size()) m_items.erase(m_items.begin() + index); }
    virtual void ClearItems() { m_items.clear(); }

    virtual void Close() {
        m_isOpen = false;
        if (FocusManager::Instance().IsActiveControl(this)) {
            FocusManager::Instance().ClearActiveControl();
        }
    }

    // Shared Colors (moved to theme later)
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
