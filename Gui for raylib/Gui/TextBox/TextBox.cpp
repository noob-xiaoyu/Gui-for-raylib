#include "TextBox.h"
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include "../../Package/win/win.h"

// 自定义 clamp 函数，兼容 C++11
inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// --- Windows IME Helper (RAII Wrapper) ---
// Platform Integration
namespace Platform {
    void SetImeLocation(int x, int y) {
#if defined(_WIN32)
        // 1. 获取 Raylib 的原生窗口句柄 (void*)
        void* handle = GetWindowHandle();

        if (handle) {
            // 2. 使用你的 RAII 对象封装
            // 假设 win::ImeContext 构造函数里调用了 ImmGetContext
            // 析构函数里调用了 ImmReleaseContext
            win::ImeContext ime(handle);

            // 3. 设置位置
            ime.SetCompositionWindowPos(x, y);
        }
#endif
    }
}
// --- TextBox Implementation ---
// UTF-8 Helpers
namespace TextUtils {
    // 将 Unicode 码点转换为 UTF-8 字符串
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
// --- TextBox Class ---
// 构造函数
TextBox::TextBox(Rectangle bounds)
    : m_bounds(bounds), m_text(""), m_isFocused(false),
    m_cursorIndex(0), m_selectionAnchor(0), m_blinkTimer(0),
    m_isDragging(false), m_scrollOffset({ 0,0 }),
    m_contentHeight(0), m_showScrollbar(false),
    m_isDraggingScroll(false), m_scrollbarWidth(12.0f) // 初始化滚动条
{
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
}
// 析构函数
TextBox::~TextBox() {}

void TextBox::SetSize(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
}

void TextBox::SetPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

// 设置字体配置
void TextBox::SetFont(Font font, float fontSize, float spacing, float lineHeight) {
    m_font = font;
    m_fontSize = fontSize;
    m_spacing = spacing;
    // 如果未指定行高，则默认使用 fontSize，或者可以 fontSize * 1.2f 增加可读性
    m_lineHeight = (lineHeight > 0.0f) ? lineHeight : fontSize;
}
// 设置文本内容，并将光标移动到文本末尾
void TextBox::SetText(const std::string& text) {
    m_text = text;
    m_cursorIndex = (int)m_text.length();
    m_selectionAnchor = m_cursorIndex;
}
// 获取当前文本内容
std::string TextBox::GetText() const { return m_text; }
// 设置焦点状态
void TextBox::SetFocus(bool focus) { m_isFocused = focus; }
// 获取焦点状态
bool TextBox::IsFocused() const { return m_isFocused; }
// 主更新函数：处理输入、更新状态
void TextBox::Update() {
    // 1. 先处理滚动条，再处理鼠标，确保滚动条状态先更新
    HandleScroll();
    HandleMouse();

    // 2. 如果处理完鼠标后依然没有焦点，就不处理键盘输入了
    if (!m_isFocused) return;

    // 3. 处理键盘输入
    if (!m_isDraggingScroll) {
        HandleInput();
        if (m_cursorIndex != m_lastCursorIndex) {
            Rectangle textArea = GetTextArea();
            Vector2 cPos = GetPositionFromIndex(m_cursorIndex);

            // 计算光标相对于可见区域顶部的 Y
            float relativeY = cPos.y - textArea.y;

            // 检查光标是否超出顶部
            if (relativeY < 0) {
                m_scrollOffset.y -= relativeY;
            }
            // 检查光标是否超出底部 (relativeY + 行高 > 区域高度)
            else if (relativeY + m_lineHeight > textArea.height) {
                m_scrollOffset.y -= (relativeY + m_lineHeight - textArea.height);
            }

            // 每次强制跳转后，都要确保不超出内容边界
            UpdateScrollbarState();

            // 更新记录，防止下一帧在没按键的情况下再次跳转
            m_lastCursorIndex = m_cursorIndex;
        }
        m_blinkTimer += GetFrameTime();
    }

    // 4. 更新 IME 位置跟随
    if ((int)(m_blinkTimer * 10) % 5 == 0) {
        UpdateIMEPosition();
    }

    m_blinkTimer += GetFrameTime();
}

bool TextBox::IsKeyActionTriggered(int key) {
    if (IsKeyPressed(key)) {
        m_currentRepeatingKey = key;
        m_keyRepeatTimer = 0.0f; // 重置计时器
        return true; // 刚按下，立即触发
    }

    if (IsKeyDown(key) && m_currentRepeatingKey == key) {
        m_keyRepeatTimer += GetFrameTime();

        // 阶段 1: 等待初始延迟
        if (m_keyRepeatTimer > m_keyRepeatDelay) {
            // 阶段 2: 超过延迟后，按间隔计时
            if (m_keyRepeatTimer > m_keyRepeatDelay + m_keyRepeatInterval) {
                // 减去一个间隔，保持连续性，而不直接清零（防止帧率波动影响频率）
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
// 处理键盘输入：字符输入、快捷键、功能键、光标移动
void TextBox::HandleInput() {
    // 1. 输入字符
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32) {
            DeleteSelection();
            InsertCodepoint(key);
        }
        key = GetCharPressed();
    }

    // 2. 快捷键
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
            if (shift) Redo(); // Ctrl + Shift + Z
            else Undo();       // Ctrl + Z
        }
        else if (IsKeyActionTriggered(KEY_Y)) {
            Redo();            // Ctrl + Y
        }
    }

    // 3. 功能键
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

    // 4. 光标移动
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
        currentPos.y -= m_lineHeight; // 使用固定行高计算
        m_cursorIndex = GetIndexFromPoint(currentPos);
        if (!shift) m_selectionAnchor = m_cursorIndex;
        m_blinkTimer = 0;
    }
    else if (IsKeyActionTriggered(KEY_DOWN)) {
        Vector2 currentPos = GetPositionFromIndex(m_cursorIndex);
        currentPos.y += m_lineHeight; // 使用固定行高计算
        m_cursorIndex = GetIndexFromPoint(currentPos);
        if (!shift) m_selectionAnchor = m_cursorIndex;
        m_blinkTimer = 0;
    }
}
// 处理鼠标交互：点击获取焦点、拖拽选择、设置光标位置
void TextBox::HandleMouse() {
    Vector2 mousePos = GetMousePosition();
    Rectangle textArea = GetTextArea();

    bool hover = CheckCollisionPointRec(mousePos, m_bounds);
    bool hoverWhole = CheckCollisionPointRec(mousePos, m_bounds);

    if (hover) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM); // 悬停时设为文本输入光标
        m_wasHovered = true;                   // 记录：当前是由我(TextBox)掌控光标
    }
    else if (m_wasHovered) {
        // 只有当鼠标“刚刚离开”输入框的那一帧，才由我负责把它还原
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        m_wasHovered = false;                  // 记录：我已经交出光标控制权
    }
    // ==========================================

    // 处理点击
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (hover && !m_isDraggingScroll) {
            m_isFocused = true;
            m_isDragging = true;

            // 点击时，为了让跟随逻辑生效，我们可以手动重置一下 lastCursor
            m_lastCursorIndex = -1;
            m_cursorIndex = GetIndexFromPoint(mousePos);
            m_selectionAnchor = m_cursorIndex;
        }
        else {
            // 点击外部 -> 失去焦点
            m_isFocused = false;
        }
    }

    // 处理拖拽选择
    if (m_isDragging && !m_isDraggingScroll) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            m_isDragging = false;
        }
        else {
            // 持续更新光标位置
            m_cursorIndex = GetIndexFromPoint(mousePos);
        }
    }
}
// 插入一个 Unicode 码点（会转换为 UTF-8 字符串）
void TextBox::InsertCodepoint(int codepoint) {
    InsertText(TextUtils::CodepointToUTF8(codepoint));
}
// 插入一个字符串
void TextBox::InsertText(const std::string& str) {
    SaveUndoState();
    m_text.insert(m_cursorIndex, str);
    m_cursorIndex += (int)str.length();
    m_selectionAnchor = m_cursorIndex;
}
// 删除光标前一个字符
void TextBox::Backspace() {
    if (m_cursorIndex > 0) {
        SaveUndoState();
        int prev = GetPrevCharIndex(m_cursorIndex);
        m_text.erase(prev, m_cursorIndex - prev);
        m_cursorIndex = prev;
        m_selectionAnchor = m_cursorIndex;
    }
}
// 删除光标后一个字符
void TextBox::Delete() {
    if (m_cursorIndex < (int)m_text.length()) {
        SaveUndoState();
        int next = GetNextCharIndex(m_cursorIndex);
        m_text.erase(m_cursorIndex, next - m_cursorIndex);
    }
}
// 删除选中的文本
void TextBox::DeleteSelection() {
    if (m_cursorIndex == m_selectionAnchor) return;
    SaveUndoState();
    int start = std::min(m_cursorIndex, m_selectionAnchor);
    int end = std::max(m_cursorIndex, m_selectionAnchor);
    m_text.erase(start, end - start);
    m_cursorIndex = start;
    m_selectionAnchor = start;
}
// 自动缩进：在按下回车后，复制上一行的缩进（空格和制表符）
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
    // 如果当前状态和栈顶状态一致，则不保存（防止重复）
    if (!m_undoStack.empty() && m_undoStack.back().text == m_text) {
        return;
    }

    m_undoStack.push_back({ m_text, m_cursorIndex, m_selectionAnchor });

    // 限制历史记录大小
    if (m_undoStack.size() > MAX_HISTORY) {
        m_undoStack.pop_front();
    }

    // 每次有新操作时，清空重做栈
    m_redoStack.clear();
}
void TextBox::Undo() {
    if (m_undoStack.empty()) return;

    // 将当前状态存入重做栈
    m_redoStack.push_back({ m_text, m_cursorIndex, m_selectionAnchor });

    // 恢复到上一个状态
    TextState last = m_undoStack.back();
    m_undoStack.pop_back();

    m_text = last.text;
    m_cursorIndex = last.cursorIndex;
    m_selectionAnchor = last.selectionAnchor;

    m_lastCursorIndex = -1; // 强制更新滚动位置
}
void TextBox::Redo() {
    if (m_redoStack.empty()) return;

    // 将当前状态存入撤销栈
    m_undoStack.push_back({ m_text, m_cursorIndex, m_selectionAnchor });

    // 恢复到重做状态
    TextState next = m_redoStack.back();
    m_redoStack.pop_back();

    m_text = next.text;
    m_cursorIndex = next.cursorIndex;
    m_selectionAnchor = next.selectionAnchor;

    m_lastCursorIndex = -1;
}

