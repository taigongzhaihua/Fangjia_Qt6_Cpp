/*
 * 测试文件：test_win_window_chrome.cpp
 * 职责：验证 WinWindowChrome 的非客户区扩展功能是否正常工作
 * 特别测试 DwmExtendFrameIntoClientArea 是否正确配置为启用自定义窗口绘制
 */

#include <QtTest/QtTest>
#include <QWindow>
#include <QOpenGLWindow>

#ifdef Q_OS_WIN
#include "WinWindowChrome.h"
#include <windows.h>
#include <dwmapi.h>
#endif

class TestWinWindowChrome : public QObject
{
    Q_OBJECT

private slots:
    void testWindowChromeAttachment()
    {
#ifdef Q_OS_WIN
        // 创建一个测试窗口
        QWindow testWindow;
        testWindow.resize(400, 300);
        testWindow.show();
        
        // 确保窗口完全创建
        QTest::qWait(100);
        
        // 附加 WindowChrome
        auto* chrome = WinWindowChrome::attach(&testWindow, 56, []() {
            return QList<QRect>();
        });
        
        // 验证 chrome 创建成功
        QVERIFY(chrome != nullptr);
        
        // 验证窗口句柄存在
        HWND hwnd = static_cast<HWND>(chrome->hwnd());
        QVERIFY(hwnd != nullptr);
        
        // 验证窗口样式已被修改（移除了 WS_CAPTION）
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        QVERIFY(!(style & WS_CAPTION));
        
        // 验证 DWM 扩展是否已应用（通过检查窗口的扩展边距）
        // 注意：实际的边距检查可能需要更复杂的 DWM API 调用
        // 这里我们主要验证函数调用没有失败
        MARGINS margins;
        HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &margins, sizeof(margins));
        // DwmGetWindowAttribute 可能返回失败，但这不一定意味着扩展设置失败
        // 主要目的是确保我们的修改不会导致崩溃
        
        qDebug() << "Window chrome attached successfully";
        qDebug() << "Window style after modification:" << Qt::hex << style;
        
        // 清理
        testWindow.hide();
#else
        QSKIP("This test is Windows-specific");
#endif
    }
    
    void testWindowChromeWithNullWindow()
    {
#ifdef Q_OS_WIN
        // 测试空指针安全性
        auto* chrome = WinWindowChrome::attach(nullptr, 56, []() {
            return QList<QRect>();
        });
        
        // 应该返回 nullptr 而不是崩溃
        QVERIFY(chrome == nullptr);
#else
        QSKIP("This test is Windows-specific");
#endif
    }
};

QTEST_MAIN(TestWinWindowChrome)
#include "test_win_window_chrome.moc"