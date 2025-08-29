/*
 * 文件名：Binding.h
 * 职责：声明式UI绑定系统，提供"数据变化即重建"的响应式编程模型。
 * 依赖：RebuildHost、Widget基类、Qt信号机制。
 * 线程：仅在UI线程使用。
 * 备注：基于观察者模式，支持ViewModel信号绑定到UI重建的自动化响应。
 */

#pragma once
#include "RebuildHost.h"
#include "Widget.h"
#include <functional>
#include <memory>
#include <QObject>
#include <vector>

namespace UI {

	/// 功能：通用信号观察辅助函数
	/// 参数：obj — 信号发射对象
	/// 参数：sig — 信号成员函数指针
	/// 参数：fn — 响应回调函数
	/// 返回：Qt信号连接对象，用于后续断开连接
	/// 使用示例：observe(vm, &VmType::dataChanged, [host]{ host->requestRebuild(); });
	template<typename Obj, typename Signal, typename Fn>
	inline QMetaObject::Connection observe(Obj* obj, Signal sig, Fn fn) {
		return QObject::connect(obj, sig, std::move(fn));
	}

	/// 绑定宿主：基于RebuildHost的"变化即重建"声明式UI容器
	/// 
	/// 功能：
	/// - 响应式UI：数据变化自动触发子树重建
	/// - 信号绑定：简化ViewModel到UI的连接代码
	/// - 声明式语法：支持流式API配置绑定关系
	/// 
	/// 使用模式：
	/// 1. 提供Builder函数生成UI子树
	/// 2. 通过Connector函数配置信号绑定
	/// 3. 数据变化时自动重建UI内容
	class BindingHost : public Widget {
	public:
		using Builder = std::function<WidgetPtr()>;            // UI构建函数：返回声明式子树
		using Connector = std::function<void(UI::RebuildHost*)>; // 连接器函数：配置信号绑定

		explicit BindingHost(Builder b) : m_builder(std::move(b)) {}

		/// 功能：注册信号连接器
		/// 参数：c — 连接器函数，接收RebuildHost指针用于配置信号绑定
		/// 返回：当前BindingHost实例（支持链式调用）
		/// 说明：在连接器中使用observe()函数连接ViewModel信号到requestRebuild()
		std::shared_ptr<BindingHost> connect(Connector c) {
			m_connectors.push_back(std::move(c));
			return self<BindingHost>();
		}

		std::unique_ptr<IUiComponent> build() const override {
			// 1) 构建一个可重建宿主
			auto host = std::make_unique<UI::RebuildHost>();
			// 2) 设置 builder：外部 Builder 产生 WidgetPtr，再 build 成 IUiComponent
			host->setBuilder([b = m_builder]() -> std::unique_ptr<IUiComponent> {
				if (!b) return {};
				if (const WidgetPtr w = b()) return w->build();
				return {};
				});
			// 3) 首次立即构建
			host->requestRebuild();
			// 4) 注册所有连接器（外部在连接器里可使用 observe(...) 订阅 VM 信号）
			for (const auto& fn : m_connectors) if (fn) fn(host.get());
			// 5) 支持统一装饰
			return decorate(std::move(host));
		}

	private:
		Builder m_builder;
		std::vector<Connector> m_connectors;
	};

	// 便捷工厂
	inline std::shared_ptr<BindingHost> bindingHost(BindingHost::Builder b) {
		return make_widget<BindingHost>(std::move(b));
	}

} // namespace UI