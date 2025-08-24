#include <QtTest>
#include "../../src/framework/containers/UiBoxLayout.h"

// 测试用的简单组件
class TestLayoutChild : public IUiComponent {
public:
    explicit TestLayoutChild(const QString& name) : m_name(name) {}
    
    void updateLayout(const QSize&) override {}
    void updateResourceContext(IconLoader&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return m_bounds; }
    
    void setBounds(const QRect& r) { m_bounds = r; }
    QString name() const { return m_name; }
    
private:
    QString m_name;
    QRect m_bounds{0, 0, 100, 100};
};

class TestBoxLayout : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing BoxLayout tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "BoxLayout tests completed.";
    }
    
    void testHorizontalLayout() {
        UiHBoxLayout layout;
        layout.setViewportRect(QRect(0, 0, 300, 100));
        layout.setSpacing(10);
        
        auto child1 = new TestLayoutChild("child1");
        auto child2 = new TestLayoutChild("child2");
        auto child3 = new TestLayoutChild("child3");
        
        layout.addChild(child1, 1.0f);  // 权重1
        layout.addChild(child2, 2.0f);  // 权重2
        layout.addChild(child3, 1.0f);  // 权重1
        
        layout.updateLayout(QSize(300, 100));
        
        QCOMPARE(layout.childCount(), 3);
        
        // 清理
        delete child1;
        delete child2;
        delete child3;
    }
    
    void testVerticalLayout() {
        UiVBoxLayout layout;
        layout.setViewportRect(QRect(0, 0, 100, 300));
        layout.setSpacing(5);
        layout.setMargins(QMargins(10, 10, 10, 10));
        
        auto child1 = new TestLayoutChild("child1");
        auto child2 = new TestLayoutChild("child2");
        
        layout.addChild(child1, 0.0f);  // 固定大小
        layout.addChild(child2, 1.0f);  // 弹性大小
        
        layout.updateLayout(QSize(100, 300));
        
        QCOMPARE(layout.childCount(), 2);
        
        // 清理
        delete child1;
        delete child2;
    }
    
    void testChildManagement() {
        UiBoxLayout layout;
        
        auto child1 = new TestLayoutChild("child1");
        auto child2 = new TestLayoutChild("child2");
        auto child3 = new TestLayoutChild("child3");
        
        // 添加子控件
        layout.addChild(child1);
        layout.addChild(child2);
        QCOMPARE(layout.childCount(), 2);
        
        // 插入子控件
        layout.insertChild(1, child3);
        QCOMPARE(layout.childCount(), 3);
        QCOMPARE(layout.childAt(1), child3);
        
        // 移除子控件
        layout.removeChild(child2);
        QCOMPARE(layout.childCount(), 2);
        
        // 按索引移除
        layout.removeChildAt(0);
        QCOMPARE(layout.childCount(), 1);
        
        // 清空
        layout.clearChildren();
        QCOMPARE(layout.childCount(), 0);
        
        // 清理
        delete child1;
        delete child2;
        delete child3;
    }
    
    void testChildVisibility() {
        UiBoxLayout layout;
        
        auto child1 = new TestLayoutChild("child1");
        auto child2 = new TestLayoutChild("child2");
        
        layout.addChild(child1);
        layout.addChild(child2);
        
        QVERIFY(layout.isChildVisible(0));
        QVERIFY(layout.isChildVisible(1));
        
        layout.setChildVisible(1, false);
        QVERIFY(layout.isChildVisible(0));
        QVERIFY(!layout.isChildVisible(1));
        
        // 清理
        delete child1;
        delete child2;
    }
    
    void testAlignment() {
        UiBoxLayout layout;
        
        auto child = new TestLayoutChild("child");
        layout.addChild(child, 1.0f, UiBoxLayout::Alignment::Center);
        
        QCOMPARE(layout.childAlignment(0), UiBoxLayout::Alignment::Center);
        
        layout.setChildAlignment(0, UiBoxLayout::Alignment::End);
        QCOMPARE(layout.childAlignment(0), UiBoxLayout::Alignment::End);
        
        // 清理
        delete child;
    }
    
    void testBuilderPattern() {
        UiBoxLayout layout;
        
        layout.withSpacing(20)
              .withMargins(QMargins(5, 5, 5, 5))
              .withBackground(QColor(255, 255, 255), 10.0f);
        
        QCOMPARE(layout.spacing(), 20);
        QCOMPARE(layout.margins(), QMargins(5, 5, 5, 5));
        
        // 验证背景已设置（需要通过append验证）
        Render::FrameData fd;
        layout.setViewportRect(QRect(0, 0, 100, 100));
        layout.append(fd);
        QVERIFY(!fd.roundedRects.empty());
    }
};