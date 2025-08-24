#pragma once

// 可选的内容接口：被 UiPage 用于下发内容区域（viewport）
// 内容组件如果实现了该接口，UiPage 会在布局阶段调用 setViewportRect。
class IUiContent {
public:
	virtual ~IUiContent() = default;
	virtual void setViewportRect(const QRect& r) = 0;
};