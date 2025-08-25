#include "AppConfig.h"
#include "MainOpenGlWindow.h"
#include "ServiceRegistry.h"
#include <exception>
#include <qapplication.h>
#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qlogging.h>
#include <qstringliteral.h>
#include <qsurfaceformat.h>
#include <ServiceLocator.h>

int main(int argc, char* argv[])
{
	try {
		QApplication app(argc, argv);

		QCoreApplication::setOrganizationName(QStringLiteral("TaiGongZhaiHua"));
		QCoreApplication::setOrganizationDomain(QStringLiteral("Fangjia.com"));
		QCoreApplication::setApplicationName(QStringLiteral("Fangjia"));

		// 设置默认的 OpenGL 上下文参数
		QSurfaceFormat fmt;
		fmt.setDepthBufferSize(24);
		fmt.setStencilBufferSize(16);
		fmt.setVersion(3, 3);
		fmt.setProfile(QSurfaceFormat::CoreProfile);
		QSurfaceFormat::setDefaultFormat(fmt);

		qDebug() << "Registering services...";
		// 注册所有服务（DI）
		ServiceRegistry::registerAll();

		qDebug() << "Creating main window...";
		// 创建主窗口
		MainOpenGlWindow window;

		// 从配置恢复窗口大小
		if (auto config = DI.get<AppConfig>()) {
			// QOpenGLWindow 没有 restoreGeometry，我们只恢复大小
			// 可以保存 x, y, width, height 分别
			QByteArray geo = config->windowGeometry();
			if (!geo.isEmpty() && geo.size() == sizeof(int) * 4) {
				const int* data = reinterpret_cast<const int*>(geo.data());
				window.setPosition(data[0], data[1]);
				window.resize(data[2], data[3]);
			}
			else {
				window.resize(1200, 760);
			}
		}
		else {
			window.resize(1200, 760);
		}

		qDebug() << "Showing window...";
		window.show();

		int result = QApplication::exec();

		qDebug() << "Cleaning up...";
		// 清理服务
		ServiceRegistry::cleanup();

		return result;
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in main:" << e.what();
		return -1;
	}
	catch (...) {
		qCritical() << "Unknown exception in main";
		return -1;
	}
}