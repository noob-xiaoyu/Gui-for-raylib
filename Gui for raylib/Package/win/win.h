#pragma once

namespace win
{
    struct HWND__;
    using HWND = HWND__*;

    struct Point
    {
        long x;
        long y;
    };

    struct Rect
    {
        long left;
        long top;
        long right;
        long bottom;

        long Width() const { return right - left; }
        long Height() const { return bottom - top; }
    };

    struct Vec2 {
        float x, y;
    };

    class ImeContext {
    public:
        // 构造函数：获取上下文 (ImmGetContext)
        // rawWindowHandle: 来自 Raylib GetWindowHandle() 的指针
        explicit ImeContext(void* rawWindowHandle);

        // 析构函数：释放上下文 (ImmReleaseContext)
        ~ImeContext();

        // 禁用拷贝，允许移动 (Move Semantic)
        ImeContext(const ImeContext&) = delete;
        ImeContext& operator=(const ImeContext&) = delete;
        ImeContext(ImeContext&&) noexcept;
        ImeContext& operator=(ImeContext&&) noexcept;

        // 核心功能：设置候选框位置 (包装 COMPOSITIONFORM, CFS_POINT, ImmSetCompositionWindow)
        void SetCompositionWindowPos(int x, int y);

        // 检查上下文是否有效
        bool IsValid() const;

    private:
        void* m_hwnd; // 存储 HWND
        void* m_himc; // 存储 HIMC
    };

    Vec2 GetScreenMousePos();

    Rect GetWindowRectSafe(HWND hwnd, Rect& out);

    void SetProcessDPIAware_();
    void GetWorkArea(int* w, int* h);
    void InitLayeredWindow(HWND hwnd);
    void SetClickThrough(HWND hwnd, bool enable);
    unsigned int GetWindowsDirectoryA_(char* buffer, unsigned int size);

    void InstallKeyboardHook();
    void RemoveKeyboardHook();
    void PumpMessages();
    void SetWindowRectLogical(HWND hwnd, int x, int y, int w, int h, float dpi);
}