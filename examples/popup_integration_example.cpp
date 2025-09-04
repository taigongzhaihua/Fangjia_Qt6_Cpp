/*
 * 文件名：popup_integration_example.cpp
 * 职责：展示如何在主应用程序中集成弹出控件的示例代码。
 * 备注：这是一个代码示例，展示集成方式，不是可执行文件。
 */

#include "UiPopup.h"
#include "UiPopupWindow.h"
// ... 其他必要的包含

// 示例：在主窗口中添加一个带弹出菜单的按钮
class MainWindowWithPopup : public MainOpenGlWindow 
{
public:
    MainWindowWithPopup(/* 构造参数 */) : MainOpenGlWindow(/* 参数 */) {
        // 在初始化完成后添加弹出控件
        initializePopupComponents();
    }

private:
    void initializePopupComponents() {
        // 1. 创建菜单内容
        auto menuContent = std::make_unique<MenuContentPanel>();
        menuContent->addMenuItem("选项1", []() { qDebug() << "选择了选项1"; });
        menuContent->addMenuItem("选项2", []() { qDebug() << "选择了选项2"; });
        menuContent->addMenuItem("设置", []() { qDebug() << "打开设置"; });
        
        // 2. 创建触发按钮
        auto menuButton = std::make_unique<UiPushButton>();
        menuButton->setText("菜单");
        menuButton->setSize(UiPushButton::Size::M);
        menuButton->setVariant(UiPushButton::Variant::Secondary);
        
        // 3. 创建弹出控件
        m_menuPopup = std::make_unique<UiPopup>(this); // this 是 QWindow*
        m_menuPopup->setTrigger(menuButton.get());
        m_menuPopup->setPopupContent(menuContent.get());
        m_menuPopup->setPopupSize(QSize(180, 120));
        m_menuPopup->setPlacement(UiPopup::Placement::BottomLeft);
        m_menuPopup->setPopupStyle(QColor(255, 255, 255, 240), 8.0f);
        
        // 4. 设置可见性回调
        m_menuPopup->setOnPopupVisibilityChanged([this](bool visible) {
            qDebug() << "菜单弹出窗口" << (visible ? "显示" : "隐藏");
            // 可以在这里添加其他逻辑，比如更新按钮状态
        });
        
        // 5. 将组件添加到UI层次结构
        // 方式1：直接添加到UiRoot（适用于简单情况）
        m_uiRoot.add(m_menuPopup.get());
        
        // 方式2：添加到特定的容器中（推荐用于复杂布局）
        // someContainer->addChild(m_menuPopup.get());
        
        // 6. 保存组件引用以管理生命周期
        m_menuButton = std::move(menuButton);
        m_menuContent = std::move(menuContent);
    }
    
    // 程序化控制弹出显示（可选）
    void showContextMenu(const QPoint& position) {
        if (m_menuPopup) {
            // 设置自定义位置
            m_menuPopup->setPlacement(UiPopup::Placement::Custom);
            m_menuPopup->setOffset(position);
            m_menuPopup->showPopup();
        }
    }
    
    // 重写鼠标事件以支持右键菜单（可选）
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::RightButton) {
            showContextMenu(e->pos());
            e->accept();
            return;
        }
        
        // 调用基类处理
        MainOpenGlWindow::mousePressEvent(e);
    }

private:
    // 弹出控件相关成员
    std::unique_ptr<UiPopup> m_menuPopup;
    std::unique_ptr<UiPushButton> m_menuButton;
    std::unique_ptr<MenuContentPanel> m_menuContent;
};

// 示例：自定义弹出内容组件
class MenuContentPanel : public IUiComponent, public IUiContent, public ILayoutable 
{
public:
    struct MenuItem {
        QString text;
        std::function<void()> action;
        bool enabled = true;
    };
    
    void addMenuItem(const QString& text, std::function<void()> action) {
        m_items.push_back({text, action, true});
        updateLayout();
    }
    
    // ILayoutable 实现
    QSize measure(const SizeConstraints& cs) override {
        const int itemHeight = 32;
        const int padding = 16;
        const int width = std::clamp(150, cs.minW, cs.maxW);
        const int height = std::clamp(
            static_cast<int>(m_items.size()) * itemHeight + padding, 
            cs.minH, 
            cs.maxH
        );
        return QSize(width, height);
    }
    
    void arrange(const QRect& finalRect) override {
        setViewportRect(finalRect);
    }
    
    // IUiContent 实现
    void setViewportRect(const QRect& r) override {
        m_viewport = r;
        layoutMenuItems();
    }
    
