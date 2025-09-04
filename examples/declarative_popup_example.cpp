/*
 * 文件名：declarative_popup_example.cpp
 * 职责：展示声明式弹出控件的使用示例。
 * 备注：这是一个代码示例，展示如何使用新的声明式弹出控件API。
 */

#include "../presentation/ui/declarative/UI.h"
#include <QGuiApplication>
#include <QOpenGLWindow>

// 示例：使用声明式API创建弹出菜单
void createDeclarativePopupMenu() {
    // 创建触发按钮
    auto menuButton = text("菜单按钮")
        ->fontSize(14)
        ->padding(12, 8)
        ->background(QColor(100, 150, 200), 4.0f)
        ->onTap([]() { 
            // 按钮点击时的逻辑可以在这里处理
            // 弹出窗口的显示由popup组件自动管理
        });

    // 创建弹出内容
    auto menuContent = panel({
        text("选项 1")->fontSize(14)->padding(12, 8)->onTap([]() { 
            qDebug() << "选择了选项1"; 
        }),
        text("选项 2")->fontSize(14)->padding(12, 8)->onTap([]() { 
            qDebug() << "选择了选项2"; 
        }),
        text("设置")->fontSize(14)->padding(12, 8)->onTap([]() { 
            qDebug() << "打开设置"; 
        })
    })->background(QColor(255, 255, 255, 240), 8.0f)
      ->padding(8);

    // 创建声明式弹出组件
    auto popupMenu = popup()
        ->trigger(menuButton)                           // 设置触发器
        ->content(menuContent)                          // 设置弹出内容
        ->size(QSize(150, 100))                        // 设置大小
        ->placement(Popup::Placement::BottomLeft)       // 设置位置
        ->style(QColor(255, 255, 255, 240), 8.0f)     // 设置样式
        ->closeOnClickOutside(true)                     // 点击外部关闭
        ->onVisibilityChanged([](bool visible) {       // 可见性回调
            qDebug() << "弹出菜单" << (visible ? "显示" : "隐藏");
        });

    // 构建组件
    auto component = popupMenu->build();
    
    // 注意：由于技术限制，需要手动设置窗口上下文
    // 在实际应用中，parentWindow 应该是你的主窗口实例
    // Popup::configurePopupWindow(component.get(), parentWindow);
    
    // 添加到UI根容器
    // uiRoot.add(component.release());
}

// 示例：在主窗口类中集成声明式弹出菜单
class MainWindowWithDeclarativePopup : public QOpenGLWindow 
{
public:
    MainWindowWithDeclarativePopup() {
        initializeUI();
    }

private:
    void initializeUI() {
        // 创建工具栏弹出菜单
        auto toolsMenu = popup()
            ->trigger(text("工具")->fontSize(14)->padding(10, 6))
            ->content(panel({
                text("导入数据")->padding(10, 4)->onTap([this]() { importData(); }),
                text("导出数据")->padding(10, 4)->onTap([this]() { exportData(); }),
                text("首选项")->padding(10, 4)->onTap([this]() { openPreferences(); })
            }))
            ->placement(Popup::Placement::Bottom)
            ->style(QColor(240, 240, 240, 250), 6.0f);

        // 创建用户菜单
        auto userMenu = popup()
            ->trigger(text("用户")->fontSize(14)->padding(10, 6))
            ->content(panel({
                text("个人资料")->padding(10, 4),
                text("账户设置")->padding(10, 4),
                text("退出登录")->padding(10, 4)
            }))
            ->placement(Popup::Placement::BottomRight)
            ->size(QSize(120, 90));

        // 构建并配置组件
        auto toolsComponent = toolsMenu->build();
        auto userComponent = userMenu->build();

        // 设置窗口上下文
        Popup::configurePopupWindow(toolsComponent.get(), this);
        Popup::configurePopupWindow(userComponent.get(), this);

        // 添加到UI系统（示例代码，实际中需要适配你的UI根容器）
        // m_uiRoot.add(toolsComponent.release());
        // m_uiRoot.add(userComponent.release());
    }

    void importData() { /* 导入数据逻辑 */ }
    void exportData() { /* 导出数据逻辑 */ }
    void openPreferences() { /* 打开首选项逻辑 */ }
};

/*
 * 声明式弹出控件的优势：
 * 
 * 1. **流式API**: 支持链式调用，代码简洁易读
 * 2. **类型安全**: 编译时检查，减少运行时错误
 * 3. **自动管理**: 生命周期自动管理，减少内存泄漏
 * 4. **一致性**: 与其他声明式组件API保持一致
 * 5. **可组合**: 可以轻松与其他声明式组件组合使用
 * 
 * 使用注意事项：
 * 
 * 1. 必须调用 Popup::configurePopupWindow() 设置窗口上下文
 * 2. 弹出内容应该是轻量级组件，避免复杂的嵌套
 * 3. 回调函数中避免捕获可能失效的对象引用
 * 4. 在窗口关闭前确保弹出窗口已正确清理
 */

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 示例用法（仅作演示，实际需要完整的窗口和UI系统）
    createDeclarativePopupMenu();
    
    MainWindowWithDeclarativePopup window;
    window.show();
    
    return app.exec();
}