// 复制、粘贴、剪切
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
// 将屏幕点击坐标转换为文本索引
int TextBox::GetIndexFromPoint(Vector2 point) {
    Rectangle textArea = GetTextArea(); // 获取动态调整后的区域
    point.x -= textArea.x + m_scrollOffset.x;
    point.y -= textArea.y + m_scrollOffset.y;

    // 1. 确定行号
    int line = (int)(point.y / m_lineHeight);
    if (line < 0) line = 0;

    int currentLine = 0;
    int idx = 0;
    int lastLineStart = 0;

    // 2. 找到该行的起始索引 (lastLineStart) 和结束索引 (lineEnd)
    while (idx < (int)m_text.length()) {
        if (m_text[idx] == '\n') {
            if (currentLine == line) break; // 找到了
            currentLine++;
            idx++;
            lastLineStart = idx;
        }
        else {
            idx++;
        }
    }

    // 如果点击的行超过了实际行数，返回文本末尾
    if (currentLine < line) return (int)m_text.length();

    int lineEnd = idx; // 指向换行符或文本末尾

    // 3. 在行内寻找具体的字符位置 (更精准的测量方式)
    for (int i = lastLineStart; i < lineEnd; ) {
        int nextI = GetNextCharIndex(i);

        // 截取从行首到当前字符的子串
        std::string subStr = m_text.substr(lastLineStart, nextI - lastLineStart);

        // 测量整个子串的宽度 (比累加单个字符更准)
        float widthToEnd = MeasureTextEx(m_font, subStr.c_str(), m_fontSize, m_spacing).x;

        // 获取当前字符的单独宽度用于计算中心点
        std::string charStr = m_text.substr(i, nextI - i);
        float charWidth = MeasureTextEx(m_font, charStr.c_str(), m_fontSize, m_spacing).x;

        // 计算字符左边缘 X 坐标
        float charLeftX = widthToEnd - charWidth;

        // 如果点击位置在字符的左半边，光标在字符前；否则在字符后
        if (point.x < charLeftX + charWidth / 2) {
            return i;
        }

        i = nextI;
    }

    return lineEnd;
}
// 将文本索引转换为屏幕坐标
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
// 更新 IME 输入法窗口位置，使其跟随光标
void TextBox::UpdateIMEPosition() {
    if (m_isFocused) {
        Vector2 localPos = GetPositionFromIndex(m_cursorIndex);
        // IME 窗口显示在当前行下方
        Platform::SetImeLocation((int)localPos.x, (int)(localPos.y + m_lineHeight));
    }
}
// 计算内容高度、判断是否需要滚动条，并限制滚动偏移范围
void TextBox::UpdateScrollbarState() {
    // 1. 计算内容高度
    int lineCount = 1;
    for (char c : m_text) if (c == '\n') lineCount++;
    m_contentHeight = lineCount * m_lineHeight;

    // 2. 计算可滚动的最大高度 (内容高度 - 显示区域高度)
    // 加上一点 Padding (如 10.0f) 可以让最后一行不紧贴底边
    float maxScroll = m_contentHeight - m_bounds.height;
    if (maxScroll < 0) maxScroll = 0;

    // 3. 判断是否需要显示滚动条
    m_showScrollbar = m_contentHeight > m_bounds.height;

    // 4. 核心：强制限制滚动范围
    // 顶部是 0，底部是 -maxScroll
    if (m_scrollOffset.y > 0) m_scrollOffset.y = 0;
    if (m_scrollOffset.y < -maxScroll) m_scrollOffset.y = -maxScroll;
}
// 处理鼠标滚轮和滚动条交互
void TextBox::HandleScroll() {
    UpdateScrollbarState();

    // 1. 处理鼠标滚轮
    Vector2 mousePos = GetMousePosition();
    Rectangle textArea = m_bounds;
    float wheel = GetMouseWheelMove();
    if (m_showScrollbar) {
        textArea.width -= m_scrollbarWidth; // 减去右侧滚动条宽度
    }

    if (wheel != 0 && CheckCollisionPointRec(mousePos, textArea)) {
        float maxScroll = m_contentHeight - m_bounds.height;
        if (maxScroll < 0) maxScroll = 0;

        // 只有在没到顶或者没到底的时候才允许修改 offset
        m_scrollOffset.y += wheel * m_lineHeight * 3;

        // 再次即时修正，确保滚动丝滑且不越界
        if (m_scrollOffset.y > 0) m_scrollOffset.y = 0;
        if (m_scrollOffset.y < -maxScroll) m_scrollOffset.y = -maxScroll;
    }

    // 2. 滚动条点击与拖拽
    if (m_showScrollbar) {
        // 滚动条在文本框内部右侧
        Rectangle scrollTrack = {
            m_bounds.x + m_bounds.width - m_scrollbarWidth - 4, // 向内偏移4像素
            m_bounds.y + 4, // 向下偏移4像素
            m_scrollbarWidth - 2, // 稍微缩小宽度
            m_bounds.height - 8 // 稍微缩小高度
        };

        // 计算滑块（Thumb）高度和位置
        float thumbHeight = (scrollTrack.height / m_contentHeight) * scrollTrack.height;
        if (thumbHeight < 20) thumbHeight = 20; // 最小高度

        float scrollPercent = -m_scrollOffset.y / (m_contentHeight - m_bounds.height);
        float thumbY = scrollTrack.y + (scrollTrack.height - thumbHeight) * scrollPercent;
        Rectangle thumbRect = { scrollTrack.x + 1, thumbY, scrollTrack.width - 2, thumbHeight };

        Vector2 mousePos = GetMousePosition();

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
// 获取前一个字符的索引（考虑多字节 UTF-8 字符）
int TextBox::GetPrevCharIndex(int index) const {
    if (index <= 0) return 0;
    int i = index - 1;
    while (i > 0 && !TextUtils::IsCharStart((unsigned char)m_text[i])) i--;
    return i;
}
// 获取下一个字符的索引（考虑多字节 UTF-8 字符）
int TextBox::GetNextCharIndex(int index) const {
    if (index >= (int)m_text.length()) return (int)m_text.length();
    int i = index + 1;
    while (i < (int)m_text.length() && !TextUtils::IsCharStart((unsigned char)m_text[i])) i++;
    return i;
}
// 获取文本绘制区域（考虑滚动条和内边距）
Rectangle TextBox::GetTextArea() const {
    Rectangle textArea = m_bounds;
    // 如果显示滚动条，则内部可用宽度减少
    if (m_showScrollbar) {
        textArea.width -= m_scrollbarWidth;
    }
    // 可以在这里预留内边距 (Padding)
    float padding = 4.0f;
    textArea.x += padding;
    textArea.y += padding;
    textArea.width -= padding * 2;
    textArea.height -= padding * 2;

    return textArea;
}

// Draw
void TextBox::Draw(Color bgcolor, Color textcolor) {
    Rectangle textArea = GetTextArea();
    // 1. 绘制背景
    DrawRectangleRec(m_bounds, bgcolor);
    DrawRectangleLinesEx(m_bounds, 1, IsFocused() ? BLUE : DARKGRAY);

    // 2. 文字区域裁剪 (预留出滚动条宽度)
    float textAreaWidth = m_showScrollbar ? m_bounds.width - m_scrollbarWidth : m_bounds.width;
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
            if (width < 2.0f && lineEnd < (int)m_text.length() && m_text[lineEnd] == '\n') width = 8.0f; // 选中换行符

            // 只有在屏幕范围内的选区才画
            if (posStart.y + m_lineHeight > m_bounds.y && posStart.y < m_bounds.y + m_bounds.height) {
                DrawRectangleRec({ posStart.x, posStart.y, width, m_lineHeight }, { 0, 120, 255, 100 });
            }
            p = lineEnd;
            if (p < end && m_text[p] == '\n') p++;
        }
    }

    // --- 绘制文字 ---
    int start = 0;
    int lineCount = 0;
    for (int i = 0; i <= (int)m_text.length(); i++) {
        if (i == (int)m_text.length() || m_text[i] == '\n') {
            std::string lineStr = m_text.substr(start, i - start);
            Vector2 drawPos = {
                textArea.x + m_scrollOffset.x,
                textArea.y + m_scrollOffset.y + lineCount * m_lineHeight
            };
            // 性能优化检查
            if (drawPos.y + m_lineHeight > textArea.y && drawPos.y < textArea.y + textArea.height) {
                DrawTextEx(m_font, lineStr.c_str(), drawPos, m_fontSize, m_spacing, textcolor);
            }
            lineCount++;
            start = i + 1;
        }
    }

    // --- 绘制光标 ---
    if (m_isFocused && ((int)(m_blinkTimer * 2) % 2 == 0)) {
        Vector2 cPos = GetPositionFromIndex(m_cursorIndex);
        DrawLineEx({ cPos.x, cPos.y }, { cPos.x, cPos.y + m_lineHeight }, 2, RED);
    }

    EndScissorMode();

    // 3. 绘制滚动条
    if (m_showScrollbar) {
        // 滚动条在文本框内部右侧
        Rectangle scrollTrack = {
            m_bounds.x + m_bounds.width - m_scrollbarWidth - 4, // 向内偏移4像素
            m_bounds.y + 4, // 向下偏移4像素
            m_scrollbarWidth - 2, // 稍微缩小宽度
            m_bounds.height - 8 // 稍微缩小高度
        };
        DrawRectangleRec(scrollTrack, { 45, 45, 45, 255 }); // 轨道颜色

        float thumbHeight = (scrollTrack.height / m_contentHeight) * scrollTrack.height;
        if (thumbHeight < 20) thumbHeight = 20;
        float scrollPercent = -m_scrollOffset.y / (m_contentHeight - m_bounds.height);
        float thumbY = scrollTrack.y + (scrollTrack.height - thumbHeight) * scrollPercent;

        Rectangle thumbRect = { scrollTrack.x + 1, thumbY, scrollTrack.width - 2, thumbHeight };
        DrawRectangleRounded(thumbRect, 0.5f, 4, m_isDraggingScroll ? DARKGRAY : GRAY);
    }
}

