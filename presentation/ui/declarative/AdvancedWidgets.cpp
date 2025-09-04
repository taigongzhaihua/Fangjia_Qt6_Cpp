#include "AdvancedWidgets.h"
#include "Decorators.h"   // 新增
#include "UiComponent.hpp"
#include "UiListBox.h"    // 新增
#include "UiPopup.h"  // 新增
#include <algorithm>
#include <functional>
#include <IconCache.h>
#include <memory>
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qwindow.h>
#include <RenderData.hpp>
#include <UiContent.hpp>
#include <utility>

namespace UI {

	std::unique_ptr<IUiComponent> Card::build() const
	{
		// 先构建内部内容
		std::unique_ptr<IUiComponent> inner = m_child ? m_child->build() : std::unique_ptr<IUiComponent>{};

		// 用 DecoratedBox 承担 Card 的背景/边框/圆角/内边距，并支持按主题切换
		DecoratedBox::Props p;
		p.padding = m_pal.padding;

		// 亮/暗主题背景
		p.useThemeBg = true;
		p.bgLight = m_pal.bgLight;
		p.bgDark = m_pal.bgDark;
		p.bgRadius = m_pal.radius;

		// 亮/暗主题边框
		if (m_pal.borderLight.alpha() > 0 || m_pal.borderDark.alpha() > 0) {
			p.useThemeBorder = true;
			p.borderLight = m_pal.borderLight;
			p.borderDark = m_pal.borderDark;
			p.borderW = std::max(0.0f, m_pal.borderW);
			p.borderRadius = m_pal.radius;
		}

		// 将 elevation 映射为阴影效果
		if (m_elevation > 0.0f) {
			p.useShadow = true;
			p.shadowColor = QColor(100, 100, 100, static_cast<int>(std::clamp(10 + m_elevation * 5, 15.0f, 60.0f))); // 降低阴影透明度：30-120 范围，更加透明
			p.shadowBlurPx = std::clamp(m_elevation * 2.0f, 2.0f, 24.0f);  // 模糊半径：elevation * 2，范围 2-24px
			p.shadowOffset = QPoint(0, static_cast<int>(std::clamp(m_elevation * 0.5f, 1.0f, 8.0f))); // Y 偏移：elevation * 0.5，范围 1-8px
			p.shadowSpreadPx = std::clamp(m_elevation * 0.25f, 0.0f, 4.0f); // 扩展：elevation * 0.25，范围 0-4px
		}

		// 透传用户在基类 Widget 上设置的 size / margin / 可见性 / 透明度 / 交互
		// 注意：如果用户同时在 Card 上设置了 background/border（旧 API），这里优先使用主题化配置
		p.fixedSize = m_decorations.fixedSize;
		p.margin = m_decorations.margin;
		p.visible = m_decorations.isVisible;
		p.opacity = m_decorations.opacity;
		p.onTap = m_decorations.onTap;
		p.onHover = m_decorations.onHover;

		return std::make_unique<DecoratedBox>(std::move(inner), std::move(p));
	}

	std::unique_ptr<IUiComponent> ListBox::build() const
	{
		auto listBox = std::make_unique<UiListBox>();

		// 应用配置
		listBox->setItems(m_items);
		listBox->setItemHeight(m_itemHeight);
		listBox->setSelectedIndex(m_selectedIndex);

		if (m_onActivated) {
			listBox->setOnActivated(m_onActivated);
		}

		// 应用声明式装饰器（padding, margin, background等）
		return decorate(std::move(listBox));
	}

	// PopupHost: 内部组件，管理UiPopup的延迟创建和生命周期
	// 注意：需要通过setParentWindow()设置父窗口才能正常工作
	class PopupHost : public IUiComponent, public IUiContent {
	public:
		struct Config {
			std::unique_ptr<IUiComponent> trigger;
			std::unique_ptr<IUiComponent> content;
			QSize popupSize{ 200, 150 };
			UiPopup::Placement placement{ UiPopup::Placement::Bottom };
			QPoint offset{ 0, 0 };
			QColor backgroundColor{ 255, 255, 255, 240 };
			float cornerRadius{ 8.0f };
			bool closeOnClickOutside{ true };
			std::function<void(bool)> onVisibilityChanged;
		};

