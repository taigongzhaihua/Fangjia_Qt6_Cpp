#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <functional>
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <qstring.h>
#include <vector>

class UiListBox : public IUiComponent, public IUiContent
{
public:
	// List item data structure
	struct ListItem {
		QString text;
		QColor textColor = QColor(0, 0, 0);
		QColor backgroundColor = QColor(255, 255, 255);
		bool enabled = true;
	};

	// Styling configuration
	struct Palette {
		QColor bg;              // Background color
		QColor itemHover;       // Hover background
		QColor itemPressed;     // Pressed background
		QColor itemSelected;    // Selected background
		QColor textPrimary;     // Primary text color
		QColor textSecondary;   // Secondary text color
		QColor border;          // Border color
	};

	UiListBox();
	~UiListBox() override = default;

	// Data management
	void setItems(const std::vector<ListItem>& items);
	void addItem(const ListItem& item);
	void removeItem(int index);
	void clearItems();

	// Selection
	void setSelectedIndex(int index);
	int selectedIndex() const { return m_selectedIndex; }

	// Styling
	void setPalette(const Palette& palette) { m_palette = palette; }
	void setItemHeight(int height) { m_itemHeight = height; }
	void setBorderWidth(float width) { m_borderWidth = width; }
	void setCornerRadius(float radius) { m_cornerRadius = radius; }

	// Callbacks
	void setOnItemClicked(std::function<void(int)> callback) { m_onItemClicked = callback; }

	// IUiContent
	void setViewportRect(const QRect& r) override { m_viewport = r; updateLayout(); }

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
	bool tick() override;
	QRect bounds() const override { return m_viewport; }
	void applyTheme(bool isDark) override;

private:
	void updateLayout();
	QRect getItemRect(int index) const;
	int getItemAtPosition(const QPoint& pos) const;

private:
	std::vector<ListItem> m_items;
	QRect m_viewport;
	Palette m_palette;
	
	int m_itemHeight = 32;
	float m_borderWidth = 1.0f;
	float m_cornerRadius = 4.0f;
	int m_scrollOffset = 0;
	
	int m_selectedIndex = -1;
	int m_hoveredIndex = -1;
	int m_pressedIndex = -1;
	
	std::function<void(int)> m_onItemClicked;
	
	// Resource context
	IconCache* m_cache = nullptr;
	QOpenGLFunctions* m_gl = nullptr;
	float m_dpr = 1.0f;
	
	bool m_isDark = false;
};