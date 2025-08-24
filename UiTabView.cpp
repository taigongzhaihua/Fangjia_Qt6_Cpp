#include "RenderData.hpp"
#include "TabViewModel.h"
#include "UiTabView.h"

#include "IconLoader.h"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include <algorithm>
#include <cmath>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qfont.h>
#include <qlogging.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <unordered_map>
#include <vector>

void UiTabView::setViewModel(TabViewModel* vm)
{
	if (m_vm == vm) return;
	m_vm = vm;

	// 清理交互状态
	m_hover = -1;
	m_pressed = -1;

	// 同步视图状态（无动画）
	syncFromVmInstant();
}

void UiTabView::syncFromVmInstant()
{
	if (!m_vm) return;

	const int sel = m_vm->selectedIndex();
	m_viewSelected = sel;

	if (sel >= 0 && sel < m_vm->count()) {
		const QRectF r = tabRectF(sel);
		m_highlightCenterX = r.isValid() ? static_cast<float>(r.center().x()) : -1.0f;
	}
	else {
		m_highlightCenterX = -1.0f;
	}
	m_animHighlight.active = false;
}

int UiTabView::tabCount() const
{
	return m_vm ? m_vm->count() : static_cast<int>(m_fallbackTabs.size());
}

QString UiTabView::tabLabel(int i) const
{
	if (m_vm) {
		const auto& items = m_vm->items();
		if (i >= 0 && i < items.size()) {
			return items[i].label;
		}
		return QString();
	}
	if (i >= 0 && i < m_fallbackTabs.size()) {
		return m_fallbackTabs[i];
	}
	return QString();
}

void UiTabView::setTabs(const QStringList& labels)
{
	if (m_vm) return; // VM 模式下忽略

	m_fallbackTabs = labels;
	if (m_fallbackSelected < 0 && !m_fallbackTabs.isEmpty()) {
		m_fallbackSelected = 0;
	}
	if (m_fallbackSelected >= m_fallbackTabs.size()) {
		m_fallbackSelected = std::max(0, static_cast<int>(m_fallbackTabs.size() - 1));
	}

	m_hover = -1;
	m_pressed = -1;
	m_viewSelected = m_fallbackSelected;

	// 重算高亮中心
	if (!m_viewport.isEmpty() && m_viewSelected >= 0) {
		const QRectF r = tabRectF(m_viewSelected);
		m_highlightCenterX = r.isValid() ? static_cast<float>(r.center().x()) : -1.0f;
		m_animHighlight.active = false;
	}
}

void UiTabView::setSelectedIndex(const int idx)
{
	if (m_vm) {
		// VM 模式：驱动 VM
		m_vm->setSelectedIndex(idx);
		return;
	}

	// 兼容模式
	if (idx < 0 || idx >= m_fallbackTabs.size()) return;
	if (m_fallbackSelected == idx && m_highlightCenterX >= 0.0f) return;

	const int prev = m_fallbackSelected;
	m_fallbackSelected = idx;
	m_viewSelected = idx;

	const QRectF rTarget = tabRectF(idx);
	const float targetCX = static_cast<float>(rTarget.center().x());

	if (prev < 0 || m_highlightCenterX < 0.0f) {
		m_highlightCenterX = targetCX;
		m_animHighlight.active = false;
	}
	else {
		startHighlightAnim(targetCX);
	}
}

QRectF UiTabView::contentRectF()
{
	if (!m_viewport.isValid()) return {};
	constexpr float pad = 8.0f;
	const float left = static_cast<float>(m_viewport.left()) + pad;
	const float top = static_cast<float>(m_viewport.top()) + pad + static_cast<float>(m_tabHeight) + 16.0f;
	const float width = std::max(0.0f, static_cast<float>(m_viewport.width()) - pad * 2.0f);
	const float height = std::max(0.0f, static_cast<float>(m_viewport.height()) - pad * 2.0f - static_cast<float>(m_tabHeight) - 16.0f);
	return { left, top, width, height };

}
int UiTabView::selectedIndex() const noexcept
{
	return m_vm ? m_vm->selectedIndex() : m_fallbackSelected;
}

void UiTabView::updateLayout(const QSize& /*windowSize*/)
{
	// 窗口尺寸变化时，确保高亮位置正确
	if (!m_animHighlight.active && m_viewSelected >= 0 && m_viewSelected < tabCount()) {
		const QRectF r = tabRectF(m_viewSelected);
		m_highlightCenterX = r.isValid() ? static_cast<float>(r.center().x()) : -1.0f;
	}
	if (const auto content = m_tabContents[selectedIndex()]) {
		if (auto* c = dynamic_cast<IUiContent*>(content)) {
			c->setViewportRect(contentRectF().toRect());
		}
		// 让内容组件也有机会进行它自己的内部布局（传入窗口 size 对它用处不大，但保持调用流程一致）
		content->updateLayout(m_viewport.size());
	}
}

