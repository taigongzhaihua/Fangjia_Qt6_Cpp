#include "UiListBox.h"
#include <algorithm>
#include <cmath>
#include <qfont.h>
#include <qfontmetrics.h>
#include "RenderData.hpp"
#include "RenderUtils.hpp"

UiListBox::UiListBox()
{
	// Initialize with default dark/light theme palette
	m_palette = {
		.bg = QColor(255, 255, 255),
		.itemHover = QColor(0, 0, 0, 10),
		.itemPressed = QColor(0, 0, 0, 20),
		.itemSelected = QColor(0, 122, 255, 30),
		.textPrimary = QColor(0, 0, 0),
		.textSecondary = QColor(100, 100, 100),
		.border = QColor(200, 200, 200)
	};
}

void UiListBox::setItems(const std::vector<ListItem>& items)
{
	m_items = items;
	updateLayout();
}

void UiListBox::addItem(const ListItem& item)
{
	m_items.push_back(item);
	updateLayout();
}

void UiListBox::removeItem(int index)
{
	if (index >= 0 && index < static_cast<int>(m_items.size())) {
		m_items.erase(m_items.begin() + index);
		if (m_selectedIndex == index) {
			m_selectedIndex = -1;
		} else if (m_selectedIndex > index) {
			m_selectedIndex--;
		}
		updateLayout();
	}
}

void UiListBox::clearItems()
{
	m_items.clear();
	m_selectedIndex = -1;
	m_hoveredIndex = -1;
	m_pressedIndex = -1;
	updateLayout();
}

void UiListBox::setSelectedIndex(int index)
{
	if (index >= -1 && index < static_cast<int>(m_items.size())) {
		m_selectedIndex = index;
	}
}

void UiListBox::updateLayout(const QSize& /*windowSize*/)
{
	updateLayout();
}

void UiListBox::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio)
{
	m_cache = &cache;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);
}

void UiListBox::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || !m_cache || !m_gl) return;

	// Apply clipping to viewport bounds
	const QRectF clipRect = QRectF(m_viewport);

	// Draw main background using RoundedRectCmd (CORRECT!)
	if (m_palette.bg.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = m_cornerRadius,
			.color = m_palette.bg,
			.clipRect = clipRect
		});
	}

	// Draw border using RoundedRectCmd (CORRECT!)
	if (m_borderWidth > 0.0f && m_palette.border.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = m_cornerRadius,
			.color = m_palette.border,
			.clipRect = clipRect
		});
		
		// Draw inner area (background inside border)
		const QRectF innerRect = QRectF(m_viewport).adjusted(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth);
		if (innerRect.isValid() && m_palette.bg.alpha() > 0) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = innerRect,
				.radiusPx = std::max(0.0f, m_cornerRadius - m_borderWidth),
				.color = m_palette.bg,
				.clipRect = clipRect
			});
		}
	}

	// Draw each list item
	for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
		const QRect itemRect = getItemRect(i);
		if (!itemRect.intersects(m_viewport)) continue;
		
		const auto& item = m_items[i];
		
		// Draw item background using RoundedRectCmd (CORRECT!)
		QColor bgColor = QColor(0, 0, 0, 0); // Transparent by default
		if (i == m_selectedIndex) {
			bgColor = m_palette.itemSelected;
		} else if (i == m_pressedIndex) {
			bgColor = m_palette.itemPressed;
		} else if (i == m_hoveredIndex) {
			bgColor = m_palette.itemHover;
		}
		
		if (bgColor.alpha() > 0) {
			const QRectF itemRectF = QRectF(itemRect).adjusted(2, 1, -2, -1); // Small padding
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = itemRectF,
				.radiusPx = 3.0f, // Small corner radius for items
				.color = bgColor,
				.clipRect = clipRect
			});
		}

		// Draw text using IconCache text texture approach (CORRECT!)
		const int fontPx = std::lround(14.0f * m_dpr);
		QFont font;
		font.setPixelSize(fontPx);

		// Generate stable cache key for text
		const QString cacheKey = RenderUtils::makeTextCacheKey(item.text, fontPx, item.textColor);
		
		// Generate text texture
		const int tex = m_cache->ensureTextPx(cacheKey, font, item.text, item.textColor, m_gl);
		const QSize texSize = m_cache->textureSizePx(tex);

		// Convert to logical pixels
		const float wLogical = static_cast<float>(texSize.width()) / m_dpr;
		const float hLogical = static_cast<float>(texSize.height()) / m_dpr;

		// Position text with padding
		const float textX = itemRect.x() + 8.0f;
		const float textY = itemRect.center().y() - hLogical * 0.5f;

		// Create destination rectangle for text
		const QRectF textDst(textX, textY, wLogical, hLogical);

		// Add text as image command with clipping to component bounds (CORRECT!)
		fd.images.push_back(Render::ImageCmd{
			.dstRect = textDst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, texSize.width(), texSize.height()),
			.tint = QColor(255, 255, 255, 255), // White tint (texture already has color)
			.clipRect = clipRect // Clip to component bounds
		});
	}
}

