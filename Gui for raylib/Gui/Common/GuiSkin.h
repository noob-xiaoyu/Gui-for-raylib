#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include "../Focus/FocusManager.h"

class IGuiSkin {
public:
    virtual ~IGuiSkin() = default;

    virtual std::string GetSkinName() const = 0;

    virtual void DrawButton(const PaintContext& ctx, const char* text) = 0;
    virtual void DrawCheckBox(const PaintContext& ctx, bool checked, const char* label) = 0;
    virtual void DrawSlider(const PaintContext& ctx, float value, float min, float max, bool isVertical) = 0;
    virtual void DrawIntSlider(const PaintContext& ctx, int value, int min, int max, bool isVertical) = 0;
    virtual void DrawTextBox(const PaintContext& ctx, const std::string& text, int cursorIndex, int selectionAnchor) = 0;
    virtual void DrawComboBox(const PaintContext& ctx, const std::string& currentItem, bool isOpen) = 0;
    virtual void DrawColorPicker(const PaintContext& ctx, Color color, bool isOpen) = 0;

    // --- Dropdown/Popup elements (Secondary GUI) ---
    virtual void DrawDropdownBackground(const PaintContext& ctx, const Rectangle& bounds) = 0;
    virtual void DrawDropdownItem(const PaintContext& ctx, const Rectangle& bounds, const char* text, bool isHovered, bool isSelected) = 0;
    virtual void DrawColorPickerPanel(const PaintContext& ctx, const Rectangle& bounds, Color current, float hue, float sat, float val) = 0;
};
