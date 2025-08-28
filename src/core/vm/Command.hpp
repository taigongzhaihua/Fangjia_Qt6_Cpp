#pragma once
#include <functional>
#include <qobject.h>

// 通用命令封装，用于 View 触发 ViewModel 动作
// 支持 execute/canExecute 模式，便于 UI 绑定
class Command : public QObject
{
    Q_OBJECT

public:
    using ExecuteFunc = std::function<void()>;
    using CanExecuteFunc = std::function<bool()>;

    explicit Command(QObject* parent = nullptr) : QObject(parent) {}
    
    Command(ExecuteFunc executeFunc, QObject* parent = nullptr)
        : QObject(parent), m_executeFunc(std::move(executeFunc)) {}
    
    Command(ExecuteFunc executeFunc, CanExecuteFunc canExecuteFunc, QObject* parent = nullptr)
        : QObject(parent), m_executeFunc(std::move(executeFunc)), m_canExecuteFunc(std::move(canExecuteFunc)) {}

    ~Command() override = default;

    // 禁用拷贝，确保命令的唯一性
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    // 执行命令
    void execute() {
        if (canExecute() && m_executeFunc) {
            m_executeFunc();
        }
    }

    // 检查是否可执行
    bool canExecute() const {
        return m_canExecuteFunc ? m_canExecuteFunc() : true;
    }

    // 设置执行函数
    void setExecuteFunction(ExecuteFunc func) {
        m_executeFunc = std::move(func);
    }

    // 设置可执行检查函数
    void setCanExecuteFunction(CanExecuteFunc func) {
        m_canExecuteFunc = std::move(func);
        emit canExecuteChanged();
    }

    // 手动触发可执行状态变更通知（当外部条件改变时调用）
    void raiseCanExecuteChanged() {
        emit canExecuteChanged();
    }

signals:
    // 可执行状态变更通知，UI 可绑定此信号来更新按钮状态等
    void canExecuteChanged();

private:
    ExecuteFunc m_executeFunc;
    CanExecuteFunc m_canExecuteFunc;
};