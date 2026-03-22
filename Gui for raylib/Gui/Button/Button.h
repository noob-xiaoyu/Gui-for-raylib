#pragma once

#include <raylib.h>
#include <string>
#include <functional>
#include "../Focus/FocusManager.h"

class Button : public GuiControl {
public:
    Button(Rectangle bounds, std::string text, std::function<void()> onClick);

    void SetPos(float x, float y);
    void SetPos(Vector2 pos);
    void SetSize(float width, float height);
    void SetSize(Vector2 size);

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText);

    Vector2 GetPos() const;
    Vector2 GetSize() const;
    std::string GetText() const;
    int GetFontSize() const;

    bool IsHovered() const;
    bool IsPressed() const;

    Color baseColor = GRAY;
    Color hoverColor = LIGHTGRAY;
    Color pressedColor = DARKGRAY;
    Color textColor = WHITE;
    int fontSize = 20;

private:
    Rectangle m_bounds;
    std::string m_text;
    enum ButtonState {
        BUTTON_STATE_NORMAL,
        BUTTON_STATE_HOVER,
        BUTTON_STATE_PRESSED
    } m_state;
    std::function<void()> m_onClick;
};