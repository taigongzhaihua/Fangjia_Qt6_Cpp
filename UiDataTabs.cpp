#include "RenderData.hpp"
#include "UiDataTabs.h"

#include <algorithm>
#include <cmath>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qfont.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>

void UiDataTabs::setTabs(const QStringList& labels)
{
	m_tabs = labels;
	if (m_selected < 0 && !m_tabs.isEmpty()) m_selected = 0;
	if (m_selected >= m_tabs.size()) m_selected = std::max(0, static_cast<int>(m_tabs.size() - 1));
	m_hover = -1;
	m_pressed = -1;

	// 重算高亮中心（不启用动画）
	if (!m_viewport.isEmpty())
	{
		const QRectF rSel = tabRectF(m_selected);
		m_highlightCenterX = rSel.isValid() ? static_cast<float>(rSel.center().x()) : -1.0f;
		m_animHighlight.active = false;
	}
}

void UiDataTabs::setSelectedIndex(const int idx)
{
	if (idx < 0 || idx >= m_tabs.size()) return;
	if (m_selected == idx && m_highlightCenterX >= 0.0f) return;

	const int prev = m_selected;
	m_selected = idx;

	const QRectF rTarget = tabRectF(m_selected);
	const float targetCX = static_cast<float>(rTarget.center().x());

	if (prev < 0 || m_highlightCenterX < 0.0f)
	{
		// 首次：直接跳到位
		m_highlightCenterX = targetCX;
		m_animHighlight.active = false;
	}
	else
	{
		startHighlightAnim(targetCX, 220);
	}
}

void UiDataTabs::updateLayout(const QSize& /*windowSize*/)
{
	// 非动画时，确保高亮中心与选中项中心对齐（窗口尺寸/布局变更后）
	if (!m_animHighlight.active && m_selected >= 0 && m_selected < m_tabs.size())
	{
		const QRectF rSel = tabRectF(m_selected);
		m_highlightCenterX = rSel.isValid() ? static_cast<float>(rSel.center().x()) : -1.0f;
	}
}

QRectF UiDataTabs::tabBarRectF() const
{
	if (!m_viewport.isValid()) return {};
	constexpr float padLr = 16.0f; // 左右内边距
	constexpr float barH = 43.0f;
	return {
		static_cast<float>(m_viewport.left()) + padLr,
		static_cast<float>(m_viewport.top()) + padLr, // 距内容区顶部 16
		std::max(0.0, static_cast<double>(m_viewport.width()) - padLr * 2.0),
		barH
	};
}

QRectF UiDataTabs::tabRectF(const int i) const
{
	const QRectF bar = tabBarRectF();
	const int n = m_tabs.size();
	if (i < 0 || i >= n || bar.width() <= 0.0) return {};
	const qreal w = bar.width() / std::max(1, n);
	return { bar.left() + w * i, bar.top(), w, bar.height() };
}

