#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"

#include <qopenglfunctions.h>
#include <qrect.h>
#include <vector>

// 组件根：统一驱动布局/资源上下文、事件分发、绘制收集、动画推进
class UiRoot
{
public:
	void add(IUiComponent* c);
	void remove(IUiComponent* c);
	void clear();

	void updateLayout(const QSize& windowSize) const;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) const;

	void append(Render::FrameData& fd) const;

	bool onMousePress(const QPoint& pos);
	bool onMouseMove(const QPoint& pos);
	bool onMouseRelease(const QPoint& pos);

	[[nodiscard]] bool tick() const;

	// 简单聚合边界（union）
	[[nodiscard]] QRect boundsUnion() const;

private:
	std::vector<IUiComponent*> m_children; // 不拥有生命周期

	// 指针捕获：按下命中某组件后，直到释放前，事件都路由给该组件
	IUiComponent* m_pointerCapture{ nullptr };
};