#include "TextBox.h"
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include "../../Package/win/win.h"
#include "../Common/GuiTheme.h"
#include "../Common/GuiRenderer.h"
#include "../Common/GuiSkin.h"

inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return value;
    return value;
}

namespace Platform {
    void SetImeLocation(int x, int y) {
#if defined(_WIN32)
        void* handle = GetWindowHandle();
        if (handle) {
            win::ImeContext ime(handle);
            ime.SetCompositionWindowPos(x, y);
        }
#endif
    }
}

namespace TextUtils {
    std::string CodepointToUTF8(int codepoint) {
        char result[5] = { 0 };
        if (codepoint <= 0x7f) result[0] = (char)codepoint;
        else if (codepoint <= 0x7ff) {
            result[0] = (char)(0xc0 | ((codepoint >> 6) & 0x1f));
            result[1] = (char)(0x80 | (codepoint & 0x3f));
        }
        else if (codepoint <= 0xffff) {
            result[0] = (char)(0xe0 | ((codepoint >> 12) & 0x0f));
            result[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
            result[2] = (char)(0x80 | (codepoint & 0x3f));
        }
        else {
            result[0] = (char)(0xf0 | ((codepoint >> 18) & 0x07));
            result[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
            result[2] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
            result[3] = (char)(0x80 | (codepoint & 0x3f));
        }
        return std::string(result);
    }

    bool IsCharStart(unsigned char c) {
        return (c & 0xC0) != 0x80;
    }
}

TextBox::TextBox(Rectangle bounds)
    : m_bounds(bounds), m_text(""),
    m_cursorIndex(0), m_selectionAnchor(0), m_blinkTimer(0),
    m_isDragging(false), m_scrollOffset({ 0,0 }),
    m_contentHeight(0), m_showScrollbar(false),
    m_isDraggingScroll(false), m_scrollbarWidth(12.0f)
{
    m_wantsFocus = true; // TextBox needs keyboard focus
    m_font = GetFontDefault();
    m_fontSize = 20.0f;
    m_spacing = 1.0f;
    m_lineHeight = m_fontSize;
    m_lastCursorIndex = 0;
    m_currentRepeatingKey = 0;
    m_keyRepeatTimer = 0.0f;
    m_keyRepeatDelay = 0.5f;
    m_keyRepeatInterval = 0.05f;
    m_wasHovered = false;
    FocusManager::Instance().RegisterControl(this);
}

TextBox::~TextBox() {}

Rectangle TextBox::GetBounds() const {
    return m_bounds;
}

void TextBox::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void TextBox::SetPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void TextBox::SetFont(Font font, float fontSize, float spacing, float lineHeight) {
    m_font = font;
    m_fontSize = fontSize;
    m_spacing = spacing;
    m_lineHeight = (lineHeight > 0.0f) ? lineHeight : fontSize;
}

void TextBox::SetText(const std::string& text) {
    m_text = text;
    m_cursorIndex = (int)m_text.length();
    m_selectionAnchor = m_cursorIndex;
}

std::string TextBox::GetText() const { return m_text; }

void TextBox::SetFocus(bool focus) {
    if (focus) {
        FocusManager::Instance().SetFocusedControl(this);
    } else if (FocusManager::Instance().GetFocusedControl() == this) {
        FocusManager::Instance().ClearFocusedControl();
    }
}

bool TextBox::IsFocused() const { return m_isFocused; }

void TextBox::Update() {
    if (!m_isVisible || !m_isEnabled) return;

    // 如果有活跃控件（如弹出框）且不是自己，拦截所有操作
    if (FocusManager::Instance().HasActiveControl() && !FocusManager::Instance().IsActiveControl(this)) {
        return;
    }

    HandleScroll();
    HandleMouse();

    if (!m_isFocused) return;

    if (!m_isDraggingScroll) {
        HandleInput();
        if (m_cursorIndex != m_lastCursorIndex) {
            Rectangle textArea = GetTextArea();
            Vector2 cPos = GetPositionFromIndex(m_cursorIndex);

            float relativeY = cPos.y - textArea.y;

            if (relativeY < 0) {
                m_scrollOffset.y -= relativeY;
            }
            else if (relativeY + m_lineHeight > textArea.height) {
                m_scrollOffset.y -= (relativeY + m_lineHeight - textArea.height);
            }

            UpdateScrollbarState();
            m_lastCursorIndex = m_cursorIndex;
        }
        m_blinkTimer += GetFrameTime();
    }

    if ((int)(m_blinkTimer * 10) % 5 == 0) {
        UpdateIMEPosition();
    }

    m_blinkTimer += GetFrameTime();
}

bool TextBox::IsKeyActionTriggered(int key) {
    if (IsKeyPressed(key)) {
        m_currentRepeatingKey = key;
        m_keyRepeatTimer = 0.0f;
        return true;
    }

    if (IsKeyDown(key) && m_currentRepeatingKey == key) {
        m_keyRepeatTimer += GetFrameTime();

        if (m_keyRepeatTimer > m_keyRepeatDelay) {
            if (m_keyRepeatTimer > m_keyRepeatDelay + m_keyRepeatInterval) {
                m_keyRepeatTimer -= m_keyRepeatInterval;
                return true;
            }
        }
    }

    if (IsKeyReleased(key)) {
        if (m_currentRepeatingKey == key) m_currentRepeatingKey = 0;
    }

    return false;
}

void TextBox::HandleInput() {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32) {
            DeleteSelection();
            InsertCodepoint(key);
        }
        key = GetCharPressed();
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (ctrl) {
        if (IsKeyPressed(KEY_C)) Copy();
        else if (IsKeyActionTriggered(KEY_V)) { SaveUndoState(); Paste(); }
        else if (IsKeyActionTriggered(KEY_X)) { SaveUndoState(); Cut(); }
        else if (IsKeyPressed(KEY_A)) {
            m_selectionAnchor = 0;
            m_cursorIndex = (int)m_text.length();
        }
        else if (IsKeyActionTriggered(KEY_Z)) {
            if (shift) Redo();
            else Undo();
        }
        else if (IsKeyActionTriggered(KEY_Y)) {
            Redo();
        }
    }

    if (IsKeyActionTriggered(KEY_ENTER)) {
        SaveUndoState();
        DeleteSelection();
        InsertText("\n");
        PerformAutoIndent();
    }
    else if (IsKeyActionTriggered(KEY_BACKSPACE)) {
        if (m_cursorIndex != m_selectionAnchor) DeleteSelection();
        else Backspace();
    }
    else if (IsKeyActionTriggered(KEY_DELETE)) {
        if (m_cursorIndex != m_selectionAnchor) DeleteSelection();
        else Delete();
    }

    if (IsKeyActionTriggered(KEY_LEFT)) {
        m_cursorIndex = GetPrevCharIndex(m_cursorIndex);
        if (!shift) m_selectionAnchor = m_cursorIndex;
        m_blinkTimer = 0;
    }
    else if (IsKeyActionTriggered(KEY_RIGHT)) {
        m_cursorIndex = GetNextCharIndex(m_cursorIndex);
        if (!shift) m_selectionAnchor = m_cursorIndex;
        m_blinkTimer = 0;
    }
    else if (IsKeyActionTriggered(KEY_UP)) {
        Vector2 currentPos = GetPositionFromIndex(m_cursorIndex);
        currentPos.y -= m_lineHeight;
        m_cursorIndex = GetIndexFromPoint(currentPos);
        if (!shift) m_selectionAnchor = m_cursorIndex;
        m_blinkTimer = 0;
    }
    else if (IsKeyActionTriggered(KEY_DOWN)) {
        Vector2 currentPos = GetPositionFromIndex(m_cursorIndex);
        currentPos.y += m_lineHeight;
        m_cursorIndex = GetIndexFromPoint(currentPos);
        if (!shift) m_selectionAnchor = m_cursorIndex;
        m_blinkTimer = 0;
    }
}

void TextBox::HandleMouse() {
    Vector2 mousePos = GetMousePosition();
    Rectangle textArea = GetTextArea();

    if (m_isHovered) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        m_wasHovered = true;
    }
    else if (m_wasHovered) {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        m_wasHovered = false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (m_isHovered && !m_isDraggingScroll) {
            // 注意：FocusManager::Update 已经自动设置了 m_isFocused = true
            m_isDragging = true;
            m_lastCursorIndex = -1;
            m_cursorIndex = GetIndexFromPoint(mousePos);
            m_selectionAnchor = m_cursorIndex;
        }
    }

