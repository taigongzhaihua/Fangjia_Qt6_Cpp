#pragma once
#include "IconLoader.h"
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
	// 简易页面调色板
	struct Palette {
		QColor cardBg;       // 内容卡片背景
		QColor headingColor; // 标题颜色
		QColor bodyColor;    // 正文/次级文字颜色（预留）
	};

	UiPage() = default;
	~UiPage() override = default;

	void setTitle(QString title) { m_title = std::move(title); }
	QString title() const { return m_title; }

	void setPalette(const Palette& p) { m_pal = p; }
	const Palette& palette() const { return m_pal; }

	// 设置页面内容的可用区域（逻辑像素），避免与导航栏重叠
	void setViewportRect(const QRect& r) { m_viewport = r; }

	// 设置内容组件（可为 nullptr）
	void setContent(IUiComponent* content) { m_content = content; }
	IUiComponent* content() const { return m_content; }

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override { return m_viewport; }

	// IUiComponent - 主题支持
	void onThemeChanged(bool isDark) override;

	// 设置是否为深色主题
	void setDarkTheme(bool dark) { m_isDark = dark; }
	bool isDarkTheme() const { return m_isDark; }

	// 计算内部卡片矩形（供需要时查询）
	QRectF cardRectF() const;

	// 内容区矩形（卡片内，避开标题区域）
	QRectF contentRectF() const;

	void setMargins(const QMargins& m) { m_margins = m; }
	QMargins margins() const { return m_margins; }

	void setPadding(const QMargins& p) { m_padding = p; }
	QMargins padding() const { return m_padding; }

	void setCornerRadius(float r) { m_cornerRadius = r; }
	float cornerRadius() const { return m_cornerRadius; }

protected:
	// 子类重写此方法来初始化内容
	virtual void initializeContent() {}

	virtual void applyPageTheme(bool isDark);

private:
	static QString textCacheKey(const QString& baseKey, int px, const QColor& color) {
		const QString colorKey = color.name(QColor::HexArgb);
		return QString("page:%1@%2px@%3").arg(baseKey).arg(px).arg(colorKey);
	}

protected:
	QMargins m_margins{ 8, 52, 8, 8 }; // 页面边距（由上层设置）
	QMargins m_padding{ 16,0,16,8 }; // 页面内边距（由上层设置）
	float m_cornerRadius = 8.0f;   // 页面圆角（由上层设置）

	QRect m_viewport; // 页面内容区域（由上层设置）

	QString m_title{ QStringLiteral("页面") };
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
	bool m_isDark{ false };  // 添加主题状态

	// 标题区域高度（卡片内从顶部预留）
	static constexpr int kTitleAreaH = 84;
};