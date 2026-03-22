#pragma once

#include <raylib.h>
#include <string>
#include <functional>
#include "../Focus/FocusManager.h"

class Slider : public GuiControl {
public:
    enum class Direction {
        Horizontal,
        Vertical
    };

    Slider(Rectangle bounds, float minValue, float maxValue, float initialValue, Direction direction = Direction::Horizontal, std::function<void(float)> onValueChanged = nullptr);

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText);

    void SetValue(float value);
    float GetValue() const;
    void SetMinValue(float minValue);
    void SetMaxValue(float maxValue);

    void SetFocus(bool focus);
    bool IsFocused() const;

    Color trackColor = LIGHTGRAY;
    Color thumbColor = GRAY;
    Color thumbHoverColor = DARKGRAY;
    Color thumbPressedColor = SKYBLUE;
    Color textColor = BLACK;

private:
    Rectangle m_bounds;
    float m_minValue;
    float m_maxValue;
    float m_value;
    Direction m_direction;
    std::function<void(float)> m_onValueChanged;

    enum SliderState {
        SLIDER_STATE_NORMAL,
        SLIDER_STATE_HOVER,
        SLIDER_STATE_DRAGGING
    } m_state;

    float m_thumbSize;
    Rectangle GetThumbBounds() const;
    float ValueToPosition(float value) const;
    float PositionToValue(float position) const;
    bool IsPointOnThumb(Vector2 point) const;
    void HandleKeyboardInput();
};

class IntSlider : public GuiControl {
public:
    enum class Direction {
        Horizontal,
        Vertical
    };

    IntSlider(Rectangle bounds, int minValue, int maxValue, int initialValue, Direction direction = Direction::Horizontal, std::function<void(int)> onValueChanged = nullptr);

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;
    void Draw(std::function<void(Rectangle, Color)> drawRect, std::function<void(Rectangle, Color, float)> drawBorder, std::function<void(const char*, Vector2, float, float, Color)> drawText);

    void SetValue(int value);
    int GetValue() const;
    void SetMinValue(int minValue);
    void SetMaxValue(int maxValue);

    void SetFocus(bool focus);
    bool IsFocused() const;

    Color trackColor = LIGHTGRAY;
    Color thumbColor = GRAY;
    Color thumbHoverColor = DARKGRAY;
    Color thumbPressedColor = SKYBLUE;
    Color textColor = BLACK;

private:
    Rectangle m_bounds;
    int m_minValue;
    int m_maxValue;
    int m_value;
    Direction m_direction;
    std::function<void(int)> m_onValueChanged;

    enum IntSliderState {
        INTSLIDER_STATE_NORMAL,
        INTSLIDER_STATE_HOVER,
        INTSLIDER_STATE_DRAGGING
    } m_state;

    float m_thumbSize;
    Rectangle GetThumbBounds() const;
    float ValueToPosition(float value) const;
    float PositionToValue(float position) const;
    bool IsPointOnThumb(Vector2 point) const;
    void HandleKeyboardInput();
};