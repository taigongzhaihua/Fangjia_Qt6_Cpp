#include <QGuiApplication>
#include <QTimer>
#include <QObject>
#include <QDebug>
#include <iostream>
#include "framework/declarative/RebuildHost.h"
#include "framework/declarative/Binding.h"
#include "framework/base/UiComponent.hpp"
#include "core/rendering/RenderData.hpp"

// 简单的计数器 ViewModel 用于演示
class CounterViewModel : public QObject
{
    Q_OBJECT
public:
    explicit CounterViewModel(QObject* parent = nullptr) 
        : QObject(parent), m_count(0)
    {
        // 每秒自增计数器
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &CounterViewModel::increment);
        m_timer->start(1000); // 1秒间隔
    }
    
    int count() const { return m_count; }
    
public slots:
    void increment() {
        m_count++;
        std::cout << "Counter incremented to: " << m_count << std::endl;
        emit countChanged();
    }

signals:
    void countChanged();

private:
    int m_count;
    QTimer* m_timer;
};

// 简单的测试组件，用于演示重建
class SimpleTestComponent : public IUiComponent
{
public:
    explicit SimpleTestComponent(int counter) : m_counter(counter) {
        std::cout << "SimpleTestComponent created with counter: " << m_counter << std::endl;
    }
    
    // IUiComponent interface - 最小实现
    void updateLayout(const QSize&) override {}
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return QRect(); }
    void onThemeChanged(bool) override {}

private:
    int m_counter;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    std::cout << "=== MVVM Binding Example ===" << std::endl;
    std::cout << "This example demonstrates RebuildHost binding and rebuilding." << std::endl;
    std::cout << "The counter will increment every second and trigger rebuilds." << std::endl;
    std::cout << std::endl;
    
    // 创建 CounterViewModel
    CounterViewModel counterVm;
    
    // 创建 RebuildHost
    UI::RebuildHost rebuildHost;
    
    // 设置 builder 函数
    rebuildHost.setBuilder([&counterVm]() -> std::unique_ptr<IUiComponent> {
        std::cout << "🔄 Rebuilding UI component..." << std::endl;
        return std::make_unique<SimpleTestComponent>(counterVm.count());
    });
    
    // 连接 ViewModel 的信号到 RebuildHost 的重建请求
    QObject::connect(&counterVm, &CounterViewModel::countChanged,
                     [&rebuildHost]() { rebuildHost.requestRebuild(); });
    
    std::cout << "✅ MVVM binding established!" << std::endl;
    std::cout << "Counter changes will now trigger UI rebuilds." << std::endl;
    std::cout << std::endl;
    
    // 初始构建
    rebuildHost.requestRebuild();
    
    // 设置定时器在 8 秒后退出
    QTimer::singleShot(8000, &app, &QGuiApplication::quit);
    
    std::cout << "Running for 8 seconds..." << std::endl;
    
    // 运行事件循环
    int result = app.exec();
    
    std::cout << std::endl;
    std::cout << "=== Example completed ===" << std::endl;
    std::cout << "Final counter value: " << counterVm.count() << std::endl;
    
    return result;
}

#include "main.moc"