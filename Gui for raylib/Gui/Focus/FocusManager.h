#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

class GuiControl;

enum class ControlState {
    NORMAL,
    HOVER,
    PRESSED,
    DISABLED
};

struct PaintContext {
    Rectangle bounds;
    ControlState state;
    bool isFocused;
};

class IGuiSkin;

using FocusChangeCallback = std::function<void(GuiControl* lost, GuiControl* gained)>;
using HoverChangeCallback = std::function<void(GuiControl* lost, GuiControl* gained)>;
using ActiveChangeCallback = std::function<void(GuiControl* lost, GuiControl* gained)>;

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

    void SetOnFocusChange(FocusChangeCallback callback);
    void SetOnHoverChange(HoverChangeCallback callback);
    void SetOnActiveChange(ActiveChangeCallback callback);

    bool IsDragging() const;
    GuiControl* GetDraggingControl() const;
    void StartDrag(GuiControl* control);
    void EndDrag();
    bool IsDraggingControl(const GuiControl* control) const;

    void SetActiveControl(GuiControl* control);
    void ClearActiveControl();
    GuiControl* GetActiveControl() const;
    bool HasActiveControl() const;
    bool IsActiveControl(const GuiControl* control) const;

    GuiControl* GetCapturedControl() const { return m_capturedControl; }
    void SetCapturedControl(GuiControl* control) { m_capturedControl = control; }

    void CloseAllActiveControls();

private:
    FocusManager();
    ~FocusManager() = default;
    FocusManager(const FocusManager&) = delete;
    FocusManager& operator=(const FocusManager&) = delete;

    void NotifyFocusChange(GuiControl* lost, GuiControl* gained);
    void NotifyHoverChange(GuiControl* lost, GuiControl* gained);
    void NotifyActiveChange(GuiControl* lost, GuiControl* gained);
    GuiControl* FindControlAtMouse(Vector2 mousePos);

    std::vector<GuiControl*> m_controls;
    std::unordered_map<int, GuiControl*> m_controlMap;
    GuiControl* m_focusedControl = nullptr;
    GuiControl* m_hoveredControl = nullptr;
    GuiControl* m_draggingControl = nullptr;
    GuiControl* m_activeControl = nullptr;
    GuiControl* m_capturedControl = nullptr;
    Vector2 m_lastMousePos = { 0, 0 };
    bool m_mouseMoved = false;

    FocusChangeCallback m_onFocusChange;
    HoverChangeCallback m_onHoverChange;
    ActiveChangeCallback m_onActiveChange;
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
    bool IsEnabled() const { return m_isEnabled; }
    void SetEnabled(bool enabled) { m_isEnabled = enabled; }
    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

    void SetSkin(IGuiSkin* skin) { m_skin = skin; }
    IGuiSkin* GetSkin() const { return m_skin; }

    ControlState GetState() const {
        if (!m_isEnabled) return ControlState::DISABLED;
        if (m_isHovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return ControlState::PRESSED;
        if (m_isHovered) return ControlState::HOVER;
        return ControlState::NORMAL;
    }

protected:
    bool m_wantsFocus = false;
    bool m_isFocused = false;
    bool m_isHovered = false;
    bool m_isEnabled = true;
    bool m_isVisible = true;
    std::string m_tag;
    IGuiSkin* m_skin = nullptr;
};