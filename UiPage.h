#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"

#include <algorithm>
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <qstring.h>
#include <qstringliteral.h>
#include <utility>

class UiPage final : public IUiComponent
{
public:
	// 简易页面调色板
	struct Palette {
		QColor cardBg;       // 内容卡片背景
		QColor headingColor; // 标题颜色
		QColor bodyColor;    // 正文/次级文字颜色（预留）
	};

	UiPage() = default;
	~UiPage() override = default;

	void setTitle(QString title) { m_title = std::move(title); }
	void setPalette(const Palette& p) { m_pal = p; }

	// 设置页面内容的可用区域（逻辑像素），避免与导航栏重叠
	void setViewportRect(const QRect& r) { m_viewport = r; }

	// IUiComponent
	void updateLayout(const QSize& /*windowSize*/) override { /* 尺寸由 viewport 决定，上层负责设置 */ }
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
		m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
	}
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint&) override { return false; }
	bool onMouseMove(const QPoint&) override { return false; }
	bool onMouseRelease(const QPoint&) override { return false; }
	bool tick() override { return false; }
	QRect bounds() const override { return m_viewport; }

private:
	static QString textCacheKey(const QString& baseKey, int px, const QColor& color) {
		const QString colorKey = color.name(QColor::HexArgb);
		return QString("page:%1@%2px@%3").arg(baseKey).arg(px).arg(colorKey);
	}

private:
	QRect m_viewport; // 页面内容区域（由上层设置）

	QString m_title{ QStringLiteral("首页") };
	Palette m_pal{
		.cardBg = QColor(255,255,255,240),
		.headingColor = QColor(32, 38, 46, 255),
		.bodyColor = QColor(60, 70, 84, 220)
	};

	// 资源上下文
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
};