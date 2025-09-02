#include "UiTreeList.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <IconCache.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qfont.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
#include <RenderData.hpp>
#include <RenderUtils.hpp>
#include <UiComponent.hpp>

UiTreeList::UiTreeList()
{
}

void UiTreeList::reloadData()
{
	updateVisibleNodes();
}

void UiTreeList::updateLayout(const QSize& /*windowSize*/)
{
	updateVisibleNodes();
}

void UiTreeList::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_cache = &cache;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);
}

void UiTreeList::updateVisibleNodes()
{
	m_visibleNodes.clear();

	// Support both traditional Model* and functional ModelFns
	auto getRoots = [&]() -> QVector<int>
		{
			if (m_model) return m_model->rootIndices();
			if (m_modelFns.rootIndices) return m_modelFns.rootIndices();
			return {};
		};

	auto getChildren = [&](int nodeId) -> QVector<int>
		{
			if (m_model) return m_model->childIndices(nodeId);
			if (m_modelFns.childIndices) return m_modelFns.childIndices(nodeId);
			return {};
		};

	auto getNodeInfo = [&](int nodeId) -> NodeInfo
		{
			if (m_model) return m_model->nodeInfo(nodeId);
			if (m_modelFns.nodeInfo) return m_modelFns.nodeInfo(nodeId);
			return {};
		};

	const auto roots = getRoots();

	std::function<void(int, int)> addVisibleChildren = [&](const int nodeId, const int depth)
		{
			for (const auto children = getChildren(nodeId); const int childId : children)
			{
				const auto info = getNodeInfo(childId);

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
				if (info.expanded)
				{
					addVisibleChildren(childId, depth + 1);
				}
			}
		};

	// 根层
	for (const int rootId : roots)
	{
		const auto info = getNodeInfo(rootId);

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

		if (info.expanded)
		{
			addVisibleChildren(rootId, 1);
		}
	}
}

int UiTreeList::contentHeight() const
{
	return static_cast<int>(m_visibleNodes.size()) * m_itemHeight;
}

QRect UiTreeList::nodeRect(const int visibleIdx) const
{
	if (visibleIdx < 0 || visibleIdx >= static_cast<int>(m_visibleNodes.size())) return {};
	return m_visibleNodes[visibleIdx].rect;
}

QRect UiTreeList::expandIconRect(const QRect& nodeRect, int /*depth*/) const
{
	// 右侧 8px 内边距，图标 16x16，垂直居中
	const int iconSize = 16;
	const int x = nodeRect.right() - 8 - iconSize;
	const int y = nodeRect.center().y() - iconSize / 2;
	return QRect(x, y, iconSize, iconSize);
}

