#include "IconLoader.h"

#include <algorithm>
#include <qbytearray.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qglobal.h>
#include <qimage.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qsvgrenderer.h>

QImage IconLoader::toWhiteMask(const QImage& srcRgba8888)
{
	// 将 RGB 置为 255，保留 alpha（生成"白色蒙版"）
	QImage out = srcRgba8888;
	for (int y = 0; y < out.height(); ++y) {
		uchar* line = out.scanLine(y);
		for (int x = 0; x < out.width(); ++x) {
			uchar* px = line + x * 4;
			const uchar a = px[3];
			px[0] = 255; // R
			px[1] = 255; // G
			px[2] = 255; // B
			px[3] = a;   // A
		}
	}
	return out;
}

QImage IconLoader::renderSvgToImage(const QByteArray& svg, const QSize& pixelSize)
{
	QImage img(pixelSize, QImage::Format_ARGB32_Premultiplied);
	img.fill(Qt::transparent);
	{
		QPainter p(&img);
		p.setRenderHint(QPainter::Antialiasing, true);
		QSvgRenderer renderer(svg);
		renderer.render(&p, QRectF(QPointF(0, 0), QSizeF(pixelSize)));
	}
	const QImage rgba = img.convertToFormat(QImage::Format_RGBA8888);
	return toWhiteMask(rgba);
}

QImage IconLoader::renderGlyphToImage(const QFont& font, const QChar ch, const QSize& pixelSize, const QColor& color)
{
	QImage img(pixelSize, QImage::Format_ARGB32_Premultiplied);
	img.fill(Qt::transparent);
	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing, true);
	QFont f = font;
	f.setPixelSize(qRound(pixelSize.height() * 0.9));
	p.setFont(f);
	p.setPen(color);
	p.drawText(QRect(0, 0, pixelSize.width(), pixelSize.height()), Qt::AlignCenter, QString(ch));
	p.end();
	return img.convertToFormat(QImage::Format_RGBA8888);
}

QImage IconLoader::renderTextToImage(const QFont& fontPx, const QString& text, const QColor& color)
{
	const QFont& f = fontPx;
	const QFontMetrics fm(f);
	const int w = std::max(1, fm.horizontalAdvance(text));
	const int h = std::max(1, fm.height());

	QImage img(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
	img.fill(Qt::transparent);

	QPainter p(&img);
	p.setRenderHint(QPainter::TextAntialiasing, true);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QFont renderFont = f;
	renderFont.setHintingPreference(QFont::PreferVerticalHinting);
	renderFont.setStyleStrategy(QFont::PreferAntialias);
	p.setFont(renderFont);

	p.setPen(color);
	p.drawText(0, fm.ascent(), text);

	p.end();
	return img.convertToFormat(QImage::Format_RGBA8888);
}