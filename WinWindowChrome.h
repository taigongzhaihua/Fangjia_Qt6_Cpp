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
	static WinWindowChrome* attach(QWindow* win,
		int dragHeight,
		std::function<QList<QRect>()> noDragRectsProvider);
	~WinWindowChrome() override;

	// 从 qApp 中移除事件过滤器；可重复调用，幂等
	void detach();

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
	void* m_hwnd{ nullptr };       // 缓存 HWND，避免销毁期调用 winId()
	int m_dragHeightLogical{ 56 }; // 默认 56 逻辑像素
	std::function<QList<QRect>()> m_noDragRectsProvider;
	bool m_detached{ false };      // 已从 qApp 移除
};

#endif // Q_OS_WIN || _WIN32