void UiTreeList::append(Render::FrameData& fd) const
{
	if (!m_cache || !m_gl) return;

	// Helper functions for both Model* and ModelFns
	auto getSelectedId = [&]() -> int
		{
			if (m_model) return m_model->selectedId();
			if (m_modelFns.selectedId) return m_modelFns.selectedId();
			return -1;
		};

	auto getChildren = [&](int nodeId) -> QVector<int>
		{
			if (m_model) return m_model->childIndices(nodeId);
			if (m_modelFns.childIndices) return m_modelFns.childIndices(nodeId);
			return {};
		};

	auto getNodeInfo = [&](int nodeId) -> NodeInfo
		{
			if (m_model) return m_model->nodeInfo(nodeId);
			if (m_modelFns.nodeInfo) return m_modelFns.nodeInfo(nodeId);
			return {};
		};

	// 背景
	if (m_pal.bg.alpha() > 0 && m_viewport.isValid())
	{
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = 0.0f,
			.color = m_pal.bg,
			.clipRect = QRectF(m_viewport) // 新增
			});
	}

	if (!m_model && !m_modelFns.nodeInfo) return;

	const int selectedId = getSelectedId();

	for (size_t i = 0; i < m_visibleNodes.size(); ++i)
	{
		const auto& vn = m_visibleNodes[i];
		if (!vn.rect.intersects(m_viewport)) continue;

		const auto info = getNodeInfo(vn.index);

		// 统一的圆角矩形背景（选中/悬停/按下）——与 Nav 胶囊风格一致
		const QRectF inner = QRectF(vn.rect).adjusted(5, 3, -5, -3);
		if (vn.index == selectedId)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = inner,
				.radiusPx = 6.0f,
				.color = m_pal.itemSelected,
				.clipRect = QRectF(m_viewport)
				});
			// 仅选中态绘制左侧指示条
			const float indW = 3.0f;
			const float indH = std::clamp(inner.height() * 0.6, 12.0, inner.height() - 6.0);
			const QRectF ind(inner.left() + 4.0f, inner.center().y() - indH * 0.5f, indW, indH);
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = ind,
				.radiusPx = indW * 0.5f,
				.color = m_pal.indicator,
				.clipRect = QRectF(m_viewport)
				});
		}
		else if (static_cast<int>(i) == m_pressed)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = inner,
				.radiusPx = 6.0f,
				.color = m_pal.itemPressed,
				.clipRect = QRectF(m_viewport)
				});
		}
		else if (static_cast<int>(i) == m_hover)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = inner,
				.radiusPx = 6.0f,
				.color = m_pal.itemHover,
				.clipRect = QRectF(m_viewport)
				});
		}

		// 展开/折叠图标（右侧）：有子节点才绘制
		if (!getChildren(vn.index).isEmpty())
		{
			const QRect iconRect = expandIconRect(vn.rect, vn.depth);
			const int logical = 16;
			const int px = std::lround(static_cast<float>(logical) * m_dpr);
			const QString path = info.expanded
				? QStringLiteral(":/icons/tree_arrow_up.svg")
				: QStringLiteral(":/icons/tree_arrow_down.svg");
			const QString key = RenderUtils::makeIconCacheKey(info.expanded
				? QStringLiteral("tree_arrow_up")
				: QStringLiteral("tree_arrow_down"), px);
			QByteArray svg = RenderUtils::loadSvgCached(path);
			const int tex = m_cache->ensureSvgPx(key, svg, QSize(px, px), m_gl);
			const QSize ts = m_cache->textureSizePx(tex);
			const QRectF dst(iconRect.center().x() - logical * 0.5,
				iconRect.center().y() - logical * 0.5,
				logical, logical);
			fd.images.push_back(Render::ImageCmd{
				.dstRect = dst,
				.textureId = tex,
				.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
				.tint = m_pal.expandIcon,
				.clipRect = QRectF(m_viewport)
				});
		}

		// 文字
		const int textX = vn.rect.left() + 32 + vn.depth * m_indentWidth;
		const int fontPx = std::lround(14.0f * m_dpr);

		QFont font;
		font.setPixelSize(fontPx);

		const QColor textColor = (info.level == 2) ? m_pal.textPrimary : m_pal.textSecondary;
		const QString key = QString("tree|%1|%2").arg(info.label).arg(textColor.name());

		const int tex = m_cache->ensureTextPx(key, font, info.label, textColor, m_gl);
		const QSize ts = m_cache->textureSizePx(tex);

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
			.tint = QColor(255, 255, 255, 255),
			.clipRect = QRectF(m_viewport) // 新增：所有项裁剪到列表 viewport
			});

		// 分隔线
		if (info.level == 0 && i < m_visibleNodes.size() - 1)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(vn.rect.left() + 8, vn.rect.bottom() - 1, vn.rect.width() - 16, 1),
				.radiusPx = 0.0f,
				.color = m_pal.separator,
				.clipRect = QRectF(m_viewport)
				});
		}
	}
}

bool UiTreeList::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;
	for (size_t i = 0; i < m_visibleNodes.size(); ++i)
	{
		if (m_visibleNodes[i].rect.contains(pos))
		{
			m_pressed = static_cast<int>(i);
			return true;
		}
	}
	return false;
}

