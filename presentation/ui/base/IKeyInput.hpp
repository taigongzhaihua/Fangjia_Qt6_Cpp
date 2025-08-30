/*
 * 文件名：IKeyInput.hpp
 * 职责：键盘输入处理接口，定义按键事件响应协议。
 * 依赖：Qt键盘事件类型。
 * 线程：仅在UI线程使用。
 * 备注：配合IFocusable使用，仅有焦点的组件响应键盘输入。
 */

#pragma once
#include <qevent.h>

/// 键盘输入处理接口
/// 
/// 功能：
/// - 按键按下处理：响应键盘按键按下事件
/// - 按键释放处理：响应键盘按键释放事件
/// - 事件消费控制：决定是否消费键盘事件
/// 
/// 使用场景：
/// - 按钮：Space/Enter键激活
/// - 文本输入：字符输入和编辑快捷键
/// - 列表导航：方向键选择项目
/// - 快捷键：Ctrl+C/V等组合键
/// 
/// 注意事项：
/// - 仅有焦点的组件应该响应键盘输入
/// - 消费的事件不会传递给其他组件
/// - 未消费的事件会继续向上传播
class IKeyInput {
public:
	virtual ~IKeyInput() = default;

	/// 功能：处理键盘按键按下事件
	/// 参数：key — 按下的键码（Qt::Key枚举值）
	/// 参数：modifiers — 修饰键状态（Ctrl、Shift、Alt等）
	/// 返回：true表示消费了该事件，false表示不处理让其传播
	/// 说明：在键盘按键按下时调用，用于触发按键响应动作
	virtual bool onKeyPress(int key, Qt::KeyboardModifiers modifiers) = 0;

	/// 功能：处理键盘按键释放事件
	/// 参数：key — 释放的键码（Qt::Key枚举值）
	/// 参数：modifiers — 修饰键状态（Ctrl、Shift、Alt等）
	/// 返回：true表示消费了该事件，false表示不处理让其传播
	/// 说明：在键盘按键释放时调用，通常用于完成按键激活动作
	virtual bool onKeyRelease(int key, Qt::KeyboardModifiers modifiers) = 0;
};