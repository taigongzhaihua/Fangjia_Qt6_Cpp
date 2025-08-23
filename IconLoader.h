#pragma once
#include <qbytearray.h>
#include <qchar.h>
#include <qcolor.h>
#include <qfont.h>
#include <qhash.h>
#include <qimage.h>
#include <qopenglfunctions.h>
#include <qsize.h>
#include <qstring.h>

class QSvgRenderer;

// 仅在拥有当前 OpenGL 上下文的线程中调用这些接口（通常是 GUI/渲染线程）
class IconLoader {
public:
	IconLoader() = default;
	~IconLoader() = default;

	// 确保存在一个 SVG 纹理（像素尺寸指定）。返回 GL 纹理 id。
	int ensureSvgPx(const QString& key, const QByteArray& svgData, const QSize& pixelSize, const QColor& glyphColor, QOpenGLFunctions* gl);

	// 使用字体渲染一个字符为纹理。
	int ensureFontGlyphPx(const QString& key, const QFont& font, QChar glyph, const QSize& pixelSize, const QColor& glyphColor, QOpenGLFunctions* gl);

	// 使用字体渲染一段文本为纹理（像素尺寸由字体像素大小和文本长度决定）。
	// - key 必须包含区分文本内容、颜色、像素大小的要素（调用方生成）
	// - font 需已设置 pixelSize（像素单位）
	int ensureTextPx(const QString& key, const QFont& fontPx, const QString& text, const QColor& color, QOpenGLFunctions* gl);

	// 查询纹理像素尺寸
	[[nodiscard]] QSize textureSizePx(int texId) const;

	// 释放所有纹理（窗口销毁时调用）
	void releaseAll(QOpenGLFunctions* gl);

	// 工具：移除 SVG 文本中的 alpha（opacity/fill-opacity/stroke-opacity、rgba 的 a、#RRGGBBAA 的 AA）
	static QByteArray scrubSvgAlpha(const QByteArray& svgUtf8);

private:
	struct Tex {
		int   id{ 0 };
		QSize sizePx;
	};
	QHash<QString, Tex> m_cache;        // key -> Tex
	QHash<int, QSize>   m_idToSize;     // id  -> size

	static QImage renderSvgToImage(const QByteArray& svg, const QSize& pixelSize, const QColor& color);
	static QImage renderGlyphToImage(const QFont& font, QChar ch, const QSize& pixelSize, const QColor& color);
	static QImage renderTextToImage(const QFont& fontPx, const QString& text, const QColor& color);

	static int createTextureFromImage(const QImage& imgRGBA, QOpenGLFunctions* gl);
};