    if (m_isDragging && !m_isDraggingScroll) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            m_isDragging = false;
        }
        else {
            m_cursorIndex = GetIndexFromPoint(mousePos);
        }
    }
}

void TextBox::InsertCodepoint(int codepoint) {
    InsertText(TextUtils::CodepointToUTF8(codepoint));
}

void TextBox::InsertText(const std::string& str) {
    SaveUndoState();
    m_text.insert(m_cursorIndex, str);
    m_cursorIndex += (int)str.length();
    m_selectionAnchor = m_cursorIndex;
}

void TextBox::Backspace() {
    if (m_cursorIndex > 0) {
        SaveUndoState();
        int prev = GetPrevCharIndex(m_cursorIndex);
        m_text.erase(prev, m_cursorIndex - prev);
        m_cursorIndex = prev;
        m_selectionAnchor = m_cursorIndex;
    }
}

void TextBox::Delete() {
    if (m_cursorIndex < (int)m_text.length()) {
        SaveUndoState();
        int next = GetNextCharIndex(m_cursorIndex);
        m_text.erase(m_cursorIndex, next - m_cursorIndex);
    }
}

void TextBox::DeleteSelection() {
    if (m_cursorIndex == m_selectionAnchor) return;
    SaveUndoState();
    int start = std::min(m_cursorIndex, m_selectionAnchor);
    int end = std::max(m_cursorIndex, m_selectionAnchor);
    m_text.erase(start, end - start);
    m_cursorIndex = start;
    m_selectionAnchor = start;
}

