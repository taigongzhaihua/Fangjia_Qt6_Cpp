#include "PageRouter.h"
#include <qlogging.h>

void PageRouter::registerPageFactory(const QString& id, PageFactory factory)
{
    if (!factory) {
        qWarning() << "PageRouter: Cannot register null factory for id:" << id;
        return;
    }
    
    m_factories[id] = std::move(factory);
    qDebug() << "PageRouter: Registered page factory for" << id;
}

UiPage* PageRouter::getPage(const QString& id)
{
    // 先检查已创建的页面缓存
    auto pageIt = m_pages.find(id);
    if (pageIt != m_pages.end()) {
        return pageIt->second.get();
    }
    
    // 如果页面尚未创建，查找工厂并创建
    auto factoryIt = m_factories.find(id);
    if (factoryIt == m_factories.end()) {
        qWarning() << "PageRouter: No factory found for page id:" << id;
        return nullptr;
    }
    
    try {
        auto page = factoryIt->second();
        if (!page) {
            qWarning() << "PageRouter: Factory returned null page for id:" << id;
            return nullptr;
        }
        
        UiPage* pagePtr = page.get();
        m_pages[id] = std::move(page);
        qDebug() << "PageRouter: Created page for" << id;
        return pagePtr;
    }
    catch (const std::exception& e) {
        qCritical() << "PageRouter: Exception creating page for id" << id << ":" << e.what();
        return nullptr;
    }
}

bool PageRouter::switchToPage(const QString& id)
{
    UiPage* page = getPage(id);
    if (!page) {
        qWarning() << "PageRouter: Failed to switch to page:" << id;
        return false;
    }
    
    m_currentPage = page;
    m_currentPageId = id;
    qDebug() << "PageRouter: Switched to page" << id;
    return true;
}

void PageRouter::clear()
{
    m_pages.clear();
    m_factories.clear();
    m_currentPage = nullptr;
    m_currentPageId.clear();
    qDebug() << "PageRouter: Cleared all pages and factories";
}

bool PageRouter::hasPageFactory(const QString& id) const
{
    return m_factories.find(id) != m_factories.end();
}

bool PageRouter::isPageCreated(const QString& id) const
{
    return m_pages.find(id) != m_pages.end();
}