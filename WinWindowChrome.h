#pragma once
#include <QtCore/qglobal.h> // 确保定义 Q_OS_WIN

#if defined(Q_OS_WIN) || defined(_WIN32)

#include <QAbstractNativeEventFilter>
#include <QList>
#include <QPointer>
#include <QRect>
#include <QWindow>
#include <functional>

class WinWindowChrome final : public QAbstractNativeEventFilter
{
public:
    // dragHeight: 认为是可拖拽“标题栏”的高度（逻辑像素）
    // noDragRectsProvider: 返回需要排除拖拽的客户区矩形（如自定义按钮、左侧导航），逻辑像素坐标
    static WinWindowChrome* attach(QWindow* win,
        int dragHeight,
        std::function<QList<QRect>()> noDragRectsProvider);
    ~WinWindowChrome() override;

    // 如无必要不要频繁调用（不要在 resizeGL/updateLayout 中反复调）
    void notifyLayoutChanged();

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    explicit WinWindowChrome(QWindow* win,
        int dragHeight,
        std::function<QList<QRect>()> noDragRectsProvider);

    void* hwnd() const;
    int   dpi() const;
    int   sysMetric(int index) const;
    int   sysMetricForDpi(int index, int dpiVal) const;
    int   resizeBorderThicknessX() const;
    int   resizeBorderThicknessY() const;

    qintptr hitTestNonClient(const QPoint& posLogical) const;

private:
    QPointer<QWindow> m_window;
    int m_dragHeightLogical{ 56 }; // 默认 56 逻辑像素
    std::function<QList<QRect>()> m_noDragRectsProvider;
};

#endif // Q_OS_WIN || _WIN32