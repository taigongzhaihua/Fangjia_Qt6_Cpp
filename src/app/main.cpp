#include "AppConfig.h"
#include "MainOpenGlWindow.h"
#include "ThemeManager.h"
#include <exception>
#include <memory>
#include <qapplication.h>
#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qlogging.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringliteral.h>
#include <qsurfaceformat.h>

namespace {
	// Helper function to convert ThemeMode to string
	QString modeToString(const ThemeManager::ThemeMode m) {
		switch (m) {
		case ThemeManager::ThemeMode::FollowSystem: return "system";
		case ThemeManager::ThemeMode::Light: return "light";
		case ThemeManager::ThemeMode::Dark: return "dark";
		}
		return "system";
	}

	// Helper function to convert string to ThemeMode
	ThemeManager::ThemeMode stringToMode(const QString& s) {
		const auto v = s.toLower();
		if (v == "light") return ThemeManager::ThemeMode::Light;
		if (v == "dark")  return ThemeManager::ThemeMode::Dark;
		return ThemeManager::ThemeMode::FollowSystem;
	}
}

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

		qDebug() << "Creating shared dependencies...";
		
		// 创建配置管理器
		auto config = std::make_shared<AppConfig>();
		config->load();

		// 创建主题管理器
		auto themeManager = std::make_shared<ThemeManager>();

		// 从配置中恢复主题设置
		const auto mode = config->themeMode();
		if (mode == "light") {
			themeManager->setMode(ThemeManager::ThemeMode::Light);
		}
		else if (mode == "dark") {
			themeManager->setMode(ThemeManager::ThemeMode::Dark);
		}
		else {
			themeManager->setMode(ThemeManager::ThemeMode::FollowSystem);
		}

		// 连接主题变化到配置保存
		QObject::connect(themeManager.get(), &ThemeManager::modeChanged,
			config.get(), [config](const ThemeManager::ThemeMode themeMode) {
				QString modeStr = modeToString(themeMode);
				config->setThemeMode(modeStr);
				config->save();
			});

		qDebug() << "Creating main window...";
		// 创建主窗口并注入依赖
		MainOpenGlWindow window(config, themeManager);

		// 从配置恢复窗口大小
		QByteArray geo = config->windowGeometry();
		if (!geo.isEmpty() && geo.size() == sizeof(int) * 4) {
			const int* data = reinterpret_cast<const int*>(geo.data());
			window.setPosition(data[0], data[1]);
			window.resize(data[2], data[3]);
		}
		else {
			window.resize(1200, 760);
		}

		qDebug() << "Showing window...";
		window.show();

		int result = QApplication::exec();

		qDebug() << "Cleaning up...";
		// 保存配置
		config->save();

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