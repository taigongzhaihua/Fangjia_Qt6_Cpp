#include "UiListBox.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <IconCache.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qfont.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
#include "RenderData.hpp"
#include "RenderUtils.hpp"

UiListBox::UiListBox() 
{
	m_animClock.start();
	updateDefaultPalette(false); // 默认浅色主题
}

void UiListBox::setItems(const std::vector<QString>& items)
{
	m_items = items;
	// 调整选中索引确保有效性
	if (m_selectedIndex >= static_cast<int>(m_items.size())) {
		m_selectedIndex = m_items.empty() ? -1 : 0;
	}
	reloadData();
}

void UiListBox::setSelectedIndex(int index)
{
	const int oldIndex = m_selectedIndex;
	m_selectedIndex = (index >= 0 && index < static_cast<int>(m_items.size())) ? index : -1;
	
	// 如果使用ModelFns，同步到模型
	if (m_modelFns.setSelectedIndex && m_selectedIndex != oldIndex) {
		m_modelFns.setSelectedIndex(m_selectedIndex);
	}
}

void UiListBox::reloadData()
{
	updateVisibleItems();
}

void UiListBox::updateVisibleItems()
{
	m_visibleItems.clear();
	
	// 获取项目列表：优先使用ModelFns，fallback到直接设置的items
	std::vector<QString> currentItems;
	if (m_modelFns.items) {
		const auto qvectorItems = m_modelFns.items();
		currentItems.assign(qvectorItems.begin(), qvectorItems.end());
		// 同步选中状态
		if (m_modelFns.selectedIndex) {
			m_selectedIndex = m_modelFns.selectedIndex();
		}
	} else {
		currentItems = m_items;
	}

	// 计算可见项目
	for (int i = 0; i < static_cast<int>(currentItems.size()); ++i) {
		const int y = i * m_itemHeight - m_scrollY;
		
		// 仅添加在视口范围内的项目
		if (y + m_itemHeight > 0 && y < m_viewport.height()) {
			VisibleItem item;
			item.index = i;
			item.rect = QRect(
				m_viewport.left(),
				m_viewport.top() + y,
				m_viewport.width(),
				m_itemHeight
			);
			m_visibleItems.push_back(item);
		}
	}
}

int UiListBox::contentHeight() const
{
	int itemCount = static_cast<int>(m_items.size());
	if (m_modelFns.items) {
		itemCount = m_modelFns.items().size();
	}
	return itemCount * m_itemHeight;
}

int UiListBox::hitTestItem(const QPoint& pos) const
{
	for (const auto& item : m_visibleItems) {
		if (item.rect.contains(pos)) {
			return item.index;
		}
	}
	return -1;
}

void UiListBox::updateDefaultPalette(bool isDark)
{
	if (isDark) {
		m_pal = {
			.bg = QColor(30, 30, 30, 255),
			.itemHover = QColor(255, 255, 255, 20),
			.itemPressed = QColor(255, 255, 255, 40),
			.itemSelected = QColor(0, 102, 204, 80),
			.textPrimary = QColor(240, 240, 240, 255),
			.textSecondary = QColor(180, 180, 180, 200),
			.separator = QColor(255, 255, 255, 20),
			.indicator = QColor(0, 102, 204, 220)
		};
	} else {
		m_pal = {
			.bg = QColor(255, 255, 255, 255),
			.itemHover = QColor(0, 0, 0, 15),
			.itemPressed = QColor(0, 0, 0, 30),
			.itemSelected = QColor(0, 102, 204, 30),
			.textPrimary = QColor(32, 38, 46, 255),
			.textSecondary = QColor(100, 110, 120, 200),
			.separator = QColor(0, 0, 0, 20),
			.indicator = QColor(0, 102, 204, 220)
		};
	}
}

QSize UiListBox::measure(const SizeConstraints& cs)
{
	// 计算内容所需高度
	const int contentH = contentHeight();
	
	// 返回合理的默认宽度和实际内容高度
	const int width = std::clamp(200, cs.minW, cs.maxW);
	const int height = std::clamp(contentH, cs.minH, cs.maxH);
	
	return QSize(width, height);
}

void UiListBox::arrange(const QRect& finalRect)
{
	setViewportRect(finalRect);
}

void UiListBox::updateLayout(const QSize& /*windowSize*/)
{
	updateVisibleItems();
}

void UiListBox::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_cache = &cache;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);
}

