#include "IconLoader.h"

#include <algorithm>
#include <gl/GL.h>
#include <qbytearray.h>
#include <qchar.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qnamespace.h>
#include <qnumeric.h>
#include <qopenglext.h>
#include <qopenglfunctions.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qsvgrenderer.h>

QImage IconLoader::renderSvgToImage(const QByteArray& svg, const QSize& pixelSize)
{
	QImage img(pixelSize, QImage::Format_ARGB32_Premultiplied);
	img.fill(Qt::transparent);
	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing, true);
	QSvgRenderer renderer(svg);
	renderer.render(&p, QRectF(QPointF(0, 0), QSizeF(pixelSize)));
	p.end();
	return img.convertToFormat(QImage::Format_RGBA8888);
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
	p.setFont(f);
	p.setPen(color);

	// 基线绘制：让文本在垂直方向完整显示
	p.drawText(0, fm.ascent(), text);

	p.end();
	return img.convertToFormat(QImage::Format_RGBA8888);
}

int IconLoader::createTextureFromImage(const QImage& imgRGBA, QOpenGLFunctions* gl)
{
	GLuint tex = 0;
	gl->glGenTextures(1, &tex);
	gl->glBindTexture(GL_TEXTURE_2D, tex);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgRGBA.width(), imgRGBA.height(),
		0, GL_RGBA, GL_UNSIGNED_BYTE, imgRGBA.constBits());
	return static_cast<int>(tex);
}

int IconLoader::ensureSvgPx(const QString& key, const QByteArray& svgData, const QSize& pixelSize, QOpenGLFunctions* gl)
{
	if (const auto it = m_cache.find(key); it != m_cache.end()) {
		return it->id;
	}
	const QImage img = renderSvgToImage(svgData, pixelSize);
	const int id = createTextureFromImage(img, gl);
	m_cache.insert(key, Tex{ .id = id, .sizePx = img.size() });
	m_idToSize.insert(id, img.size());
	return id;
}

int IconLoader::ensureFontGlyphPx(const QString& key, const QFont& font, const QChar glyph, const QSize& pixelSize, const QColor& glyphColor, QOpenGLFunctions* gl)
{
	if (const auto it = m_cache.find(key); it != m_cache.end()) {
		return it->id;
	}
	const QImage img = renderGlyphToImage(font, glyph, pixelSize, glyphColor);
	const int id = createTextureFromImage(img, gl);
	m_cache.insert(key, Tex{ .id = id, .sizePx = img.size() });
	m_idToSize.insert(id, img.size());
	return id;
}

int IconLoader::ensureTextPx(const QString& key, const QFont& fontPx, const QString& text, const QColor& color, QOpenGLFunctions* gl)
{
	if (const auto it = m_cache.find(key); it != m_cache.end()) {
		return it->id;
	}
	const QImage img = renderTextToImage(fontPx, text, color);
	const int id = createTextureFromImage(img, gl);
	m_cache.insert(key, Tex{ .id = id, .sizePx = img.size() });
	m_idToSize.insert(id, img.size());
	return id;
}

QSize IconLoader::textureSizePx(const int texId) const
{
	const auto it = m_idToSize.find(texId);
	return (it != m_idToSize.end()) ? *it : QSize();
}

void IconLoader::releaseAll(QOpenGLFunctions* gl)
{
	for (const auto& it : m_cache)
	{
		GLuint id = static_cast<GLuint>(it.id);
		if (id) gl->glDeleteTextures(1, &id);
	}
	m_cache.clear();
	m_idToSize.clear();
}