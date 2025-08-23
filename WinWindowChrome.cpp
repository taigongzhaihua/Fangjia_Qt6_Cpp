#include "WinWindowChrome.h"

#if defined(Q_OS_WIN) || defined(_WIN32)

#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <QGuiApplication>
#include <QPoint>
#include <QRect>
#include <algorithm>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

static inline bool isMaximized(HWND h) { return IsZoomed(h); }

WinWindowChrome* WinWindowChrome::attach(QWindow* win,
	const int dragHeight,
	std::function<QList<QRect>()> noDragRectsProvider)
{
	if (!win) return nullptr;
	win->winId(); // 确保原生句柄创建

	auto* chrome = new WinWindowChrome(win, dragHeight, std::move(noDragRectsProvider));
	qApp->installNativeEventFilter(chrome);

	if (HWND h = reinterpret_cast<HWND>(chrome->hwnd())) {
		const MARGINS m{ 0, 0, 0, 0 };
		DwmExtendFrameIntoClientArea(h, &m); // 保持 DWM 阴影
	}
	chrome->notifyLayoutChanged();
	return chrome;
}

WinWindowChrome::WinWindowChrome(QWindow* win,
	const int dragHeight,
	std::function<QList<QRect>()> noDragRectsProvider)
	: m_window(win),
	m_dragHeightLogical(std::max(24, dragHeight)),
	m_noDragRectsProvider(std::move(noDragRectsProvider))
{
}

WinWindowChrome::~WinWindowChrome()
{
	if (qApp) qApp->removeNativeEventFilter(this);
}

void* WinWindowChrome::hwnd() const
{
	if (!m_window) return nullptr;
	return reinterpret_cast<void*>(m_window->winId());
}