void TextBox::Draw(Color bgcolor, Color textcolor, std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor) {
    Rectangle textArea = GetTextArea();
    // 1. 绘制背景
    drawRect(m_bounds, bgcolor);
    
    // 绘制边框
    Vector2 topLeft = { m_bounds.x, m_bounds.y };
    Vector2 topRight = { m_bounds.x + m_bounds.width, m_bounds.y };
    Vector2 bottomLeft = { m_bounds.x, m_bounds.y + m_bounds.height };
    Vector2 bottomRight = { m_bounds.x + m_bounds.width, m_bounds.y + m_bounds.height };
    Color borderColor = IsFocused() ? BLUE : DARKGRAY;
    drawLine(topLeft, topRight, 1.0f, borderColor);
    drawLine(topRight, bottomRight, 1.0f, borderColor);
    drawLine(bottomRight, bottomLeft, 1.0f, borderColor);
    drawLine(bottomLeft, topLeft, 1.0f, borderColor);

    // 2. 文字区域裁剪 (预留出滚动条宽度)
    float textAreaWidth = m_showScrollbar ? m_bounds.width - m_scrollbarWidth : m_bounds.width;
    beginScissor({ textArea.x, textArea.y, textArea.width, textArea.height });

    if (m_cursorIndex != m_selectionAnchor) {
        int start = std::min(m_cursorIndex, m_selectionAnchor);
        int end = std::max(m_cursorIndex, m_selectionAnchor);
        int p = start;
        while (p < end) {
            size_t nextNL = m_text.find('\n', p);
            int lineEnd = (nextNL == std::string::npos || nextNL >= (size_t)end) ? end : (int)nextNL;

            Vector2 posStart = GetPositionFromIndex(p);
            float width = MeasureTextEx(m_font, m_text.substr(p, lineEnd - p).c_str(), m_fontSize, m_spacing).x;
            if (width < 2.0f && lineEnd < (int)m_text.length() && m_text[lineEnd] == '\n') width = 8.0f; // 选中换行符

            // 只有在屏幕范围内的选区才画
            if (posStart.y + m_lineHeight > m_bounds.y && posStart.y < m_bounds.y + m_bounds.height) {
                drawRect({ posStart.x, posStart.y, width, m_lineHeight }, { 0, 120, 255, 100 });
            }
            p = lineEnd;
            if (p < end && m_text[p] == '\n') p++;
        }
    }

    // --- 绘制文字 ---
    int start = 0;
    int lineCount = 0;
    for (int i = 0; i <= (int)m_text.length(); i++) {
        if (i == (int)m_text.length() || m_text[i] == '\n') {
            std::string lineStr = m_text.substr(start, i - start);
            Vector2 drawPos = {
                textArea.x + m_scrollOffset.x,
                textArea.y + m_scrollOffset.y + lineCount * m_lineHeight
            };
            // 性能优化检查
            if (drawPos.y + m_lineHeight > textArea.y && drawPos.y < textArea.y + textArea.height) {
                drawText(lineStr.c_str(), drawPos, m_fontSize, m_spacing, textcolor);
            }
            lineCount++;
            start = i + 1;
        }
    }

    // --- 绘制光标 ---
    if (m_isFocused && ((int)(m_blinkTimer * 2) % 2 == 0)) {
        Vector2 cPos = GetPositionFromIndex(m_cursorIndex);
        drawLine({ cPos.x, cPos.y }, { cPos.x, cPos.y + m_lineHeight }, 2, RED);
    }

    endScissor();

    // 3. 绘制滚动条
    if (m_showScrollbar) {
        // 滚动条在文本框内部右侧
        Rectangle scrollTrack = {
            m_bounds.x + m_bounds.width - m_scrollbarWidth - 4, // 向内偏移4像素
            m_bounds.y + 4, // 向下偏移4像素
            m_scrollbarWidth - 2, // 稍微缩小宽度
            m_bounds.height - 8 // 稍微缩小高度
        };
        drawRect(scrollTrack, { 45, 45, 45, 255 }); // 轨道颜色

        float thumbHeight = (scrollTrack.height / m_contentHeight) * scrollTrack.height;
        if (thumbHeight < 20) thumbHeight = 20;
        float scrollPercent = -m_scrollOffset.y / (m_contentHeight - m_bounds.height);
        float thumbY = scrollTrack.y + (scrollTrack.height - thumbHeight) * scrollPercent;

        Rectangle thumbRect = { scrollTrack.x + 1, thumbY, scrollTrack.width - 2, thumbHeight };
        // 绘制圆角矩形作为滚动条滑块
        drawRect(thumbRect, m_isDraggingScroll ? DARKGRAY : GRAY);
    }
}
// --- End of TextBox Implementation ---