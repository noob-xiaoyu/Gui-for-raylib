# Multiselect (多选框)

用于从列表中选择多个项的交互控件。

## 特性
- **多状态维护**：内部维护布尔数组记录每一项的选中状态。
- **继承架构**：共享 `DropdownControl` 的布局和滚动逻辑。
- **实时同步**：选项变更时立即触发包含所有选中索引的回调。

## 常用接口
- `Multiselect(Rectangle bounds, std::vector<std::string> items, ...)`: 构造函数。
- `GetSelectedIndices()`: 获取当前选中的所有索引列表。
