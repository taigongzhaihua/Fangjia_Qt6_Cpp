#pragma once
#include "UiPage.h"
#include <functional>
#include <memory>
#include <qstring.h>
#include <unordered_map>

// 页面路由器：支持工厂模式和惰性页面创建
class PageRouter final
{
public:
    // 页面工厂类型定义
    using PageFactory = std::function<std::unique_ptr<UiPage>()>;

    PageRouter() = default;
    ~PageRouter() = default;

    // 注册页面工厂
    void registerPageFactory(const QString& id, PageFactory factory);
    
    // 获取页面（惰性创建）
    UiPage* getPage(const QString& id);
    
    // 获取当前页面
    UiPage* currentPage() const { return m_currentPage; }
    
    // 切换页面（惰性创建）
    bool switchToPage(const QString& id);
    
    // 清理所有页面和工厂
    void clear();

    // 检查是否已注册页面工厂
    bool hasPageFactory(const QString& id) const;

    // 检查页面是否已创建
    bool isPageCreated(const QString& id) const;

    // 获取当前页面ID
    QString currentPageId() const { return m_currentPageId; }

private:
    // 页面工厂映射：id -> 工厂函数
    std::unordered_map<QString, PageFactory> m_factories;
    
    // 已创建的页面缓存：id -> 页面实例
    std::unordered_map<QString, std::unique_ptr<UiPage>> m_pages;
    
    // 当前页面状态
    UiPage* m_currentPage{ nullptr };
    QString m_currentPageId;
};