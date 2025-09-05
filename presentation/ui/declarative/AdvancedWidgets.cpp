#include "AdvancedWidgets.h"
#include "Decorators.h"   // 新增
#include "UiComponent.hpp"
#include "UiListBox.h"    // 新增
#include "Popup.h"        // 新增弹出组件
#include "PopupWithAttachment.h"  // 新增带依附对象的弹出组件

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

	// 新弹出组件实现 - 只返回弹出窗口，无触发器逻辑
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

		// 检查是否有依附对象
		if (m_attachmentObject) {
			// 创建带依附对象支持的弹出组件
			auto popupWithAttachment = std::make_unique<PopupWithAttachment>(parentWindow);
			
			// 设置内容
			if (m_content) {
				popupWithAttachment->setContent(m_content->build());
			}
			
			// 设置依附对象
			auto attachmentComponent = m_attachmentObject->build();
			popupWithAttachment->setAttachmentObject(attachmentComponent.get());
			
			// 配置弹出窗口
			popupWithAttachment->setPopupSize(m_popupSize);
			popupWithAttachment->setOffset(m_offset);
			popupWithAttachment->setBackgroundColor(m_backgroundColor);
			popupWithAttachment->setCornerRadius(m_cornerRadius);
			
			// 转换位置枚举
			switch (m_placement) {
			case Placement::Bottom:      popupWithAttachment->setPlacement(::Popup::Placement::Bottom); break;
			case Placement::Top:         popupWithAttachment->setPlacement(::Popup::Placement::Top); break;
			case Placement::Right:       popupWithAttachment->setPlacement(::Popup::Placement::Right); break;
			case Placement::Left:        popupWithAttachment->setPlacement(::Popup::Placement::Left); break;
			case Placement::BottomLeft:  popupWithAttachment->setPlacement(::Popup::Placement::BottomLeft); break;
			case Placement::BottomRight: popupWithAttachment->setPlacement(::Popup::Placement::BottomRight); break;
			case Placement::TopLeft:     popupWithAttachment->setPlacement(::Popup::Placement::TopLeft); break;
			case Placement::TopRight:    popupWithAttachment->setPlacement(::Popup::Placement::TopRight); break;
			case Placement::Center:      popupWithAttachment->setPlacement(::Popup::Placement::Center); break;
			}

			// 设置回调
			if (m_onVisibilityChanged) {
				popupWithAttachment->setOnVisibilityChanged(m_onVisibilityChanged);
			}

			// 返回带依附对象支持的弹出组件
			return decorate(std::move(popupWithAttachment));
		}

		// 创建基础弹出组件（无依附对象）
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

		// 直接返回弹出窗口组件，无触发器逻辑
		return decorate(std::move(popup));
	}


} // namespace UI