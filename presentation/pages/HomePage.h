#pragma once
#include "UiPage.h"
#include <memory>
#include <QObject>

// 简单的计数器 ViewModel 用于演示绑定功能
class CounterViewModel : public QObject
{
    Q_OBJECT
public:
    explicit CounterViewModel(QObject* parent = nullptr);
    
    void increment();
    void decrement();
    int count() const { return m_count; }
    
signals:
    void countChanged();
    
private:
    int m_count{0};
};

class HomePage : public UiPage
{
public:
    HomePage();
    ~HomePage() override;

protected:
    void initializeContent() override;
    void applyPageTheme(bool isDark) override;

    // 页面生命周期钩子
    void onAppear() override;
    void onDisappear() override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};