void TextBox::PerformAutoIndent() {
    if (m_cursorIndex <= 1) return;
    int prevLineEnd = m_cursorIndex - 1;
    int prevLineStart = (int)m_text.rfind('\n', prevLineEnd - 1);

    if (prevLineStart == std::string::npos) prevLineStart = 0;
    else prevLineStart++;

    std::string indent = "";
    for (int i = prevLineStart; i < prevLineEnd; i++) {
        char c = m_text[i];
        if (c == ' ' || c == '\t') indent += c;
        else break;
    }
    if (!indent.empty()) InsertText(indent);
}

void TextBox::SaveUndoState() {
    if (!m_undoStack.empty() && m_undoStack.back().text == m_text) {
        return;
    }

    m_undoStack.push_back({ m_text, m_cursorIndex, m_selectionAnchor });

    if (m_undoStack.size() > MAX_HISTORY) {
        m_undoStack.pop_front();
    }

    m_redoStack.clear();
}

void TextBox::Undo() {
    if (m_undoStack.empty()) return;

    m_redoStack.push_back({ m_text, m_cursorIndex, m_selectionAnchor });

    TextState last = m_undoStack.back();
    m_undoStack.pop_back();

    m_text = last.text;
    m_cursorIndex = last.cursorIndex;
    m_selectionAnchor = last.selectionAnchor;

    m_lastCursorIndex = -1;
}

void TextBox::Redo() {
    if (m_redoStack.empty()) return;

    m_undoStack.push_back({ m_text, m_cursorIndex, m_selectionAnchor });

    TextState next = m_redoStack.back();
    m_redoStack.pop_back();

    m_text = next.text;
    m_cursorIndex = next.cursorIndex;
    m_selectionAnchor = next.selectionAnchor;

    m_lastCursorIndex = -1;
}

void TextBox::Copy() {
    if (m_cursorIndex != m_selectionAnchor) {
        int start = std::min(m_cursorIndex, m_selectionAnchor);
        int end = std::max(m_cursorIndex, m_selectionAnchor);
        SetClipboardText(m_text.substr(start, end - start).c_str());
    }
}

void TextBox::Paste() {
    const char* clip = GetClipboardText();
    if (clip) {
        DeleteSelection();
        InsertText(clip);
    }
}

void TextBox::Cut() {
    Copy();
    DeleteSelection();
}

int TextBox::GetIndexFromPoint(Vector2 point) {
    Rectangle textArea = GetTextArea();
    point.x -= textArea.x + m_scrollOffset.x;
    point.y -= textArea.y + m_scrollOffset.y;

    int line = (int)(point.y / m_lineHeight);
    if (line < 0) line = 0;

    int currentLine = 0;
    int idx = 0;
    int lastLineStart = 0;

    while (idx < (int)m_text.length()) {
        if (m_text[idx] == '\n') {
            if (currentLine == line) break;
            currentLine++;
            idx++;
            lastLineStart = idx;
        }
        else {
            idx++;
        }
    }

    if (currentLine < line) return (int)m_text.length();

    int lineEnd = idx;

    for (int i = lastLineStart; i < lineEnd; ) {
        int nextI = GetNextCharIndex(i);

        std::string subStr = m_text.substr(lastLineStart, nextI - lastLineStart);
        float widthToEnd = MeasureTextEx(m_font, subStr.c_str(), m_fontSize, m_spacing).x;

        std::string charStr = m_text.substr(i, nextI - i);
        float charWidth = MeasureTextEx(m_font, charStr.c_str(), m_fontSize, m_spacing).x;

        float charLeftX = widthToEnd - charWidth;

        if (point.x < charLeftX + charWidth / 2) {
            return i;
        }

        i = nextI;
    }

    return lineEnd;
}