    // IUiComponent 实现
    void updateLayout(const QSize& windowSize) override {
        layoutMenuItems();
    }
    
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
        m_cache = &cache;
        m_gl = gl;
        m_dpr = devicePixelRatio;
    }
    
    void append(Render::FrameData& fd) const override {
        if (!m_viewport.isValid()) return;
        
        // 绘制菜单背景
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(m_viewport),
            .radiusPx = 8.0f,
            .color = m_isDark ? QColor(50, 50, 50, 240) : QColor(255, 255, 255, 240),
            .clipRect = QRectF(m_viewport)
        });
        
        // 绘制菜单项
        for (size_t i = 0; i < m_items.size(); ++i) {
            drawMenuItem(fd, static_cast<int>(i));
        }
    }
    
    bool onMousePress(const QPoint& pos) override {
        const int itemIndex = getItemIndexAt(pos);
        if (itemIndex >= 0 && itemIndex < static_cast<int>(m_items.size())) {
            m_pressedIndex = itemIndex;
            return true;
        }
        return false;
    }
    
    bool onMouseMove(const QPoint& pos) override {
        const int newHoverIndex = getItemIndexAt(pos);
        if (newHoverIndex != m_hoverIndex) {
            m_hoverIndex = newHoverIndex;
            return true; // 需要重绘
        }
        return false;
    }
    
    bool onMouseRelease(const QPoint& pos) override {
        const int itemIndex = getItemIndexAt(pos);
        if (itemIndex == m_pressedIndex && itemIndex >= 0 && 
            itemIndex < static_cast<int>(m_items.size())) {
            
            const auto& item = m_items[itemIndex];
            if (item.enabled && item.action) {
                item.action(); // 执行菜单项动作
            }
            
            m_pressedIndex = -1;
            return true;
        }
        m_pressedIndex = -1;
        return false;
    }
    
    bool tick() override { return false; }
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool isDark) override { m_isDark = isDark; }

private:
    void layoutMenuItems() {
        // 菜单项布局逻辑
        m_itemRects.clear();
        if (!m_viewport.isValid() || m_items.empty()) return;
        
        const int itemHeight = 32;
        const int padding = 8;
        int y = m_viewport.y() + padding;
        
        for (size_t i = 0; i < m_items.size(); ++i) {
            const QRect itemRect(
                m_viewport.x() + padding,
                y,
                m_viewport.width() - 2 * padding,
                itemHeight
            );
            m_itemRects.push_back(itemRect);
            y += itemHeight;
        }
    }
    
    void drawMenuItem(Render::FrameData& fd, int index) const {
        if (index < 0 || index >= static_cast<int>(m_itemRects.size())) return;
        
        const QRect& itemRect = m_itemRects[index];
        const auto& item = m_items[index];
        
        // 绘制项目背景（悬停/按下状态）
        if (index == m_hoverIndex || index == m_pressedIndex) {
            QColor bgColor = m_isDark ? QColor(70, 70, 70, 180) : QColor(240, 240, 240, 180);
            if (index == m_pressedIndex) {
                bgColor = bgColor.darker(120);
            }
            
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(itemRect),
                .radiusPx = 4.0f,
                .color = bgColor,
                .clipRect = QRectF(m_viewport)
            });
        }
        
        // 绘制文本（这里简化为一个矩形，实际应该使用文本渲染）
        const QRect textRect = itemRect.adjusted(8, 4, -8, -4);
        const QColor textColor = item.enabled ? 
            (m_isDark ? QColor(255, 255, 255) : QColor(50, 50, 50)) :
            (m_isDark ? QColor(150, 150, 150) : QColor(180, 180, 180));
            
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(textRect),
            .radiusPx = 2.0f,
            .color = QColor(textColor.red(), textColor.green(), textColor.blue(), 100),
            .clipRect = QRectF(m_viewport)
        });
    }
    
    int getItemIndexAt(const QPoint& pos) const {
        for (size_t i = 0; i < m_itemRects.size(); ++i) {
            if (m_itemRects[i].contains(pos)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

private:
    std::vector<MenuItem> m_items;
    std::vector<QRect> m_itemRects;
    QRect m_viewport;
    
    int m_hoverIndex = -1;
    int m_pressedIndex = -1;
    bool m_isDark = false;
    
    IconCache* m_cache = nullptr;
    QOpenGLFunctions* m_gl = nullptr;
    float m_dpr = 1.0f;
};

// 使用示例总结：
/*
1. 创建弹出内容组件（继承IUiComponent等接口）
2. 创建触发器组件（可以是任何UI组件）
3. 创建UiPopup实例并配置
4. 将UiPopup添加到UI层次结构
5. 通过交互或程序控制显示/隐藏弹出窗口

主要优势：
- 弹出窗口可以超出主窗口边界
- 完全集成到现有UI框架
- 支持自定义位置策略
- 事件处理和主题支持
- 资源共享和生命周期管理
*/