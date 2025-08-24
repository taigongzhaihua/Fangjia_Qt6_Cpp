#include "UiTreeList.h"
#include <algorithm>
#include <qfont.h>
#include <qlogging.h>
#include <cmath>

UiTreeList::UiTreeList()
{
}

void UiTreeList::setViewModel(FormulaViewModel* vm)
{
    if (m_vm == vm) return;
    m_vm = vm;
    
    m_hover = -1;
    m_pressed = -1;
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
    if (!m_vm) return;
    
    const auto& nodes = m_vm->nodes();
    
    // 递归构建可见节点列表
    std::function<void(int, int)> addVisibleChildren = [&](int parentIdx, int depth) {
        auto children = m_vm->childIndices(parentIdx);
        for (int childIdx : children) {
            const auto& node = nodes[childIdx];
            
            // 添加节点
            VisibleNode vn;
            vn.index = childIdx;
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
            if (node.expanded) {
                addVisibleChildren(childIdx, depth + 1);
            }
        }
    };
    
    // 从根节点开始
    addVisibleChildren(-1, 0);
}

int UiTreeList::contentHeight() const
{
    return static_cast<int>(m_visibleNodes.size()) * m_itemHeight;
}

QRect UiTreeList::nodeRect(int visibleIdx) const
{
    if (visibleIdx < 0 || visibleIdx >= static_cast<int>(m_visibleNodes.size())) {
        return {};
    }
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
    if (!m_vm || !m_loader || !m_gl) return;
    
    // 背景
    if (m_pal.bg.alpha() > 0) {
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(m_viewport),
            .radiusPx = 0.0f,
            .color = m_pal.bg
        });
    }
    
    const auto& nodes = m_vm->nodes();
    const int selectedIdx = m_vm->selectedIndex();
    
    // 绘制可见节点
    for (size_t i = 0; i < m_visibleNodes.size(); ++i) {
        const auto& vn = m_visibleNodes[i];
        
        // 跳过视口外的节点
        if (!vn.rect.intersects(m_viewport)) continue;
        
        const auto& node = nodes[vn.index];
        
        // 背景（选中/悬停）
        if (vn.index == selectedIdx) {
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(vn.rect),
                .radiusPx = 0.0f,
                .color = m_pal.itemSelected
            });
        } else if (static_cast<int>(i) == m_hover) {
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(vn.rect),
                .radiusPx = 0.0f,
                .color = m_pal.itemHover
            });
        }
        
        // 展开/折叠图标（非叶子节点）
        if (!m_vm->childIndices(vn.index).isEmpty()) {
            const QRect iconRect = expandIconRect(vn.rect, vn.depth);
            
            // 简单绘制三角形图标
            const float cx = iconRect.center().x();
            const float cy = iconRect.center().y();
            const float size = 6.0f;
            
            if (node.expanded) {
                // 向下三角形
                fd.roundedRects.push_back(Render::RoundedRectCmd{
                    .rect = QRectF(cx - size/2, cy - size/4, size, size/2),
                    .radiusPx = 1.0f,
                    .color = m_pal.expandIcon
                });
            } else {
                // 向右三角形
                fd.roundedRects.push_back(Render::RoundedRectCmd{
                    .rect = QRectF(cx - size/4, cy - size/2, size/2, size),
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
        
        const QColor textColor = (node.level == 2) ? m_pal.textPrimary : m_pal.textSecondary;
        const QString key = QString("tree|%1|%2").arg(node.label).arg(textColor.name());
        
        const int tex = m_loader->ensureTextPx(key, font, node.label, textColor, m_gl);
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
        
        // 分隔线（分类节点下方）
        if (node.level == 0 && i < m_visibleNodes.size() - 1) {
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
    
    if (!m_viewport.contains(pos) || !m_vm) {
        return (wasPressed >= 0);
    }
    
    if (wasPressed >= 0 && wasPressed < static_cast<int>(m_visibleNodes.size())) {
        const auto& vn = m_visibleNodes[wasPressed];
        if (vn.rect.contains(pos)) {
            const auto& node = m_vm->nodes()[vn.index];
            
            // 检查是否点击了展开/折叠图标
            if (!m_vm->childIndices(vn.index).isEmpty()) {
                const QRect iconRect = expandIconRect(vn.rect, vn.depth);
                if (iconRect.adjusted(-4, -4, 4, 4).contains(pos)) {
                    m_vm->toggleExpanded(vn.index);
                    updateVisibleNodes();
                    return true;
                }
            }
            
            // 选中节点
            m_vm->setSelectedIndex(vn.index);
            return true;
        }
    }
    
    return (wasPressed >= 0);
}

bool UiTreeList::tick()
{
    if (!m_expandAnim.active) return false;
    
    if (!m_animClock.isValid()) m_animClock.start();
    
    // 简单的展开/折叠动画
    m_expandAnim.progress += 0.1f;
    if (m_expandAnim.progress >= 1.0f) {
        m_expandAnim.active = false;
        updateVisibleNodes();
    }
    
    return m_expandAnim.active;
}