/*
 * 文件名：CurrentPageHost.h
 * 职责：当前页面宿主适配器，桥接声明式UI与现有页面路由系统。
 * 依赖：IUiComponent、IUiContent、PageRouter。
 * 线程：仅在UI线程使用。
 * 备注：作为声明式shell的内容区域，将布局信息转发给当前活跃页面。
 */

#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "PageRouter.h"

/// 当前页面宿主：声明式UI与页面路由系统的适配器
/// 
/// 功能：
/// - 接收声明式布局分配的视口矩形
/// - 将所有UI操作委托给PageRouter的当前页面
/// - 在声明式shell中作为内容区域的占位符
/// 
/// 使用场景：
/// - AppShell的content区域需要一个IUiComponent
/// - 该组件将视口信息转发给当前页面
/// - 所有交互和渲染委托给实际的UiPage实例
class CurrentPageHost final : public IUiComponent, public IUiContent {
public:
    /// 构造函数：注入页面路由器引用
    /// 参数：router — 页面路由器，提供当前活跃页面
    explicit CurrentPageHost(PageRouter& router) : m_router(router) {}

    /// 析构函数：确保安全关闭
    ~CurrentPageHost() {
        // 标记为无效，防止在析构过程中访问引用
        m_valid = false;
    }

    // IUiContent接口实现
    void setViewportRect(const QRect& r) override;

    // IUiComponent接口实现
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
    bool tick() override;
    QRect bounds() const override;

    // IThemeAware接口实现（通过IUiComponent继承）
    void onThemeChanged(bool isDark) override;

private:
    PageRouter& m_router;
    QRect m_viewport;  // 当前分配的视口区域
    bool m_valid{ true };  // 用于跟踪对象是否仍然有效
};