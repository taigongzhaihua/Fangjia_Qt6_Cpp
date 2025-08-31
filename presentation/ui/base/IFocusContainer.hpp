/*
 * 文件名：IFocusContainer.hpp
 * 职责：焦点容器接口，用于枚举可获得焦点的子组件。
 * 依赖：IFocusable接口、标准库容器。
 * 线程：仅在UI线程使用。
 * 备注：支持Tab键焦点遍历和键盘导航。
 */

#pragma once
#include <vector>

// 前向声明
class IFocusable;

/// 焦点容器接口
/// 
/// 功能：
/// - 枚举可焦点组件：收集容器内所有可获得焦点的子组件
/// - 支持深度遍历：递归遍历嵌套容器中的可焦点组件
/// - 遍历顺序控制：按容器定义的顺序返回可焦点组件
/// 
/// 使用场景：
/// - Tab键导航：UiRoot通过此接口构建焦点遍历列表
/// - 焦点搜索：查找下一个/上一个可获得焦点的组件
/// - 可访问性：为屏幕阅读器提供焦点遍历信息
/// 
/// 实现要求：
/// - 按照期望的Tab顺序将可焦点组件添加到输出向量
/// - 递归调用子容器的enumerateFocusables方法
/// - 只添加canFocus()返回true的组件
/// - 保持遍历顺序的一致性和可预测性
class IFocusContainer {
public:
	virtual ~IFocusContainer() = default;

	/// 功能：枚举容器内的可焦点组件
	/// 参数：out — 输出向量，用于收集可焦点组件（按遍历顺序）
	/// 说明：实现应按Tab键导航的期望顺序将组件添加到out中
	/// 注意：应递归处理子容器，确保深度遍历所有可焦点组件
	virtual void enumerateFocusables(std::vector<IFocusable*>& out) const = 0;
};