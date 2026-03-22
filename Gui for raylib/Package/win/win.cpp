#include "win.h"
#include <windows.h>
#include "../raylib/ray.h"
#include <stdio.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")

namespace win
{
    //用于使当前进程对 DPI（每英寸点数）感知，从而在高分辨率显示器上正确缩放其用户界面。
    void SetProcessDPIAware_()
    {
        SetProcessDPIAware();
    }

    // 获取当前工作区的宽度和高度（不包括任务栏等系统保留区域）
    void GetWorkArea(int* w, int* h)
    {
        RECT rc;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

        *w = rc.right - rc.left;
        *h = rc.bottom - rc.top;
    }

    void InitLayeredWindow(HWND hwnd)
    {
        ::HWND real = (::HWND)hwnd;
        LONG_PTR ex = GetWindowLongPtr(real, GWL_EXSTYLE);
        ex |= WS_EX_LAYERED;
        SetWindowLongPtr(real, GWL_EXSTYLE, ex);

        SetLayeredWindowAttributes(real, 0, 255, LWA_ALPHA);
    }

    // 根据 enable 参数启用或禁用窗口的点击穿透功能
    void SetClickThrough(HWND hwnd, bool enable)
    {
        ::HWND real = (::HWND)hwnd;

        LONG ex = GetWindowLong(real, GWL_EXSTYLE);

        ex |= WS_EX_LAYERED;

        if (enable)
            ex |= WS_EX_TRANSPARENT;
        else
            ex &= ~WS_EX_TRANSPARENT;

        SetWindowLong(real, GWL_EXSTYLE, ex);

        SetLayeredWindowAttributes(real, 0, 255, LWA_ALPHA);
    }

    unsigned int GetWindowsDirectoryA_(char* buffer, unsigned int size)
    {
        return ::GetWindowsDirectoryA(buffer, size);
    }

    static HHOOK g_keyboardHook = nullptr;
    static bool  g_insertDown = false;

    void PumpMessages()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void SetWindowRectLogical(HWND hwnd, int x, int y, int w, int h, float dpi)
    {
        ::HWND real = (::HWND)hwnd;
        int physical_x = (int)(x * dpi);
        int physical_y = (int)(y * dpi);
        int physical_w = (int)(w * dpi);
        int physical_h = (int)(h * dpi);

        static RECT last = { 0 };

        RECT now;
        now.left = physical_x;
        now.top = physical_y;
        now.right = physical_x + physical_w;
        now.bottom = physical_y + physical_h;

        // 如果没有变化就不调用（防止抖动）
        if (memcmp(&last, &now, sizeof(RECT)) == 0)
            return;

        SetWindowPos(
            real,
            NULL,
            physical_x,
            physical_y,
            physical_w,
            physical_h,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        last = now;
    }

    Vec2 GetScreenMousePos()
    {
        POINT p;
        if (::GetCursorPos(&p)) {
            return { (float)p.x, (float)p.y };
        }
        return { 0.f, 0.f };
    }

    Rect GetWindowRectSafe(HWND hwnd, Rect& out)
    {
        ::HWND real = (::HWND)hwnd;
        RECT r{};
        ::GetWindowRect(real, &r);

        out = { r.left, r.top, r.right, r.bottom };
        return out;
    }

    ImeContext::ImeContext(void* rawWindowHandle)
        : m_hwnd(rawWindowHandle), m_himc(nullptr)
    {
#if defined(_WIN32)
        if (m_hwnd) {
            // 对应: m_himc = ImmGetContext(hwnd);
            m_himc = ::ImmGetContext((::HWND)m_hwnd);
        }
#endif
    }

    ImeContext::~ImeContext() {
        if (m_hwnd && m_himc) {
            // 对应: ImmReleaseContext(hwnd, himc);
            ::ImmReleaseContext((::HWND)m_hwnd, static_cast<HIMC>(m_himc));
        }
        m_hwnd = nullptr;
        m_himc = nullptr;
    }

    // 移动构造函数
    ImeContext::ImeContext(ImeContext&& other) noexcept : m_hwnd(other.m_hwnd), m_himc(other.m_himc) {
        other.m_hwnd = nullptr;
        other.m_himc = nullptr;
    }

    // 移动赋值运算符
    ImeContext& ImeContext::operator=(ImeContext&& other) noexcept {
        if (this != &other) {
            // 先释放自己的资源
            this->~ImeContext();

            // 接管对方资源
            m_hwnd = other.m_hwnd;
            m_himc = other.m_himc;

            other.m_hwnd = nullptr;
            other.m_himc = nullptr;
        }
        return *this;
    }

    void ImeContext::SetCompositionWindowPos(int x, int y) {
        if (m_himc) {
            // 包装 COMPOSITIONFORM 结构体
            COMPOSITIONFORM cf = {};

            // 对应: cf.dwStyle = CFS_POINT;
            cf.dwStyle = CFS_POINT;

            // 对应: cf.ptCurrentPos.x / y
            cf.ptCurrentPos.x = x;
            cf.ptCurrentPos.y = y;

            // 对应: ImmSetCompositionWindow(himc, &cf);
            ::ImmSetCompositionWindow(static_cast<HIMC>(m_himc), &cf);
        }
    }

    bool ImeContext::IsValid() const {
        return m_himc != nullptr;
    }
}
