#include "IconCache.h"
#include "IconLoader.h"

#include <QtGui/qopengl.h>
#include <qopenglfunctions.h>

int IconCache::createTextureFromImage(const QImage& imgRGBA, QOpenGLFunctions* gl)
{
	// Critical fix: Add null check for OpenGL functions
	if (!gl)
	{
		qCritical() << "Null OpenGL functions pointer in createTextureFromImage";
		return 0;
	}

	// Validate image data
	if (imgRGBA.isNull() || imgRGBA.width() <= 0 || imgRGBA.height() <= 0)
	{
		qWarning() << "Invalid image data in createTextureFromImage";
		return 0;
	}

	GLuint tex = 0;
	gl->glGenTextures(1, &tex);
	
	// Critical fix: Check texture generation success
	if (tex == 0)
	{
		qCritical() << "Failed to generate OpenGL texture";
		return 0;
	}

	// Check for OpenGL errors after texture generation
	GLenum glError = gl->glGetError();
	if (glError != GL_NO_ERROR)
	{
		qWarning() << "OpenGL error after texture generation:" << glError;
		gl->glDeleteTextures(1, &tex);
		return 0;
	}

	gl->glBindTexture(GL_TEXTURE_2D, tex);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgRGBA.width(), imgRGBA.height(),
		0, GL_RGBA, GL_UNSIGNED_BYTE, imgRGBA.constBits());

	// Check for OpenGL errors after texture upload
	glError = gl->glGetError();
	if (glError != GL_NO_ERROR)
	{
		qWarning() << "OpenGL error after texture upload:" << glError;
		gl->glDeleteTextures(1, &tex);
		return 0;
	}

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
	// Critical fix: Add null check and context validation for OpenGL operations
	if (!gl)
	{
		qWarning() << "Null OpenGL functions pointer in IconCache::releaseAll";
		// Still clear the cache even if we can't delete OpenGL textures
		m_cache.clear();
		m_idToSize.clear();
		return;
	}

	// Check for OpenGL errors before deletion
	GLenum glError = gl->glGetError();
	if (glError != GL_NO_ERROR)
	{
		qWarning() << "OpenGL error before texture cleanup:" << glError;
	}

	for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
		GLuint id = static_cast<GLuint>(it->id);
		if (id) 
		{
			gl->glDeleteTextures(1, &id);
			
			// Check for errors after each deletion (only in debug builds for performance)
			#ifdef _DEBUG
			glError = gl->glGetError();
			if (glError != GL_NO_ERROR)
			{
				qWarning() << "OpenGL error deleting texture" << id << ":" << glError;
			}
			#endif
		}
	}
	
	m_cache.clear();
	m_idToSize.clear();
}