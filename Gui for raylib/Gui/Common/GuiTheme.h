#pragma once

#include <raylib.h>

struct GuiTheme {
    struct {
        Color background = WHITE;
        Color border = GRAY;
        Color text = BLACK;
        Color hover = { 200, 200, 200, 255 };
        Color pressed = { 150, 150, 150, 255 };
        Color disabled = { 220, 220, 220, 255 };
    } colors;

    struct {
        Color background = { 230, 230, 230, 255 };
        Color border = GRAY;
        Color itemHover = { 210, 210, 210, 255 };
        Color itemSelected = { 180, 210, 255, 255 };
        Color scrollbar = { 100, 100, 100, 150 };
    } dropdown;

    struct {
        Color track = { 220, 220, 220, 255 };
        Color thumb = { 120, 120, 120, 255 };
        Color thumbHover = { 100, 100, 100, 255 };
        Color thumbPressed = { 0, 121, 241, 255 };
    } slider;

    struct {
        int fontSize = 20;
        float spacing = 1.0f;
    } typography;

    static GuiTheme& Instance() {
        static GuiTheme theme;
        return theme;
    }
};
