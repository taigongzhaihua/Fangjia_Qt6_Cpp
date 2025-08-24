#pragma once
#include "UiPage.h"
#include <memory>
#include <qstring.h>
#include <unordered_map>

// 页面管理器：集中管理所有页面实例
class PageManager final
{
public:
    PageManager() = default;
    ~PageManager() = default;

    // 注册页面
    void registerPage(const QString& id, std::unique_ptr<UiPage> page);
    
    // 获取页面
    UiPage* getPage(const QString& id) const;
    
    // 获取当前页面
    UiPage* currentPage() const { return m_currentPage; }
    
    // 切换页面
    bool switchToPage(const QString& id);
    
    // 清理所有页面
    void clear();

private:
    std::unordered_map<QString, std::unique_ptr<UiPage>> m_pages;
    UiPage* m_currentPage{ nullptr };
    QString m_currentPageId;
};