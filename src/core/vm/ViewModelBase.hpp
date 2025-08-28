#pragma once
#include <qobject.h>

// 轻量 ViewModel 基类，统一 VM 的继承入口
// 提供通用的 ViewModel 基础功能和约定
class ViewModelBase : public QObject
{
    Q_OBJECT

public:
    explicit ViewModelBase(QObject* parent = nullptr) : QObject(parent) {}
    ~ViewModelBase() override = default;

    // 禁用拷贝和移动，确保 ViewModel 实例的唯一性
    ViewModelBase(const ViewModelBase&) = delete;
    ViewModelBase& operator=(const ViewModelBase&) = delete;
    ViewModelBase(ViewModelBase&&) = delete;
    ViewModelBase& operator=(ViewModelBase&&) = delete;

protected:
    // 预留给子类的生命周期钩子（可用于日志、资源管理等）
    virtual void onInitialized() {}
    virtual void onDestroying() {}

private:
    // 预留给框架的扩展点：
    // - 属性变更通知统一处理
    // - 命令状态同步
    // - 数据绑定生命周期管理
    // - 日志注入等
};