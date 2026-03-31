#pragma once
#include <raylib.h>
#include <string>
#include <deque>
#include <functional>
#include "../Focus/FocusManager.h"

struct TextState {
    std::string text;
    int cursorIndex;
    int selectionAnchor;
};

class TextBox : public GuiControl {
public:
    TextBox(Rectangle bounds);
    ~TextBox();

    Rectangle GetBounds() const override;
    void Update() override;
    void Draw() override;
    void Draw(Color bgcolor, Color textcolor);

    void SetText(const std::string& text);
    std::string GetText() const;
    void SetFocus(bool focus);
    bool IsFocused() const;

    void SetFont(Font font, float fontSize, float spacing, float lineHeight = 0.0f);
    void SetSize(float width, float height);
    void SetPosition(float x, float y);

private:
    std::deque<TextState> m_undoStack;
    std::deque<TextState> m_redoStack;
    const size_t MAX_HISTORY = 100;
    void SaveUndoState();
    void Undo();
    void Redo();

    void HandleInput();
    void HandleMouse();

    void InsertCodepoint(int codepoint);
    void InsertText(const std::string& str);
    void Backspace();
    void Delete();
    void DeleteSelection();
    void Copy();
    void Paste();
    void Cut();
    void PerformAutoIndent();

    void UpdateScrollbarState();
    void HandleScroll();

    int GetIndexFromPoint(Vector2 point);
    Vector2 GetPositionFromIndex(int index);
    void UpdateIMEPosition();

    int GetPrevCharIndex(int index) const;
    int GetNextCharIndex(int index) const;

    Rectangle GetTextArea() const;

    bool IsKeyActionTriggered(int key);
private:
    Rectangle m_bounds;
    std::string m_text;

    Font m_font;
    float m_fontSize;
    float m_spacing;
    float m_lineHeight;

    float m_contentHeight;
    bool m_showScrollbar;
    bool m_isDraggingScroll;
    float m_scrollbarWidth;

    int m_cursorIndex;
    int m_selectionAnchor;
    int m_lastCursorIndex;

    double m_blinkTimer;
    bool m_isDragging;
    Vector2 m_scrollOffset;

    int m_currentRepeatingKey;
    float m_keyRepeatTimer;
    float m_keyRepeatDelay;
    float m_keyRepeatInterval;

    bool m_wasHovered;
};