		explicit PopupHost(Config config) : m_config(std::move(config)) {}

		// IUiComponent
		void updateLayout(const QSize& windowSize) override {
			if (m_popup) {
				m_popup->updateLayout(windowSize);
			}
			else {
				// 如果弹出窗口尚未创建，更新触发器布局
				if (m_config.trigger) {
					m_config.trigger->updateLayout(windowSize);
				}
				// 尝试创建弹出窗口
				tryCreatePopup();
			}
		}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
			m_cache = &cache;
			m_gl = gl;
			m_dpr = devicePixelRatio;

			// 尝试创建弹出窗口（如果还没有创建）
			tryCreatePopup();

			if (m_popup) {
				m_popup->updateResourceContext(cache, gl, devicePixelRatio);
			}
			else {
				// 如果弹出窗口尚未创建，更新触发器资源上下文
				if (m_config.trigger) {
					m_config.trigger->updateResourceContext(cache, gl, devicePixelRatio);
				}
			}
		}

		void append(Render::FrameData& fd) const override {
			if (m_popup) {
				m_popup->append(fd);
			}

			// 如果弹出窗口尚未创建，可以绘制占位符或触发器
			if (m_config.trigger) {
				m_config.trigger->append(fd);
			}

		}

		bool onMousePress(const QPoint& pos) override {
			if (m_popup) {
				return m_popup->onMousePress(pos);
			}
			// 如果弹出窗口尚未创建，将鼠标事件转发给触发器
			else if (m_config.trigger) {
				return m_config.trigger->onMousePress(pos);
			}
			return false;
		}

		bool onMouseMove(const QPoint& pos) override {
			if (m_popup) {
				return m_popup->onMouseMove(pos);
			}
			// 如果弹出窗口尚未创建，将鼠标事件转发给触发器
			else if (m_config.trigger) {
				return m_config.trigger->onMouseMove(pos);
			}
			return false;
		}

