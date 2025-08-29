/*
 * 文件名：main.cpp
 * 职责：应用程序入口点，负责初始化Qt应用、配置OpenGL上下文、创建并配置主窗口及其依赖服务。
 * 依赖：Qt6 Core/Gui/Widgets、AppConfig、ThemeManager、MainOpenGlWindow。
 * 线程：仅在主线程运行。
 * 备注：通过依赖注入模式组装应用程序根对象，避免全局状态。
 */

#include "AppConfig.h"
#include "MainOpenGlWindow.h"
#include "ThemeManager.h"
#include "SettingsRepository.h"
#include "usecases/GetSettingsUseCase.h"
#include "usecases/UpdateSettingsUseCase.h"
#include "usecases/ToggleThemeUseCase.h"
#include "usecases/GetThemeModeUseCase.h"
#include "usecases/SetThemeModeUseCase.h"
#include "usecases/GetRecentTabUseCase.h"
#include "usecases/SetRecentTabUseCase.h"
#include "DependencyProvider.h"
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
	/// 功能：将字符串转换为主题模式枚举
	/// 参数：s — 字符串标识符
	/// 返回：对应的主题模式，无效值时默认为跟随系统
	[[maybe_unused]] ThemeManager::ThemeMode stringToMode(const QString& s) {
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

		// 设置默认的OpenGL上下文参数
		QSurfaceFormat fmt;
		fmt.setDepthBufferSize(24);
		fmt.setStencilBufferSize(16);
		fmt.setVersion(3, 3);
		fmt.setProfile(QSurfaceFormat::CoreProfile);
		QSurfaceFormat::setDefaultFormat(fmt);

		qDebug() << "Creating shared dependencies...";

		// 创建配置管理器并加载持久化设置
		auto config = std::make_shared<AppConfig>();
		config->load();

		// === Domain/Data Layer Composition Root ===
		// 创建数据层仓储（适配器模式：包装AppConfig）
		auto settingsRepository = std::make_shared<data::repositories::SettingsRepository>(config);
		
		// 创建领域用例
		auto getSettingsUseCase = std::make_shared<domain::usecases::GetSettingsUseCase>(settingsRepository);
		auto updateSettingsUseCase = std::make_shared<domain::usecases::UpdateSettingsUseCase>(settingsRepository);
		auto toggleThemeUseCase = std::make_shared<domain::usecases::ToggleThemeUseCase>(settingsRepository);
		auto getThemeModeUseCase = std::make_shared<domain::usecases::GetThemeModeUseCase>(settingsRepository);
		auto setThemeModeUseCase = std::make_shared<domain::usecases::SetThemeModeUseCase>(settingsRepository);
		auto getRecentTabUseCase = std::make_shared<domain::usecases::GetRecentTabUseCase>(settingsRepository);
		auto setRecentTabUseCase = std::make_shared<domain::usecases::SetRecentTabUseCase>(settingsRepository);

		// 配置依赖提供者（临时服务定位器）
		auto& deps = DependencyProvider::instance();
		deps.setGetSettingsUseCase(getSettingsUseCase);
		deps.setUpdateSettingsUseCase(updateSettingsUseCase);
		deps.setToggleThemeUseCase(toggleThemeUseCase);
		deps.setGetThemeModeUseCase(getThemeModeUseCase);
		deps.setSetThemeModeUseCase(setThemeModeUseCase);
		deps.setGetRecentTabUseCase(getRecentTabUseCase);
		deps.setSetRecentTabUseCase(setRecentTabUseCase);

		// 创建主题管理器（使用依赖注入的新构造函数）
		const auto themeManager = std::make_shared<ThemeManager>(getThemeModeUseCase, setThemeModeUseCase);
		
		// 从设置中加载主题状态
		themeManager->load();

		// 连接主题变化信号到配置持久化（通过ThemeManager内置的save方法）
		QObject::connect(themeManager.get(), &ThemeManager::modeChanged,
			[themeManager](const ThemeManager::ThemeMode) {
				themeManager->save();
			});

		qDebug() << "Creating main window...";
		// 创建主窗口并注入依赖
		MainOpenGlWindow window(config, themeManager);

		// 从配置恢复窗口大小和位置
		QByteArray geo = config->windowGeometry();
		if (!geo.isEmpty() && geo.size() == sizeof(int) * 4) {
			const auto data = reinterpret_cast<const int*>(geo.data());
			window.setPosition(data[0], data[1]);
			window.resize(data[2], data[3]);
		}
		else {
			window.resize(1200, 760);
		}

		qDebug() << "Showing window...";
		window.show();

		const int result = QApplication::exec();

		qDebug() << "Cleaning up...";
		// 保存配置到持久化存储
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