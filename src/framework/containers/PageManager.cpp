#include "PageManager.h"
#include <qlogging.h>

void PageManager::registerPage(const QString& id, std::unique_ptr<UiPage> page)
{
    if (!page) return;
    m_pages[id] = std::move(page);
    qDebug() << "PageManager: Registered page" << id;
}

UiPage* PageManager::getPage(const QString& id) const
{
    auto it = m_pages.find(id);
    return (it != m_pages.end()) ? it->second.get() : nullptr;
}

bool PageManager::switchToPage(const QString& id)
{
    auto it = m_pages.find(id);
    if (it == m_pages.end()) {
        qWarning() << "PageManager: Page not found:" << id;
        return false;
    }
    
    m_currentPage = it->second.get();
    m_currentPageId = id;
    qDebug() << "PageManager: Switched to page" << id;
    return true;
}

void PageManager::clear()
{
    m_pages.clear();
    m_currentPage = nullptr;
    m_currentPageId.clear();
}