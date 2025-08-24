#pragma once
#include "FormulaViewModel.h"
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <qcolor.h>
#include <qhash.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

class UiFormulaDetail final : public IUiComponent, public IUiContent
{
public:
	struct Palette {
		QColor bg;           // 背景色
		QColor titleColor;   // 标题颜色
		QColor labelColor;   // 标签颜色
		QColor textColor;    // 正文颜色
		QColor borderColor;  // 边框颜色
	};

	UiFormulaDetail() = default;
	~UiFormulaDetail() override = default;

	void setFormula(const FormulaViewModel::FormulaDetail* formula);
	void setPalette(const Palette& p);

	// IUiContent
	void setViewportRect(const QRect& r) override {
		m_viewport = r;
	}

	// 滚动支持
	void setScrollOffset(int y) { m_scrollY = y; }
	[[nodiscard]] int contentHeight() const;

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override { return m_viewport.contains(pos); }
	bool onMouseMove(const QPoint& pos) override { return false; }
	bool onMouseRelease(const QPoint& pos) override { return false; }
	bool tick() override { return false; }
	QRect bounds() const override { return m_viewport; }

private:
	void drawSection(Render::FrameData& fd, const QString& label, const QString& content, int& y) const;
	void drawHintText(Render::FrameData& fd) const;

private:
	const FormulaViewModel::FormulaDetail* m_formula{ nullptr };
	QRect m_viewport;
	Palette m_pal{
		.bg = QColor(255,255,255,250),
		.titleColor = QColor(20,25,30,255),
		.labelColor = QColor(60,120,180,255),
		.textColor = QColor(50,55,60,230),
		.borderColor = QColor(0,0,0,30)
	};

	int m_scrollY{ 0 };
	int m_contentHeight{ 0 };

	// 资源上下文
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 纹理缓存键（用于主题切换时清理）
	mutable QHash<QString, int> m_textureCache;
};