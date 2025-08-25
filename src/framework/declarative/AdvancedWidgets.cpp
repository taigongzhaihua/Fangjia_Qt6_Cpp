#include "AdvancedWidgets.h"
#include "UiBoxLayout.h"

namespace UI {

std::unique_ptr<IUiComponent> ListTile::build() const {
    auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
    layout->setSpacing(12);
    
    // Leading
    if (m_leading) {
        layout->addChild(m_leading->build().release(), 0.0f, UiBoxLayout::Alignment::Center);
    }
    
    // Title和Subtitle
    if (m_title || m_subtitle) {
        auto contentLayout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
        contentLayout->setSpacing(4);
        
        if (m_title) {
            contentLayout->addChild(m_title->build().release(), 0.0f, UiBoxLayout::Alignment::Start);
        }
        
        if (m_subtitle) {
            contentLayout->addChild(m_subtitle->build().release(), 0.0f, UiBoxLayout::Alignment::Start);
        }
        
        layout->addChild(contentLayout.release(), 1.0f, UiBoxLayout::Alignment::Stretch);
    }
    
    // Trailing
    if (m_trailing) {
        layout->addChild(m_trailing->build().release(), 0.0f, UiBoxLayout::Alignment::Center);
    }
    
    return layout;
}

std::unique_ptr<IUiComponent> TabBar::build() const {
    auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
    
    // 简化实现：创建一行按钮
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        auto container = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
        
        if (m_tabs[i].icon) {
            container->addChild(m_tabs[i].icon->build().release(), 0.0f);
        }
        
        // 这里需要创建文本组件，暂时跳过
        
        layout->addChild(container.release(), 1.0f);
    }
    
    return layout;
}

} // namespace UI