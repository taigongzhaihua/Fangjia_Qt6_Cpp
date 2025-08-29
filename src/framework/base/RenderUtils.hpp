/*
 * 文件名：RenderUtils.hpp
 * 职责：渲染辅助工具集，提供剪裁处理、缓存键生成和SVG加载等通用功能。
 * 依赖：渲染数据结构、Qt Core。
 * 线程：多数函数线程安全，SVG缓存使用线程局部存储。
 * 备注：内联函数优化性能，缓存键设计需考虑所有影响渲染结果的参数。
 */

#pragma once
#include "RenderData.hpp"

#include <qbytearray.h>
#include <qcolor.h>
#include <qfile.h>
#include <qhash.h>
#include <qiodevice.h>
#include <qrect.h>
#include <qstring.h>

namespace RenderUtils {

	/// 功能：对新增绘制命令应用父级剪裁区域
	/// 参数：fd — 帧数据容器
	/// 参数：rr0 — 圆角矩形命令的起始索引
	/// 参数：im0 — 图像命令的起始索引  
	/// 参数：parentClip — 父级剪裁矩形（逻辑像素）
	/// 说明：将父容器的剪裁区域与子组件的剪裁区域求交，实现剪裁层级传递
	inline void applyParentClip(Render::FrameData& fd, const int rr0, const int im0, const QRectF& parentClip) {
		if (parentClip.width() <= 0.0 || parentClip.height() <= 0.0) return;

		for (int i = rr0; i < static_cast<int>(fd.roundedRects.size()); ++i) {
			auto& cmd = fd.roundedRects[i];
			if (cmd.clipRect.width() > 0.0 && cmd.clipRect.height() > 0.0) {
				cmd.clipRect = cmd.clipRect.intersected(parentClip);
			}
			else {
				cmd.clipRect = parentClip;
			}
		}
		for (int i = im0; i < static_cast<int>(fd.images.size()); ++i) {
			auto& cmd = fd.images[i];
			if (cmd.clipRect.width() > 0.0 && cmd.clipRect.height() > 0.0) {
				cmd.clipRect = cmd.clipRect.intersected(parentClip);
			}
			else {
				cmd.clipRect = parentClip;
			}
		}
	}

	/// 功能：生成文本纹理的统一缓存键
	/// 参数：baseKey — 基础键值（通常包含文本内容）
	/// 参数：fontPx — 字体像素大小
	/// 参数：color — 文本颜色
	/// 返回：包含所有渲染参数的唯一缓存键
	/// 说明：确保相同文本、字体、颜色组合复用纹理，不同组合生成新纹理
	inline QString makeTextCacheKey(const QString& baseKey, const int fontPx, const QColor& color) {
		const QString colorKey = color.name(QColor::HexArgb);
		return QString("txt:%1@%2px@%3").arg(baseKey).arg(fontPx).arg(colorKey);
	}

	/// 功能：生成图标纹理的统一缓存键
	/// 参数：baseKey — 图标基础键值（通常为文件名或标识符）
	/// 参数：pixelSize — 渲染像素尺寸
	/// 参数：variant — 可选的变体标识（如主题、状态等）
	/// 返回：唯一的图标缓存键
	/// 说明：支持同一图标的多尺寸、多变体缓存
	inline QString makeIconCacheKey(const QString& baseKey, const int pixelSize, const QString& variant = QString()) {
		if (variant.isEmpty()) {
			return QString("icon:%1@%2px").arg(baseKey).arg(pixelSize);
		}
		return QString("icon:%1@%2@%3px").arg(baseKey, variant).arg(pixelSize);
	}

	/// 功能：加载SVG文件数据并进行线程局部缓存
	/// 参数：path — SVG文件路径
	/// 返回：SVG文件的字节数据，失败时返回空
	/// 说明：使用线程局部缓存避免重复文件读取，提高性能
	inline QByteArray loadSvgCached(const QString& path) {
		thread_local QHash<QString, QByteArray> cache;
		if (const auto it = cache.find(path); it != cache.end()) return it.value();
		QFile f(path);
		if (!f.open(QIODevice::ReadOnly)) return {};
		QByteArray data = f.readAll();
		cache.insert(path, data);
		return data;
	}

} // namespace RenderUtils