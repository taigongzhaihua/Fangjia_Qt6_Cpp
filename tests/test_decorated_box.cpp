#include <QtTest>
#include <QDebug>
#include <memory>
#include <functional>

// Include DecoratedBox
#include "framework/declarative/Decorators.h"
#include "framework/base/UiComponent.hpp"
#include "core/rendering/RenderData.hpp"

class TestDecoratedBoxOnTap : public QObject
{
    Q_OBJECT

private slots:
    void testOnTapWithPadding()
    {
        qDebug() << "=== Testing DecoratedBox onTap with padding ===";
        
        // Mock child component
        class MockChild : public IUiComponent {
        public:
            void updateLayout(const QSize&) override {}
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; } // Child doesn't handle
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { return QRect(0, 0, 50, 20); }
            void onThemeChanged(bool) override {}
        };
        
        auto child = std::make_unique<MockChild>();
        
        // Set up DecoratedBox with padding (like HomePage buttons)
        UI::DecoratedBox::Props props;
        props.padding = QMargins(8, 4, 8, 4); // Same as HomePage buttons: padding(8, 4)
        props.visible = true;
        
        bool tapCalled = false;
        props.onTap = [&tapCalled]() {
            tapCalled = true;
        };
        
        auto decoratedBox = std::make_unique<UI::DecoratedBox>(std::move(child), props);
        
        // Set viewport (simulating button layout)
        QRect buttonRect(0, 0, 66, 28); // Width: 50 + 8*2 padding, Height: 20 + 4*2 padding
        decoratedBox->setViewportRect(buttonRect);
        
        // Test 1: Click in the padding area (left edge) - should work with fix
        QPoint paddingClick(4, 14); // In the left padding area
        qDebug() << "Testing click in padding area at" << paddingClick;
        
        bool paddingPressHandled = decoratedBox->onMousePress(paddingClick);
        QVERIFY(paddingPressHandled); // Should handle the press with fix
        
        bool paddingReleaseHandled = decoratedBox->onMouseRelease(paddingClick);
        QVERIFY(paddingReleaseHandled); // Should handle the release
        QVERIFY(tapCalled); // onTap should have been called
        
        // Reset for next test
        tapCalled = false;
        
        // Test 2: Click in the content area - should also work
        QPoint contentClick(33, 14); // In the center of the button
        qDebug() << "Testing click in content area at" << contentClick;
        
        bool contentPressHandled = decoratedBox->onMousePress(contentClick);
        QVERIFY(contentPressHandled);
        
        bool contentReleaseHandled = decoratedBox->onMouseRelease(contentClick);
        QVERIFY(contentReleaseHandled);
        QVERIFY(tapCalled);
        
        // Reset for next test
        tapCalled = false;
        
        // Test 3: Click outside the viewport - should not work
        QPoint outsideClick(70, 14); // Outside the button
        qDebug() << "Testing click outside at" << outsideClick;
        
        bool outsidePressHandled = decoratedBox->onMousePress(outsideClick);
        QVERIFY(!outsidePressHandled); // Should not handle clicks outside
        QVERIFY(!tapCalled); // onTap should not be called
        
        qDebug() << "DecoratedBox onTap with padding tests PASSED ✅";
    }
    
    void testHoverWithPadding()
    {
        qDebug() << "=== Testing DecoratedBox hover with padding ===";
        
        class MockChild : public IUiComponent {
        public:
            void updateLayout(const QSize&) override {}
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; }
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { return QRect(0, 0, 50, 20); }
            void onThemeChanged(bool) override {}
        };
        
        auto child = std::make_unique<MockChild>();
        
        UI::DecoratedBox::Props props;
        props.padding = QMargins(8, 4, 8, 4);
        props.visible = true;
        
        bool isHovered = false;
        props.onHover = [&isHovered](bool hover) {
            isHovered = hover;
        };
        
        auto decoratedBox = std::make_unique<UI::DecoratedBox>(std::move(child), props);
        decoratedBox->setViewportRect(QRect(0, 0, 66, 28));
        
        // Test hover in padding area
        QPoint paddingPos(4, 14);
        bool hoverHandled = decoratedBox->onMouseMove(paddingPos);
        QVERIFY(hoverHandled);
        QVERIFY(isHovered);
        
        // Test hover outside
        QPoint outsidePos(70, 14);
        hoverHandled = decoratedBox->onMouseMove(outsidePos);
        QVERIFY(hoverHandled); // Should handle the change
        QVERIFY(!isHovered); // Should not be hovered anymore
        
        qDebug() << "DecoratedBox hover with padding tests PASSED ✅";
    }
};

#include "test_decorated_box.moc"