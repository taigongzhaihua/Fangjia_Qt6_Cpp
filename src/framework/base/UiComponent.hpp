#pragma once
#include "IThemeAware.hpp"
#include "RenderData.hpp"
class IconCache;
class QOpenGLFunctions;

#include <qrect.h>

// 通用 UI 组件接口：继承主题感知接口
class IUiComponent : public IThemeAware {
public:
	~IUiComponent() override = default;

	// 基于窗口逻辑尺寸的布局更新
	virtual void updateLayout(const QSize& windowSize) = 0;

	// 资源上下文（IconCache / GL / DPR）更新
	virtual void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) = 0;

	// 将自身的绘制命令追加到帧数据
	virtual void append(Render::FrameData& fd) const = 0;

	// 输入事件处理：返回是否消耗
	virtual bool onMousePress(const QPoint& pos) = 0;
	virtual bool onMouseMove(const QPoint& pos) = 0;
	virtual bool onMouseRelease(const QPoint& pos) = 0;
	
	// 鼠标滚轮事件：返回是否消耗，默认实现不消耗
	virtual bool onWheel(const QPoint& pos, const QPoint& angleDelta) { return false; }

	// 推进动画：返回是否仍有动画在进行
	virtual bool tick() = 0;

	// 组件边界（逻辑像素）
	virtual QRect bounds() const = 0;

	// 默认实现：调用 applyTheme
	void onThemeChanged(bool isDark) override {
		applyTheme(isDark);
	}
};