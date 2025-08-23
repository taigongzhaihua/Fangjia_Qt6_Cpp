#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp" // 新增

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

	// 设置内容组件（可为 nullptr）
	void setContent(IUiComponent* content) { m_content = content; }

	// IUiComponent
	void updateLayout(const QSize& /*windowSize*/) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override { return m_viewport; }

	// 计算内部卡片矩形（供需要时查询）
	QRectF cardRectF() const;

	// 内容区矩形（卡片内，避开标题区域）
	QRectF contentRectF() const;

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

	// 内容组件（不拥有），UiPage 负责把 contentRect 传给实现了 IUiContent 的组件
	IUiComponent* m_content{ nullptr };

	// 资源上下文
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 布局常量（逻辑像素）
	static constexpr int kMargin = 8;
	static constexpr int kMarginTop = 52;
	static constexpr int kCardPad = 24;
	// 标题区域高度（卡片内从顶部预留）
	static constexpr int kTitleAreaH = 44;
};