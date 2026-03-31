#pragma once

#include <raylib.h>
#include <string>
#include <functional>
#include "../Focus/FocusManager.h"

class CheckBox : public GuiControl {
private:
    Rectangle m_bounds;
    bool m_checked;
    std::string m_label;
    std::function<void(bool)> m_onCheckChange;

    enum CheckBoxState {
        CHECKBOX_STATE_NORMAL,
        CHECKBOX_STATE_HOVER,
        CHECKBOX_STATE_PRESSED
    } m_state;

public:
    CheckBox(Rectangle bounds, std::string label = "", bool checked = false, std::function<void(bool)> onCheckChange = nullptr);

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;

    bool IsChecked() const;
    void SetChecked(bool checked);
    void SetLabel(const std::string& label);
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetOnCheckChange(std::function<void(bool)> callback);

    Color borderColor = DARKGRAY;
    Color fillColor = WHITE;
    Color checkColor = BLUE;
    Color textColor = BLACK;
    Color hoverColor = LIGHTGRAY;
    Color pressedColor = GRAY;
};