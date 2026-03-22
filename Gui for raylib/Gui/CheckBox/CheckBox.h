#pragma once

#include <raylib.h>
#include <string>
#include <functional>

class CheckBox {
private:
    Rectangle bounds;
    bool checked;
    std::string label;
    std::function<void(bool)> onCheckChange;
    
    enum CheckBoxState {
        CHECKBOX_STATE_NORMAL,
        CHECKBOX_STATE_HOVER,
        CHECKBOX_STATE_PRESSED
    } state;
    
public:
    CheckBox(Rectangle bounds, std::string label = "", bool checked = false, std::function<void(bool)> onCheckChange = nullptr);
    
    void Update();
    void Draw();
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText);
    
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
