#include "NavViewModel.h"
#include "RenderData.hpp"
#include "UiNav.h"

#include "IconLoader.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <qbytearray.h>
#include <qcolor.h>
#include <qfile.h>
#include <qfont.h>
#include <qiodevice.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
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

		// 选中项与“整体高亮”锚点（使用选中项中心 Y）
		const int sel = m_vm->selectedIndex();
		if (sel >= 0 && sel < vmCount()) {
			const QRectF r = itemRectF(sel);
			m_indicatorY = static_cast<float>(r.center().y());
		}
		else {
			m_indicatorY = -1.0f;
		}
		m_animIndicator.active = false;

		// 视图层高亮索引
		m_selected = sel;
	}

	// 顶部项目起始 Y：顶部留 8px，再放 32/36px 的 toggle，再留 8px
	qreal NavRail::topItemsStartY() const
	{
		constexpr int size = 36;
		constexpr int margin = 8;
		return static_cast<qreal>(m_rect.top()) + margin + size + margin;
	}

	// 将“设置”固定到底部。若找不到 id=="settings"，返回 -1（全部走顶部流式布局）。
	int NavRail::findSettingsIndex() const
	{
		if (m_vm) {
			const auto& vitems = m_vm->items();
			for (int i = 0; i < vitems.size(); ++i) {
				if (vitems[i].id.compare(QStringLiteral("settings"), Qt::CaseInsensitive) == 0)
					return i;
			}
			return -1;
		}
		for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
			if (m_items[i].id.compare(QStringLiteral("settings"), Qt::CaseInsensitive) == 0)
				return i;
		}
		return -1;
	}

	QRectF NavRail::itemRectF(const int i) const
	{
		const int n = count();
		if (i < 0 || i >= n) return {};
		const int settingsIdx = findSettingsIndex();

		// 底部“设置”
		if (i == settingsIdx) {
			constexpr int margin = 8;
			qreal y = static_cast<qreal>(m_rect.bottom()) - margin - m_itemH;
			return { static_cast<qreal>(m_rect.left()), y, static_cast<qreal>(m_rect.width()), static_cast<qreal>(m_itemH) };
		}

		// 顶部其它项目：在 toggle 下方开始，自上而下排列
		int rank = 0;
		for (int j = 0; j < n; ++j) {
			if (j == settingsIdx) continue;
			if (j == i) break;
			++rank;
		}
		const qreal y0 = topItemsStartY() + static_cast<qreal>(rank * m_itemH);
		return { static_cast<qreal>(m_rect.left()), y0, static_cast<qreal>(m_rect.width()), static_cast<qreal>(m_itemH) };
	}

	// 顶部“展开/收起”按钮区域：与左右留白 8px，对齐顶边下方 8px，尺寸 36x36
	QRectF NavRail::toggleRectF() const
	{
		constexpr int size = 36;
		constexpr int margin = 8;
		qreal x = static_cast<qreal>(m_rect.left()) + margin;
		qreal y = static_cast<qreal>(m_rect.top()) + margin;
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

		// 优先命中“展开/收起”按钮（顶部）
		if (toggleRectF().toRect().contains(pos)) {
			m_togglePressed = true;
			return true;
		}

		// 命中 item（扫描矩形）
		for (int i = 0; i < count(); ++i) {
			if (itemRectF(i).toRect().contains(pos)) {
				m_pressed = i;
				return true;
			}
		}
		return false;
	}

	bool NavRail::onMouseMove(const QPoint& pos)
	{
		bool changed = false;

		// toggle hover（顶部）
		const bool toggleHov = m_rect.contains(pos) && toggleRectF().toRect().contains(pos);
		if (toggleHov != m_toggleHovered) {
			m_toggleHovered = toggleHov;
			changed = true;
		}

		// item hover（扫描）
		int hov = -1;
		if (m_rect.contains(pos)) {
			for (int i = 0; i < count(); ++i) {
				if (itemRectF(i).toRect().contains(pos)) { hov = i; break; }
			}
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
		int iHit = -1;
		for (int i = 0; i < count(); ++i) {
			if (itemRectF(i).toRect().contains(pos)) { iHit = i; break; }
		}
		if (iHit >= 0 && iHit == wasPressed) {
			if (m_vm) {
				m_vm->setSelectedIndex(iHit);
				const QRectF targetR = itemRectF(iHit);
				startIndicatorAnim(static_cast<float>(targetR.center().y()), 240);
				m_selected = iHit; // 视图高亮立即对齐
				return true;
			}
			setSelectedIndex(iHit);
			return true;
		}
		return (wasPressed >= 0) || toggleWasPressed;
	}

	void NavRail::append(Render::FrameData& fd) const
	{
		// 1) 导航栏背景
		fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = QRectF(m_rect), .radiusPx = 0.0f, .color = m_pal.railBg });

		// 2) “整体高亮单元”（将原选中背景矩形 + 指示条合并为一体，并整体移动）
		const int selForHighlight = m_vm ? m_vm->selectedIndex() : m_selected;
		if (selForHighlight >= 0 && m_indicatorY >= 0.0f) {
			const QRectF rSelTmpl = itemRectF(selForHighlight);
			// 背景胶囊：相对模板项左右/上下内缩，Y 由 m_indicatorY 控制（与动画一致）
			constexpr float padX = 5.0f;
			constexpr float padY = 5.0f;
			const float bgH = static_cast<float>(rSelTmpl.height()) - padY * 2.0f;
			const QRectF bgRect(
				rSelTmpl.left() + padX,
				m_indicatorY - bgH * 0.5f,
				rSelTmpl.width() - padX * 2.0f,
				bgH
			);
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = bgRect,
				.radiusPx = 6.0f,
				.color = m_pal.itemSelected
				});

			// 左侧指示条：贴合在高亮胶囊底部内部，随同移动
			constexpr float indW = 3.0f;
			const float indH = std::clamp(static_cast<float>(bgRect.height()) * 0.5f, 16.0f, static_cast<float>(bgRect.height()) - 10.0f);
			constexpr float indOffsetLeft = 3.0f; // 离左边一点距离
			const QRectF indRect(
				bgRect.left() + indOffsetLeft,
				bgRect.center().y() - indH * 0.5f,
				indW,
				indH
			);
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = indRect,
				.radiusPx = indW * 0.5f,
				.color = m_pal.indicator
				});
		}

		// 3) 各项内容与状态（不再绘制“选中项专属背景”，避免与整体高亮重复）
		if (!m_loader || !m_gl) return;

		const int iconPx = std::lround(static_cast<float>(m_iconLogical) * m_dpr);
		const bool isExpanded = expanded();

		const float iconLeftExpanded = static_cast<float>(m_rect.left()) + 13.0f;

		if (m_vm) {
			const auto& vitems = m_vm->items();
			for (int i = 0; i < vitems.size(); ++i) {
				const QRectF r = itemRectF(i);

				// 背景态（仅 hover/pressed；选中背景由“整体高亮单元”承担）
				if (i == m_selected) {
				}
				else if (i == m_pressed) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(5, 5, -5, -5), .radiusPx = 6.0f,
						.color = m_pal.itemPressed });
				}
				else if (i == m_hover) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(5, 5, -5, -5), .radiusPx = 6.0f,
						.color = m_pal.itemHover });
				}

				// 图标纹理
				const QString path = m_isDark ? vitems[i].svgDark : vitems[i].svgLight;
				QByteArray svg = svgDataCached(path);
				const QString key = iconCacheKey(vitems[i].id, iconPx, m_isDark);

				const int tex = m_loader->ensureSvgPx(key, svg, QSize(iconPx, iconPx), QColor(255, 255, 255, 255), m_gl);
				const QSize texSz = m_loader->textureSizePx(tex);

				QRectF iconDst;
				if (isExpanded) {
					iconDst = QRectF(iconLeftExpanded, r.center().y() - static_cast<float>(m_iconLogical) * 0.5f, m_iconLogical, m_iconLogical);
				}
				else {
					// 居中
					iconDst = QRectF(r.left() + 13, r.center().y() - static_cast<float>(m_iconLogical) * 0.5f, m_iconLogical, m_iconLogical);
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
						.tint = m_pal.labelColor
						});
				}
			}
		}
		else {
			// 未接 VM：使用内部 items
			for (int i = 0; i < count(); ++i) {
				const QRectF r = itemRectF(i);

				// 背景态（仅 hover/pressed；选中背景由“整体高亮单元”承担）
				if (i == m_pressed) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(6, 6, -6, -6), .radiusPx = 10.0f,
						.color = m_pal.itemPressed });
				}
				else if (i == m_hover) {
					fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r.adjusted(6, 6, -6, -6), .radiusPx = 10.0f,
						.color = m_pal.itemHover });
				}

				// 图标纹理
				const QString path = m_isDark ? m_items[i].svgDark : m_items[i].svgLight;
				QByteArray svg = svgDataCached(path);
				const QString key = iconCacheKey(m_items[i].id, iconPx, m_isDark);

				const int tex = m_loader->ensureSvgPx(key, svg, QSize(iconPx, iconPx), QColor(255, 255, 255, 255), m_gl);
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
						.tint = m_pal.labelColor
						});
				}
			}
		}

		// 4) 顶部“展开/收起”按钮（背景 + SVG 图标）
		const QRectF tgl = toggleRectF();
		const QColor tglBg = m_togglePressed ? m_pal.itemPressed
			: (m_toggleHovered ? m_pal.itemHover : QColor(0, 0, 0, 0));
		if (tglBg.alpha() > 0) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = tgl, .radiusPx = 6.0f, .color = tglBg });
		}

		// 选择 SVG：展开时显示“向左收起”，收起时显示“向右展开”
		const bool isOpen = expanded();
		const QString svgPath = isOpen ? m_svgToggleCollapse : m_svgToggleExpand;
		const QString baseKey = isOpen ? QStringLiteral("nav_toggle_collapse") : QStringLiteral("nav_toggle_expand");

		constexpr int iconLogical = 24; // 视觉大小
		const int px = std::lround(static_cast<float>(iconLogical) * m_dpr);
		QByteArray svg = svgDataCached(svgPath);
		const QString key = iconCacheKey(baseKey, px, false);

		const int tex = m_loader->ensureSvgPx(key, svg, QSize(px, px), QColor(255, 255, 255, 255), m_gl);
		const QSize texSz = m_loader->textureSizePx(tex);

		// 居中放置（用固定逻辑尺寸，不依赖纹理像素大小）
		const QRectF iconDst(
			tgl.center().x() - static_cast<float>(iconLogical) * 0.5f,
			tgl.center().y() - static_cast<float>(iconLogical) * 0.5f,
			static_cast<float>(iconLogical),
			static_cast<float>(iconLogical)
		);

		fd.images.push_back(Render::ImageCmd{
			.dstRect = iconDst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
			.tint = m_pal.iconColor // 着色
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
				// 同步视图高亮并启动“整体高亮单元”动画/或清除
				if (vmSel >= 0 && vmSel < vmCount()) {
					const QRectF targetR = itemRectF(vmSel);
					startIndicatorAnim(static_cast<float>(targetR.center().y()), 240);
				}
				else {
					// 无选中：直接隐藏
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