int WinWindowChrome::dpi() const
{
	if (HWND h = reinterpret_cast<HWND>(hwnd())) {
		static auto pGetDpiForWindow = reinterpret_cast<UINT(WINAPI*)(HWND)>(
			GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
		if (pGetDpiForWindow) return static_cast<int>(pGetDpiForWindow(h));
	}
	return 96;
}

int WinWindowChrome::sysMetric(const int index) const
{
	return GetSystemMetrics(index);
}

int WinWindowChrome::sysMetricForDpi(const int index, const int dpiVal) const
{
	static auto pGetSystemMetricsForDpi = reinterpret_cast<int (WINAPI*)(int, UINT)>(
		GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetSystemMetricsForDpi"));
	if (pGetSystemMetricsForDpi) return pGetSystemMetricsForDpi(index, dpiVal);
	return GetSystemMetrics(index);
}

int WinWindowChrome::resizeBorderThicknessX() const
{
	const int d = dpi();
	const int frame = sysMetricForDpi(SM_CXSIZEFRAME, d);
	const int pad = sysMetricForDpi(SM_CXPADDEDBORDER, d);
	return std::max(1, frame + pad);
}

int WinWindowChrome::resizeBorderThicknessY() const
{
	const int d = dpi();
	const int frame = sysMetricForDpi(SM_CYSIZEFRAME, d);
	const int pad = sysMetricForDpi(SM_CXPADDEDBORDER, d);
	return std::max(1, frame + pad);
}

void WinWindowChrome::notifyLayoutChanged()
{
	if (HWND h = reinterpret_cast<HWND>(hwnd())) {
		// 注意：不要在连续 resize 时调用，否则会闪烁
		SetWindowPos(h, nullptr, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

qintptr WinWindowChrome::hitTestNonClient(const QPoint& posLogical) const
{
	if (!m_window) return HTCLIENT;

	const QSize sz = m_window->size();
	if (sz.isEmpty()) return HTCLIENT;

	const int borderX = resizeBorderThicknessX();
	const int borderY = resizeBorderThicknessY();

	const int w = sz.width();
	const int h = sz.height();

	// 边框命中（优先）
	const bool left = posLogical.x() < borderX;
	const bool right = posLogical.x() >= (w - borderX);
	const bool top = posLogical.y() < borderY;
	const bool bottom = posLogical.y() >= (h - borderY);

	if (top && left)     return HTTOPLEFT;
	if (top && right)    return HTTOPRIGHT;
	if (bottom && left)  return HTBOTTOMLEFT;
	if (bottom && right) return HTBOTTOMRIGHT;
	if (left)            return HTLEFT;
	if (right)           return HTRIGHT;
	if (top)             return HTTOP;
	if (bottom)          return HTBOTTOM;

	// 顶部拖拽区（减去排除区）
	const int dragH = std::max(24, m_dragHeightLogical);
	if (posLogical.y() < dragH) {
		if (m_noDragRectsProvider) {
			const auto rects = m_noDragRectsProvider();
			for (const auto& r : rects) {
				if (r.contains(posLogical)) return HTCLIENT;
			}
		}
		return HTCAPTION;
	}

	return HTCLIENT;
}

bool WinWindowChrome::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
	if (eventType != "windows_generic_MSG" || !m_window) return false;

	MSG* msg = reinterpret_cast<MSG*>(message);
	if (msg->hwnd != reinterpret_cast<HWND>(hwnd())) return false;

	const UINT uMsg = msg->message;

	LRESULT dwmResult = 0;
	if (DwmDefWindowProc(msg->hwnd, uMsg, msg->wParam, msg->lParam, &dwmResult)) {
		if (result) *result = dwmResult;
		return true;
	}

	switch (uMsg) {
	case WM_NCCALCSIZE: {
		// 目标：吞掉系统标题栏，但保留“可调整边框”的厚度，确保阴影/系统缩放存在
		if (msg->wParam) {
			auto* p = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
			RECT& r = p->rgrc[0];

			if (isMaximized(msg->hwnd)) {
				// 最大化时对齐工作区，避免覆盖/顶到自动隐藏任务栏
				MONITORINFO mi{ sizeof(MONITORINFO) };
				if (GetMonitorInfo(MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST), &mi)) {
					r = mi.rcWork;
				}
			}
			else {
				const int bx = resizeBorderThicknessX();
				const int by = resizeBorderThicknessY();
				r.left += bx;
				r.right -= bx;
				r.top += 1;  // 只保留 sizeframe 顶边，标题栏部分由我们自绘
				r.bottom -= by;
			}
			if (result) *result = 0;
			return true;
		}
		break;
	}
	case WM_NCHITTEST: {
		POINT ptScreen{ GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam) };
		POINT ptClient = ptScreen;
		ScreenToClient(msg->hwnd, &ptClient);
		const QPoint posLogical(ptClient.x, ptClient.y);
		if (result) *result = hitTestNonClient(posLogical);
		return true;
	}
	case WM_GETMINMAXINFO: {
		// 修正最大化尺寸/位置
		auto* mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
		HMONITOR mon = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi{ sizeof(MONITORINFO) };
		if (GetMonitorInfo(mon, &mi)) {
			const RECT& rcWork = mi.rcWork;
			const RECT& rcMon = mi.rcMonitor;
			mmi->ptMaxSize.x = rcWork.right - rcWork.left;
			mmi->ptMaxSize.y = rcWork.bottom - rcWork.top;
			mmi->ptMaxPosition.x = rcWork.left - rcMon.left;
			mmi->ptMaxPosition.y = rcWork.top - rcMon.top;
			if (result) *result = 0;
			return true;
		}
		break;
	}
	case WM_ERASEBKGND:
		// 避免 GDI 擦背景与 GL 清屏叠加引起闪烁
		if (result) *result = 1;
		return true;

	case WM_NCACTIVATE:
		// 允许默认处理（避免闪烁）
		return false;

	default:
		break;
	}

	return false;
}

#endif // Q_OS_WIN || _WIN32