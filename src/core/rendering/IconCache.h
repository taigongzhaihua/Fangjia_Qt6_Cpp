/*
 * 文件名：IconCache.h  
 * 职责：图标与文本纹理缓存管理，负责SVG渲染、字体栅格化和OpenGL纹理生命周期。
 * 依赖：Qt6 OpenGL/Gui/Svg。
 * 线程：仅在拥有OpenGL上下文的线程中使用（通常为UI线程）。
 * 备注：所有纹理创建和销毁必须在同一OpenGL上下文中进行，支持白膜（tint）策略。
 */

#pragma once
#include <qbytearray.h>
#include <qcolor.h>
#include <qhash.h>
#include <qopenglfunctions.h>
#include <qsize.h>
#include <qstring.h>

/// 图标与文本纹理缓存：管理OpenGL纹理资源的创建、缓存和释放
/// 
/// 功能：
/// - SVG图标渲染为OpenGL纹理（支持指定像素尺寸）  
/// - 字体字符栅格化（单字符精确控制）
/// - 文本字符串渲染（多字符组合，支持颜色）
/// - 纹理生命周期管理（创建、查询、释放）
/// 
/// 白膜策略：
/// - SVG图标通常为单色白色模板，运行时通过着色器着色
/// - 文本渲染直接生成带颜色的纹理，无需额外着色
class IconCache {
public:
	IconCache() = default;
	~IconCache() = default;

	/// 功能：确保SVG图标纹理存在
	/// 参数：key — 缓存键（由调用方生成，需包含尺寸等区分要素）
	/// 参数：svgData — SVG文件的字节数据
	/// 参数：pixelSize — 目标渲染尺寸（设备像素）
	/// 参数：gl — OpenGL函数表
	/// 返回：OpenGL纹理ID
	/// 说明：相同key的重复调用会直接返回已缓存的纹理
	int ensureSvgPx(const QString& key, const QByteArray& svgData, const QSize& pixelSize, QOpenGLFunctions* gl);

	/// 功能：渲染单个字符为纹理
	/// 参数：key — 缓存键（需包含字符、字体、尺寸、颜色等要素）
	/// 参数：font — 字体对象
	/// 参数：glyph — 要渲染的字符
	/// 参数：pixelSize — 渲染尺寸（设备像素）
	/// 参数：glyphColor — 字符颜色
	/// 参数：gl — OpenGL函数表
	/// 返回：OpenGL纹理ID
	int ensureFontGlyphPx(const QString& key, const QFont& font, QChar glyph, const QSize& pixelSize, const QColor& glyphColor, QOpenGLFunctions* gl);

	/// 功能：渲染文本字符串为纹理  
	/// 参数：key — 缓存键（必须包含文本内容、颜色、字体大小等区分要素）
	/// 参数：fontPx — 字体对象（需已设置像素大小）
	/// 参数：text — 要渲染的文本字符串
	/// 参数：color — 文本颜色
	/// 参数：gl — OpenGL函数表
	/// 返回：OpenGL纹理ID
	/// 说明：纹理尺寸由字体像素大小和文本长度自动计算
	int ensureTextPx(const QString& key, const QFont& fontPx, const QString& text, const QColor& color, QOpenGLFunctions* gl);

	/// 功能：查询纹理的像素尺寸
	/// 参数：texId — OpenGL纹理ID
	/// 返回：纹理的像素尺寸
	[[nodiscard]] QSize textureSizePx(int texId) const;

	/// 功能：释放所有缓存的纹理
	/// 参数：gl — OpenGL函数表
	/// 说明：在窗口或OpenGL上下文销毁时调用
	void releaseAll(QOpenGLFunctions* gl);

private:
	struct Tex {
		int   id{ 0 };        // OpenGL纹理ID
		QSize sizePx;         // 纹理尺寸（设备像素）
	};
	QHash<QString, Tex> m_cache;        // 缓存键 -> 纹理信息
	QHash<int, QSize>   m_idToSize;     // 纹理ID -> 尺寸快速查询

	/// 功能：从RGBA图像创建OpenGL纹理
	/// 参数：imgRGBA — 32位RGBA格式的图像数据
	/// 参数：gl — OpenGL函数表  
	/// 返回：创建的OpenGL纹理ID
	static int createTextureFromImage(const QImage& imgRGBA, QOpenGLFunctions* gl);
};