#include "FocusManager.h"
#include <algorithm>

FocusManager& FocusManager::Instance() {
    static FocusManager instance;
    return instance;
}

FocusManager::FocusManager() = default;

void FocusManager::RegisterControl(GuiControl* control) {
    if (control) {
        auto it = std::find(m_controls.begin(), m_controls.end(), control);
        if (it == m_controls.end()) {
            m_controls.push_back(control);
        }
    }
}

void FocusManager::UnregisterControl(GuiControl* control) {
    auto it = std::find(m_controls.begin(), m_controls.end(), control);
    if (it != m_controls.end()) {
        if (m_focusedControl == control) m_focusedControl = nullptr;
        if (m_hoveredControl == control) m_hoveredControl = nullptr;
        if (m_draggingControl == control) m_draggingControl = nullptr;
        if (m_activeControl == control) m_activeControl = nullptr;
        
        m_controls.erase(it);
    }
}

void FocusManager::SetFocusedControl(GuiControl* control) {
    if (m_focusedControl != control) {
        GuiControl* oldFocus = m_focusedControl;
        if (m_focusedControl) {
            m_focusedControl->m_isFocused = false;
        }
        m_focusedControl = control;
        if (m_focusedControl) {
            m_focusedControl->m_isFocused = true;
        }
        NotifyFocusChange(oldFocus, m_focusedControl);
    }
}

void FocusManager::ClearFocusedControl() {
    if (m_focusedControl) {
        GuiControl* oldFocus = m_focusedControl;
        m_focusedControl->m_isFocused = false;
        m_focusedControl = nullptr;
        NotifyFocusChange(oldFocus, nullptr);
    }
}

GuiControl* FocusManager::GetFocusedControl() const {
    return m_focusedControl;
}

bool FocusManager::HasFocus(const GuiControl* control) const {
    return m_focusedControl == control;
}

void FocusManager::Update() {
    Vector2 mousePos = GetMousePosition();

    if (mousePos.x != m_lastMousePos.x || mousePos.y != m_lastMousePos.y) {
        m_mouseMoved = true;
        m_lastMousePos = mousePos;
    }

    // 处理鼠标捕获 (Mouse Capture)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_capturedControl = m_hoveredControl;
    }

    GuiControl* newHovered = nullptr;
    
    if (m_capturedControl) {
        newHovered = m_capturedControl;
    } else if (m_activeControl) {
        // 如果存在活动控件，只有它或其子区域能被悬停
        if (CheckCollisionPointRec(mousePos, m_activeControl->GetBounds())) {
            newHovered = m_activeControl;
        }
    } else {
        newHovered = FindControlAtMouse(mousePos);
    }

    // 更新 Hover 状态
    if (m_hoveredControl != newHovered) {
        GuiControl* oldHover = m_hoveredControl;
        if (m_hoveredControl) {
            m_hoveredControl->m_isHovered = false;
        }
        m_hoveredControl = newHovered;
        if (m_hoveredControl) {
            m_hoveredControl->m_isHovered = true;
        }
        NotifyHoverChange(oldHover, m_hoveredControl);
    }

    // 处理焦点清理
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (m_hoveredControl) {
            if (m_hoveredControl->m_wantsFocus) {
                SetFocusedControl(m_hoveredControl);
            } else {
                // 点击了不支持焦点的控件（如按钮），也应清除之前的焦点
                ClearFocusedControl();
            }
        } else {
            // 点击空白处清理焦点
            ClearFocusedControl();
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        m_capturedControl = nullptr;
    }

    m_mouseMoved = false;
}

GuiControl* FocusManager::FindControlAtMouse(Vector2 mousePos) {
    for (auto it = m_controls.rbegin(); it != m_controls.rend(); ++it) {
        GuiControl* ctrl = *it;
        if (!ctrl->m_isEnabled) continue;
        Rectangle bounds = ctrl->GetBounds();
        if (CheckCollisionPointRec(mousePos, bounds)) {
            return ctrl;
        }
    }
    return nullptr;
}

GuiControl* FocusManager::GetControlAtMouse() const {
    return m_hoveredControl;
}

bool FocusManager::IsMouseOverAnyControl() const {
    return m_hoveredControl != nullptr;
}

bool FocusManager::IsControlEnabled(const GuiControl* control) const {
    return control && control->m_isEnabled;
}

void FocusManager::SetControlEnabled(GuiControl* control, bool enabled) {
    if (control) {
        control->m_isEnabled = enabled;
    }
}

void FocusManager::SetOnFocusChange(FocusChangeCallback callback) {
    m_onFocusChange = callback;
}

void FocusManager::SetOnHoverChange(HoverChangeCallback callback) {
    m_onHoverChange = callback;
}

void FocusManager::SetOnActiveChange(ActiveChangeCallback callback) {
    m_onActiveChange = callback;
}

void FocusManager::NotifyFocusChange(GuiControl* lost, GuiControl* gained) {
    if (m_onFocusChange) {
        m_onFocusChange(lost, gained);
    }
}

void FocusManager::NotifyHoverChange(GuiControl* lost, GuiControl* gained) {
    if (m_onHoverChange) {
        m_onHoverChange(lost, gained);
    }
}

void FocusManager::NotifyActiveChange(GuiControl* lost, GuiControl* gained) {
    if (m_onActiveChange) {
        m_onActiveChange(lost, gained);
    }
}

bool FocusManager::IsDragging() const {
    return m_draggingControl != nullptr;
}

GuiControl* FocusManager::GetDraggingControl() const {
    return m_draggingControl;
}

void FocusManager::StartDrag(GuiControl* control) {
    m_draggingControl = control;
}

void FocusManager::EndDrag() {
    m_draggingControl = nullptr;
}

bool FocusManager::IsDraggingControl(const GuiControl* control) const {
    return m_draggingControl == control;
}

void FocusManager::SetActiveControl(GuiControl* control) {
    if (m_activeControl != control) {
        GuiControl* oldActive = m_activeControl;
        m_activeControl = control;
        NotifyActiveChange(oldActive, m_activeControl);
    }
}

void FocusManager::ClearActiveControl() {
    if (m_activeControl) {
        GuiControl* oldActive = m_activeControl;
        m_activeControl = nullptr;
        NotifyActiveChange(oldActive, nullptr);
    }
}

GuiControl* FocusManager::GetActiveControl() const {
    return m_activeControl;
}

bool FocusManager::HasActiveControl() const {
    return m_activeControl != nullptr;
}

bool FocusManager::IsActiveControl(const GuiControl* control) const {
    return m_activeControl == control;
}

void FocusManager::CloseAllActiveControls() {
    ClearActiveControl();
}

bool GuiControl::IsFocused() const {
    return m_isFocused;
}

bool GuiControl::IsHovered() const {
    return m_isHovered;
}