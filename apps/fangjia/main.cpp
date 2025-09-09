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
#include "CompositionRoot.h"
#include "UnifiedDependencyProvider.h"
#include "DependencyMigrationTool.h"
#include "UnifiedDIUsageExample.h"
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

		// Critical fix: Enhanced OpenGL context configuration for NVIDIA driver compatibility
		QSurfaceFormat fmt;
		fmt.setDepthBufferSize(24);
		fmt.setStencilBufferSize(16);
		fmt.setVersion(3, 3);
		fmt.setProfile(QSurfaceFormat::CoreProfile);
		
		// Add additional NVIDIA driver compatibility settings
		fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
		fmt.setSwapInterval(1);  // Enable VSync to reduce driver issues
		fmt.setSamples(0);       // Disable multisampling to avoid driver conflicts
		fmt.setRenderableType(QSurfaceFormat::OpenGL);
		
		// Request debug context in debug builds for better error reporting
		#ifdef _DEBUG
		fmt.setOption(QSurfaceFormat::DebugContext);
		#endif
		
		QSurfaceFormat::setDefaultFormat(fmt);

		qDebug() << "Creating shared dependencies...";

		// 创建配置管理器并加载持久化设置
		auto config = std::make_shared<AppConfig>();
		config->load();

		// === Pure Boost.DI Configuration (Phase 4 Complete) ===
		// All services are now managed through CompositionRoot and Boost.DI
		auto& unifiedDeps = UnifiedDependencyProvider::instance();
		
		qDebug() << "Pure Boost.DI Dependency Provider initialized successfully";
		
		// === Get services from Boost.DI ===
		auto getThemeModeUseCase = unifiedDeps.get<domain::usecases::GetThemeModeUseCase>();
		auto setThemeModeUseCase = unifiedDeps.get<domain::usecases::SetThemeModeUseCase>();
		
		// === Migration Status Report ===
		// Phase 4 Complete: All services migrated to pure Boost.DI
		auto& migrationTool = DependencyMigrationTool::instance();
		auto migrationReport = migrationTool.generateMigrationReport();
		
		qDebug() << "DI Migration Status: Phase 4 Complete!" 
				 << migrationReport.migratedServices << "/" << migrationReport.totalServices 
				 << "services migrated (" << migrationReport.completionPercentage << "%)";

		// === Demonstrate Pure Boost.DI Usage ===
		// 展示纯Boost.DI依赖注入的使用模式
		UnifiedDIUsageExample example;
		example.demonstrateUnifiedAccess();
		example.demonstrateViewModelUsage();

		// 创建主题管理器（使用纯Boost.DI获取的服务）
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