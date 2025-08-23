#include "NavViewModel.h"
#include "RenderData.hpp"
#include "UiNav.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <qbytearray.h>
#include <qchar.h>
#include <qcolor.h>
#include <qfile.h>
#include <qfont.h>
#include <qiodevice.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <utility>
#include <vector>

namespace Ui {

	void NavRail::setItems(std::vector<NavItem> items)
	{
		m_items = std::move(items);
		m_hover = m_pressed = -1;
		m_toggleHovered = m_togglePressed = false;
		if (!m_vm) {
			// 仅在未接 VM 的旧模式下设置初始选中/指示条
			if (m_selected < 0 && count() > 0) {
				m_selected = 0;
				const QRectF r = itemRectF(m_selected);
				m_indicatorY = r.center().y();
			}
		}
		else {
			// 已接 VM 时，以 VM 为准
			syncFromVmInstant();
		}
	}

	int NavRail::vmCount() const noexcept
	{
		return m_vm ? m_vm->count() : 0;
	}

	void NavRail::setViewModel(NavViewModel* vm)
	{
		if (m_vm == vm) return;
		m_vm = vm;
		// 清理交互态
		m_hover = m_pressed = -1;
		m_toggleHovered = m_togglePressed = false;

		// 以 VM 为真值，视图状态瞬时对齐
		syncFromVmInstant();
	}

	void NavRail::syncFromVmInstant()
	{
		if (!m_vm) return;

		// 展开插值
		m_expandT = m_vm->expanded() ? 1.0f : 0.0f;
		m_animExpand.active = false;

		// 选中项与指示条
		const int sel = m_vm->selectedIndex();
		if (sel >= 0 && sel < vmCount()) {
			const QRectF r = itemRectF(sel);
			m_indicatorY = static_cast<float>(r.center().y());
		}
		else {
			m_indicatorY = -1.0f;
		}
		m_animIndicator.active = false;

		// 高亮（视图层变量，仅用于 append 的选中态着色）
		m_selected = sel;
	}

	QRectF NavRail::itemRectF(const int i) const
	{
		qreal y = m_rect.top() + i * m_itemH;
		return { static_cast<qreal>(m_rect.left()), y, static_cast<qreal>(m_rect.width()), static_cast<qreal>(m_itemH) };
	}

	// 底部“展开/收起”按钮区域：与左右留白 8px，对齐底边上方 8px，尺寸 32x32
	QRectF NavRail::toggleRectF() const
	{
		constexpr int size = 32;
		constexpr int margin = 8;
		qreal x = m_rect.left() + margin;
		qreal y = m_rect.bottom() - margin - size;
		return { x, y, size, size };
	}

	void NavRail::setSelectedIndex(const int idx)
	{
		if (m_vm) {
			// 接入 VM 后，优先驱动 VM，并立即触发视图动画
			if (idx < 0 || idx >= vmCount()) return;
			if (m_vm->selectedIndex() == idx && m_indicatorY >= 0.0f) return;

			m_vm->setSelectedIndex(idx);
			const QRectF targetR = itemRectF(idx);
			startIndicatorAnim(static_cast<float>(targetR.center().y()), 240);
			m_selected = idx; // 视图高亮立即对齐
			return;
		}

		// 旧模式（未接 VM）
		if (idx < 0 || idx >= count()) return;
		if (m_selected == idx && m_indicatorY >= 0.0f) return;

		const int prev = m_selected;
		m_selected = idx;

		const QRectF targetR = itemRectF(m_selected);
		const float  targetY = static_cast<float>(targetR.center().y());

		if (prev < 0 || m_indicatorY < 0.0f) {
			// 首次：直接跳到位
			m_indicatorY = targetY;
			m_animIndicator.active = false;
		}
		else {
			startIndicatorAnim(targetY, 240);
		}
	}

	void NavRail::toggleExpanded()
	{
		if (m_vm) {
			const bool newExpanded = !m_vm->expanded();
			m_vm->setExpanded(newExpanded);
			startExpandAnim(newExpanded ? 1.0f : 0.0f, 220);
			return;
		}
		const float target = expanded() ? 0.0f : 1.0f;
		startExpandAnim(target, 220);
	}