Vector2 TextBox::GetPositionFromIndex(int index) {
    Rectangle textArea = GetTextArea();
    if (index > (int)m_text.length()) index = (int)m_text.length();

    int line = 0;
    int lineStart = 0;
    for (int i = 0; i < index; i++) {
        if (m_text[i] == '\n') {
            line++;
            lineStart = i + 1;
        }
    }

    std::string sub = m_text.substr(lineStart, index - lineStart);
    Vector2 size = MeasureTextEx(m_font, sub.c_str(), m_fontSize, m_spacing);

    return Vector2{
        textArea.x + m_scrollOffset.x + size.x,
        textArea.y + m_scrollOffset.y + line * m_lineHeight
    };
}

void TextBox::UpdateIMEPosition() {
    if (m_isFocused) {
        Vector2 localPos = GetPositionFromIndex(m_cursorIndex);
        Platform::SetImeLocation((int)localPos.x, (int)(localPos.y + m_lineHeight));
    }
}

void TextBox::UpdateScrollbarState() {
    int lineCount = 1;
    for (char c : m_text) if (c == '\n') lineCount++;
    m_contentHeight = lineCount * m_lineHeight;

    float maxScroll = m_contentHeight - m_bounds.height;
    if (maxScroll < 0) maxScroll = 0;

    m_showScrollbar = m_contentHeight > m_bounds.height;

    if (m_scrollOffset.y > 0) m_scrollOffset.y = 0;
    if (m_scrollOffset.y < -maxScroll) m_scrollOffset.y = -maxScroll;
}

void TextBox::HandleScroll() {
    UpdateScrollbarState();

    Vector2 mousePos = GetMousePosition();
    Rectangle textArea = m_bounds;
    float wheel = GetMouseWheelMove();
    if (m_showScrollbar) {
        textArea.width -= m_scrollbarWidth;
    }

    if (wheel != 0 && CheckCollisionPointRec(mousePos, textArea)) {
        float maxScroll = m_contentHeight - m_bounds.height;
        if (maxScroll < 0) maxScroll = 0;

        m_scrollOffset.y += wheel * m_lineHeight * 3;

        if (m_scrollOffset.y > 0) m_scrollOffset.y = 0;
        if (m_scrollOffset.y < -maxScroll) m_scrollOffset.y = -maxScroll;
    }

    if (m_showScrollbar) {
        Rectangle scrollTrack = {
            m_bounds.x + m_bounds.width - m_scrollbarWidth - 4,
            m_bounds.y + 4,
            m_scrollbarWidth - 2,
            m_bounds.height - 8
        };

        float thumbHeight = (scrollTrack.height / m_contentHeight) * scrollTrack.height;
        if (thumbHeight < 20) thumbHeight = 20;

        float scrollPercent = -m_scrollOffset.y / (m_contentHeight - m_bounds.height);
        float thumbY = scrollTrack.y + (scrollTrack.height - thumbHeight) * scrollPercent;
        Rectangle thumbRect = { scrollTrack.x + 1, thumbY, scrollTrack.width - 2, thumbHeight };

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, thumbRect)) {
            m_isDraggingScroll = true;
        }

        if (m_isDraggingScroll) {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                m_isDraggingScroll = false;
            }
            else {
                float mouseYRelative = mousePos.y - m_bounds.y - thumbHeight / 2;
                float newPercent = mouseYRelative / (m_bounds.height - thumbHeight);
                newPercent = clamp(newPercent, 0.0f, 1.0f);
                m_scrollOffset.y = -newPercent * (m_contentHeight - m_bounds.height);
            }
        }
    }
    else {
        m_isDraggingScroll = false;
    }
}

int TextBox::GetPrevCharIndex(int index) const {
    if (index <= 0) return 0;
    int i = index - 1;
    while (i > 0 && !TextUtils::IsCharStart((unsigned char)m_text[i])) i--;
    return i;
}

int TextBox::GetNextCharIndex(int index) const {
    if (index >= (int)m_text.length()) return (int)m_text.length();
    int i = index + 1;
    while (i < (int)m_text.length() && !TextUtils::IsCharStart((unsigned char)m_text[i])) i++;
    return i;
}

Rectangle TextBox::GetTextArea() const {
    Rectangle textArea = m_bounds;
    if (m_showScrollbar) {
        textArea.width -= m_scrollbarWidth;
    }
    float padding = 4.0f;
    textArea.x += padding;
    textArea.y += padding;
    textArea.width -= padding * 2;
    textArea.height -= padding * 2;

    return textArea;
}

