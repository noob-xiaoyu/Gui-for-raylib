#pragma once
#include <raylib.h>
#include <string>
#include <deque>
#include <functional>

struct TextState {
    std::string text;
    int cursorIndex;
    int selectionAnchor;
};

// 文本框类
class TextBox {
public:
    TextBox(Rectangle bounds);
    ~TextBox();

    void Update();
    void Draw(Color bgcolor, Color textcolor);
    void Draw(Color bgcolor, Color textcolor, std::function<void(Rectangle, Color)> drawRect, std::function<void(Vector2, Vector2, float, Color)> drawLine, std::function<void(const char*, Vector2, float, float, Color)> drawText, std::function<void(Rectangle)> beginScissor, std::function<void()> endScissor);

    // 设置/获取属性
    void SetText(const std::string& text);
    std::string GetText() const;
    void SetFocus(bool focus);
    bool IsFocused() const;

    // 设置字体配置
    // spacing: 仅影响水平字符间距
    // lineHeight: 显式指定行高（若为0，默认等于fontSize）
    void SetFont(Font font, float fontSize, float spacing, float lineHeight = 0.0f);
    void SetSize(float width, float height);
    void SetPosition(float x, float y);
private:
    std::deque<TextState> m_undoStack;
    std::deque<TextState> m_redoStack;
    const size_t MAX_HISTORY = 100; // 最大撤销步数
    void SaveUndoState();
    void Undo();
    void Redo();

    // 内部输入处理
    void HandleInput();
    void HandleMouse();

    // 文本修改操作
    void InsertCodepoint(int codepoint);
    void InsertText(const std::string& str);
    void Backspace();
    void Delete();
    void DeleteSelection();
    void Copy();
    void Paste();
    void Cut();
    void PerformAutoIndent();

    void UpdateScrollbarState(); // 计算内容高度和滚动条可见性
    void HandleScroll();         // 处理滚轮和滚动条交互

    // 坐标与光标计算 (核心修复部分)
    int GetIndexFromPoint(Vector2 point);
    Vector2 GetPositionFromIndex(int index);
    void UpdateIMEPosition();

    // UTF-8 辅助
    int GetPrevCharIndex(int index) const;
    int GetNextCharIndex(int index) const;

    Rectangle GetTextArea() const;
    
    bool IsKeyActionTriggered(int key);
private:
    Rectangle m_bounds;
    std::string m_text;
    bool m_isFocused;

    Font m_font;
    float m_fontSize;
    float m_spacing;     // 仅水平字距
    float m_lineHeight;  // 垂直行高 (关键修复变量)

    // 滚动条相关
    float m_contentHeight;    // 文本内容的总高度
    bool m_showScrollbar;     // 是否需要显示滚动条
    bool m_isDraggingScroll;  // 是否正在拖动滚动条
    float m_scrollbarWidth;   // 滚动条宽度（默认12px）

    int m_cursorIndex;
    int m_selectionAnchor;
    int m_lastCursorIndex;

    double m_blinkTimer;
    bool m_isDragging;
    Vector2 m_scrollOffset;

    int m_currentRepeatingKey;   // 当前正在重复的按键
    float m_keyRepeatTimer;   // 计时器
    float m_keyRepeatDelay;   // 第一次触发前的延迟（秒）
    float m_keyRepeatInterval; // 连续触发的间隔（秒）

    bool m_wasHovered;
};