#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <algorithm>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>

// 数据页的“选项卡内容组件”：不绘制卡片，仅绘制 Tab Bar 与指示条
class UiDataTabs final : public IUiComponent, public IUiContent
{
public:
	struct Palette {
		QColor barBg;            // 选项卡条背景（轻微衬底，可为透明）
		QColor tabHover;         // 悬停背景
		QColor tabSelectedBg;    // 选中背景（轻微）
		QColor indicator;        // 选中指示条颜色
		QColor label;            // 标签文字颜色
		QColor labelSelected;    // 选中标签文字颜色
	};

	UiDataTabs() = default;
	~UiDataTabs() override = default;

	// IUiContent
	void setViewportRect(const QRect& r) override { m_viewport = r; }

	// 业务
	void setTabs(const QStringList& labels);
	void setSelectedIndex(int idx);
	int  selectedIndex() const noexcept { return m_selected; }

	void setPalette(const Palette& p) { m_pal = p; }

	// IUiComponent
	void updateLayout(const QSize& /*windowSize*/) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
		m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
	}
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override { return m_viewport; }

	// 可选：供上层判断动画是否活跃（目前上层无需调用）
	bool hasActiveAnimation() const { return m_animHighlight.active; }

private:
	QRectF tabBarRectF() const;
	QRectF tabRectF(int i) const;

	static QString textCacheKey(const QString& baseKey, int px, const QColor& color) {
		const QString colorKey = color.name(QColor::HexArgb);
		return QString("data-tabs:%1@%2px@%3").arg(baseKey).arg(px).arg(colorKey);
	}

	static float easeInOut(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
	void startHighlightAnim(float toCenterX, int durationMs = 220);

private:
	QRect m_viewport;
	QStringList m_tabs{ QStringList{ "方剂","中药","经典","医案","内科","诊断" } };
	int m_selected{ 0 };
	int m_hover{ -1 };
	int m_pressed{ -1 };

	// “整体高亮单元”的中心 X（逻辑像素）。<0 表示未初始化
	float m_highlightCenterX{ -1.0f };

	// 动画
	struct ScalarAnim {
		bool active{ false };
		float start{ 0 }, end{ 0 };
		qint64 startMs{ 0 };
		int durationMs{ 0 };
	} m_animHighlight;
	QElapsedTimer m_clock;

	Palette m_pal{
		.barBg = QColor(0,0,0,0),
		.tabHover = QColor(0,0,0,16),
		.tabSelectedBg = QColor(0,0,0,22),
		.indicator = QColor(0,122,255,220),
		.label = QColor(50,60,70,255),
		.labelSelected = QColor(20,32,48,255)
	};

	// 资源上下文
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
};