#include "PageRouter.h"
#include <qlogging.h>

void PageRouter::registerPage(const QString& id, Factory factory)
{
    if (!factory) return;
    m_factories[id] = std::move(factory);
    qDebug() << "PageRouter: Registered page factory" << id;
}

UiPage* PageRouter::getPage(const QString& id)
{
    // 如果页面已缓存，直接返回
    auto pageIt = m_pages.find(id);
    if (pageIt != m_pages.end()) {
        return pageIt->second.get();
    }
    
    // 查找工厂并创建页面
    auto factoryIt = m_factories.find(id);
    if (factoryIt == m_factories.end()) {
        qWarning() << "PageRouter: No factory found for page" << id;
        return nullptr;
    }
    
    // 懒加载：通过工厂创建页面实例
    auto page = factoryIt->second();
    if (!page) {
        qWarning() << "PageRouter: Factory returned null for page" << id;
        return nullptr;
    }
    
    UiPage* result = page.get();
    m_pages[id] = std::move(page);
    qDebug() << "PageRouter: Created page instance" << id;
    return result;
}

bool PageRouter::switchToPage(const QString& id)
{
    // 获取或创建目标页面
    UiPage* newPage = getPage(id);
    if (!newPage) {
        qWarning() << "PageRouter: Failed to get page" << id;
        return false;
    }
    
    // 调用当前页面的 onDisappear 钩子
    if (m_currentPage && m_currentPage != newPage) {
        m_currentPage->onDisappear();
    }
    
    // 更新当前页面
    m_currentPage = newPage;
    m_currentPageId = id;
    
    // 调用新页面的 onAppear 钩子
    m_currentPage->onAppear();
    
    qDebug() << "PageRouter: Switched to page" << id;
    return true;
}

void PageRouter::clear()
{
    // 调用当前页面的 onDisappear 钩子
    if (m_currentPage) {
        m_currentPage->onDisappear();
    }
    
    m_factories.clear();
    m_pages.clear();
    m_currentPage = nullptr;
    m_currentPageId.clear();
}