void UiTabView::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
	m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
	int curIdx = selectedIndex();
	if (IUiComponent* curContent = content(curIdx)) {
		curContent->updateResourceContext(loader, gl, devicePixelRatio);
	}
}

QRectF UiTabView::tabBarRectF() const
{
	if (!m_viewport.isValid()) return {};
	constexpr float padLr = 16.0f;
	return {
		static_cast<float>(m_viewport.left()) + padLr,
		static_cast<float>(m_viewport.top()) + padLr,
		std::max(0.0, static_cast<double>(m_viewport.width()) - padLr * 2.0),
		static_cast<float>(m_tabHeight)
	};
}

QRectF UiTabView::tabRectF(const int i) const
{
	const QRectF bar = tabBarRectF();
	const int n = tabCount();
	if (i < 0 || i >= n || bar.width() <= 0.0) return {};
	const qreal w = bar.width() / std::max(1, n);
	return { bar.left() + w * i, bar.top(), w, bar.height() };
}

void UiTabView::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || m_viewport.width() <= 0 || m_viewport.height() <= 0) return;
	if (!m_loader || !m_gl) return;

	const QRectF bar = tabBarRectF();

	// TabBar 背景
	if (m_pal.barBg.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = bar.adjusted(-4, -3, 4, 3),
			.radiusPx = 8.0f,
			.color = m_pal.barBg
			});
	}

	// 整体高亮单元
	if (m_viewSelected >= 0 && m_viewSelected < tabCount() && m_highlightCenterX >= 0.0f) {
		const QRectF rSelTmpl = tabRectF(m_viewSelected);

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

		if (m_indicatorStyle == IndicatorStyle::Full || m_pal.tabSelectedBg.alpha() > 0) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = bgRect,
				.radiusPx = 6.0f,
				.color = m_pal.tabSelectedBg
				});
		}

		// 指示条
		if (m_indicatorStyle != IndicatorStyle::Full) {
			const float indW = std::clamp(bgW * 0.5f, 24.0f, std::max(24.0f, bgW - 10.0f));
			constexpr float indH = 3.0f;

			QRectF indRect;
			if (m_indicatorStyle == IndicatorStyle::Bottom) {
				constexpr float indOffsetUp = 6.0f;
				indRect = QRectF(
					bgRect.center().x() - indW * 0.5f,
					bgRect.bottom() - indOffsetUp,
					indW,
					indH
				);
			}
			else { // Top
				constexpr float indOffsetDown = 6.0f;
				indRect = QRectF(
					bgRect.center().x() - indW * 0.5f,
					bgRect.top() + indOffsetDown,
					indW,
					indH
				);
			}

			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = indRect,
				.radiusPx = indH * 0.5f,
				.color = m_pal.indicator
				});
		}
	}

	// hover/press 背景
	const int n = tabCount();
	for (int i = 0; i < n; ++i) {
		if (i == m_viewSelected) continue;

		const QRectF r = tabRectF(i);
		if (i == m_pressed) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = r.adjusted(6, 4, -6, -4),
				.radiusPx = 6.0f,
				.color = m_pal.tabHover.darker(115)
				});
		}
		else if (i == m_hover) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = r.adjusted(6, 4, -6, -4),
				.radiusPx = 6.0f,
				.color = m_pal.tabHover
				});
		}
	}

	// 文本
	const int fontPx = std::lround(14.0f * m_dpr);
	QFont font;
	font.setPixelSize(fontPx);
	font.setStyleStrategy(QFont::PreferAntialias);

	for (int i = 0; i < n; ++i) {
		const QRectF r = tabRectF(i);
		const QString label = tabLabel(i);
		if (label.isEmpty()) continue;

		const QColor textColor = (i == m_viewSelected ? m_pal.labelSelected : m_pal.label);

		// 调试输出
		qDebug() << "Tab" << i << "color:" << textColor.name()
			<< "selected:" << (i == m_viewSelected);

		const QString key = textCacheKey(QString("tab|%1").arg(label), fontPx, textColor);
		const int tex = m_loader->ensureTextPx(key, font, label, textColor, m_gl);
		const QSize ts = m_loader->textureSizePx(tex);

		const float wLogical = static_cast<float>(ts.width()) / m_dpr;
		const float hLogical = static_cast<float>(ts.height()) / m_dpr;

		// 对齐到整数像素边界
		const float centerX = std::round(r.center().x());
		const float centerY = std::round(r.center().y());
		const float textX = std::round(centerX - wLogical * 0.5f);
		const float textY = std::round(centerY - hLogical * 0.5f);

		const QRectF textDst(textX, textY, wLogical, hLogical);

		fd.images.push_back(Render::ImageCmd{
			.dstRect = textDst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
			.tint = QColor(255,255,255,255)
			});
	}

	int curIdx = selectedIndex();
	if (IUiComponent* curContent = content(curIdx)) {
		curContent->append(fd);
	}
}