void UiListBox::updateLayout()
{
	// Calculate total content height
	// Layout logic here
}

QRect UiListBox::getItemRect(int index) const
{
	if (index < 0 || index >= static_cast<int>(m_items.size())) {
		return QRect();
	}
	
	const int y = m_viewport.y() + index * m_itemHeight - m_scrollOffset;
	return QRect(m_viewport.x(), y, m_viewport.width(), m_itemHeight);
}

int UiListBox::getItemAtPosition(const QPoint& pos) const
{
	if (!m_viewport.contains(pos)) return -1;
	
	for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
		if (getItemRect(i).contains(pos)) {
			return i;
		}
	}
	return -1;
}

bool UiListBox::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;
	
	m_pressedIndex = getItemAtPosition(pos);
	return m_pressedIndex >= 0;
}

bool UiListBox::onMouseMove(const QPoint& pos)
{
	const int newHovered = m_viewport.contains(pos) ? getItemAtPosition(pos) : -1;
	const bool changed = (newHovered != m_hoveredIndex);
	m_hoveredIndex = newHovered;
	return changed;
}

bool UiListBox::onMouseRelease(const QPoint& pos)
{
	const int wasPressed = m_pressedIndex;
	m_pressedIndex = -1;
	
	if (wasPressed >= 0 && m_viewport.contains(pos)) {
		const int clickedIndex = getItemAtPosition(pos);
		if (clickedIndex == wasPressed) {
			setSelectedIndex(clickedIndex);
			if (m_onItemClicked) {
				m_onItemClicked(clickedIndex);
			}
		}
	}
	
	return wasPressed >= 0;
}

bool UiListBox::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	if (!m_viewport.contains(pos)) return false;
	
	const int scrollDelta = -(angleDelta.y() * 32) / 120;
	const int maxScroll = std::max(0, static_cast<int>(m_items.size()) * m_itemHeight - m_viewport.height());
	m_scrollOffset = std::clamp(m_scrollOffset + scrollDelta, 0, maxScroll);
	
	return maxScroll > 0;
}

bool UiListBox::tick()
{
	return false;
}

void UiListBox::applyTheme(bool isDark)
{
	m_isDark = isDark;
	
	if (isDark) {
		m_palette = {
			.bg = QColor(30, 30, 30),
			.itemHover = QColor(255, 255, 255, 10),
			.itemPressed = QColor(255, 255, 255, 20),
			.itemSelected = QColor(0, 122, 255, 30),
			.textPrimary = QColor(255, 255, 255),
			.textSecondary = QColor(180, 180, 180),
			.border = QColor(80, 80, 80)
		};
	} else {
		m_palette = {
			.bg = QColor(255, 255, 255),
			.itemHover = QColor(0, 0, 0, 10),
			.itemPressed = QColor(0, 0, 0, 20),
			.itemSelected = QColor(0, 122, 255, 30),
			.textPrimary = QColor(0, 0, 0),
			.textSecondary = QColor(100, 100, 100),
			.border = QColor(200, 200, 200)
		};
	}
}