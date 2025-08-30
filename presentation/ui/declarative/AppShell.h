#pragma once
#include "Binding.h"
#include "Layouts.h"
#include "Widget.h"
#include <functional>

namespace UI {

	// Declarative AppShell: Nav (left) + TopBar (top-right) + Content (bottom-right)
	// - Nav width is provided via a function so it can reflect runtime animation/VM state
	// - Content is hosted inside a BindingHost, allowing connectors to request rebuild
	class AppShell : public Widget {
	public:
		AppShell() = default;

		// Provide Nav/TopBar as Widgets (e.g., UI::wrap(runtimeComponent))
		std::shared_ptr<AppShell> nav(WidgetPtr w) { m_nav = std::move(w); return self<AppShell>(); }
		std::shared_ptr<AppShell> topBar(WidgetPtr w) { m_topBar = std::move(w); return self<AppShell>(); }

		// Provide a content builder that returns a Widget subtree each rebuild
		// Example: [this]{ return UI::wrap(pageRouter.currentPage()); }
		std::shared_ptr<AppShell> content(BindingHost::Builder b) { m_contentBuilder = std::move(b); return self<AppShell>(); }

		// Register BindingHost connectors, typically observing VM/Theme signals to call requestRebuild()
		std::shared_ptr<AppShell> connect(BindingHost::Connector c) { m_connectors.push_back(std::move(c)); return self<AppShell>(); }

		// Layout configuration
		std::shared_ptr<AppShell> navWidthProvider(std::function<int()> fn) { m_navWidthProvider = std::move(fn); return self<AppShell>(); }
		std::shared_ptr<AppShell> topBarHeight(int px) { m_topBarH = std::max(0, px); return self<AppShell>(); }

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_nav;
		WidgetPtr m_topBar;
		BindingHost::Builder m_contentBuilder; // produces content widget subtree
		std::vector<BindingHost::Connector> m_connectors;

		// Layout providers
		int m_topBarH{ 56 };
		std::function<int()> m_navWidthProvider = []() { return 200; };
	};

} // namespace UI