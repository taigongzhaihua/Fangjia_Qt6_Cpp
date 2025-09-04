/*
 * 新弹出系统集成示例
 * 
 * 演示如何在实际应用中使用新的弹出系统
 * 包含：外部控制弹出窗口的各种模式和用例
 * 
 * 注意：新架构中popup不再包含触发器功能，完全由外部控制：
 * - 弹出窗口只维护开启/关闭状态和内容显示
 * - 外部控件通过事件控制弹出窗口的显示/隐藏
 * - 触发器和弹出窗口完全解耦，支持多种控制模式
 * - 更灵活的架构，允许多个控件控制同一个弹出窗口
 */

#include "UI.h"
#include "BasicWidgets.h"
#include "AdvancedWidgets.h"
#include <QDebug>
#include <QString>
#include <QColor>

namespace Examples {

/// 示例1：外部控制的下拉选择器演示
class ExternalDropdownExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        // 创建不包含触发器的弹出窗口（演示概念）
        auto dropdown = popup()
            ->content(
                vbox()
                    ->padding(4)
                    ->child(createOption("中文", "zh"))
                    ->child(createOption("English", "en"))
                    ->child(createOption("日本語", "ja"))
                    ->child(createOption("Français", "fr"))
            )
            ->size(QSize(140, 140))
            ->placement(UI::Popup::Placement::Bottom)
            ->backgroundColor(QColor(255, 255, 255))
            ->cornerRadius(8.0f)
            ->onVisibilityChanged([](bool visible) {
                qDebug() << "语言选择器" << (visible ? "打开" : "关闭");
            })
            ->buildWithWindow(parentWindow);

        // 返回演示说明，展示外部控制概念
        return vbox()
            ->child(
                text("外部控制下拉选择器")
                    ->fontSize(14)
                    ->fontWeight(QFont::Medium)
                    ->textColor(QColor(60, 60, 60))
            )
            ->child(
                pushButton("触发器按钮 ▼")
                    ->padding(12, 8)
                    ->backgroundColor(QColor(70, 130, 180))
                    ->textColor(Qt::white)
                    ->onClick([]() {
                        qDebug() << "外部控制演示：应该显示语言选择弹出窗口";
                        qDebug() << "实际实现：dropdown->showPopupAt(buttonPosition);";
                    })
            )
            ->child(
                text("💡 触发器与弹出窗口分离")
                    ->fontSize(11)
                    ->textColor(QColor(100, 100, 100))
            )
            ->spacing(8);
    }

private:
    static WidgetPtr createOption(const QString& text, const QString& code) {
        using namespace UI;
        return pushButton(text)
            ->fullWidth()
            ->padding(8, 6)
            ->textAlign(Qt::AlignLeft)
            ->onClick([code, text]() {
                qDebug() << "选择了语言:" << text << "(" << code << ")";
                // 在实际应用中，这里会更新应用语言设置
            });
    }
};

/// 示例2：外部控制的工具提示演示  
class ExternalTooltipExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return vbox()
            ->child(
                text("外部控制工具提示")
                    ->fontSize(14)
                    ->fontWeight(QFont::Medium)
                    ->textColor(QColor(60, 60, 60))
            )
            ->child(
                pushButton("🛈 帮助按钮")
                    ->size(QSize(120, 32))
                    ->backgroundColor(QColor(100, 150, 200))
                    ->textColor(Qt::white)
                    ->cornerRadius(4.0f)
                    ->onClick([]() {
                        qDebug() << "外部控制演示：应该显示工具提示";
                        qDebug() << "实际实现：tooltip->showPopupAtPosition(mousePosition);";
                    })
            )
            ->child(
                text("💡 支持悬停和点击触发")
                    ->fontSize(11)
                    ->textColor(QColor(100, 100, 100))
            )
            ->spacing(8);
    }
};

/// 示例3：外部控制的上下文菜单演示
class ExternalContextMenuExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return vbox()
            ->child(
                text("外部控制上下文菜单")
                    ->fontSize(14)
                    ->fontWeight(QFont::Medium)
                    ->textColor(QColor(60, 60, 60))
            )
            ->child(
                pushButton("右键区域 📋")
                    ->padding(16, 12)
                    ->backgroundColor(QColor(240, 240, 240))
                    ->textColor(QColor(60, 60, 60))
                    ->onClick([]() {
                        qDebug() << "外部控制演示：应该显示上下文菜单";
                        qDebug() << "实际实现：contextMenu->showPopupAt(rightClickPosition);";
                    })
            )
            ->child(
                text("💡 支持右键和长按触发")
                    ->fontSize(11)
                    ->textColor(QColor(100, 100, 100))
            )
            ->spacing(8);
    }
};

/// 示例4：外部控制的复杂表单弹出窗口演示
class ExternalFormPopupExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return vbox()
            ->child(
                text("外部控制表单弹出")
                    ->fontSize(14)
                    ->fontWeight(QFont::Medium)
                    ->textColor(QColor(60, 60, 60))
            )
            ->child(
                pushButton("📝 新建项目")
                    ->padding(16, 10)
                    ->backgroundColor(QColor(34, 139, 34))
                    ->textColor(Qt::white)
                    ->cornerRadius(4.0f)
                    ->onClick([]() {
                        qDebug() << "外部控制演示：应该显示项目创建表单";
                        qDebug() << "实际实现：formPopup->showPopupAt(center);";
                    })
            )
            ->child(
                text("💡 支持复杂交互场景")
                    ->fontSize(11)
                    ->textColor(QColor(100, 100, 100))
            )
            ->spacing(8);
    }
};

/// 主集成示例页面
class PopupIntegrationPage {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return vbox()
            ->padding(20)
            ->spacing(20)
            ->child(
                text("外部控制弹出系统集成示例")
                    ->fontSize(24)
                    ->fontWeight(QFont::Bold)
                    ->textAlign(Qt::AlignCenter)
            )
            ->child(
                text("展示无触发器弹出窗口的外部控制模式")
                    ->fontSize(14)
                    ->textAlign(Qt::AlignCenter)
                    ->textColor(QColor(100, 100, 100))
            )
            ->child(
                hbox()
                    ->spacing(16)
                    ->child(ExternalDropdownExample::create(parentWindow))
                    ->child(ExternalTooltipExample::create(parentWindow))
            )
            ->child(
                hbox()
                    ->spacing(16)
                    ->child(ExternalContextMenuExample::create(parentWindow))
                    ->child(ExternalFormPopupExample::create(parentWindow))
            )
            ->child(
                card()
                    ->padding(16)
                    ->backgroundColor(QColor(240, 248, 255))
                    ->child(
                        vbox()
                            ->child(
                                text("💡 新架构优势")
                                    ->fontSize(16)
                                    ->fontWeight(QFont::Bold)
                            )
                            ->child(
                                text("• 弹出窗口不包含触发器逻辑，完全解耦\n"
                                     "• 外部组件通过事件控制显示/隐藏\n" 
                                     "• 支持多个控件控制同一弹出窗口\n"
                                     "• 更灵活的控制逻辑，适应复杂场景")
                                    ->fontSize(12)
                                    ->lineHeight(1.4)
                            )
                    )
            );
    }
};

} // namespace Examples