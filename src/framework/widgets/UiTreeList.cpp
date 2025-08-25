#include "UiTreeList.h"
#include <algorithm>
#include <cmath>
#include <qfont.h>
#include <qlogging.h>

UiTreeList::UiTreeList() {}

void UiTreeList::reloadData()
{
	updateVisibleNodes();
}

void UiTreeList::updateLayout(const QSize& /*windowSize*/)
{
	updateVisibleNodes();
}

void UiTreeList::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
	m_loader = &loader;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);
}

void UiTreeList::updateVisibleNodes()
{
	m_visibleNodes.clear();
	if (!m_model) return;

	const auto roots = m_model->rootIndices();

	std::function<void(int, int)> addVisibleChildren = [&](int nodeId, int depth) {
		const auto children = m_model->childIndices(nodeId);
		for (int childId : children) {
			const auto info = m_model->nodeInfo(childId);

			// 添加节点
			VisibleNode vn;
			vn.index = childId;
			vn.depth = depth;

			const int y = static_cast<int>(m_visibleNodes.size()) * m_itemHeight - m_scrollY;
			vn.rect = QRect(
				m_viewport.left(),
				m_viewport.top() + y,
				m_viewport.width(),
				m_itemHeight
			);
			m_visibleNodes.push_back(vn);

			// 如果展开，递归添加子节点
			if (info.expanded) {
				addVisibleChildren(childId, depth + 1);
			}
		}
		};

	// 根层
	for (int rootId : roots) {
		const auto info = m_model->nodeInfo(rootId);

		VisibleNode vn;
		vn.index = rootId;
		vn.depth = 0;

		const int y = static_cast<int>(m_visibleNodes.size()) * m_itemHeight - m_scrollY;
		vn.rect = QRect(
			m_viewport.left(),
			m_viewport.top() + y,
			m_viewport.width(),
			m_itemHeight
		);
		m_visibleNodes.push_back(vn);

		if (info.expanded) {
			addVisibleChildren(rootId, 1);
		}
	}
}

int UiTreeList::contentHeight() const
{
	return static_cast<int>(m_visibleNodes.size()) * m_itemHeight;
}

QRect UiTreeList::nodeRect(int visibleIdx) const
{
	if (visibleIdx < 0 || visibleIdx >= static_cast<int>(m_visibleNodes.size())) return {};
	return m_visibleNodes[visibleIdx].rect;
}

QRect UiTreeList::expandIconRect(const QRect& nodeRect, int depth) const
{
	const int iconSize = 16;
	const int x = nodeRect.left() + 8 + depth * m_indentWidth;
	const int y = nodeRect.center().y() - iconSize / 2;
	return QRect(x, y, iconSize, iconSize);
}

