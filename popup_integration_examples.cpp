/*
 * 新弹出系统集成示例
 * 
 * 演示如何在实际应用中使用新的弹出系统
 * 包含：下拉菜单、工具提示、上下文菜单等常见用例
 */

#include "UI.h"
#include "BasicWidgets.h"
#include "AdvancedWidgets.h"
#include <QDebug>
#include <QString>
#include <QColor>

namespace Examples {

/// 示例1：简单的下拉选择器
class DropdownExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return popup()
            ->trigger(
                pushButton("选择语言 ▼")
                    ->padding(12, 8)
                    ->backgroundColor(QColor(70, 130, 180))
                    ->textColor(Qt::white)
            )
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

/// 示例2：工具提示弹出窗口
class TooltipExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return popup()
            ->trigger(
                pushButton("🛈")
                    ->size(QSize(24, 24))
                    ->backgroundColor(QColor(100, 150, 200))
                    ->cornerRadius(12.0f)
            )
            ->content(
                card()
                    ->padding(12)
                    ->child(
                        vbox()
                            ->child(
                                text("帮助信息")
                                    ->fontSize(14)
                                    ->fontWeight(QFont::Bold)
                            )
                            ->child(
                                text("这是一个工具提示示例，\n显示有用的帮助信息。")
                                    ->fontSize(12)
                                    ->wordWrap(true)
                            )
                    )
            )
            ->size(QSize(200, 100))
            ->placement(UI::Popup::Placement::TopRight)
            ->offset(QPoint(8, -8))
            ->backgroundColor(QColor(255, 255, 200, 240))
            ->cornerRadius(6.0f)
            ->buildWithWindow(parentWindow);
    }
};

/// 示例3：上下文菜单
class ContextMenuExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return popup()
            ->trigger(
                pushButton("右键菜单示例")
                    ->padding(16, 12)
                    ->backgroundColor(QColor(240, 240, 240))
                    ->onRightClick([]() {
                        qDebug() << "显示右键菜单";
                    })
            )
            ->content(
                vbox()
                    ->padding(4)
                    ->child(createMenuItem("📋", "复制", []() { 
                        qDebug() << "执行复制"; 
                    }))
                    ->child(createMenuItem("✂️", "剪切", []() { 
                        qDebug() << "执行剪切"; 
                    }))
                    ->child(createMenuItem("📄", "粘贴", []() { 
                        qDebug() << "执行粘贴"; 
                    }))
                    ->child(createSeparator())
                    ->child(createMenuItem("🗑️", "删除", []() { 
                        qDebug() << "执行删除"; 
                    }))
            )
            ->size(QSize(120, 140))
            ->placement(UI::Popup::Placement::BottomRight)
            ->backgroundColor(QColor(250, 250, 250))
            ->cornerRadius(4.0f)
            ->buildWithWindow(parentWindow);
    }

private:
    static WidgetPtr createMenuItem(const QString& icon, const QString& text, std::function<void()> action) {
        using namespace UI;
        return pushButton(icon + " " + text)
            ->fullWidth()
            ->padding(8, 6)
            ->textAlign(Qt::AlignLeft)
            ->onClick(action);
    }
    
    static WidgetPtr createSeparator() {
        using namespace UI;
        return divider()
            ->height(1)
            ->color(QColor(200, 200, 200));
    }
};

/// 示例4：复杂的表单弹出窗口
class FormPopupExample {
public:
    static std::unique_ptr<IUiComponent> create(QWindow* parentWindow) {
        using namespace UI;
        
        return popup()
            ->trigger(
                pushButton("📝 新建项目")
                    ->padding(16, 10)
                    ->backgroundColor(QColor(34, 139, 34))
                    ->textColor(Qt::white)
                    ->cornerRadius(4.0f)
            )
            ->content(
                card()
                    ->padding(20)
                    ->child(
                        vbox()
                            ->spacing(12)
                            ->child(
                                text("新建项目")
                                    ->fontSize(16)
                                    ->fontWeight(QFont::Bold)
                                    ->textAlign(Qt::AlignCenter)
                            )
                            ->child(createFormField("项目名称:", "输入项目名称"))
                            ->child(createFormField("描述:", "可选的项目描述"))
                            ->child(
                                hbox()
                                    ->spacing(8)
                                    ->child(
                                        pushButton("取消")
                                            ->backgroundColor(QColor(160, 160, 160))
                                            ->onClick([]() {
                                                qDebug() << "取消新建项目";
                                            })
                                    )
                                    ->child(
                                        pushButton("创建")
                                            ->backgroundColor(QColor(34, 139, 34))
                                            ->textColor(Qt::white)
                                            ->onClick([]() {
                                                qDebug() << "创建新项目";
                                            })
                                    )
                            )
                    )
            )
            ->size(QSize(300, 200))
            ->placement(UI::Popup::Placement::Center)
            ->backgroundColor(QColor(255, 255, 255))
            ->cornerRadius(12.0f)
            ->onVisibilityChanged([](bool visible) {
                qDebug() << "项目创建对话框" << (visible ? "打开" : "关闭");
            })
            ->buildWithWindow(parentWindow);
    }

private:
    static WidgetPtr createFormField(const QString& label, const QString& placeholder) {
        using namespace UI;
        return vbox()
            ->spacing(4)
            ->child(
                text(label)
                    ->fontSize(12)
                    ->textAlign(Qt::AlignLeft)
            )
            ->child(
                textInput()
                    ->placeholder(placeholder)
                    ->padding(8, 6)
                    ->borderColor(QColor(180, 180, 180))
            );
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
                text("新弹出系统集成示例")
                    ->fontSize(24)
                    ->fontWeight(QFont::Bold)
                    ->textAlign(Qt::AlignCenter)
            )
            ->child(
                text("展示各种弹出窗口的实际使用场景")
                    ->fontSize(14)
                    ->textAlign(Qt::AlignCenter)
                    ->textColor(QColor(100, 100, 100))
            )
            ->child(
                hbox()
                    ->spacing(16)
                    ->child(DropdownExample::create(parentWindow))
                    ->child(TooltipExample::create(parentWindow))
            )
            ->child(
                hbox()
                    ->spacing(16)
                    ->child(ContextMenuExample::create(parentWindow))
                    ->child(FormPopupExample::create(parentWindow))
            )
            ->child(
                card()
                    ->padding(16)
                    ->backgroundColor(QColor(240, 248, 255))
                    ->child(
                        vbox()
                            ->child(
                                text("💡 新系统优势")
                                    ->fontSize(16)
                                    ->fontWeight(QFont::Bold)
                            )
                            ->child(
                                text("• 立即初始化，无延迟创建问题\n"
                                     "• 简单的两层架构，易于理解\n"
                                     "• 直接事件处理，响应迅速\n"
                                     "• 声明式API，代码简洁")
                                    ->fontSize(12)
                                    ->lineHeight(1.4)
                            )
                    )
            );
    }
};

} // namespace Examples