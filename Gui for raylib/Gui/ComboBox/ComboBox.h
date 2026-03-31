#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>
#include "../Common/DropdownControl.h"

class ComboBox : public DropdownControl {
private:
    int m_selectedIndex;
    std::function<void(int)> m_onSelectionChange;

    enum ComboBoxState {
        COMBOBOX_STATE_NORMAL,
        COMBOBOX_STATE_HOVER,
        COMBOBOX_STATE_PRESSED
    } m_state;

public:
    ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex = -1, std::function<void(int)> onSelectionChange = nullptr);

    void Update() override;
    void Draw() override;

    void ClearItems() override;

    int GetSelectedIndex() const;
    std::string GetSelectedItem() const;
    void SetSelectedIndex(int index);

    void SetOnSelectionChange(std::function<void(int)> callback);
};