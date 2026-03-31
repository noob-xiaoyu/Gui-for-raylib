#pragma once

#include "GuiSkin.h"
#include "GuiRenderer.h"

// --- 现代暗黑皮肤 (Modern Dark Skin) ---
class DarkSkin : public IGuiSkin {
public:
    std::string GetSkinName() const override { return "DarkSkin"; }
    void DrawButton(const PaintContext& ctx, const char* text) override {
        Color base = { 45, 45, 48, 255 };
        if (ctx.state == ControlState::HOVER) base = { 62, 62, 66, 255 };
        if (ctx.state == ControlState::PRESSED) base = { 0, 122, 204, 255 };

        DrawRectangleRounded(ctx.bounds, 0.15f, 4, base);
        if (ctx.isFocused) DrawRectangleRoundedLinesEx(ctx.bounds, 0.15f, 4, 1.0f, { 0, 122, 204, 255 });
        else DrawRectangleRoundedLinesEx(ctx.bounds, 0.15f, 4, 1.0f, { 80, 80, 80, 255 });

        GuiRenderer::DrawCenteredText(ctx.bounds, text, 18, WHITE);
    }

    void DrawCheckBox(const PaintContext& ctx, bool checked, const char* label) override {
        Color boxColor = { 30, 30, 30, 255 };
        if (ctx.state == ControlState::HOVER) boxColor = { 50, 50, 50, 255 };
        
        DrawRectangleRec(ctx.bounds, boxColor);
        DrawRectangleLinesEx(ctx.bounds, 1, { 100, 100, 100, 255 });

        if (checked) {
            Rectangle inner = { ctx.bounds.x + 4, ctx.bounds.y + 4, ctx.bounds.width - 8, ctx.bounds.height - 8 };
            DrawRectangleRec(inner, { 0, 122, 204, 255 });
        }

        if (label) {
            DrawText(label, (int)ctx.bounds.x + (int)ctx.bounds.width + 8, (int)ctx.bounds.y + 2, 16, { 220, 220, 220, 255 });
        }
    }

    void DrawSlider(const PaintContext& ctx, float value, float min, float max, bool isVertical) override {
        // 绘制轨道
        Color trackColor = { 60, 60, 60, 255 };
        DrawRectangleRec(ctx.bounds, trackColor);

        // 绘制滑块
        float progress = (value - min) / (max - min);
        if (isVertical) {
            float thumbHeight = 12;
            float thumbY = ctx.bounds.y + (1.0f - progress) * (ctx.bounds.height - thumbHeight);
            Rectangle thumb = { ctx.bounds.x - 2, thumbY, ctx.bounds.width + 4, thumbHeight };
            DrawRectangleRec(thumb, { 0, 122, 204, 255 });
        } else {
            float thumbWidth = 12;
            float thumbX = ctx.bounds.x + progress * (ctx.bounds.width - thumbWidth);
            Rectangle thumb = { thumbX, ctx.bounds.y - 2, thumbWidth, ctx.bounds.height + 4 };
            DrawRectangleRec(thumb, { 0, 122, 204, 255 });
        }
    }

    void DrawIntSlider(const PaintContext& ctx, int value, int min, int max, bool isVertical) override {
        DrawSlider(ctx, (float)value, (float)min, (float)max, isVertical);
    }
    
    void DrawTextBox(const PaintContext& ctx, const std::string& text, int cursorIndex, int selectionAnchor) override {
        DrawRectangleRec(ctx.bounds, { 37, 37, 38, 255 });
        DrawRectangleLinesEx(ctx.bounds, 1, ctx.isFocused ? Color{ 0, 122, 204, 255 } : Color{ 63, 63, 70, 255 });
        // 不再这里直接 DrawText，交给组件内部逻辑处理复杂的编辑状态
    }

    void DrawComboBox(const PaintContext& ctx, const std::string& currentItem, bool isOpen) override {
        DrawRectangleRec(ctx.bounds, { 51, 51, 52, 255 });
        DrawRectangleLinesEx(ctx.bounds, 1, { 67, 67, 70, 255 });
        DrawText(currentItem.c_str(), (int)ctx.bounds.x + 8, (int)ctx.bounds.y + (int)ctx.bounds.height/2 - 8, 16, WHITE);
        
        // 绘制小箭头
        Vector2 v1 = { ctx.bounds.x + ctx.bounds.width - 20, ctx.bounds.y + ctx.bounds.height/2 - 4 };
        Vector2 v2 = { ctx.bounds.x + ctx.bounds.width - 10, ctx.bounds.y + ctx.bounds.height/2 - 4 };
        Vector2 v3 = { ctx.bounds.x + ctx.bounds.width - 15, ctx.bounds.y + ctx.bounds.height/2 + 4 };
        DrawTriangle(v1, v2, v3, LIGHTGRAY);
    }

    void DrawColorPicker(const PaintContext& ctx, Color color, bool isOpen) override {
        DrawRectangleRec(ctx.bounds, color);
        DrawRectangleLinesEx(ctx.bounds, 1, WHITE);
    }

    void DrawDropdownBackground(const PaintContext& ctx, const Rectangle& bounds) override {
        DrawRectangleRec(bounds, { 30, 30, 30, 255 });
        DrawRectangleLinesEx(bounds, 1, { 80, 80, 80, 255 });
    }