void UiListBox::append(Render::FrameData& fd) const
{
	// 绘制背景
	fd.roundedRects.push_back(Render::RoundedRectCmd{
		.rect = QRectF(m_viewport),
		.radiusPx = 0.0f,
		.color = m_pal.bg,
		.clipRect = QRectF(m_viewport)
	});

	// 获取当前项目列表
	std::vector<QString> currentItems;
	if (m_modelFns.items) {
		const auto qvectorItems = m_modelFns.items();
		currentItems.assign(qvectorItems.begin(), qvectorItems.end());
	} else {
		currentItems = m_items;
	}

	// 绘制可见项目
	for (const auto& visibleItem : m_visibleItems) {
		const int index = visibleItem.index;
		if (index < 0 || index >= static_cast<int>(currentItems.size())) continue;

		const QRect& itemRect = visibleItem.rect;
		
		// 绘制项目背景
		QColor itemBg = m_pal.bg;
		if (index == m_selectedIndex) {
			itemBg = m_pal.itemSelected;
		} else if (index == m_pressedIndex) {
			itemBg = m_pal.itemPressed;
		} else if (index == m_hoverIndex) {
			itemBg = m_pal.itemHover;
		}
		
		if (itemBg != m_pal.bg) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(itemRect),
				.radiusPx = 0.0f,
				.color = itemBg,
				.clipRect = QRectF(m_viewport)
			});
		}

		// 绘制选中指示器
		if (index == m_selectedIndex) {
			const QRect indicatorRect(itemRect.left(), itemRect.top(), 3, itemRect.height());
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(indicatorRect),
				.radiusPx = 0.0f,
				.color = m_pal.indicator,
				.clipRect = QRectF(m_viewport)
			});
		}

		// 绘制文本
		const QString& itemText = currentItems[index];
		const QRect textRect = itemRect.adjusted(12, 0, -8, 0); // 左边距12px，右边距8px
		
		// Guard on m_cache && m_gl before creating textures
		if (m_cache && m_gl && !itemText.isEmpty()) {
			// Use QFont with logical size derived from item height and device pixel ratio
			QFont font;
			const int logicalFontSize = std::max(10, m_itemHeight - 22); // default 14 for itemHeight 36
			const int fontPx = std::lround(static_cast<float>(logicalFontSize) * m_dpr);
			font.setPixelSize(fontPx);
			
			// Generate cache key with text, font size, and color
			const QString cacheKey = RenderUtils::makeTextCacheKey(itemText, fontPx, m_pal.textPrimary);
			
			// Create text texture
			const int textTex = m_cache->ensureTextPx(cacheKey, font, itemText, m_pal.textPrimary, m_gl);
			const QSize texSize = m_cache->textureSizePx(textTex);
			
			// Compute logical size from texture size
			const float wLogical = static_cast<float>(texSize.width()) / m_dpr;
			const float hLogical = static_cast<float>(texSize.height()) / m_dpr;
			
			// Place text left-aligned vertically centered within textRect
			const float textX = static_cast<float>(textRect.left());
			const float textY = static_cast<float>(textRect.center().y()) - hLogical * 0.5f;
			const QRectF textDst(textX, textY, wLogical, hLogical);
			
			// Push ImageCmd with proper clipping
			fd.images.push_back(Render::ImageCmd{
				.dstRect = textDst,
				.textureId = textTex,
				.srcRectPx = QRectF(0, 0, texSize.width(), texSize.height()),
				.tint = QColor(255, 255, 255, 255), // White tint since texture is pre-colored
				.clipRect = QRectF(textRect)
			});
		}

		// 绘制分隔线（除了最后一项）
		if (index < static_cast<int>(currentItems.size()) - 1) {
			const QRect separatorRect(itemRect.left() + 8, itemRect.bottom() - 1, itemRect.width() - 16, 1);
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(separatorRect),
				.radiusPx = 0.0f,
				.color = m_pal.separator,
				.clipRect = QRectF(m_viewport)
			});
		}
	}
}

bool UiListBox::onMousePress(const QPoint& pos)
{
	const int hitIndex = hitTestItem(pos);
	if (hitIndex >= 0) {
		m_pressedIndex = hitIndex;
		return true;
	}
	return false;
}

bool UiListBox::onMouseMove(const QPoint& pos)
{
	const int hitIndex = hitTestItem(pos);
	const int oldHover = m_hoverIndex;
	m_hoverIndex = hitIndex;
	return oldHover != m_hoverIndex;
}

bool UiListBox::onMouseRelease(const QPoint& pos)
{
	const bool wasPressed = (m_pressedIndex >= 0);
	const int hitIndex = hitTestItem(pos);
	
	if (wasPressed && hitIndex == m_pressedIndex && hitIndex >= 0) {
		// 单击选择
		setSelectedIndex(hitIndex);
		
		// 触发激活回调
		if (m_onActivated) {
			m_onActivated(hitIndex);
		} else if (m_modelFns.onActivated) {
			m_modelFns.onActivated(hitIndex);
		}
	}
	
	m_pressedIndex = -1;
	return wasPressed;
}

bool UiListBox::onWheel(const QPoint& /*pos*/, const QPoint& angleDelta)
{
	// 滚轮事件通常由外层UiScrollView处理
	// 这里可以实现基本的滚动逻辑作为fallback
	const int delta = -angleDelta.y() / 8; // 标准滚轮单位
	const int oldScrollY = m_scrollY;
	const int maxScroll = std::max(0, contentHeight() - m_viewport.height());
	
	m_scrollY = std::clamp(m_scrollY + delta, 0, maxScroll);
	
	if (m_scrollY != oldScrollY) {
		updateVisibleItems();
		return true;
	}
	return false;
}

bool UiListBox::tick()
{
	// 目前无动画需求，返回false
	return false;
}

void UiListBox::onThemeChanged(bool isDark)
{
	updateDefaultPalette(isDark);
}