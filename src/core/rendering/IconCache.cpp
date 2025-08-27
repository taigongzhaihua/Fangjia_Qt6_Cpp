#include "IconCache.h"
#include "IconLoader.h"

#include <QtGui/qopengl.h>
#include <qopenglfunctions.h>

int IconCache::createTextureFromImage(const QImage& imgRGBA, QOpenGLFunctions* gl)
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

int IconCache::ensureSvgPx(const QString& key, const QByteArray& svgData, const QSize& pixelSize, QOpenGLFunctions* gl)
{
	if (const auto it = m_cache.find(key); it != m_cache.end()) {
		return it->id;
	}
	const QImage img = IconLoader::renderSvgToImage(svgData, pixelSize);
	const int id = createTextureFromImage(img, gl);
	m_cache.insert(key, Tex{ .id = id, .sizePx = img.size() });
	m_idToSize.insert(id, img.size());
	return id;
}

int IconCache::ensureFontGlyphPx(const QString& key, const QFont& font, const QChar glyph, const QSize& pixelSize, const QColor& glyphColor, QOpenGLFunctions* gl)
{
	if (const auto it = m_cache.find(key); it != m_cache.end()) {
		return it->id;
	}
	const QImage img = IconLoader::renderGlyphToImage(font, glyph, pixelSize, glyphColor);
	const int id = createTextureFromImage(img, gl);
	m_cache.insert(key, Tex{ .id = id, .sizePx = img.size() });
	m_idToSize.insert(id, img.size());
	return id;
}

int IconCache::ensureTextPx(const QString& key, const QFont& fontPx, const QString& text, const QColor& color, QOpenGLFunctions* gl)
{
	if (const auto it = m_cache.find(key); it != m_cache.end()) {
		return it->id;
	}
	const QImage img = IconLoader::renderTextToImage(fontPx, text, color);
	const int id = createTextureFromImage(img, gl);
	m_cache.insert(key, Tex{ .id = id, .sizePx = img.size() });
	m_idToSize.insert(id, img.size());
	return id;
}

QSize IconCache::textureSizePx(const int texId) const
{
	const auto it = m_idToSize.find(texId);
	return (it != m_idToSize.end()) ? *it : QSize();
}

void IconCache::releaseAll(QOpenGLFunctions* gl)
{
	for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
		GLuint id = static_cast<GLuint>(it->id);
		if (id) gl->glDeleteTextures(1, &id);
	}
	m_cache.clear();
	m_idToSize.clear();
}