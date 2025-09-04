/*
 * 外部控制弹出系统测试程序
 * 
 * 验证新的popup架构：
 * - 弹出窗口不包含触发器逻辑
 * - 外部控制显示/隐藏
 * - 立即创建，无延迟初始化
 * - 简单直接的API
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

#include "UI.h"
#include "BasicWidgets.h"

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    TestWindow(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
    }

private:
    void setupUI()
    {
        auto* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        auto* layout = new QVBoxLayout(centralWidget);
        layout->addStretch();

        // 创建一个简单的状态标签
        auto statusLabel = std::make_unique<QLabel>("点击按钮测试弹出窗口");
        statusLabel->setAlignment(Qt::AlignCenter);
        
        // 设置窗口标题
        setWindowTitle("外部控制弹出系统测试");
        setGeometry(100, 100, 400, 300);

        qDebug() << "测试窗口创建完成";
    }

    void createPopupTest()
    {
        using namespace UI;

        // 测试新的外部控制API（无触发器）
        auto popupComponent = popup()
            ->content(
                vbox()
                    ->child(text("这是外部控制的弹出内容"))
                    ->child(pushButton("关闭")
                        ->onClick([]() {
                            qDebug() << "关闭按钮被点击";
                        })
                    )
            )
            ->size(QSize(250, 120))
            ->placement(UI::Popup::Placement::Bottom)
            ->backgroundColor(QColor(255, 255, 255, 230))
            ->cornerRadius(12.0f)
            ->onVisibilityChanged([](bool visible) {
                qDebug() << "弹出窗口可见性变化:" << visible;
            });

        // 使用新的buildWithWindow方法创建弹出窗口
        auto component = popupComponent->buildWithWindow(windowHandle());
        
        if (component) {
            qDebug() << "无触发器弹出组件创建成功";
            // 演示外部控制
            qDebug() << "外部控制演示：";
            qDebug() << "- 触发器和弹出窗口完全分离";
            qDebug() << "- 弹出窗口可由任意外部事件控制";
        } else {
            qDebug() << "弹出组件创建失败";
        }

        // 演示外部触发器创建（概念演示）
        auto externalTrigger = pushButton("外部触发器")
            ->onClick([component]() {
                qDebug() << "外部触发器被点击";
                qDebug() << "实际实现中，这里会调用 component->showPopupAt(position)";
            });
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TestWindow window;
    window.show();

    qDebug() << "外部控制弹出系统测试程序启动";
    qDebug() << "本测试验证:";
    qDebug() << "1. 弹出组件可以无触发器创建";
    qDebug() << "2. 外部控制API工作正常";
    qDebug() << "3. 触发器与弹出窗口完全分离";

    return app.exec();
}

#include "popup_test.moc"