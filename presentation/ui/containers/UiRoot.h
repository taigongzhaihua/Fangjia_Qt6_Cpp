/*
 * 文件名：UiRoot.h
 * 职责：UI组件根容器，负责统一驱动布局计算、资源更新、事件分发和主题传播。
 * 依赖：UI组件接口、渲染数据结构、图标缓存。
 * 线程：仅在UI线程使用。
 * 备注：采用指针捕获机制处理鼠标事件，确保拖拽等操作的连续性。
 */

#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"

#include <qopenglfunctions.h>
#include <vector>

/// UI组件根容器：统一管理所有顶级UI组件的生命周期和交互
/// 
/// 功能：
/// - 子组件层次结构管理（添加、移除、清理）
/// - 布局计算协调（measure-arrange两阶段布局）
/// - 渲染资源上下文同步（图标缓存、OpenGL函数表、DPR）
/// - 事件分发与指针捕获管理
/// - 动画帧推进与主题变化传播
/// 
/// 事件处理：
/// - 指针捕获：按下时命中的组件会捕获后续的移动和释放事件
/// - 事件冒泡：从最前面的组件开始分发，直到某个组件处理为止
/// - 滚轮事件：支持位置相关的滚轮事件分发
class UiRoot
{
public:
	/// 功能：添加顶级UI组件
	/// 参数：c — 组件指针（不转移所有权）
	void add(IUiComponent* c);
	
	/// 功能：移除顶级UI组件
	/// 参数：c — 要移除的组件指针
	void remove(IUiComponent* c);
	
	/// 功能：清空所有顶级组件
	void clear();

	/// 功能：更新所有组件的布局
	/// 参数：windowSize — 窗口逻辑像素尺寸
	/// 说明：触发所有组件的measure和arrange阶段
	void updateLayout(const QSize& windowSize) const;
	
	/// 功能：更新所有组件的渲染资源上下文
	/// 参数：cache — 图标纹理缓存
	/// 参数：gl — OpenGL函数表
	/// 参数：devicePixelRatio — DPR（设备像素比）
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) const;

	/// 功能：收集所有组件的绘制命令
	/// 参数：fd — 帧数据容器，用于收集绘制命令
	void append(Render::FrameData& fd) const;

	/// 功能：分发鼠标按下事件
	/// 参数：pos — 鼠标位置（逻辑像素）
	/// 返回：是否有组件处理了该事件
	/// 说明：命中的组件会被设为指针捕获目标
	bool onMousePress(const QPoint& pos);
	
	/// 功能：分发鼠标移动事件
	/// 参数：pos — 鼠标位置（逻辑像素）
	/// 返回：是否有组件处理了该事件
	/// 说明：优先发送给指针捕获组件
	bool onMouseMove(const QPoint& pos);
	
	/// 功能：分发鼠标释放事件
	/// 参数：pos — 鼠标位置（逻辑像素）
	/// 返回：是否有组件处理了该事件
	/// 说明：事件后清除指针捕获状态
	bool onMouseRelease(const QPoint& pos);
	
	/// 功能：分发滚轮事件
	/// 参数：pos — 鼠标位置（逻辑像素）
	/// 参数：angleDelta — 滚轮角度增量
	/// 返回：是否有组件处理了该事件
	bool onWheel(const QPoint& pos, const QPoint& angleDelta);

	/// 功能：推进动画帧
	/// 返回：是否有组件需要重绘
	/// 说明：遍历所有组件调用tick()方法
	[[nodiscard]] bool tick() const;

	/// 功能：传播主题变化到所有子组件
	/// 参数：isDark — 是否为暗色主题
	/// 说明：递归调用所有组件的onThemeChanged()方法
	void propagateThemeChange(bool isDark) const;

private:
	std::vector<IUiComponent*> m_children; // 顶级组件列表（不拥有所有权）

	// 指针捕获：按下命中的组件会捕获后续移动和释放事件，直到释放为止
	IUiComponent* m_pointerCapture{ nullptr };
};