	bool NavRail::onMousePress(const QPoint& pos)
	{
		if (!m_rect.contains(pos)) return false;

		// 优先命中“展开/收起”按钮
		if (toggleRectF().toRect().contains(pos)) {
			m_togglePressed = true;
			return true;
		}

		// 命中 item
		if (const int i = (pos.y() - m_rect.top()) / m_itemH; i >= 0 && i < count()) {
			m_pressed = i;
			return true;
		}
		return false;
	}

	bool NavRail::onMouseMove(const QPoint& pos)
	{
		bool changed = false;

		// toggle hover
		const bool toggleHov = m_rect.contains(pos) && toggleRectF().toRect().contains(pos);
		if (toggleHov != m_toggleHovered) {
			m_toggleHovered = toggleHov;
			changed = true;
		}

		// item hover
		int hov = -1;
		if (m_rect.contains(pos)) {
			if (const int i = (pos.y() - m_rect.top()) / m_itemH; i >= 0 && i < count()) hov = i;
		}
		if (hov != m_hover) {
			m_hover = hov;
			changed = true;
		}
		return changed;
	}

	bool NavRail::onMouseRelease(const QPoint& pos)
	{
		const int  wasPressed = m_pressed;
		const bool toggleWasPressed = m_togglePressed;

		m_pressed = -1;
		m_togglePressed = false;

		if (!m_rect.contains(pos)) {
			// 按下过但释放在外，也算消耗
			return (wasPressed >= 0) || toggleWasPressed;
		}

		// 释放在 toggle 上：切换展开
		if (toggleWasPressed && toggleRectF().toRect().contains(pos)) {
			if (m_vm) {
				const bool newExpanded = !m_vm->expanded();
				m_vm->setExpanded(newExpanded);
				startExpandAnim(newExpanded ? 1.0f : 0.0f, 220);
			}
			else {
				toggleExpanded();
			}
			return true;
		}

		// 释放在某个 item 上：激活选中
		if (const int i = (pos.y() - m_rect.top()) / m_itemH; i >= 0 && i < count() && i == wasPressed) {
			if (m_vm) {
				m_vm->setSelectedIndex(i);
				const QRectF targetR = itemRectF(i);
				startIndicatorAnim(static_cast<float>(targetR.center().y()), 240);
				m_selected = i; // 视图高亮立即对齐
				return true;
			}
			setSelectedIndex(i);
			return true;
		}
		return (wasPressed >= 0) || toggleWasPressed;
	}

