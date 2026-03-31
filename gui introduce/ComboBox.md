# ComboBox (组合框/下拉菜单)

用于从列表中选择单个项的交互控件。

## 特性
- **继承架构**：基于 `DropdownControl` 基类，代码逻辑精简。
- **滚动支持**：当列表项过多时，支持鼠标滚轮滚动。
- **活跃管理**：展开时自动成为 `ActiveControl`，并遮挡下方控件的交互。
- **防重开机制**：修复了点击选择后立即重新触发展开的逻辑缺陷。

## 常用接口
- `ComboBox(Rectangle bounds, std::vector<std::string> items, int selectedIndex, ...)`: 构造函数。
- `AddItem(const std::string& item)`: 动态添加项。
