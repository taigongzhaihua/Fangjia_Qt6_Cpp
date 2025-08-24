#include "UiFormulaDetail.h"
#include <qfont.h>
#include <algorithm>

void UiFormulaDetail::setFormula(const FormulaViewModel::FormulaDetail* formula)
{
    m_formula = formula;
    // 估算内容高度
    m_contentHeight = formula ? 600 : 0; // 简化处理，实际应根据内容计算
}

void UiFormulaDetail::updateLayout(const QSize& /*windowSize*/)
{
    // 重新计算内容高度
    if (m_formula) {
        m_contentHeight = 600; // 简化处理
    }
}

void UiFormulaDetail::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
    m_loader = &loader;
    m_gl = gl;
    m_dpr = std::max(0.5f, devicePixelRatio);
}

int UiFormulaDetail::contentHeight() const
{
    return m_contentHeight;
}

void UiFormulaDetail::append(Render::FrameData& fd) const
{
    if (!m_formula || !m_loader || !m_gl) return;
    
    // 背景
    fd.roundedRects.push_back(Render::RoundedRectCmd{
        .rect = QRectF(m_viewport),
        .radiusPx = 0.0f,
        .color = m_pal.bg
    });
    
    int y = m_viewport.top() + 20 - m_scrollY;
    
    // 方剂名称（大标题）
    {
        const int titleFontPx = std::lround(22.0f * m_dpr);
        QFont font;
        font.setPixelSize(titleFontPx);
        font.setWeight(QFont::Bold);
        
        const QString key = QString("formula_title|%1").arg(m_formula->name);
        const int tex = m_loader->ensureTextPx(key, font, m_formula->name, m_pal.titleColor, m_gl);
        const QSize ts = m_loader->textureSizePx(tex);
        
        const float wLogical = static_cast<float>(ts.width()) / m_dpr;
        const float hLogical = static_cast<float>(ts.height()) / m_dpr;
        
        const QRectF textDst(
            m_viewport.left() + 24,
            y,
            wLogical,
            hLogical
        );
        
        fd.images.push_back(Render::ImageCmd{
            .dstRect = textDst,
            .textureId = tex,
            .srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
            .tint = QColor(255,255,255,255)
        });
        
        y += static_cast<int>(hLogical) + 20;
    }
    
    // 各个段落
    drawSection(const_cast<Render::FrameData&>(fd), "出处", m_formula->source, const_cast<int&>(y));
    drawSection(const_cast<Render::FrameData&>(fd), "组成", m_formula->composition, const_cast<int&>(y));
    drawSection(const_cast<Render::FrameData&>(fd), "用法", m_formula->usage, const_cast<int&>(y));
    drawSection(const_cast<Render::FrameData&>(fd), "功效", m_formula->function, const_cast<int&>(y));
    drawSection(const_cast<Render::FrameData&>(fd), "主治", m_formula->indication, const_cast<int&>(y));
    
    if (!m_formula->note.isEmpty()) {
        drawSection(const_cast<Render::FrameData&>(fd), "备注", m_formula->note, const_cast<int&>(y));
    }
}

void UiFormulaDetail::drawSection(Render::FrameData& fd, const QString& label, const QString& content, int& y) const
{
    if (content.isEmpty()) return;
    
    const int fontPx = std::lround(14.0f * m_dpr);
    const int labelFontPx = std::lround(13.0f * m_dpr);
    
    // 绘制标签
    {
        QFont font;
        font.setPixelSize(labelFontPx);
        font.setWeight(QFont::DemiBold);
        
        const QString key = QString("label|%1").arg(label);
        const int tex = m_loader->ensureTextPx(key, font, label + "：", m_pal.labelColor, m_gl);
        const QSize ts = m_loader->textureSizePx(tex);
        
        const float wLogical = static_cast<float>(ts.width()) / m_dpr;
        const float hLogical = static_cast<float>(ts.height()) / m_dpr;
        
        fd.images.push_back(Render::ImageCmd{
            .dstRect = QRectF(m_viewport.left() + 24, y, wLogical, hLogical),
            .textureId = tex,
            .srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
            .tint = QColor(255,255,255,255)
        });
        
        y += static_cast<int>(hLogical) + 8;
    }
    
    // 绘制内容
    {
        QFont font;
        font.setPixelSize(fontPx);
        
        // 简化处理：假设内容不换行
        const QString key = QString("content|%1|%2").arg(label).arg(content.left(20));
        const int tex = m_loader->ensureTextPx(key, font, content, m_pal.textColor, m_gl);
        const QSize ts = m_loader->textureSizePx(tex);
        
        const float wLogical = static_cast<float>(ts.width()) / m_dpr;
        const float hLogical = static_cast<float>(ts.height()) / m_dpr;
        
        // 限制最大宽度
        const float maxWidth = m_viewport.width() - 48;
        const float displayW = std::min(wLogical, maxWidth);
        
        fd.images.push_back(Render::ImageCmd{
            .dstRect = QRectF(m_viewport.left() + 40, y, displayW, hLogical),
            .textureId = tex,
            .srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
            .tint = QColor(255,255,255,255)
        });
        
        y += static_cast<int>(hLogical) + 16;
    }
}