void UiDataTabs::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || m_viewport.width() <= 0 || m_viewport.height() <= 0) return;
	if (!m_loader || !m_gl) return;

	// TabBar 背景（轻微衬底）
	const QRectF bar = tabBarRectF();
	if (m_pal.barBg.alpha() > 0)
	{
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = bar.adjusted(-4, -3, 4, 3),
			.radiusPx = 8.0f,
			.color = m_pal.barBg
			});
	}

	// 1) 整体高亮单元（胶囊背景 + 底部指示条）—— 作为一个整体随水平动画移动
	if (m_selected >= 0 && m_selected < m_tabs.size() && m_highlightCenterX >= 0.0f)
	{
		const QRectF rSelTmpl = tabRectF(m_selected);

		// 胶囊背景尺寸取模板项，X 用动画中心
		constexpr float padX = 6.0f;
		constexpr float padY = 4.0f;
		const float bgW = std::max(8.0f, static_cast<float>(rSelTmpl.width()) - padX * 2.0f);
		const float bgH = std::max(8.0f, static_cast<float>(rSelTmpl.height()) - padY * 2.0f);

		const QRectF bgRect(
			static_cast<double>(m_highlightCenterX) - bgW * 0.5,
			rSelTmpl.top() + padY,
			bgW,
			bgH
		);
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = bgRect,
			.radiusPx = 6.0f,
			.color = m_pal.tabSelectedBg
			});

		// 指示条：贴合在胶囊底部，随同移动
		const float indW = std::clamp(bgW * 0.5f, 24.0f, std::max(24.0f, bgW - 10.0f));
		constexpr float indH = 3.0f;
		constexpr float indOffsetUp = 6.0f; // 离底边一点距离
		const QRectF indRect(
			bgRect.center().x() - indW * 0.5f,
			bgRect.bottom() - indOffsetUp,
			indW,
			indH
		);
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = indRect,
			.radiusPx = indH * 0.5f,
			.color = m_pal.indicator
			});
	}

	// 2) 各标签 hover/press 背景（不再绘制“选中项专属背景”，避免与整体高亮重复）
	const int n = m_tabs.size();
	for (int i = 0; i < n; ++i)
	{
		const QRectF r = tabRectF(i);
		if (i == m_selected) continue; // 选中项由整体高亮单元承担
		if (i == m_pressed)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = r.adjusted(6, 4, -6, -4),
				.radiusPx = 6.0f,
				.color = m_pal.tabHover.darker(115)
				});
		}
		else if (i == m_hover)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = r.adjusted(6, 4, -6, -4),
				.radiusPx = 6.0f,
				.color = m_pal.tabHover
				});
		}
	}

	// 3) 文本
	const int fontPx = std::lround(14.0f * m_dpr);
	QFont font;
	font.setPixelSize(fontPx);
	font.setWeight(QFont::Medium);


	for (int i = 0; i < n; ++i)
	{
		const QRectF r = tabRectF(i);
		const QColor textColor = (i == m_selected ? m_pal.labelSelected : m_pal.label);
		const QString key = textCacheKey(QString("tab|%1").arg(m_tabs[i]), fontPx, textColor);
		const int tex = m_loader->ensureTextPx(key, font, m_tabs[i], textColor, m_gl);
		const QSize ts = m_loader->textureSizePx(tex);

		const float wLogical = static_cast<float>(ts.width()) / m_dpr;
		const float hLogical = static_cast<float>(ts.height()) / m_dpr;

		const QRectF textDst(
			r.center().x() - wLogical * 0.5f,
			r.center().y() - hLogical * 0.5f,
			wLogical,
			hLogical
		);
		fd.images.push_back(Render::ImageCmd{
			.dstRect = textDst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
			.tint = m_pal.labelSelected // 文字已按目标色渲染
			});
	}

	// 内容区域 TODO：根据选中 Tab 渲染具体内容（此处略）
}

bool UiDataTabs::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;
	// 命中哪个 tab
	for (int i = 0; i < m_tabs.size(); ++i)
	{
		if (tabRectF(i).toRect().contains(pos))
		{
			m_pressed = i;
			return true;
		}
	}
	return false;
}

bool UiDataTabs::onMouseMove(const QPoint& pos)
{
	int hov = -1;
	if (m_viewport.contains(pos))
	{
		for (int i = 0; i < m_tabs.size(); ++i)
		{
			if (tabRectF(i).toRect().contains(pos))
			{
				hov = i;
				break;
			}
		}
	}
	const bool changed = (hov != m_hover);
	m_hover = hov;
	return changed;
}

bool UiDataTabs::onMouseRelease(const QPoint& pos)
{
	const int wasPressed = m_pressed;
	m_pressed = -1;

	if (!m_viewport.contains(pos))
	{
		return (wasPressed >= 0);
	}
	int hit = -1;
	for (int i = 0; i < m_tabs.size(); ++i)
	{
		if (tabRectF(i).toRect().contains(pos))
		{
			hit = i;
			break;
		}
	}
	if (hit >= 0 && hit == wasPressed)
	{
		setSelectedIndex(hit); // 内部会启动动画
		return true;
	}
	return (wasPressed >= 0);
}

bool UiDataTabs::tick()
{
	if (!m_clock.isValid()) m_clock.start();
	if (m_animHighlight.active)
	{
		const qint64 now = m_clock.elapsed();
		const float t = easeInOut(
			static_cast<float>(now - m_animHighlight.startMs) / static_cast<float>(std::max(
				1, m_animHighlight.durationMs)));
		m_highlightCenterX = m_animHighlight.start + (m_animHighlight.end - m_animHighlight.start) * std::clamp(
			t, 0.0f, 1.0f);
		if (t >= 1.0f) m_animHighlight.active = false;
		return true;
	}
	return false;
}

void UiDataTabs::startHighlightAnim(const float toCenterX, const int durationMs)
{
	if (!m_clock.isValid()) m_clock.start();
	m_animHighlight.active = true;
	m_animHighlight.start = (m_highlightCenterX < 0.0f ? toCenterX : m_highlightCenterX);
	m_animHighlight.end = toCenterX;
	m_animHighlight.startMs = m_clock.elapsed();
	m_animHighlight.durationMs = durationMs;
}
