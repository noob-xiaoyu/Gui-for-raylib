#pragma once

#include <raylib.h>
#include <vector>
#include <functional>

class GuiControl;

class FocusManager {
public:
    static FocusManager& Instance();

    void RegisterControl(GuiControl* control);
    void UnregisterControl(GuiControl* control);

    void SetFocusedControl(GuiControl* control);
    void ClearFocusedControl();
    GuiControl* GetFocusedControl() const;
    bool HasFocus(const GuiControl* control) const;

    void Update();
    GuiControl* GetControlAtMouse() const;
    bool IsMouseOverAnyControl() const;
    bool IsControlEnabled(const GuiControl* control) const;
    void SetControlEnabled(GuiControl* control, bool enabled);

private:
    FocusManager() = default;
    ~FocusManager() = default;
    FocusManager(const FocusManager&) = delete;
    FocusManager& operator=(const FocusManager&) = delete;

    std::vector<GuiControl*> m_controls;
    GuiControl* m_focusedControl = nullptr;
    GuiControl* m_hoveredControl = nullptr;
};

class GuiControl {
    friend class FocusManager;
public:
    virtual ~GuiControl() = default;

    virtual Rectangle GetBounds() const = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;

    bool IsFocused() const;
    bool IsHovered() const;

protected:
    bool m_wantsFocus = false;
    bool m_isFocused = false;
    bool m_isHovered = false;
    bool m_isEnabled = true;
};