#include <QtTest>
#include <QRect>
#include <QPoint>
#include "../../src/framework/base/UiComponent.hpp"
#include "../../src/framework/base/UiButton.hpp"
#include "../../src/core/rendering/RenderData.hpp"

// 测试用的具体组件
class TestComponent : public IUiComponent {
public:
    void updateLayout(const QSize& windowSize) override {
        m_bounds = QRect(0, 0, windowSize.width() / 2, windowSize.height() / 2);
    }
    
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
        m_dpr = devicePixelRatio;
    }
    
    void append(Render::FrameData& fd) const override {
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(m_bounds),
            .radiusPx = 5.0f,
            .color = QColor(255, 0, 0)
        });
    }
    
    bool onMousePress(const QPoint& pos) override {
        if (m_bounds.contains(pos)) {
            m_pressed = true;
            return true;
        }
        return false;
    }
    
    bool onMouseMove(const QPoint& pos) override {
        bool wasHovered = m_hovered;
        m_hovered = m_bounds.contains(pos);
        return wasHovered != m_hovered;
    }
    
    bool onMouseRelease(const QPoint& pos) override {
        if (m_pressed) {
            m_pressed = false;
            if (m_bounds.contains(pos)) {
                m_clicked = true;
                return true;
            }
        }
        return false;
    }
    
    bool tick() override {
        return false;
    }
    
    QRect bounds() const override {
        return m_bounds;
    }
    
    // 测试辅助
    bool isPressed() const { return m_pressed; }
    bool isHovered() const { return m_hovered; }
    bool wasClicked() const { return m_clicked; }
    float dpr() const { return m_dpr; }
    
private:
    QRect m_bounds;
    bool m_pressed{false};
    bool m_hovered{false};
    bool m_clicked{false};
    float m_dpr{1.0f};
};

class TestUiComponents : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing UiComponents tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "UiComponents tests completed.";
    }
    
    void testComponentInterface() {
        TestComponent comp;
        
        // 测试布局
        comp.updateLayout(QSize(800, 600));
        QCOMPARE(comp.bounds(), QRect(0, 0, 400, 300));
        
        // 测试绘制命令生成
        Render::FrameData fd;
        comp.append(fd);
        QCOMPARE(fd.roundedRects.size(), 1);
    }
    
    void testMouseInteraction() {
        TestComponent comp;
        comp.updateLayout(QSize(800, 600));
        
        // 测试鼠标按下
        QVERIFY(comp.onMousePress(QPoint(100, 100)));
        QVERIFY(comp.isPressed());
        QVERIFY(!comp.onMousePress(QPoint(500, 500))); // 外部
        
        // 测试鼠标移动
        QVERIFY(!comp.isHovered());
        QVERIFY(comp.onMouseMove(QPoint(100, 100)));
        QVERIFY(comp.isHovered());
        QVERIFY(comp.onMouseMove(QPoint(500, 500)));
        QVERIFY(!comp.isHovered());
        
        // 测试鼠标释放
        comp.onMousePress(QPoint(100, 100));
        QVERIFY(comp.onMouseRelease(QPoint(100, 100)));
        QVERIFY(comp.wasClicked());
    }
    
    void testButton() {
        Ui::Button btn;
        btn.setBaseRect(QRect(10, 10, 100, 50));
        btn.setPalette(
            QColor(200, 200, 200), // bg
            QColor(220, 220, 220), // bgHover
            QColor(180, 180, 180), // bgPressed
            QColor(50, 50, 50)     // icon
        );
        
        // 测试透明度
        btn.setOpacity(0.5f);
        QVERIFY(qFuzzyCompare(btn.opacity(), 0.5f));
        
        // 测试偏移
        btn.setOffset(QPointF(5, 5));
        QCOMPARE(btn.visualRectF(), QRectF(15, 15, 100, 50));
        
        // 测试启用/禁用
        btn.setEnabled(false);
        QVERIFY(!btn.enabled());
        QVERIFY(!btn.onMousePress(QPoint(20, 20))); // 禁用时不响应
        
        btn.setEnabled(true);
        QVERIFY(btn.enabled());
        QVERIFY(btn.onMousePress(QPoint(20, 20))); // 启用时响应
    }
    
    void testButtonClick() {
        Ui::Button btn;
        btn.setBaseRect(QRect(0, 0, 100, 100));
        btn.setEnabled(true);
        
        bool clicked = false;
        
        // 模拟点击
        QVERIFY(btn.onMousePress(QPoint(50, 50)));
        QVERIFY(btn.pressed());
        
        QVERIFY(btn.onMouseRelease(QPoint(50, 50), clicked));
        QVERIFY(clicked);
        QVERIFY(!btn.pressed());
    }
    
    void testButtonHover() {
        Ui::Button btn;
        btn.setBaseRect(QRect(0, 0, 100, 100));
        btn.setEnabled(true);
        
        QVERIFY(!btn.hovered());
        
        // 移入
        QVERIFY(btn.onMouseMove(QPoint(50, 50)));
        QVERIFY(btn.hovered());
        
        // 移出
        QVERIFY(btn.onMouseMove(QPoint(150, 150)));
        QVERIFY(!btn.hovered());
    }
};