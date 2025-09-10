/*
 * 文件名：Window.hpp
 * 职责：基础窗口类，提供窗口循环和通用事件处理，增加封装性和代码复用性。
 * 依赖：Qt6 Core/Gui/OpenGL、主题管理、配置管理。
 * 线程：仅在UI线程使用。
 * 备注：所有应用窗口的基类，封装了OpenGL上下文管理和标准事件循环。
 */

#pragma once
#include <memory>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qtimer.h>
#include <qevent.h>
#include <qrect.h>

// 前向声明
class AppConfig;
class ThemeManager;
class IconCache;
class Renderer;

namespace fj::presentation::ui::base {

/// 基础窗口类：封装OpenGL窗口的通用功能
/// 
/// 功能：
/// - OpenGL上下文管理和资源生命周期
/// - 标准事件循环处理（鼠标、键盘、滚轮）
/// - 主题切换支持
/// - 动画循环驱动
/// - 窗口几何状态管理
/// 
/// 设计原则：
/// - 提供通用窗口功能的基础实现
/// - 子类通过虚函数定制特定行为
/// - 避免业务逻辑耦合，保持框架层纯净性
class Window : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    enum class Theme { Light, Dark };

    /// 构造函数：初始化基础窗口
    /// 参数：updateBehavior — Qt窗口更新行为控制
    explicit Window(UpdateBehavior updateBehavior = NoPartialUpdate);
    ~Window() override;

    // 主题管理接口
    virtual void setTheme(Theme theme);
    Theme theme() const noexcept { return m_theme; }
    
    // 配置接口 - 子类可重写以提供具体配置
    virtual void saveWindowGeometry();
    virtual void restoreWindowGeometry();

signals:
    void themeChanged(Theme newTheme);

protected:
    /// OpenGL生命周期回调 - 子类可重写
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    /// 事件处理 - 子类可重写以添加特定处理逻辑
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    /// 虚函数接口 - 子类必须实现
    
    /// 功能：初始化窗口特定的OpenGL资源
    virtual void initializeWindowGL() = 0;
    
    /// 功能：更新窗口布局
    /// 参数：w, h — 新的窗口尺寸
    virtual void updateWindowLayout(int w, int h) = 0;
    
    /// 功能：渲染窗口内容
    virtual void renderWindow() = 0;
    
    /// 功能：处理主题变化
    /// 参数：newTheme — 新的主题模式
    virtual void onThemeChanged(Theme newTheme) = 0;

    /// 功能：处理动画帧更新
    /// 返回：是否需要继续动画
    virtual bool onAnimationTick() = 0;

protected:
    // 基础窗口状态
    Theme m_theme{ Theme::Dark };
    QColor m_clearColor;
    
    // 动画驱动（目标60fps）
    QTimer m_animTimer;
    QElapsedTimer m_animClock;
    
    // 窗口尺寸状态
    int m_fbWpx{ 0 };    // 帧缓冲宽度（像素）
    int m_fbHpx{ 0 };    // 帧缓冲高度（像素）

private slots:
    void onAnimationTimerTick();

private:
    void updateThemeColors();
    void startAnimationLoop();
    void stopAnimationLoop();
};

} // namespace fj::presentation::ui::base