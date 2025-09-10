/*
 * 文件名：main.cpp
 * 职责：应用程序入口点，使用派生的App类替代直接使用QApplication。
 * 依赖：FangjiaApp（业务应用程序类）。
 * 线程：仅在主线程运行。
 * 备注：使用基础Application类的派生实现，提高封装性和代码复用性。
 */

#include "FangjiaApp.hpp"
#include <exception>
#include <qlogging.h>

int main(int argc, char* argv[])
{
	try {
		// 创建房价应用程序实例
		FangjiaApp app(argc, argv);
		
		// 运行应用程序
		return app.run();
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