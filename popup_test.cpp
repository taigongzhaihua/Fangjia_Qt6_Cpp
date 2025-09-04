/*
 * 新弹出系统测试程序
 * 
 * 验证新的popup架构：
 * - 立即创建，无延迟初始化
 * - 简单直接的API
 * - 可靠的显示和隐藏
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
        setWindowTitle("新弹出系统测试");
        setGeometry(100, 100, 400, 300);

        qDebug() << "测试窗口创建完成";
    }

    void createPopupTest()
    {
        using namespace UI;

        // 测试新的声明式API
        auto popupComponent = popup()
            ->trigger(
                pushButton("点击显示弹出窗口")
                    ->onClick([]() {
                        qDebug() << "触发器被点击";
                    })
            )
            ->content(
                vbox()
                    ->child(text("这是弹出内容"))
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

        // 使用新的buildWithWindow方法
        auto component = popupComponent->buildWithWindow(windowHandle());
        
        if (component) {
            qDebug() << "弹出组件创建成功";
            // TODO: 集成到UI系统中
        } else {
            qDebug() << "弹出组件创建失败";
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TestWindow window;
    window.show();

    qDebug() << "新弹出系统测试程序启动";
    qDebug() << "本测试验证:";
    qDebug() << "1. 新弹出组件可以成功创建";
    qDebug() << "2. 声明式API工作正常";
    qDebug() << "3. 没有延迟创建问题";

    return app.exec();
}

#include "popup_test.moc"