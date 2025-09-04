#include "AdvancedWidgets.h"
#include "Decorators.h"   // 新增
#include "UiComponent.hpp"
#include "UiListBox.h"    // 新增
#include "SimplePopup.h"  // 使用简化的弹出实现替代UiPopup.h
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

	// SimplePopupHost: 简化的弹出主机，替代复杂的PopupHost
	// 直接使用SimplePopup，避免延迟创建和复杂的资源依赖问题
	class SimplePopupHost : public IUiComponent, public IUiContent {
	public:
		struct Config {
			std::unique_ptr<IUiComponent> trigger;
			std::unique_ptr<IUiComponent> content;
			QSize popupSize{ 200, 150 };
			SimplePopup::Placement placement{ SimplePopup::Placement::Bottom };
			QPoint offset{ 0, 0 };
			QColor backgroundColor{ 255, 255, 255, 240 };
			float cornerRadius{ 8.0f };
			bool closeOnClickOutside{ true };
			std::function<void(bool)> onVisibilityChanged;
		};

		explicit SimplePopupHost(Config config, QWindow* parentWindow) 
			: m_config(std::move(config))
		{
			// 立即创建SimplePopup（不延迟）
			m_popup = std::make_unique<SimplePopup>(parentWindow);
			
			// 立即配置SimplePopup
			if (m_config.trigger) {
				m_popup->setTrigger(std::move(m_config.trigger));
			}
			
			if (m_config.content) {
				m_popup->setPopupContent(std::move(m_config.content));
			}
			
			// 应用配置
			m_popup->setPopupSize(m_config.popupSize);
			m_popup->setPlacement(m_config.placement);
			m_popup->setOffset(m_config.offset);
			m_popup->setBackgroundStyle(m_config.backgroundColor, m_config.cornerRadius);
			m_popup->setCloseOnClickOutside(m_config.closeOnClickOutside);
			
			if (m_config.onVisibilityChanged) {
				m_popup->setOnPopupVisibilityChanged(m_config.onVisibilityChanged);
			}
		}

		// IUiComponent - 直接委托给SimplePopup
		void updateLayout(const QSize& windowSize) override {
			if (m_popup) {
				m_popup->updateLayout(windowSize);
			}
		}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
			if (m_popup) {
				m_popup->updateResourceContext(cache, gl, devicePixelRatio);
			}
		}

		void append(Render::FrameData& fd) const override {
			if (m_popup) {
				m_popup->append(fd);
			}
		}

		bool onMousePress(const QPoint& pos) override {
			return m_popup ? m_popup->onMousePress(pos) : false;
		}

		bool onMouseMove(const QPoint& pos) override {
			return m_popup ? m_popup->onMouseMove(pos) : false;
		}

		bool onMouseRelease(const QPoint& pos) override {
			return m_popup ? m_popup->onMouseRelease(pos) : false;
		}

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
			return m_popup ? m_popup->onWheel(pos, angleDelta) : false;
		}

		bool tick() override {
			return m_popup ? m_popup->tick() : false;
		}

		QRect bounds() const override {
			return m_popup ? m_popup->bounds() : QRect();
		}

		void onThemeChanged(bool isDark) override {
			if (m_popup) {
				m_popup->onThemeChanged(isDark);
			}
		}

		// IUiContent
		void setViewportRect(const QRect& r) override {
			m_viewport = r;
			if (m_popup) {
				m_popup->setViewportRect(r);
			}
		}

		// 获取底层SimplePopup实例（供高级用法）
		SimplePopup* getPopup() const { return m_popup.get(); }

	private:
		Config m_config;
		std::unique_ptr<SimplePopup> m_popup;
		QRect m_viewport;
	};

	std::unique_ptr<IUiComponent> Popup::build() const
	{
		// 注意：由于我们需要父窗口来创建SimplePopupHost，
		// 但build()方法不接受参数，我们需要一个不同的方法
		// 这里暂时返回nullptr，实际应该使用buildWithWindow()方法
		return nullptr;
	}
	
	std::unique_ptr<IUiComponent> Popup::buildWithWindow(QWindow* parentWindow) const
	{
		// 准备配置
		SimplePopupHost::Config config;
		config.trigger = m_trigger ? m_trigger->build() : nullptr;
		config.content = m_content ? m_content->build() : nullptr;
		config.popupSize = m_popupSize;

		// 转换枚举类型
		switch (m_placement) {
		case Placement::Bottom:      config.placement = SimplePopup::Placement::Bottom; break;
		case Placement::Top:         config.placement = SimplePopup::Placement::Top; break;
		case Placement::Right:       config.placement = SimplePopup::Placement::Right; break;
		case Placement::Left:        config.placement = SimplePopup::Placement::Left; break;
		case Placement::BottomLeft:  config.placement = SimplePopup::Placement::BottomLeft; break;
		case Placement::BottomRight: config.placement = SimplePopup::Placement::BottomRight; break;
		case Placement::TopLeft:     config.placement = SimplePopup::Placement::TopLeft; break;
		case Placement::TopRight:    config.placement = SimplePopup::Placement::TopRight; break;
		case Placement::Custom:      config.placement = SimplePopup::Placement::Custom; break;
		}

		config.offset = m_offset;
		config.backgroundColor = m_backgroundColor;
		config.cornerRadius = m_cornerRadius;
		config.closeOnClickOutside = m_closeOnClickOutside;
		config.onVisibilityChanged = m_onVisibilityChanged;

		// 创建 SimplePopupHost 并应用装饰器
		auto host = std::make_unique<SimplePopupHost>(std::move(config), parentWindow);
		return decorate(std::move(host));
	}

	void Popup::configurePopupWindow(IUiComponent* component, QWindow* parentWindow)
	{
		// 尝试向下转换到SimplePopupHost（可能经过装饰器包装）
		// 注意：由于我们现在在构造时就传递了parentWindow，这个方法主要用于兼容性
		if (auto* host = dynamic_cast<SimplePopupHost*>(component)) {
			// SimplePopupHost在构造时已经设置了父窗口，这里不需要额外操作
			qDebug() << "Popup::configurePopupWindow: SimplePopupHost已经配置了父窗口";
		}
		// 如果组件被装饰器包装，可能需要额外的逻辑来访问内部的SimplePopupHost
		// 这里为了简单起见，假设用户直接传递了正确的组件
	}

} // namespace UI