void TextBox::Draw() {
    if (!m_isVisible) return;

    auto& theme = GuiTheme::Instance();
    Color bgColor = theme.colors.background;
    Color textColor = theme.colors.text;

    if (m_skin) {
        PaintContext ctx = { m_bounds, GetState(), m_isFocused };
        m_skin->DrawTextBox(ctx, m_text, m_cursorIndex, m_selectionAnchor);
        
        // 如果是 DarkSkin，我们自动适配内部文字颜色（后续可从 Skin 获取颜色属性）
        if (m_skin->GetSkinName() == "DarkSkin") textColor = WHITE; 
    } else {
        GuiRenderer::DrawRect(m_bounds, bgColor, true, m_isFocused ? BLUE : DARKGRAY);
    }

    Draw(BLANK, textColor); // 调用内部逻辑绘制文本、光标、选择区域，背景设为透明
}

void TextBox::Draw(Color bgcolor, Color textcolor) {
    if (!m_isVisible) return;
    Rectangle textArea = GetTextArea();

    DrawRectangleRec(m_bounds, bgcolor);
    DrawRectangleLinesEx(m_bounds, 1, IsFocused() ? BLUE : DARKGRAY);

    BeginScissorMode((int)textArea.x, (int)textArea.y, (int)textArea.width, (int)textArea.height);

    if (m_cursorIndex != m_selectionAnchor) {
        int start = std::min(m_cursorIndex, m_selectionAnchor);
        int end = std::max(m_cursorIndex, m_selectionAnchor);
        int p = start;
        while (p < end) {
            size_t nextNL = m_text.find('\n', p);
            int lineEnd = (nextNL == std::string::npos || nextNL >= (size_t)end) ? end : (int)nextNL;

            Vector2 posStart = GetPositionFromIndex(p);
            float width = MeasureTextEx(m_font, m_text.substr(p, lineEnd - p).c_str(), m_fontSize, m_spacing).x;
            if (width < 2.0f && lineEnd < (int)m_text.length() && m_text[lineEnd] == '\n') width = 8.0f;

            if (posStart.y + m_lineHeight > m_bounds.y && posStart.y < m_bounds.y + m_bounds.height) {
                DrawRectangleRec({ posStart.x, posStart.y, width, m_lineHeight }, { 0, 120, 255, 100 });
            }
            p = lineEnd;
            if (p < end && m_text[p] == '\n') p++;
        }
    }

    int start = 0;
    int lineCount = 0;
    for (int i = 0; i <= (int)m_text.length(); i++) {
        if (i == (int)m_text.length() || m_text[i] == '\n') {
            std::string lineStr = m_text.substr(start, i - start);
            Vector2 drawPos = {
                textArea.x + m_scrollOffset.x,
                textArea.y + m_scrollOffset.y + lineCount * m_lineHeight
            };
            if (drawPos.y + m_lineHeight > textArea.y && drawPos.y < textArea.y + textArea.height) {
                DrawTextEx(m_font, lineStr.c_str(), drawPos, m_fontSize, m_spacing, textcolor);
            }
            lineCount++;
            start = i + 1;
        }
    }

    if (m_isFocused && ((int)(m_blinkTimer * 2) % 2 == 0)) {
        Vector2 cPos = GetPositionFromIndex(m_cursorIndex);
        DrawLineEx({ cPos.x, cPos.y }, { cPos.x, cPos.y + m_lineHeight }, 2, RED);
    }

    EndScissorMode();

    if (m_showScrollbar) {
        Rectangle scrollTrack = {
            m_bounds.x + m_bounds.width - m_scrollbarWidth - 4,
            m_bounds.y + 4,
            m_scrollbarWidth - 2,
            m_bounds.height - 8
        };
        DrawRectangleRec(scrollTrack, { 45, 45, 45, 255 });

        float thumbHeight = (scrollTrack.height / m_contentHeight) * scrollTrack.height;
        if (thumbHeight < 20) thumbHeight = 20;
        float scrollPercent = -m_scrollOffset.y / (m_contentHeight - m_bounds.height);
        float thumbY = scrollTrack.y + (scrollTrack.height - thumbHeight) * scrollPercent;

        Rectangle thumbRect = { scrollTrack.x + 1, thumbY, scrollTrack.width - 2, thumbHeight };
        DrawRectangleRounded(thumbRect, 0.5f, 4, m_isDraggingScroll ? DARKGRAY : GRAY);
    }
}

