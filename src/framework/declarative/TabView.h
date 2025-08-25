#pragma once
#include "Widget.h"
#include "UiTabView.h"
#include "TabViewModel.h"
#include <qstringlist.h>
#include <functional>
#include <memory>
#include <vector>

namespace UI {

// 声明式 TabView：包装 UiTabView
class TabView : public Widget {
public:
    TabView() = default;

    // 配置：VM 或 fallback tabs（二选一；同时存在以 VM 为准）
    std::shared_ptr<TabView> viewModel(TabViewModel* vm) {
        m_vm = vm;
        return self<TabView>();
    }

    std::shared_ptr<TabView> tabs(QStringList labels) {
        m_tabs = std::move(labels);
        return self<TabView>();
    }

    std::shared_ptr<TabView> selectedIndex(int idx) {
        m_selectedIndex = idx;
        return self<TabView>();
    }

    std::shared_ptr<TabView> indicatorStyle(UiTabView::IndicatorStyle s) {
        m_indicatorStyle = s;
        return self<TabView>();
    }

    std::shared_ptr<TabView> tabHeight(int h) {
        m_tabHeight = h;
        return self<TabView>();
    }

    std::shared_ptr<TabView> animationDuration(int ms) {
        m_animDuration = ms;
        return self<TabView>();
    }

    // 设置内容：与 tabs 序列一致的 Widget 列表（可为空）
    std::shared_ptr<TabView> contents(WidgetList items) {
        m_contents = std::move(items);
        return self<TabView>();
    }

    std::shared_ptr<TabView> setContent(int index, WidgetPtr item) {
        if (index < 0) return self<TabView>();
        if (static_cast<size_t>(index) >= m_contents.size()) {
            m_contents.resize(static_cast<size_t>(index) + 1);
        }
        m_contents[static_cast<size_t>(index)] = std::move(item);
        return self<TabView>();
    }

    // 非 VM 模式下使用（VM 模式建议直接监听 TabViewModel::selectedIndexChanged）
    std::shared_ptr<TabView> onChanged(std::function<void(int)> cb) {
        m_onChanged = std::move(cb);
        return self<TabView>();
    }

    std::unique_ptr<IUiComponent> build() const override;

private:
    TabViewModel* m_vm{nullptr};
    QStringList m_tabs;
    int m_selectedIndex{0};
    UiTabView::IndicatorStyle m_indicatorStyle{UiTabView::IndicatorStyle::Bottom};
    int m_tabHeight{43};
    int m_animDuration{220};
    WidgetList m_contents;
    std::function<void(int)> m_onChanged;
};

} // namespace UI