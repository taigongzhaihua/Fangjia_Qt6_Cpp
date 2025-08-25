#include "HomePage.h"
#include "UI.h"
#include <qcolor.h>

class HomePage::Impl {
public:
    bool isDark = false;
    std::unique_ptr<IUiComponent> builtComponent;

    WidgetPtr buildUI() {
        return column({
            // 欢迎标题
            container(
                text("欢迎使用方家")
                    ->fontSize(32)
                    ->fontWeight(QFont::Bold)
                    ->color(isDark ? QColor(250,252,255) : QColor(20,25,30))
            )->padding(0, 40, 0, 20),

                // 副标题
                container(
                    text("中医方剂数据管理系统")
                        ->fontSize(18)
                        ->color(isDark ? QColor(200,210,220) : QColor(80,90,100))
                )->padding(0, 0, 0, 40),

                // 功能卡片网格
                buildFeatureGrid(),

                // 底部空间
                expanded(spacer())
            })
            ->mainAxisAlignment(Alignment::Start)
            ->crossAxisAlignment(Alignment::Center);
    }

private:
    WidgetPtr buildFeatureGrid() {
        return container(
            column({
                row({
                    buildFeatureCard(
                        ":/icons/data_light.svg",
                        "方剂数据",
                        "查看和管理中医方剂"
                    ),
                    spacer(16),
                    buildFeatureCard(
                        ":/icons/explore_light.svg",
                        "探索发现",
                        "发现新的方剂组合"
                    )
                })->mainAxisAlignment(Alignment::Center),

                spacer(16),

                row({
                    buildFeatureCard(
                        ":/icons/fav_light.svg",
                        "我的收藏",
                        "管理收藏的方剂"
                    ),
                    spacer(16),
                    buildFeatureCard(
                        ":/icons/settings_light.svg",
                        "系统设置",
                        "自定义应用偏好"
                    )
                })->mainAxisAlignment(Alignment::Center)
                })
        )->padding(40);
    }

    WidgetPtr buildFeatureCard(const QString& iconPath, const QString& title, const QString& desc) {
        return card(
            column({
                icon(iconPath)
                    ->size(48)
                    ->color(isDark ? QColor(100,160,220) : QColor(60,120,180)),

                spacer(16),

                text(title)
                    ->fontSize(16)
                    ->fontWeight(QFont::DemiBold)
                    ->color(isDark ? QColor(240,245,250) : QColor(30,35,40)),

                spacer(8),

                text(desc)
                    ->fontSize(13)
                    ->color(isDark ? QColor(180,190,200) : QColor(100,110,120))
                })
            ->mainAxisAlignment(Alignment::Center)
            ->crossAxisAlignment(Alignment::Center)
        )
            ->elevation(2.0f)
            ->size(200, 180);
    }
};

HomePage::HomePage() : m_impl(std::make_unique<Impl>()) {
    setTitle("首页");
    HomePage::initializeContent();
}

HomePage::~HomePage() = default;

void HomePage::initializeContent() {
    auto widget = m_impl->buildUI();
    m_impl->builtComponent = widget->build();
    setContent(m_impl->builtComponent.get());
}

void HomePage::applyPageTheme(bool isDark) {
    m_impl->isDark = isDark;

    // 重新构建UI
    auto widget = m_impl->buildUI();
    m_impl->builtComponent = widget->build();
    setContent(m_impl->builtComponent.get());
}