		bool onMouseRelease(const QPoint& pos) override {
			if (m_popup) {
				return m_popup->onMouseRelease(pos);
			}
			// 如果弹出窗口尚未创建，将鼠标事件转发给触发器
			else if (m_config.trigger) {
				bool handled = m_config.trigger->onMouseRelease(pos);
				// 当触发器被点击时，尝试创建并显示弹出窗口
				if (handled) {
					tryCreatePopup();
					if (m_popup) {
						// 触发器被点击，显示弹出窗口
						// 这里可以添加弹出窗口的显示逻辑
						qDebug() << "PopupHost: 触发器被点击，弹出窗口已创建";
					}
				}
				return handled;
			}
			return false;
		}

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
			return m_popup ? m_popup->onWheel(pos, angleDelta) : false;
		}

		bool tick() override {
			if (m_popup) {
				return m_popup->tick();
			}
			else if (m_config.trigger) {
				return m_config.trigger->tick();
			}
			return false;
		}

		QRect bounds() const override {
			return m_popup ? m_popup->bounds() : m_viewport;
		}

		void onThemeChanged(bool isDark) override {
			if (m_popup) {
				m_popup->onThemeChanged(isDark);
			}
			else if (m_config.trigger) {
				m_config.trigger->onThemeChanged(isDark);
			}
		}

		// IUiContent
		void setViewportRect(const QRect& r) override {
			m_viewport = r;
			if (m_popup) {
				m_popup->setViewportRect(r);
			}
			else if (m_config.trigger) {
				// 如果弹出窗口尚未创建，设置触发器视口（如果触发器实现了IUiContent接口）
				if (auto* triggerContent = dynamic_cast<IUiContent*>(m_config.trigger.get())) {
					triggerContent->setViewportRect(r);
				}
			}
		}

		// 设置父窗口引用（必须由应用程序代码调用）
		void setParentWindow(QWindow* window) {
			m_parentWindow = window;
			tryCreatePopup();
		}

		// 获取底层UiPopup实例（供高级用法）
		UiPopup* getPopup() const { return m_popup.get(); }

	private:
		void tryCreatePopup() {
			// 只有同时具备窗口上下文和资源上下文时才创建
			if (m_popup || !m_parentWindow || !m_cache || !m_gl) {
				return;
			}

			createPopup();
		}

		void createPopup() {
			m_popup = std::make_unique<UiPopup>(m_parentWindow);

			// 设置触发器
			if (m_config.trigger) {
				m_popup->setTrigger(m_config.trigger.get());
			}

			// 设置内容
			if (m_config.content) {
				m_popup->setPopupContent(m_config.content.get());
			}

			// 配置属性
			m_popup->setPopupSize(m_config.popupSize);
			m_popup->setPlacement(m_config.placement);
			m_popup->setOffset(m_config.offset);
			m_popup->setPopupStyle(m_config.backgroundColor, m_config.cornerRadius);
			m_popup->setCloseOnClickOutside(m_config.closeOnClickOutside);

			if (m_config.onVisibilityChanged) {
				m_popup->setOnPopupVisibilityChanged(m_config.onVisibilityChanged);
			}

			// 设置视口
			if (!m_viewport.isEmpty()) {
				m_popup->setViewportRect(m_viewport);
			}

			// 立即更新资源上下文
			m_popup->updateResourceContext(*m_cache, m_gl, m_dpr);
		}

	private:
		Config m_config;
		std::unique_ptr<UiPopup> m_popup;
		QWindow* m_parentWindow{ nullptr };
		QRect m_viewport;

		// 缓存的资源上下文
		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

	std::unique_ptr<IUiComponent> Popup::build() const
	{
		// 准备配置
		PopupHost::Config config;
		config.trigger = m_trigger ? m_trigger->build() : nullptr;
		config.content = m_content ? m_content->build() : nullptr;
		config.popupSize = m_popupSize;

		// 转换枚举类型
		switch (m_placement) {
		case Placement::Bottom:      config.placement = UiPopup::Placement::Bottom; break;
		case Placement::Top:         config.placement = UiPopup::Placement::Top; break;
		case Placement::Right:       config.placement = UiPopup::Placement::Right; break;
		case Placement::Left:        config.placement = UiPopup::Placement::Left; break;
		case Placement::BottomLeft:  config.placement = UiPopup::Placement::BottomLeft; break;
		case Placement::BottomRight: config.placement = UiPopup::Placement::BottomRight; break;
		case Placement::TopLeft:     config.placement = UiPopup::Placement::TopLeft; break;
		case Placement::TopRight:    config.placement = UiPopup::Placement::TopRight; break;
		case Placement::Custom:      config.placement = UiPopup::Placement::Custom; break;
		}

		config.offset = m_offset;
		config.backgroundColor = m_backgroundColor;
		config.cornerRadius = m_cornerRadius;
		config.closeOnClickOutside = m_closeOnClickOutside;
		config.onVisibilityChanged = m_onVisibilityChanged;

		// 创建 PopupHost 并应用装饰器
		auto host = std::make_unique<PopupHost>(std::move(config));
		return decorate(std::move(host));
	}

	void Popup::configurePopupWindow(IUiComponent* component, QWindow* parentWindow)
	{
		// 尝试向下转换到PopupHost（可能经过装饰器包装）
		// 这是一个简化的实现，实际中可能需要更复杂的遍历逻辑
		if (auto* host = dynamic_cast<PopupHost*>(component)) {
			host->setParentWindow(parentWindow);
		}
		// 如果组件被装饰器包装，可能需要额外的逻辑来访问内部的PopupHost
		// 这里为了简单起见，假设用户直接传递了正确的组件
	}

} // namespace UI