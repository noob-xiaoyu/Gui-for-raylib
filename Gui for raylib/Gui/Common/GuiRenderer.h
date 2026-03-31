#pragma once

#include <raylib.h>
#include <string>
#include "GuiTheme.h"

class GuiRenderer {
public:
    static void DrawRect(Rectangle bounds, Color color, bool drawBorder = false, Color borderColor = GRAY) {
        DrawRectangleRec(bounds, color);
        if (drawBorder) {
            DrawRectangleLinesEx(bounds, 1.0f, borderColor);
        }
    }

    static void DrawCenteredText(Rectangle bounds, const char* text, int fontSize, Color color) {
        if (!text || text[0] == '\0') return;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, (float)fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + (bounds.width - textSize.x) / 2,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text, textPos, (float)fontSize, 1.0f, color);
    }

    static void DrawLeftAlignedText(Rectangle bounds, const char* text, int fontSize, float padding, Color color) {
        if (!text || text[0] == '\0') return;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, (float)fontSize, 1.0f);
        Vector2 textPos = {
            bounds.x + padding,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawTextEx(GetFontDefault(), text, textPos, (float)fontSize, 1.0f, color);
    }
};
