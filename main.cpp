#include "MainOpenGlWindow.h"
#include <qapplication.h>
#include <qcoreapplication.h>
#include <qstringliteral.h>
#include <qsurfaceformat.h>

// 新增：演示注入

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QCoreApplication::setOrganizationName(QStringLiteral("Fangjia"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("example.com"));
	QCoreApplication::setApplicationName(QStringLiteral("Fangjia_Qt6_Cpp"));

	// 设置默认的 OpenGL 上下文参数（可按需调整）
	QSurfaceFormat fmt;
	fmt.setDepthBufferSize(24);
	fmt.setStencilBufferSize(16);
	fmt.setVersion(3, 3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(fmt);

	MainOpenGlWindow window;
	window.resize(800, 600);
	window.show();

	return QApplication::exec();
}