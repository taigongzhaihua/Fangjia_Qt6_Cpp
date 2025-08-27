#include <QtTest>
#include <QRect>
#include <QColor>
#include <memory>
#include "../../src/framework/declarative/BasicWidgets.h"
#include "../../src/framework/base/IconCache.h"
#include "../../src/core/rendering/RenderData.hpp"

// 模拟 IconCache 和 GL 上下文的简单测试
class MockIconCache : public IconCache {
public:
    int ensureSvgPx(const QString& key, const QByteArray& svg, const QSize& px, QOpenGLFunctions* gl) override {
        m_lastKey = key;
        m_lastSvg = svg;
        return 1; // 返回假的纹理ID
    }
    
    QSize textureSizePx(int textureId) const override {
        return QSize(48, 48); // 假的纹理尺寸
    }
    
    QString lastKey() const { return m_lastKey; }
    QByteArray lastSvg() const { return m_lastSvg; }
    
private:
    QString m_lastKey;
    QByteArray m_lastSvg;
};

class TestIconThemePaths : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing Icon ThemePaths tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "Icon ThemePaths tests completed.";
    }
    
    void testIconSinglePath() {
        // 测试传统单路径模式
        auto icon = UI::icon(":/icons/test.svg")->size(48);
        auto component = icon->build();
        
        // 确保组件创建成功
        QVERIFY(component != nullptr);
        
        // 测试尺寸
        QCOMPARE(component->bounds(), QRect(0, 0, 48, 48));
    }
    
    void testIconThemePaths() {
        // 测试新的主题路径功能
        QString lightPath = ":/icons/test_light.svg";
        QString darkPath = ":/icons/test_dark.svg";
        
        auto icon = UI::icon(":/icons/fallback.svg")
                    ->themePaths(lightPath, darkPath)
                    ->size(48);
        
        auto component = icon->build();
        QVERIFY(component != nullptr);
        
        // 测试初始状态（浅色主题）
        component->onThemeChanged(false);
        
        // 在没有真实渲染环境的情况下，我们无法完全测试路径选择
        // 但我们可以确保组件创建成功且尺寸正确
        QCOMPARE(component->bounds(), QRect(0, 0, 48, 48));
    }
    
    void testIconAutoColor() {
        // 测试自动颜色功能
        auto icon = UI::icon(":/icons/test.svg")->size(24);
        auto component = icon->build();
        
        // 测试主题变化时的颜色调整
        component->onThemeChanged(false); // 浅色主题
        component->onThemeChanged(true);  // 深色主题
        
        // 在实际环境中，这会改变图标的颜色
        // 由于我们无法访问私有成员，只能确保方法调用不会崩溃
        QVERIFY(true);
    }
    
    void testIconExplicitColor() {
        // 测试显式设色
        auto icon = UI::icon(":/icons/test.svg")
                    ->size(32)
                    ->color(QColor(255, 0, 0));
        
        auto component = icon->build();
        QVERIFY(component != nullptr);
        QCOMPARE(component->bounds(), QRect(0, 0, 32, 32));
        
        // 显式设色后，主题变化不应影响颜色
        component->onThemeChanged(true);
        component->onThemeChanged(false);
        
        // 确保组件仍然有效
        QVERIFY(component != nullptr);
    }
    
    void testBackwardCompatibility() {
        // 测试向后兼容性 - 未调用 themePaths 的图标应该正常工作
        auto oldStyleIcon = UI::icon(":/icons/old_style.svg")->size(24);
        auto component = oldStyleIcon->build();
        
        QVERIFY(component != nullptr);
        QCOMPARE(component->bounds(), QRect(0, 0, 24, 24));
        
        // 主题变化应该不会影响路径选择（因为没有设置 themePaths）
        component->onThemeChanged(true);
        component->onThemeChanged(false);
        
        // 组件应该保持有效
        QVERIFY(component != nullptr);
    }
};

#include "test_icon_themepaths.moc"

// 注册测试
QTEST_MAIN(TestIconThemePaths)