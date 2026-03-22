#include "FocusManager.h"
#include "../Button/Button.h"
#include "../CheckBox/CheckBox.h"
#include "../ComboBox/ComboBox.h"
#include "../Multiselect/Multiselect.h"
#include "../TextBox/TextBox.h"
#include "../ColorPicker/ColorPicker.h"

FocusManager& FocusManager::Instance() {
    static FocusManager instance;
    return instance;
}

void FocusManager::RegisterControl(GuiControl* control) {
    if (control) {
        auto it = std::find(m_controls.begin(), m_controls.end(), control);
        if (it == m_controls.end()) {
            m_controls.push_back(control);
        }
    }
}

void FocusManager::UnregisterControl(GuiControl* control) {
    for (auto it = m_controls.begin(); it != m_controls.end(); ) {
        if (*it == control) {
            if (m_focusedControl == control) {
                m_focusedControl = nullptr;
            }
            if (m_hoveredControl == control) {
                m_hoveredControl = nullptr;
            }
            it = m_controls.erase(it);
        } else {
            ++it;
        }
    }
}

void FocusManager::SetFocusedControl(GuiControl* control) {
    if (m_focusedControl != control) {
        if (m_focusedControl) {
            m_focusedControl->m_isFocused = false;
        }
        m_focusedControl = control;
        if (m_focusedControl) {
            m_focusedControl->m_isFocused = true;
        }
    }
}

void FocusManager::ClearFocusedControl() {
    if (m_focusedControl) {
        m_focusedControl->m_isFocused = false;
        m_focusedControl = nullptr;
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

    GuiControl* newHovered = nullptr;
    for (auto it = m_controls.rbegin(); it != m_controls.rend(); ++it) {
        GuiControl* ctrl = *it;
        if (!ctrl->m_isEnabled) continue;
        Rectangle bounds = ctrl->GetBounds();
        if (CheckCollisionPointRec(mousePos, bounds)) {
            newHovered = ctrl;
            break;
        }
    }

    if (m_hoveredControl != newHovered) {
        if (m_hoveredControl) {
            m_hoveredControl->m_isHovered = false;
        }
        m_hoveredControl = newHovered;
        if (m_hoveredControl) {
            m_hoveredControl->m_isHovered = true;
        }
    }
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

bool GuiControl::IsFocused() const {
    return m_isFocused;
}

bool GuiControl::IsHovered() const {
    return m_isHovered;
}