#pragma once
#include <qchar.h>
#include <qcolor.h>
#include <qfont.h>
#include <qimage.h>
#include <qsize.h>
#include <qstring.h>

class QSvgRenderer;

// 无状态 Helper：仅负责将 SVG/字体/文本栅格化为 QImage
class IconLoader {
public:
	// 渲染 SVG 为 QImage（移除了未使用的颜色参数）
	static QImage renderSvgToImage(const QByteArray& svg, const QSize& pixelSize);

	// 渲染字符为 QImage
	static QImage renderGlyphToImage(const QFont& font, QChar ch, const QSize& pixelSize, const QColor& color);

	// 渲染文本为 QImage
	static QImage renderTextToImage(const QFont& fontPx, const QString& text, const QColor& color);

	// 将 QImage 转换为白色蒙版（用于 tint 着色）
	static QImage toWhiteMask(const QImage& srcRgba8888);
};