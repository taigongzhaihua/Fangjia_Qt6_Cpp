#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"

#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
#include <utility>

class UiPage : public IUiComponent
{
public:
	struct Palette {
		QColor cardBg;
		QColor headingColor;
		QColor bodyColor;
	};

	UiPage() = default;
	~UiPage() override = default;

	void setTitle(QString title) { m_title = std::move(title); }
	QString title() const { return m_title; }

	void setPalette(const Palette& p) { m_pal = p; }
	const Palette& palette() const { return m_pal; }

	void setViewportRect(const QRect& r) { m_viewport = r; }

	void setContent(IUiComponent* content) { m_content = content; }
	IUiComponent* content() const { return m_content; }

	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override { return m_viewport; }

	void onThemeChanged(bool isDark) override;

	void setDarkTheme(bool dark) { m_isDark = dark; }
	bool isDarkTheme() const { return m_isDark; }

	QRectF cardRectF() const;
	QRectF contentRectF() const;

	void setMargins(const QMargins& m) { m_margins = m; }
	QMargins margins() const { return m_margins; }

	void setPadding(const QMargins& p) { m_padding = p; }
	QMargins padding() const { return m_padding; }

	void setCornerRadius(float r) { m_cornerRadius = r; }
	float cornerRadius() const { return m_cornerRadius; }

protected:
	virtual void initializeContent() {}
	virtual void applyPageTheme(bool isDark);

protected:
	QMargins m_margins{ 8, 52, 8, 8 };
	QMargins m_padding{ 16,0,16,8 };
	float m_cornerRadius = 8.0f;

	QRect m_viewport;

	QString m_title{ QStringLiteral("页面") };
	Palette m_pal{
		.cardBg = QColor(255,255,255,240),
		.headingColor = QColor(32, 38, 46, 255),
		.bodyColor = QColor(60, 70, 84, 220)
	};

	IUiComponent* m_content{ nullptr };

	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
	bool m_isDark{ false };

	static constexpr int kTitleAreaH = 84;
};