	void NavRail::append(Render::FrameData& fd) const
	{
		// 1) 导航栏背景
		fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = QRectF(m_rect), .radiusPx = 0.0f, .color = m_pal.railBg });

		// 2) 选中指示条（在背景之上，项之下）
		const int selForHighlight = m_vm ? m_vm->selectedIndex() : m_selected;
		if (selForHighlight >= 0 && m_indicatorY >= 0.0f) {
			constexpr float indW = 3.0f;
			const float indH = static_cast<float>(m_itemH) - 28.0f;
			const QRectF r(static_cast<float>(m_rect.left()) + 5.0f, m_indicatorY - indH * 0.5f, indW, indH);
			fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r, .radiusPx = indW * 0.5f, .color = m_pal.indicator });
		}

		// 3) 各项
		if (!m_loader || !m_gl) return;

		const int iconPx = std::lround(static_cast<float>(m_iconLogical) * m_dpr);
		const bool isExpanded = expanded();

		const float iconLeftExpanded = static_cast<float>(m_rect.left()) + 12.0f;

		if (m_vm) {
			const auto& vitems = m_vm->items();
			for (int i = 0; i < vitems.size(); ++i) {
				const QRectF r = itemRectF(i);

				// 背景态（高亮层）
				if (i == selForHighlight) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(4, 4, -4, -4), .radiusPx = 10.0f,
						.color = m_pal.itemSelected });
				}
				else if (i == m_pressed) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(6, 6, -6, -6), .radiusPx = 10.0f,
						.color = m_pal.itemPressed });
				}
				else if (i == m_hover) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(6, 6, -6, -6), .radiusPx = 10.0f,
						.color = m_pal.itemHover });
				}

				// 图标纹理
				const QString path = m_isDark ? vitems[i].svgDark : vitems[i].svgLight;
				const QByteArray svg = svgDataCached(path);
				const QString key = iconCacheKey(vitems[i].id, iconPx, m_isDark);

				const int tex = m_loader->ensureSvgPx(key, svg, QSize(iconPx, iconPx), m_gl);
				const QSize texSz = m_loader->textureSizePx(tex);

				QRectF iconDst;
				if (isExpanded) {
					iconDst = QRectF(iconLeftExpanded, r.center().y() - static_cast<float>(m_iconLogical) * 0.5f, m_iconLogical, m_iconLogical);
				}
				else {
					// 居中
					iconDst = QRectF(r.center().x() - static_cast<float>(m_iconLogical) * 0.5f, r.center().y() - static_cast<float>(m_iconLogical) * 0.5f, m_iconLogical, m_iconLogical);
				}

				fd.images.push_back(Render::ImageCmd{
					.dstRect = iconDst,
					.textureId = tex,
					.srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
					.tint = m_pal.iconColor
					});

				// 展开时绘制文字
				if (isExpanded && !vitems[i].label.isEmpty()) {
					constexpr float rightPadding = 12.0f;
					constexpr float textGap = 8.0f;

					QFont font;
					// 字体像素大小（随 DPR）
					const int fontPx = std::lround(static_cast<float>(m_labelFontPx) * m_dpr);
					font.setPixelSize(fontPx);

					// 缓存键加入颜色（避免主题切换后复用旧颜色）
					const QString tKey = textCacheKey(vitems[i].id + "|" + vitems[i].label, fontPx, m_pal.labelColor);
					const int textTex = m_loader->ensureTextPx(tKey, font, vitems[i].label, m_pal.labelColor, m_gl);
					const QSize ts = m_loader->textureSizePx(textTex);

					// 将像素尺寸换成逻辑像素放置
					float wLogical = static_cast<float>(ts.width()) / m_dpr;
					float hLogical = static_cast<float>(ts.height()) / m_dpr;

					// 右边界裁剪：如果过长，按比例缩小
					if (const float maxW = static_cast<float>(m_rect.right()) - rightPadding - static_cast<float>(iconDst.right()) - textGap; wLogical > maxW && maxW > 4.0f) {
						const float s = maxW / wLogical;
						wLogical *= s;
						hLogical *= s;
					}

					const QRectF textDst(
						iconDst.right() + textGap,
						r.center().y() - hLogical * 0.5f,
						wLogical,
						hLogical
					);

					// 文字已在纹理阶段着色，这里保持原色输出
					fd.images.push_back(Render::ImageCmd{
						.dstRect = textDst,
						.textureId = textTex,
						.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
						.tint = QColor(255,255,255,255)
						});
				}
			}
		}
		else {
			// 未接 VM：使用内部 items
			for (int i = 0; i < count(); ++i) {
				const QRectF r = itemRectF(i);

				// 背景态（高亮层）
				if (i == m_selected) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(4, 4, -4, -4), .radiusPx = 10.0f,
						.color = m_pal.itemSelected });
				}
				else if (i == m_pressed) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(6, 6, -6, -6), .radiusPx = 10.0f,
						.color = m_pal.itemPressed });
				}
				else if (i == m_hover) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(6, 6, -6, -6), .radiusPx = 10.0f,
						.color = m_pal.itemHover });
				}

				// 图标纹理
				const QString path = m_isDark ? m_items[i].svgDark : m_items[i].svgLight;
				const QByteArray svg = svgDataCached(path);
				const QString key = iconCacheKey(m_items[i].id, iconPx, m_isDark);

				const int tex = m_loader->ensureSvgPx(key, svg, QSize(iconPx, iconPx), m_gl);
				const QSize texSz = m_loader->textureSizePx(tex);

				QRectF iconDst;
				if (isExpanded) {
					iconDst = QRectF(iconLeftExpanded, r.center().y() - static_cast<float>(m_iconLogical) * 0.5f, m_iconLogical, m_iconLogical);
				}
				else {
					// 居中
					iconDst = QRectF(r.center().x() - static_cast<float>(m_iconLogical) * 0.5f, r.center().y() - static_cast<float>(m_iconLogical) * 0.5f, m_iconLogical, m_iconLogical);
				}

				fd.images.push_back(Render::ImageCmd{
					.dstRect = iconDst,
					.textureId = tex,
					.srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
					.tint = m_pal.iconColor
					});

				// 展开时绘制文字
				if (isExpanded && !m_items[i].label.isEmpty()) {
					constexpr float rightPadding = 12.0f;
					constexpr float textGap = 8.0f;

					QFont font;
					// 字体像素大小（随 DPR）
					const int fontPx = std::lround(static_cast<float>(m_labelFontPx) * m_dpr);
					font.setPixelSize(fontPx);

					// 缓存键加入颜色（避免主题切换后复用旧颜色）
					const QString tKey = textCacheKey(m_items[i].id + "|" + m_items[i].label, fontPx, m_pal.labelColor);
					const int textTex = m_loader->ensureTextPx(tKey, font, m_items[i].label, m_pal.labelColor, m_gl);
					const QSize ts = m_loader->textureSizePx(textTex);

					// 将像素尺寸换成逻辑像素放置
					float wLogical = static_cast<float>(ts.width()) / m_dpr;
					float hLogical = static_cast<float>(ts.height()) / m_dpr;

					// 右边界裁剪：如果过长，按比例缩小
					if (const float maxW = static_cast<float>(m_rect.right()) - rightPadding - static_cast<float>(iconDst.right()) - textGap; wLogical > maxW && maxW > 4.0f) {
						const float s = maxW / wLogical;
						wLogical *= s;
						hLogical *= s;
					}

					const QRectF textDst(
						iconDst.right() + textGap,
						r.center().y() - hLogical * 0.5f,
						wLogical,
						hLogical
					);

					// 文字已在纹理阶段着色，这里保持原色输出
					fd.images.push_back(Render::ImageCmd{
						.dstRect = textDst,
						.textureId = textTex,
						.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
						.tint = QColor(255,255,255,255)
						});
				}
			}
		}

		// 4) 底部“展开/收起”按钮（仅根据事件阶段维护的状态渲染）
		const QRectF tgl = toggleRectF();
		const QColor tglBg = m_togglePressed ? m_pal.itemPressed
			: (m_toggleHovered ? m_pal.itemHover : QColor(0, 0, 0, 0));
		if (tglBg.alpha() > 0) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = tgl, .radiusPx = 8.0f, .color = tglBg });
		}

		// 箭头文本
		const QChar arrow = expanded() ? QChar(0x2039) /*‹*/ : QChar(0x203A) /*›*/;
		QFont af;
		const int arrowPx = std::lround(18.0f * m_dpr); // 箭头视觉大小
		af.setPixelSize(arrowPx);

		const QString arrowKey = textCacheKey(QString("nav-toggle-%1").arg(expanded() ? "left" : "right"), arrowPx, m_pal.iconColor);
		const int arrowTex = m_loader->ensureTextPx(arrowKey, af, QString(arrow), m_pal.iconColor, m_gl);
		const QSize asz = m_loader->textureSizePx(arrowTex);

		// 居中放置箭头（纹理像素 -> 逻辑像素）
		const QRectF arrowDst(
			tgl.center().x() - static_cast<float>(asz.width()) / (2.0f * m_dpr),
			tgl.center().y() - static_cast<float>(asz.height()) / (2.0f * m_dpr),
			static_cast<float>(asz.width()) / m_dpr,
			static_cast<float>(asz.height()) / m_dpr
		);

		fd.images.push_back(Render::ImageCmd{
			.dstRect = arrowDst,
			.textureId = arrowTex,
			.srcRectPx = QRectF(0, 0, asz.width(), asz.height()),
			.tint = QColor(255,255,255,255) // 已用 iconColor 着色
			});
	}

	bool NavRail::tick()
	{
		if (!m_clock.isValid()) m_clock.start();
		const qint64 now = m_clock.elapsed();
		bool any = false;

		// 如果接入 VM，则在每次 tick 时对比 VM 真值，触发必要动画
		if (m_vm) {
			const int vmSel = m_vm->selectedIndex();
			if (vmSel != m_selected) {
				// 同步视图高亮并启动指示条动画/或清除
				if (vmSel >= 0 && vmSel < vmCount()) {
					const QRectF targetR = itemRectF(vmSel);
					startIndicatorAnim(static_cast<float>(targetR.center().y()), 240);
				}
				else {
					// 无选中：直接隐藏指示条
					m_indicatorY = -1.0f;
					m_animIndicator.active = false;
				}
				m_selected = vmSel;
				any = true;
			}
			const float targetT = m_vm->expanded() ? 1.0f : 0.0f;
			// 若没有处于动画，且当前值与目标值差异明显，则开启动画
			if (!m_animExpand.active && std::abs(targetT - m_expandT) > 0.001f) {
				startExpandAnim(targetT, 220);
				any = true;
			}
		}

		if (m_animIndicator.active) {
			const float t = easeInOut(static_cast<float>(now - m_animIndicator.startMs) / static_cast<float>(std::max(1, m_animIndicator.durationMs)));
			const float v = m_animIndicator.start + (m_animIndicator.end - m_animIndicator.start) * std::clamp(t, 0.0f, 1.0f);
			m_indicatorY = v;
			if (t >= 1.0f) m_animIndicator.active = false;
			any = true;
		}
		if (m_animExpand.active) {
			const float t = easeInOut(static_cast<float>(now - m_animExpand.startMs) / static_cast<float>(std::max(1, m_animExpand.durationMs)));
			m_expandT = m_animExpand.start + (m_animExpand.end - m_animExpand.start) * std::clamp(t, 0.0f, 1.0f);
			if (t >= 1.0f) m_animExpand.active = false;
			any = true;
		}
		return any;
	}

	void NavRail::startIndicatorAnim(const float toY, const int durationMs)
	{
		if (!m_clock.isValid()) m_clock.start();
		m_animIndicator.active = true;
		m_animIndicator.start = (m_indicatorY < 0.0f ? toY : m_indicatorY);
		m_animIndicator.end = toY;
		m_animIndicator.startMs = m_clock.elapsed();
		m_animIndicator.durationMs = durationMs;
	}

	void NavRail::startExpandAnim(const float toT, const int durationMs)
	{
		if (!m_clock.isValid()) m_clock.start();
		m_animExpand.active = true;
		m_animExpand.start = m_expandT;
		m_animExpand.end = std::clamp(toT, 0.0f, 1.0f);
		m_animExpand.startMs = m_clock.elapsed();
		m_animExpand.durationMs = durationMs;
	}

	QByteArray NavRail::svgDataCached(const QString& path) const
	{
		if (const auto it = m_svgCache.find(path); it != m_svgCache.end()) return it.value();
		QFile f(path);
		if (!f.open(QIODevice::ReadOnly)) return {};
		QByteArray data = f.readAll();
		m_svgCache.insert(path, data);
		return data;
	}

	QString NavRail::iconCacheKey(const QString& baseKey, const int px, const bool dark) const
	{
		return QString("%1@%2@%3px").arg(baseKey, dark ? "dark" : "light").arg(px);
	}

	// 修正：将颜色编码入缓存键（含 alpha，HexArgb）
	QString NavRail::textCacheKey(const QString& baseKey, const int px, const QColor& color) const
	{
		const QString colorKey = color.name(QColor::HexArgb); // 形如 #AARRGGBB
		return QString("txt:%1@%2px@%3").arg(baseKey).arg(px).arg(colorKey);
	}

} // namespace Ui