/*
 * 文件名：SimplePopup.h
 * 职责：简化的弹出控件实现，解决原有架构的复杂性问题
 * 依赖：Qt6 OpenGL、UI组件接口
 * 线程：仅在UI线程使用
 * 备注：直接实现弹出功能，避免复杂的层级间接和延迟创建问题
 */

#pragma once

#include "UiComponent.hpp"
#include "UiContent.hpp"
#include <functional>
#include <IconCache.h>
#include <memory>
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qwindow.h>
#include <qtimer.h>
#include <qelapsedtimer.h>  // 添加缺失的包含
#include <RenderData.hpp>

/// 简化的弹出窗口实现
/// 
/// 设计理念：
/// - 直接继承QOpenGLWindow，避免复杂的包装层
/// - 立即初始化，避免延迟创建的时序问题
/// - 直接事件处理，减少转发层次
/// - 简单的资源管理，避免复杂的依赖关系
class SimplePopupWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SimplePopupWindow(QWindow* parent = nullptr);
    ~SimplePopupWindow() override;

    /// 设置弹出内容（直接设置，不延迟创建）
    void setContent(std::unique_ptr<IUiComponent> content);
    
    /// 设置背景样式
    void setBackgroundStyle(const QColor& color, float cornerRadius = 8.0f);
    
    /// 在指定位置显示弹出窗口
    void showAt(const QPoint& globalPos, const QSize& size);
    
    /// 隐藏弹出窗口
    void hidePopup();
    
    /// 检查是否可见
    bool isVisible() const;
    
    /// 设置可见性变化回调
    void setOnVisibilityChanged(std::function<void(bool)> callback) {
        m_onVisibilityChanged = callback;
    }

protected:
    // QOpenGLWindow overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    // Event handling
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void onAnimationTick();

private:
    /// 更新内容布局
    void updateContentLayout();
    
    /// 更新内容资源上下文
    void updateContentResources();
    
    /// 渲染内容
    void renderContent();
    
    /// 渲染帧数据（简化实现）
    void renderFrameData(const Render::FrameData& frameData);

private:
    // 内容管理
    std::unique_ptr<IUiComponent> m_content;        // 弹出内容
    
    // 视觉样式
    QColor m_backgroundColor{ 255, 255, 255, 240 }; // 背景颜色
    float m_cornerRadius{ 8.0f };                   // 圆角半径
    
    // 渲染状态
    IconCache m_iconCache;                          // 图标缓存
    QRect m_contentRect;                            // 内容区域
    bool m_needsRedraw{ true };                     // 需要重绘
    
    // 动画系统
    QTimer m_animTimer;                             // 动画定时器
    QElapsedTimer m_animClock;                      // 动画时钟
    
    // 回调函数
    std::function<void(bool)> m_onVisibilityChanged; // 可见性变化回调
};

/// 简化的弹出控件组件
/// 
/// 特点：
/// - 直接管理SimplePopupWindow，避免复杂的主机包装
/// - 立即创建资源，避免延迟初始化问题
/// - 简化的事件处理，直接响应而不是转发
class SimplePopup : public IUiComponent, public IUiContent
{
public:
    /// 弹出位置策略
    enum class Placement {
        Bottom,      // 在触发器下方
        Top,         // 在触发器上方  
        Right,       // 在触发器右侧
        Left,        // 在触发器左侧
        BottomLeft,  // 在触发器左下方
        BottomRight, // 在触发器右下方
        TopLeft,     // 在触发器左上方
        TopRight,    // 在触发器右上方
        Custom       // 自定义位置
    };

public:
    explicit SimplePopup(QWindow* parentWindow);
    ~SimplePopup() override;

    /// 设置触发器组件
    void setTrigger(std::unique_ptr<IUiComponent> trigger);
    
    /// 设置弹出内容
    void setPopupContent(std::unique_ptr<IUiComponent> content);
    
    /// 设置弹出配置
    void setPopupSize(const QSize& size) { m_popupSize = size; }
    void setPlacement(Placement placement) { m_placement = placement; }
    void setOffset(const QPoint& offset) { m_offset = offset; }
    void setBackgroundStyle(const QColor& color, float cornerRadius = 8.0f);
    void setCloseOnClickOutside(bool close) { m_closeOnClickOutside = close; }
    
    /// 程序控制显示/隐藏
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const;
    
    /// 设置回调
    void setOnPopupVisibilityChanged(std::function<void(bool)> callback) {
        m_onVisibilityChanged = callback;
    }

    // IUiContent
    void setViewportRect(const QRect& r) override;

    // IUiComponent
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
    bool tick() override;
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool isDark) override;

private:
    /// 计算弹出窗口的全局位置
    QPoint calculatePopupPosition() const;
    
    /// 处理弹出窗口隐藏事件
    void onPopupHidden();

private:
    // 窗口管理
    QWindow* m_parentWindow{ nullptr };                    // 父窗口引用
    std::unique_ptr<SimplePopupWindow> m_popupWindow;      // 弹出窗口实例
    
    // 内容管理
    std::unique_ptr<IUiComponent> m_trigger;               // 触发器组件
    std::unique_ptr<IUiComponent> m_popupContent;          // 弹出内容组件
    
    // 视口管理
    QRect m_viewport;                                       // 当前视口
    
    // 弹出配置
    QSize m_popupSize{ 200, 150 };                         // 弹出窗口大小
    Placement m_placement{ Placement::Bottom };             // 弹出位置策略
    QPoint m_offset{ 0, 0 };                               // 位置偏移量
    bool m_closeOnClickOutside{ true };                    // 点击外部关闭
    
    // 视觉样式
    QColor m_backgroundColor{ 255, 255, 255, 240 };       // 背景颜色
    float m_cornerRadius{ 8.0f };                          // 圆角半径
    bool m_isDark{ false };                                // 当前主题
    
    // 资源上下文
    IconCache* m_cache{ nullptr };
    QOpenGLFunctions* m_gl{ nullptr };
    float m_dpr{ 1.0f };
    
    // 回调函数
    std::function<void(bool)> m_onVisibilityChanged;       // 可见性变化回调
};