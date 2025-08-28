#pragma once
#include "RebuildHost.h"
#include "Widget.h"
#include <functional>
#include <memory>
#include <QObject>
#include <vector>

namespace UI {

	// 通用观察辅助：连接任意 QObject 信号
	// 用法：observe(vm, &VmType::someSignal, [host]{ host->requestRebuild(); });
	template<typename Obj, typename Signal, typename Fn>
	inline QMetaObject::Connection observe(Obj* obj, Signal sig, Fn fn) {
		return QObject::connect(obj, sig, std::move(fn));
	}

	// 声明式 BindingHost：基于 RebuildHost 的"变化即重建"封装
	class BindingHost : public Widget {
	public:
		using Builder = std::function<WidgetPtr()>;            // 返回一个声明式子树（WidgetPtr）
		using Connector = std::function<void(UI::RebuildHost*)>; // 在此回调里完成信号连接

		explicit BindingHost(Builder b) : m_builder(std::move(b)) {}

		// 注册一个连接器：接收 RebuildHost*，由调用方完成信号->requestRebuild 的连接
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
				if (WidgetPtr w = b()) return w->build();
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


} // namespace UI