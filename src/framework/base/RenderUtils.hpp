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

	// 将 [rr0, end) 和 [im0, end) 区间内新增的命令与父裁剪矩形求交；
	// 若原命令无 clipRect，则直接赋值为父裁剪。
	inline void applyParentClip(Render::FrameData& fd, int rr0, int im0, const QRectF& parentClip) {
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

	// 统一文本纹理缓存键：包含颜色（HexArgb）
	inline QString makeTextCacheKey(const QString& baseKey, int fontPx, const QColor& color) {
		const QString colorKey = color.name(QColor::HexArgb);
		return QString("txt:%1@%2px@%3").arg(baseKey).arg(fontPx).arg(colorKey);
	}

	// 统一图标纹理缓存键
	inline QString makeIconCacheKey(const QString& baseKey, int pixelSize, const QString& variant = QString()) {
		if (variant.isEmpty()) {
			return QString("icon:%1@%2px").arg(baseKey).arg(pixelSize);
		}
		return QString("icon:%1@%2@%3px").arg(baseKey, variant).arg(pixelSize);
	}

	// 统一 SVG 数据缓存（线程局部缓存）
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