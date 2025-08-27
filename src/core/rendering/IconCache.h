#pragma once
#include <qbytearray.h>
#include <qchar.h>
#include <qcolor.h>
#include <qfont.h>
#include <qhash.h>
#include <qopenglfunctions.h>
#include <qsize.h>
#include <qstring.h>

// 管理图标/文本纹理缓存与 GL 生命周期
// 仅在拥有当前 OpenGL 上下文的线程中调用这些接口（通常是 GUI/渲染线程）
class IconCache {
public:
	IconCache() = default;
	~IconCache() = default;

	// 确保存在一个 SVG 纹理（像素尺寸指定）。返回 GL 纹理 id。
	// 移除了未使用的颜色参数
	int ensureSvgPx(const QString& key, const QByteArray& svgData, const QSize& pixelSize, QOpenGLFunctions* gl);

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

private:
	struct Tex {
		int   id{ 0 };
		QSize sizePx;
	};
	QHash<QString, Tex> m_cache;        // key -> Tex
	QHash<int, QSize>   m_idToSize;     // id  -> size

	static int createTextureFromImage(const QImage& imgRGBA, QOpenGLFunctions* gl);
};