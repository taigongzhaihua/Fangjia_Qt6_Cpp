#include "AdvancedWidgets.h"
#include "Decorators.h"   // 新增
#include "UiComponent.hpp"
#include "UiListBox.h"    // 新增
#include "Popup.h"        // 新增弹出组件

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

	// PopupTriggerComposite: Combines trigger and popup components
	class PopupTriggerComposite : public IUiComponent, public IUiContent 
	{
	public:
		PopupTriggerComposite(
			std::unique_ptr<IUiComponent> trigger,
			std::unique_ptr<::Popup> popup
		) : m_trigger(std::move(trigger)), m_popup(std::move(popup))
		{
		}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override
		{
			if (m_trigger) {
				m_trigger->updateResourceContext(cache, gl, devicePixelRatio);
			}
			if (m_popup) {
				m_popup->updateResourceContext(cache, gl, devicePixelRatio);
			}
		}

		void append(Render::FrameData& frameData) const override
		{
			if (m_trigger) {
				m_trigger->append(frameData);
			}
			// Popup content is rendered by overlay window, not here
		}

		bool onMousePress(const QPoint& pos) override
		{
			if (m_trigger && m_trigger->onMousePress(pos)) {
				// Toggle popup visibility based on trigger interaction
				if (m_popup->isPopupVisible()) {
					m_popup->hidePopup();
				} else {
					// Show popup at trigger position
					m_popup->showPopupAtPosition(m_trigger->bounds());
				}
				return true;
			}
			return false;
		}

		bool onMouseMove(const QPoint& pos) override
		{
			if (m_trigger) {
				return m_trigger->onMouseMove(pos);
			}
			return false;
		}

		bool onMouseRelease(const QPoint& pos) override
		{
			if (m_trigger) {
				return m_trigger->onMouseRelease(pos);
			}
			return false;
		}

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override
		{
			if (m_trigger) {
				return m_trigger->onWheel(pos, angleDelta);
			}
			return false;
		}

		bool tick() override
		{
			bool needsUpdate = false;
			if (m_trigger) {
				needsUpdate |= m_trigger->tick();
			}
			if (m_popup) {
				needsUpdate |= m_popup->tick();
			}
			return needsUpdate;
		}

		QRect bounds() const override
		{
			return m_trigger ? m_trigger->bounds() : QRect();
		}

		void onThemeChanged(bool isDark) override
		{
			if (m_trigger) {
				m_trigger->onThemeChanged(isDark);
			}
			if (m_popup) {
				m_popup->onThemeChanged(isDark);
			}
		}

		// IUiContent interface
		void setViewportRect(const QRect& rect) override
		{
			if (auto* triggerContent = dynamic_cast<IUiContent*>(m_trigger.get())) {
				triggerContent->setViewportRect(rect);
			}
			if (auto* popupContent = dynamic_cast<IUiContent*>(m_popup.get())) {
				popupContent->setViewportRect(rect);
			}
		}

		void updateLayout(const QSize& windowSize) override
		{
			if (m_trigger) {
				m_trigger->updateLayout(windowSize);
			}
			if (m_popup) {
				m_popup->updateLayout(windowSize);
			}
		}

	private:
		std::unique_ptr<IUiComponent> m_trigger;
		std::unique_ptr<::Popup> m_popup;
	};

	// 新弹出组件实现
	std::unique_ptr<IUiComponent> Popup::build() const
	{
		// build()方法不提供父窗口上下文，返回nullptr
		// 用户应该使用buildWithWindow()方法
		qWarning() << "Popup::build() called without window context. Use buildWithWindow() instead.";
		return nullptr;
	}

	std::unique_ptr<IUiComponent> Popup::buildWithWindow(QWindow* parentWindow) const
	{
		if (!parentWindow) {
			qWarning() << "Popup::buildWithWindow() called with null parent window";
			return nullptr;
		}

		// 创建新的弹出组件
		auto popup = std::make_unique<::Popup>(parentWindow);

		// 设置内容
		if (m_content) {
			popup->setContent(m_content->build());
		}

		// 配置弹出窗口
		popup->setPopupSize(m_popupSize);
		popup->setOffset(m_offset);
		popup->setBackgroundColor(m_backgroundColor);
		popup->setCornerRadius(m_cornerRadius);
		
		// 转换位置枚举
		switch (m_placement) {
		case Placement::Bottom:      popup->setPlacement(::Popup::Placement::Bottom); break;
		case Placement::Top:         popup->setPlacement(::Popup::Placement::Top); break;
		case Placement::Right:       popup->setPlacement(::Popup::Placement::Right); break;
		case Placement::Left:        popup->setPlacement(::Popup::Placement::Left); break;
		case Placement::BottomLeft:  popup->setPlacement(::Popup::Placement::BottomLeft); break;
		case Placement::BottomRight: popup->setPlacement(::Popup::Placement::BottomRight); break;
		case Placement::TopLeft:     popup->setPlacement(::Popup::Placement::TopLeft); break;
		case Placement::TopRight:    popup->setPlacement(::Popup::Placement::TopRight); break;
		case Placement::Center:      popup->setPlacement(::Popup::Placement::Center); break;
		}

		// 设置回调
		if (m_onVisibilityChanged) {
			popup->setOnVisibilityChanged(m_onVisibilityChanged);
		}

		// If no trigger is provided, return the popup directly
		if (!m_trigger) {
			return decorate(std::move(popup));
		}

		// Create composite that manages both trigger and popup
		auto composite = std::make_unique<PopupTriggerComposite>(
			m_trigger->build(),
			std::move(popup)
		);

		// 应用装饰器并返回
		return decorate(std::move(composite));
	}


} // namespace UI