bool UiTreeList::onMouseMove(const QPoint& pos)
{
	int hov = -1;
	if (m_viewport.contains(pos))
	{
		for (size_t i = 0; i < m_visibleNodes.size(); ++i)
		{
			if (m_visibleNodes[i].rect.contains(pos))
			{
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

	if (!m_viewport.contains(pos) || (!m_model && !m_modelFns.nodeInfo))
	{
		return (wasPressed >= 0);
	}

	// Helper functions for both Model* and ModelFns
	auto getChildren = [&](int nodeId) -> QVector<int>
		{
			if (m_model) return m_model->childIndices(nodeId);
			if (m_modelFns.childIndices) return m_modelFns.childIndices(nodeId);
			return {};
		};

	auto getNodeInfo = [&](int nodeId) -> NodeInfo
		{
			if (m_model) return m_model->nodeInfo(nodeId);
			if (m_modelFns.nodeInfo) return m_modelFns.nodeInfo(nodeId);
			return {};
		};

	auto setExpanded = [&](int nodeId, bool expanded)
		{
			if (m_model) m_model->setExpanded(nodeId, expanded);
			else if (m_modelFns.setExpanded) m_modelFns.setExpanded(nodeId, expanded);
		};

	auto setSelectedId = [&](int nodeId)
		{
			if (m_model) m_model->setSelectedId(nodeId);
			else if (m_modelFns.setSelectedId) m_modelFns.setSelectedId(nodeId);
		};

	if (wasPressed >= 0 && wasPressed < static_cast<int>(m_visibleNodes.size()))
	{
		const auto& vn = m_visibleNodes[wasPressed];
		if (vn.rect.contains(pos))
		{
			// 检查是否点击展开/折叠
			if (!getChildren(vn.index).isEmpty())
			{
				const QRect iconRect = expandIconRect(vn.rect, vn.depth);
				if (iconRect.adjusted(-4, -4, 4, 4).contains(pos))
				{
					const bool isExpanded = getNodeInfo(vn.index).expanded;
					setExpanded(vn.index, !isExpanded);
					reloadData();
					return true;
				}
			}
			// 设置选中
			setSelectedId(vn.index);
			return true;
		}
	}

	return (wasPressed >= 0);
}

bool UiTreeList::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	// 检查位置是否在当前组件边界内
	if (!bounds().contains(pos))
	{
		return false;
	}

	// 计算滚动步长：基于 angleDelta.y()，默认 48px/刻度（120单位）
	constexpr int wheelStep = 48;
	const int deltaY = angleDelta.y();
	if (deltaY == 0)
	{
		return false;
	}

	// 计算滚动偏移（向上滚动为负值，向下滚动为正值）
	const int scrollDelta = -(deltaY * wheelStep) / 120;
	const int newScrollY = m_scrollY + scrollDelta;

	// 计算滚动范围限制
	const int maxScrollY = std::max(0, contentHeight() - m_viewport.height());
	const int clampedScrollY = std::clamp(newScrollY, 0, maxScrollY);

	// 设置新的滚动位置并更新可见节点
	m_scrollY = clampedScrollY;
	updateVisibleNodes();

	// 如果有滚动内容，则消费此事件
	return maxScrollY > 0;
}

bool UiTreeList::tick()
{
	// 当前没有内部动画
	return false;
}

void UiTreeList::onThemeChanged(bool isDark)
{
	IUiComponent::onThemeChanged(isDark);
	setPalette(
		isDark
		? Palette{
			.bg = QColor(30, 30, 30, 0),
			.itemHover = QColor(255, 255, 255, 14),
			.itemPressed = QColor(255, 255, 255, 26),
			.itemSelected = QColor(0, 122, 255, 32),
			.expandIcon = QColor(150, 150, 150, 200),
			.textPrimary = QColor(220, 220, 220, 255),
			.textSecondary = QColor(150, 160, 170, 200),
			.separator = QColor(255, 255, 255, 12),
			.indicator = QColor(0, 122, 255, 220)
		}
		: Palette{
			.bg = QColor(255, 255, 255, 0),
			.itemHover = QColor(0, 0, 0, 14),
			.itemPressed = QColor(0, 0, 0, 26),
			.itemSelected = QColor(0, 122, 255, 32),
			.expandIcon = QColor(100, 100, 100, 200),
			.textPrimary = QColor(32, 38, 46, 255),
			.textSecondary = QColor(100, 110, 120, 200),
			.separator = QColor(0, 0, 0, 20),
			.indicator = QColor(0, 102, 204, 220)
		}
	);

}
