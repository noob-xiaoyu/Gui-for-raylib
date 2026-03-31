#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>
#include "../Common/DropdownControl.h"

class Multiselect : public DropdownControl {
private:
    std::vector<bool> m_selectedItems;
    std::function<void(const std::vector<int>&)> m_onSelectionChange;

    enum MultiselectState {
        MULTISELECT_STATE_NORMAL,
        MULTISELECT_STATE_HOVER,
        MULTISELECT_STATE_PRESSED
    } m_state;

public:
    Multiselect(Rectangle bounds, std::vector<std::string> items, std::function<void(const std::vector<int>&)> onSelectionChange = nullptr);

    void Update() override;
    void Draw() override;

    void AddItem(const std::string& item) override;
    void RemoveItem(int index) override;
    void ClearItems() override;

    std::vector<int> GetSelectedIndices() const;
    std::vector<std::string> GetSelectedItems() const;
    void SetSelectedIndices(const std::vector<int>& indices);
    void SetSelectedItems(const std::vector<std::string>& items);

    void SetOnSelectionChange(std::function<void(const std::vector<int>&)> callback);
};