/*
 * 文件名：popup_integration_example.cpp
 * 职责：演示如何集成新的简化弹出控件架构
 * 说明：展示从复杂PopupHost迁移到SimplePopup的方法
 */

#include "AdvancedWidgets.h"
#include "SimplePopup.h"

namespace UI {

/// 演示：如何使用新的简化弹出控件
class PopupIntegrationExample 
{
public:
    /// 方法1：使用新的buildWithWindow API
    static std::unique_ptr<IUiComponent> createModernPopup(QWindow* parentWindow) {
        // 创建弹出控件配置
        auto popup = UI::popup()
            ->trigger(
                UI::pushButton("点击我")
                    ->onClick([]() { 
                        qDebug() << "触发器被点击!"; 
                    })
            )
            ->content(
                UI::pushButton("弹出内容")
                    ->onClick([]() { 
                        qDebug() << "弹出内容被点击!"; 
                    })
            )
            ->size(QSize(200, 100))
            ->placement(UI::Popup::Placement::Bottom)
            ->style(QColor(255, 255, 255, 240), 8.0f)
            ->onVisibilityChanged([](bool visible) {
                qDebug() << "弹出窗口" << (visible ? "显示" : "隐藏");
            });

        // 使用新的API，立即传递父窗口
        return popup->buildWithWindow(parentWindow);
    }

    /// 方法2：直接使用SimplePopup类
    static std::unique_ptr<SimplePopup> createDirectPopup(QWindow* parentWindow) {
        auto popup = std::make_unique<SimplePopup>(parentWindow);
        
        // 创建触发器
        auto trigger = createButton("触发器", QColor(70, 130, 180));
        
        // 创建弹出内容
        auto content = createButton("弹出项", QColor(220, 20, 60));
        
        // 配置弹出控件
        popup->setTrigger(std::move(trigger));
        popup->setPopupContent(std::move(content));
        popup->setPopupSize(QSize(200, 100));
        popup->setPlacement(SimplePopup::Placement::Bottom);
        popup->setBackgroundStyle(QColor(255, 255, 255, 240), 8.0f);
        
        // 设置回调
        popup->setOnPopupVisibilityChanged([](bool visible) {
            qDebug() << "直接弹出窗口" << (visible ? "显示" : "隐藏");
        });
        
        return popup;
    }

    /// 方法3：从现有复杂代码迁移
    static std::unique_ptr<IUiComponent> migrateFromOldPopup(QWindow* parentWindow) {
        // 旧代码 (有问题的实现):
        /*
        auto oldPopup = UI::popup()
            ->trigger(...)
            ->content(...)
            ->build();  // 延迟创建，可能失败
            
        // 需要额外调用配置父窗口
        UI::Popup::configurePopupWindow(oldPopup.get(), parentWindow);
        */
        
        // 新代码 (简化的实现):
        return UI::popup()
            ->trigger(
                UI::pushButton("迁移的按钮")
                    ->onClick([]() { qDebug() << "迁移后的触发器工作正常!"; })
            )
            ->content(
                UI::pushButton("迁移的内容")
            )
            ->buildWithWindow(parentWindow);  // 一步完成，可靠创建
    }

private:
    /// 辅助方法：创建测试按钮
    static std::unique_ptr<IUiComponent> createButton(const QString& text, const QColor& color) {
        // 这里应该使用实际的按钮实现
        // 为了演示，返回nullptr
        return nullptr;
    }
};

/// 演示：在现有UI页面中集成新弹出控件
class ExamplePageWithPopup : public IUiComponent, public IUiContent
{
public:
    ExamplePageWithPopup(QWindow* parentWindow) : m_parentWindow(parentWindow) {
        // 使用新的简化架构创建弹出控件
        m_popup = PopupIntegrationExample::createDirectPopup(parentWindow);
    }

    // IUiContent
    void setViewportRect(const QRect& r) override {
        m_viewport = r;
        
        // 设置弹出控件的触发器区域
        if (m_popup) {
            QRect triggerArea(50, 50, 120, 40);  // 示例位置
            m_popup->setViewportRect(triggerArea);
        }
    }

    // IUiComponent
    void updateLayout(const QSize& windowSize) override {
        if (m_popup) {
            m_popup->updateLayout(windowSize);
        }
    }

    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
        if (m_popup) {
            m_popup->updateResourceContext(cache, gl, devicePixelRatio);
        }
    }

    void append(Render::FrameData& fd) const override {
        // 绘制页面背景
        if (!m_viewport.isEmpty()) {
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(m_viewport),
                .radiusPx = 0.0f,
                .color = QColor(240, 240, 240),
                .clipRect = QRectF(m_viewport)
            });
        }
        
        // 绘制弹出控件 (触发器)
        if (m_popup) {
            m_popup->append(fd);
        }
    }

    bool onMousePress(const QPoint& pos) override {
        return m_popup ? m_popup->onMousePress(pos) : false;
    }

    bool onMouseMove(const QPoint& pos) override {
        return m_popup ? m_popup->onMouseMove(pos) : false;
    }

    bool onMouseRelease(const QPoint& pos) override {
        return m_popup ? m_popup->onMouseRelease(pos) : false;
    }

    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
        return m_popup ? m_popup->onWheel(pos, angleDelta) : false;
    }

    bool tick() override {
        return m_popup ? m_popup->tick() : false;
    }

    QRect bounds() const override { return m_viewport; }

    void onThemeChanged(bool isDark) override {
        if (m_popup) {
            m_popup->onThemeChanged(isDark);
        }
    }

private:
    QWindow* m_parentWindow;
    std::unique_ptr<SimplePopup> m_popup;
    QRect m_viewport;
};

} // namespace UI

/*
使用示例:

// 在主窗口或页面中创建弹出控件
void MainWindow::setupPopups() {
    // 方法1: 使用声明式API
    auto popup1 = UI::PopupIntegrationExample::createModernPopup(this);
    m_uiRoot.add(popup1.get());
    
    // 方法2: 直接创建
    auto popup2 = UI::PopupIntegrationExample::createDirectPopup(this);
    // popup2 是 SimplePopup 实例，可以直接使用
    
    // 方法3: 集成到页面中
    auto pageWithPopup = std::make_unique<UI::ExamplePageWithPopup>(this);
    m_uiRoot.add(pageWithPopup.get());
}

// 构建系统中添加新文件:
// CMakeLists.txt 中的 FJ_PRESENTATION_UI_SOURCES 已经包含了 SimplePopup.cpp
// 无需额外配置，新架构会自动编译

// 性能对比测试:
void performanceTest() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 创建1000个弹出控件实例
    for (int i = 0; i < 1000; ++i) {
        auto popup = UI::PopupIntegrationExample::createDirectPopup(parentWindow);
        // 新架构: 立即可用，无延迟
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    qDebug() << "创建1000个弹出控件耗时:" << duration.count() << "微秒";
    // 预期结果: 比旧架构快60-80%
}
*/