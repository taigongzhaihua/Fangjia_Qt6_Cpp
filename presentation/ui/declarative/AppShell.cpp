#include "AppShell.h"
#include "Binding.h"
#include "Layouts.h"
#include "UI.h"

#include <algorithm>

namespace UI {

    std::unique_ptr<IUiComponent> AppShell::build() const {
        // Build content host via BindingHost so it can rebuild on VM changes
        WidgetPtr contentHost;
        if (m_contentBuilder) {
            auto host = UI::bindingHost(m_contentBuilder);
            for (const auto& c : m_connectors) if (c) host->connect(c);
            contentHost = host;
        }

        // Resolve sizes
        const int navW = std::max(0, m_navWidthProvider ? m_navWidthProvider() : 200);
        const int topH = std::max(0, m_topBarH);

        // Assemble Grid layout
        auto g = UI::grid()
            ->rows({ Grid::Track::Px(topH), 1.0_fr })
            ->columns({ Grid::Track::Px(navW), 1.0_fr })
            ->rowSpacing(0)
            ->colSpacing(0);

        if (m_nav)    g->add(m_nav,    /*row*/0, /*col*/0, /*rowSpan*/2, /*colSpan*/1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch);
        if (m_topBar) g->add(m_topBar, /*row*/0, /*col*/1, /*rowSpan*/1, /*colSpan*/1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch);
        if (contentHost) g->add(contentHost, /*row*/1, /*col*/1, /*rowSpan*/1, /*colSpan*/1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch);

        // Apply outer decorations if any (padding/bg/border...) and return
        return g->build();
    }

} // namespace UI