    void DrawDropdownItem(const PaintContext& ctx, const Rectangle& bounds, const char* text, bool isHovered, bool isSelected) override {
        if (isSelected) DrawRectangleRec(bounds, { 0, 122, 204, 255 });
        else if (isHovered) DrawRectangleRec(bounds, { 60, 60, 60, 255 });
        
        DrawText(text, (int)bounds.x + 8, (int)bounds.y + (int)bounds.height / 2 - 8, 16, WHITE);
    }

    void DrawColorPickerPanel(const PaintContext& ctx, const Rectangle& bounds, Color current, float hue, float sat, float val) override {
        DrawRectangleRec(bounds, { 45, 45, 48, 255 });
        DrawRectangleLinesEx(bounds, 1, { 100, 100, 100, 255 });
        // 实际的渐变绘制通常由组件处理，皮肤负责面板背景
    }
};

// --- 简约亮色皮肤 (Minimal Light Skin) ---
class LightSkin : public IGuiSkin {
public:
    std::string GetSkinName() const override { return "LightSkin"; }
    void DrawButton(const PaintContext& ctx, const char* text) override {
        Color base = RAYWHITE;
        if (ctx.state == ControlState::HOVER) base = { 240, 240, 240, 255 };
        if (ctx.state == ControlState::PRESSED) base = { 220, 220, 220, 255 };

        DrawRectangleRec(ctx.bounds, base);
        DrawRectangleLinesEx(ctx.bounds, 1, ctx.isFocused ? BLUE : DARKGRAY);
        GuiRenderer::DrawCenteredText(ctx.bounds, text, 18, BLACK);
    }

    void DrawCheckBox(const PaintContext& ctx, bool checked, const char* label) override {
        DrawRectangleLinesEx(ctx.bounds, 2, BLACK);
        if (checked) {
            DrawRectangleRec({ctx.bounds.x + 4, ctx.bounds.y + 4, ctx.bounds.width - 8, ctx.bounds.height - 8}, BLACK);
        }
        if (label) DrawText(label, (int)ctx.bounds.x + (int)ctx.bounds.width + 8, (int)ctx.bounds.y + 2, 16, DARKGRAY);
    }

    void DrawSlider(const PaintContext& ctx, float value, float min, float max, bool isVertical) override {
        float progress = (value - min) / (max - min);
        DrawLineEx({ctx.bounds.x, ctx.bounds.y + ctx.bounds.height/2}, {ctx.bounds.x + ctx.bounds.width, ctx.bounds.y + ctx.bounds.height/2}, 2, GRAY);
        float thumbX = ctx.bounds.x + progress * ctx.bounds.width;
        DrawCircle((int)thumbX, (int)(ctx.bounds.y + ctx.bounds.height/2), 8, BLACK);
    }

    void DrawIntSlider(const PaintContext& ctx, int value, int min, int max, bool isVertical) override {
        DrawSlider(ctx, (float)value, (float)min, (float)max, isVertical);
    }

    void DrawTextBox(const PaintContext& ctx, const std::string& text, int cursorIndex, int selectionAnchor) override {
        DrawRectangleRec(ctx.bounds, WHITE);
        DrawRectangleLinesEx(ctx.bounds, 1, ctx.isFocused ? BLACK : LIGHTGRAY);
    }

    void DrawComboBox(const PaintContext& ctx, const std::string& currentItem, bool isOpen) override {
        DrawRectangleLinesEx(ctx.bounds, 1, BLACK);
        DrawText(currentItem.c_str(), (int)ctx.bounds.x + 8, (int)ctx.bounds.y + 8, 16, BLACK);
    }

    void DrawColorPicker(const PaintContext& ctx, Color color, bool isOpen) override {
        DrawCircleV({ctx.bounds.x + ctx.bounds.width/2, ctx.bounds.y + ctx.bounds.height/2}, ctx.bounds.width/2, color);
        DrawCircleLines((int)(ctx.bounds.x + ctx.bounds.width/2), (int)(ctx.bounds.y + ctx.bounds.height/2), ctx.bounds.width/2, BLACK);
    }

    void DrawDropdownBackground(const PaintContext& ctx, const Rectangle& bounds) override {
        DrawRectangleRec(bounds, RAYWHITE);
        DrawRectangleLinesEx(bounds, 1, BLACK);
    }

    void DrawDropdownItem(const PaintContext& ctx, const Rectangle& bounds, const char* text, bool isHovered, bool isSelected) override {
        if (isSelected) DrawRectangleRec(bounds, { 220, 220, 220, 255 });
        else if (isHovered) DrawRectangleRec(bounds, { 240, 240, 240, 255 });
        
        DrawText(text, (int)bounds.x + 8, (int)bounds.y + (int)bounds.height / 2 - 8, 16, BLACK);
    }

    void DrawColorPickerPanel(const PaintContext& ctx, const Rectangle& bounds, Color current, float hue, float sat, float val) override {
        DrawRectangleRec(bounds, RAYWHITE);
        DrawRectangleLinesEx(bounds, 1, BLACK);
    }
};