void UiTreeList::append(Render::FrameData& fd) const
{
	if (!m_loader || !m_gl) return;

	// 背景
	if (m_pal.bg.alpha() > 0 && m_viewport.isValid()) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = 0.0f,
			.color = m_pal.bg
			});
	}

	if (!m_model) return;

	const int selectedId = m_model->selectedId();

	for (size_t i = 0; i < m_visibleNodes.size(); ++i) {
		const auto& vn = m_visibleNodes[i];
		if (!vn.rect.intersects(m_viewport)) continue;

		const auto info = m_model->nodeInfo(vn.index);

		// 背景（选中/悬停）
		if (vn.index == selectedId) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(vn.rect),
				.radiusPx = 0.0f,
				.color = m_pal.itemSelected
				});
		}
		else if (static_cast<int>(i) == m_hover) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(vn.rect),
				.radiusPx = 0.0f,
				.color = m_pal.itemHover
				});
		}

		// 展开/折叠图标（仅当有子节点）
		if (!m_model->childIndices(vn.index).isEmpty()) {
			const QRect iconRect = expandIconRect(vn.rect, vn.depth);
			const float cx = iconRect.center().x();
			const float cy = iconRect.center().y();
			const float size = 6.0f;

			if (info.expanded) {
				// 向下三角形（用矩形近似）
				fd.roundedRects.push_back(Render::RoundedRectCmd{
					.rect = QRectF(cx - size / 2, cy - size / 4, size, size / 2),
					.radiusPx = 1.0f,
					.color = m_pal.expandIcon
					});
			}
			else {
				// 向右三角形（用矩形近似）
				fd.roundedRects.push_back(Render::RoundedRectCmd{
					.rect = QRectF(cx - size / 4, cy - size / 2, size / 2, size),
					.radiusPx = 1.0f,
					.color = m_pal.expandIcon
					});
			}
		}

		// 文字
		const int textX = vn.rect.left() + 32 + vn.depth * m_indentWidth;
		const int fontPx = std::lround(14.0f * m_dpr);

		QFont font;
		font.setPixelSize(fontPx);

		const QColor textColor = (info.level == 2) ? m_pal.textPrimary : m_pal.textSecondary;
		const QString key = QString("tree|%1|%2").arg(info.label).arg(textColor.name());

		const int tex = m_loader->ensureTextPx(key, font, info.label, textColor, m_gl);
		const QSize ts = m_loader->textureSizePx(tex);

		const float wLogical = static_cast<float>(ts.width()) / m_dpr;
		const float hLogical = static_cast<float>(ts.height()) / m_dpr;

		const QRectF textDst(
			textX,
			vn.rect.center().y() - hLogical * 0.5f,
			wLogical,
			hLogical
		);

		fd.images.push_back(Render::ImageCmd{
			.dstRect = textDst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
			.tint = QColor(255,255,255,255)
			});

		// 分类分隔线（示例：顶层节点下方）
		if (info.level == 0 && i < m_visibleNodes.size() - 1) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(vn.rect.left() + 8, vn.rect.bottom() - 1, vn.rect.width() - 16, 1),
				.radiusPx = 0.0f,
				.color = m_pal.separator
				});
		}
	}
}

bool UiTreeList::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;
	for (size_t i = 0; i < m_visibleNodes.size(); ++i) {
		if (m_visibleNodes[i].rect.contains(pos)) {
			m_pressed = static_cast<int>(i);
			return true;
		}
	}
	return false;
}

bool UiTreeList::onMouseMove(const QPoint& pos)
{
	int hov = -1;
	if (m_viewport.contains(pos)) {
		for (size_t i = 0; i < m_visibleNodes.size(); ++i) {
			if (m_visibleNodes[i].rect.contains(pos)) {
				hov = static_cast<int>(i);
				break;
			}
		}
	}
	const bool changed = (hov != m_hover);
	m_hover = hov;
	return changed;
}

bool UiTreeList::onMouseRelease(const QPoint& pos)
{
	const int wasPressed = m_pressed;
	m_pressed = -1;

	if (!m_viewport.contains(pos) || !m_model) {
		return (wasPressed >= 0);
	}

	if (wasPressed >= 0 && wasPressed < static_cast<int>(m_visibleNodes.size())) {
		const auto& vn = m_visibleNodes[wasPressed];
		if (vn.rect.contains(pos)) {
			// 检查是否点击展开/折叠
			if (!m_model->childIndices(vn.index).isEmpty()) {
				const QRect iconRect = expandIconRect(vn.rect, vn.depth);
				if (iconRect.adjusted(-4, -4, 4, 4).contains(pos)) {
					const bool isExpanded = m_model->nodeInfo(vn.index).expanded;
					m_model->setExpanded(vn.index, !isExpanded);
					reloadData();
					return true;
				}
			}
			// 设置选中
			m_model->setSelectedId(vn.index);
			return true;
		}
	}

	return (wasPressed >= 0);
}

bool UiTreeList::tick()
{
	if (!m_expandAnim.active) return false;
	if (!m_animClock.isValid()) m_animClock.start();

	// 简单的展开/折叠动画（留空或后续扩展）
	m_expandAnim.active = false;
	return false;
}