bool UiTabView::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;

	// 先判断是否点在tab bar上，若否则转发到内容
	for (int i = 0; i < tabCount(); ++i) {
		if (tabRectF(i).toRect().contains(pos)) {
			m_pressed = i;
			return true;
		}
	}

	int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent && m_viewport.contains(pos)) {
		return curContent->onMousePress(pos);
	}
	return false;
}

bool UiTabView::onMouseMove(const QPoint& pos)
{
	int hov = -1;
	if (m_viewport.contains(pos)) {
		for (int i = 0; i < tabCount(); ++i) {
			if (tabRectF(i).toRect().contains(pos)) {
				hov = i;
				break;
			}
		}
	}
	const bool changed = (hov != m_hover);
	m_hover = hov;

	int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent && m_viewport.contains(pos)) {
		return curContent->onMouseMove(pos) || changed;
	}
	return changed;
}

bool UiTabView::onMouseRelease(const QPoint& pos)
{
	const int wasPressed = m_pressed;
	m_pressed = -1;

	if (!m_viewport.contains(pos)) {
		return (wasPressed >= 0);
	}

	int hit = -1;
	for (int i = 0; i < tabCount(); ++i) {
		if (tabRectF(i).toRect().contains(pos)) {
			hit = i;
			break;
		}
	}

	if (hit >= 0 && hit == wasPressed) {
		if (m_vm) {
			m_vm->setSelectedIndex(hit);
		}
		else {
			setSelectedIndex(hit);
		}
		return true;
	}

	int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent && m_viewport.contains(pos)) {
		return curContent->onMouseRelease(pos);
	}
	return (wasPressed >= 0);
}


bool UiTabView::tick()
{
	bool any = false;
	if (!m_clock.isValid()) m_clock.start();

	// VM 模式：检查并同步变化
	if (m_vm) {
		const int vmSel = m_vm->selectedIndex();
		if (vmSel != m_viewSelected) {
			if (vmSel >= 0 && vmSel < m_vm->count()) {
				const QRectF targetR = tabRectF(vmSel);
				startHighlightAnim(static_cast<float>(targetR.center().x()));
			}
			else {
				m_highlightCenterX = -1.0f;
				m_animHighlight.active = false;
			}
			m_viewSelected = vmSel;
		}
	}

	// 处理动画
	if (m_animHighlight.active) {
		const qint64 now = m_clock.elapsed();
		const float t = easeInOut(
			static_cast<float>(now - m_animHighlight.startMs) /
			static_cast<float>(std::max(1, m_animHighlight.durationMs))
		);
		m_highlightCenterX = m_animHighlight.start +
			(m_animHighlight.end - m_animHighlight.start) * std::clamp(t, 0.0f, 1.0f);
		if (t >= 1.0f) m_animHighlight.active = false;
		return true;
	}

	int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent) any = curContent->tick() || any;
	return any;
}

void UiTabView::startHighlightAnim(const float toCenterX)
{
	if (!m_clock.isValid()) m_clock.start();
	m_animHighlight.active = true;
	m_animHighlight.start = (m_highlightCenterX < 0.0f ? toCenterX : m_highlightCenterX);
	m_animHighlight.end = toCenterX;
	m_animHighlight.startMs = m_clock.elapsed();
	m_animHighlight.durationMs = m_animDuration;
}

QString UiTabView::textCacheKey(const QString& baseKey, int px, const QColor& color)
{
	const QString colorKey = color.name(QColor::HexArgb);  // 包含 alpha 通道
	return QString("tabview:%1@%2px@%3").arg(baseKey).arg(px).arg(colorKey);
}

float UiTabView::easeInOut(float t)
{
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

void UiTabView::setContent(int tabIdx, IUiComponent* content)
{
	if (tabIdx < 0) return;
	m_tabContents[tabIdx] = content;
}

void UiTabView::setContents(const std::vector<IUiComponent*>& contents)
{
	m_tabContents.clear();
	for (size_t i = 0; i < contents.size(); ++i) {
		if (contents[i]) m_tabContents[(int)i] = contents[i];
	}
}

IUiComponent* UiTabView::content(int tabIdx) const
{
	auto it = m_tabContents.find(tabIdx);
	return (it != m_tabContents.end()) ? it->second : nullptr;
}