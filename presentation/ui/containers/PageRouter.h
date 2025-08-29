#pragma once
#include "UiPage.h"
#include <functional>
#include <memory>
#include <qstring.h>
#include <unordered_map>

// 页面路由：集中管理页面工厂与生命周期，支持懒加载
class PageRouter final
{
public:
    using Factory = std::function<std::unique_ptr<UiPage>()>;

    PageRouter() = default;
    ~PageRouter() = default;

    // 注册页面工厂
    void registerPage(const QString& id, Factory factory);
    
    // 获取页面（如果不存在则通过工厂创建）
    UiPage* getPage(const QString& id);
    
    // 获取当前页面
    UiPage* currentPage() const { return m_currentPage; }
    
    // 切换页面（支持生命周期钩子）
    bool switchToPage(const QString& id);
    
    // 清理所有页面和工厂
    void clear();

private:
    std::unordered_map<QString, Factory> m_factories;
    std::unordered_map<QString, std::unique_ptr<UiPage>> m_pages;
    UiPage* m_currentPage{ nullptr };
    QString m_currentPageId;
};