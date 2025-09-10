/*
 * 文件名：Window.h
 * 职责：基础窗口类，提供OpenGL渲染、UI事件处理、动画循环的通用框架。
 * 依赖：Qt6 OpenGL、渲染器、图标缓存、UI根容器。
 * 线程：仅在UI线程使用，所有OpenGL操作在当前上下文中执行。
 * 备注：派生类通过虚函数实现应用特定的UI初始化、布局和事件处理逻辑。
 */

#pragma once

#include <memory>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qtimer.h>

#include "IconCache.h"
#include "Renderer.h"
#include "UiRoot.h"

/// 基础窗口类：提供OpenGL窗口的通用框架和渲染管线
///
/// 功能：
/// - OpenGL上下文管理与资源初始化
/// - 统一渲染管线：清屏 → UI组件渲染 → 缓冲区交换
/// - 事件系统：鼠标、键盘、滚轮事件的UI组件分发
/// - 动画循环：60fps目标的帧驱动与组件动画更新
/// - 窗口生命周期管理：初始化、调整大小、清理
///
/// 生命周期：
/// 1. 构造时设置基础状态
/// 2. initializeGL()中初始化OpenGL和调用派生类的initializeUI()
/// 3. resizeGL()中更新视口和调用派生类的updateLayout()
/// 4. paintGL()中执行标准渲染流程
/// 5. 析构时清理资源
///
/// 派生类职责：
/// - 实现initializeUI()设置应用特定的UI组件
/// - 实现updateLayout()计算布局
/// - 可选重写事件处理方法实现特定逻辑
class Window : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
    /// 构造函数：初始化基础窗口状态
    /// 参数：updateBehavior — Qt窗口更新行为控制
    explicit Window(UpdateBehavior updateBehavior = NoPartialUpdate);
    
    /// 析构函数：清理资源
    ~Window() override;

    /// 获取UI根容器：用于添加顶级UI组件
    UiRoot& uiRoot() { return m_uiRoot; }
    const UiRoot& uiRoot() const { return m_uiRoot; }

    /// 获取渲染器：用于自定义渲染逻辑
    Renderer& renderer() { return m_renderer; }
    const Renderer& renderer() const { return m_renderer; }

    /// 获取图标缓存：用于纹理资源管理
    IconCache& iconCache() { return m_iconCache; }
    const IconCache& iconCache() const { return m_iconCache; }

protected:
    // ========== OpenGL生命周期回调 ==========
    
    /// OpenGL上下文初始化：设置OpenGL状态并调用派生类初始化
    void initializeGL() override;
    
    /// 窗口大小调整：更新视口和布局
    void resizeGL(int w, int h) override;
    
    /// 渲染帧：执行标准渲染流程
    void paintGL() override;

    // ========== 事件处理回调 ==========
    
    /// 鼠标事件：转发到UI根容器
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

    /// 键盘事件：转发到UI根容器
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    // ========== 派生类接口 ==========
    
    /// 初始化UI组件：派生类实现应用特定的UI设置
    /// 调用时机：initializeGL()中，OpenGL上下文已就绪
    virtual void initializeUI() = 0;
    
    /// 更新布局：派生类实现布局计算逻辑
    /// 调用时机：窗口大小改变时或手动调用
    virtual void updateLayout() = 0;
    
    /// 动画帧更新：派生类可重写实现自定义动画逻辑
    /// 参数：deltaTime — 距离上次更新的时间间隔（毫秒）
    /// 返回：是否需要继续动画（true表示需要下一帧）
    virtual bool onAnimationTick(qint64 deltaTime);
    
    /// 获取清屏颜色：派生类可重写实现主题相关的背景色
    virtual QColor getClearColor() const;

    // ========== 工具方法 ==========
    
    /// 启动动画循环：开始60fps动画帧更新
    void startAnimationLoop();
    
    /// 停止动画循环：暂停动画帧更新
    void stopAnimationLoop();
    
    /// 请求重绘：标记窗口需要重新渲染
    void requestRedraw();

private slots:
    /// 动画计时器回调：处理每一帧的动画更新
    void onAnimationFrame();

private:
    // ========== 核心组件 ==========
    
    /// UI根容器：管理所有UI组件的层次结构
    UiRoot m_uiRoot;
    
    /// 渲染器：执行OpenGL渲染命令
    Renderer m_renderer;
    
    /// 图标缓存：管理纹理资源的加载和缓存
    IconCache m_iconCache;

    // ========== 渲染状态 ==========
    
    /// 帧缓冲区尺寸（像素）
    int m_framebufferWidth{ 0 };
    int m_framebufferHeight{ 0 };

    // ========== 动画管理 ==========
    
    /// 动画计时器：驱动60fps帧更新
    QTimer m_animationTimer;
    
    /// 动画时钟：测量帧间隔时间
    QElapsedTimer m_animationClock;
    
    /// 动画状态：是否正在运行动画循环
    bool m_animationActive{ false };
};