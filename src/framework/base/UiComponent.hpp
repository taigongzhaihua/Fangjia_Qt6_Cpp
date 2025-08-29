/*
 * 文件名：UiComponent.hpp
 * 职责：UI组件通用接口定义，规范组件生命周期、事件处理和渲染协议。
 * 依赖：主题感知接口、渲染数据结构。
 * 线程：仅在UI线程使用。
 * 备注：所有自绘UI组件的基础接口，定义了标准的更新-渲染-交互流程。
 */

#pragma once
#include "IThemeAware.hpp"
#include "RenderData.hpp"
class IconCache;
class QOpenGLFunctions;

#include <qrect.h>

/// UI组件通用接口：定义自绘组件的标准生命周期和交互协议
/// 
/// 生命周期顺序：
/// 1. updateLayout() — 基于窗口尺寸计算布局
/// 2. updateResourceContext() — 更新渲染资源上下文  
/// 3. append() — 生成绘制命令
/// 4. tick() — 推进动画状态
/// 
/// 事件处理：
/// - 鼠标事件：按下、移动、释放（支持指针捕获）
/// - 滚轮事件：位置相关的滚动处理
/// - 主题事件：亮色/暗色模式切换
class IUiComponent : public IThemeAware {
public:
	~IUiComponent() override = default;

	/// 功能：基于窗口逻辑尺寸更新组件布局
	/// 参数：windowSize — 窗口逻辑像素尺寸
	/// 说明：在每次窗口尺寸变化时调用，组件应重新计算自身位置和大小
	virtual void updateLayout(const QSize& windowSize) = 0;

	/// 功能：更新渲染资源上下文
	/// 参数：cache — 图标纹理缓存
	/// 参数：gl — OpenGL函数表
	/// 参数：devicePixelRatio — DPR（设备像素比）
	/// 说明：在绘制前调用，确保纹理等资源就绪
	virtual void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) = 0;

	/// 功能：生成绘制命令并追加到帧数据
	/// 参数：fd — 帧数据容器，用于收集绘制命令
	/// 说明：组件应将自身的绘制指令添加到fd中
	virtual void append(Render::FrameData& fd) const = 0;

	/// 功能：处理鼠标按下事件
	/// 参数：pos — 鼠标位置（逻辑像素坐标）
	/// 返回：是否消耗了该事件
	virtual bool onMousePress(const QPoint& pos) = 0;
	
	/// 功能：处理鼠标移动事件
	/// 参数：pos — 鼠标位置（逻辑像素坐标）
	/// 返回：是否消耗了该事件
	virtual bool onMouseMove(const QPoint& pos) = 0;
	
	/// 功能：处理鼠标释放事件
	/// 参数：pos — 鼠标位置（逻辑像素坐标）
	/// 返回：是否消耗了该事件
	virtual bool onMouseRelease(const QPoint& pos) = 0;
	
	/// 功能：处理鼠标滚轮事件
	/// 参数：pos — 鼠标位置（逻辑像素坐标）
	/// 参数：angleDelta — 滚轮角度增量
	/// 返回：是否消耗了该事件
	/// 说明：默认实现不处理滚轮事件
	virtual bool onWheel(const QPoint& pos, const QPoint& angleDelta) { return false; }

	/// 功能：推进动画状态
	/// 返回：是否仍有动画在进行（需要后续帧继续更新）
	/// 说明：每帧调用，用于更新动画插值、淡入淡出等效果
	virtual bool tick() = 0;

	/// 功能：获取组件边界矩形
	/// 返回：组件的边界矩形（逻辑像素坐标）
	/// 说明：用于命中测试和布局计算
	virtual QRect bounds() const = 0;

	/// 功能：主题变化回调（默认实现）
	/// 参数：isDark — 是否为暗色主题
	/// 说明：默认调用applyTheme()方法，子类可重写以自定义行为
	void onThemeChanged(bool isDark) override {
		applyTheme(isDark);
	}
};