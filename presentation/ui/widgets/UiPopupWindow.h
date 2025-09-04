/*
 * 文件名：UiPopupWindow.h
 * 职责：弹出窗口类，基于QOpenGLWindow实现可超出父窗口边界的弹出控件。
 * 依赖：Qt6 OpenGL、自绘UI框架、渲染器、图标缓存。
 * 线程：仅在UI线程使用，所有OpenGL操作在当前上下文中执行。
 * 备注：作为子窗口存在，可以绘制在父窗口边界之外，同时保持与父窗口的关联关系。
 */

#pragma once

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QElapsedTimer>
#include <QColor>
#include <memory>

#include "../containers/UiRoot.h"
#include "../../infrastructure/gfx/Renderer.h"
#include "../../infrastructure/gfx/IconCache.h"
#include "../base/RenderData.hpp"
#include "../base/UiComponent.hpp"

class IUiComponent;

/// 弹出窗口：基于OpenGL的可超出父窗口边界的弹出控件
/// 
/// 功能：
/// - 作为子窗口可以绘制在父窗口外部区域
/// - 集成现有的UI组件系统和渲染管线
/// - 支持事件处理和动画
/// - 维持与父窗口的关联关系
/// 
/// 使用场景：
/// - 下拉菜单、工具提示、上下文菜单等需要超出窗口边界的UI元素
/// - 悬浮工具栏、通知窗口等临时显示内容
class UiPopupWindow final : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    /// 构造函数
    /// 参数：parent — 父窗口，popup将作为其子窗口
    /// 参数：updateBehavior — Qt窗口更新行为控制
    explicit UiPopupWindow(QWindow* parent = nullptr, UpdateBehavior updateBehavior = NoPartialUpdate);
    ~UiPopupWindow() override;

    /// 设置弹出窗口内容
    /// 参数：content — UI组件内容指针（不转移所有权）
    void setContent(IUiComponent* content);

    /// 获取弹出窗口内容
    IUiComponent* content() const noexcept { return m_content; }

    /// 设置背景颜色
    void setBackgroundColor(const QColor& color);

    /// 设置圆角半径
    void setCornerRadius(float radius) { m_cornerRadius = radius; }

    /// 显示弹出窗口在指定位置
    /// 参数：globalPos — 全局屏幕坐标位置
    /// 参数：size — 窗口大小
    void showAt(const QPoint& globalPos, const QSize& size);

    /// 隐藏弹出窗口
    void hidePopup();

    /// 检查是否可见
    bool isPopupVisible() const { return isVisible(); }

    /// 应用主题
    void applyTheme(bool isDark);

protected:
    /// OpenGL生命周期回调
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    /// 事件处理
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    /// 窗口事件
    void focusOutEvent(QFocusEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

signals:
    /// 弹出窗口被隐藏时发出
    void popupHidden();

private slots:
    /// 动画定时器回调
    void onAnimationTick();

private:
    /// 更新布局
    void updateLayout();

    /// 更新资源上下文
    void updateResourceContext();

private:
    // 内容管理
    IUiComponent* m_content{ nullptr };  // 非拥有指针
    UiRoot m_uiRoot;                     // UI根组件管理器

    // 渲染子系统
    Renderer m_renderer;                 // OpenGL渲染器
    IconCache m_iconCache;               // 图标缓存
    int m_fbWpx{ 0 };                   // 帧缓冲宽度（像素）
    int m_fbHpx{ 0 };                   // 帧缓冲高度（像素）

    // 视觉样式
    QColor m_backgroundColor;            // 背景颜色
    float m_cornerRadius{ 0.0f };       // 圆角半径
    bool m_isDark{ false };             // 当前主题状态

    // 动画驱动
    QTimer m_animTimer;                 // 动画定时器
    QElapsedTimer m_animClock;          // 动画时钟
};