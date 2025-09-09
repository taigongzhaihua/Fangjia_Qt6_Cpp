#include "RenderOptimizer.hpp"
#include <algorithm>
#include <unordered_map>

namespace Render {

	void DirtyRegionManager::markDirty(const QRect& region) {
		if (region.isEmpty()) return;
		
		// 裁剪到视口范围内
		QRect clippedRegion = region;
		if (!m_viewport.isEmpty()) {
			clippedRegion = clippedRegion.intersected(m_viewport);
		}
		
		if (!clippedRegion.isEmpty()) {
			m_dirtyRegion = m_dirtyRegion.united(QRegion(clippedRegion));
		}
	}

	void DirtyRegionManager::markDirty(const std::vector<QRect>& regions) {
		for (const auto& region : regions) {
			markDirty(region);
		}
	}

	QRegion DirtyRegionManager::getDirtyRegion() const {
		return m_dirtyRegion;
	}

	bool DirtyRegionManager::hasDirtyRegions() const {
		return !m_dirtyRegion.isEmpty();
	}

	void DirtyRegionManager::clearDirtyRegions() {
		m_dirtyRegion = QRegion();
	}

	void DirtyRegionManager::setViewport(const QRect& viewport) {
		m_viewport = viewport;
		
		// 裁剪现有脏区域到新视口
		if (!m_dirtyRegion.isEmpty() && !viewport.isEmpty()) {
			m_dirtyRegion = m_dirtyRegion.intersected(QRegion(viewport));
		}
	}

	int DirtyRegionManager::optimizeRegions() {
		if (m_dirtyRegion.isEmpty()) return 0;
		
		// Qt6中使用begin()和end()迭代器而不是rects()方法
		QRegion::const_iterator it = m_dirtyRegion.begin();
		QRegion::const_iterator end = m_dirtyRegion.end();
		
		int regionCount = 0;
		for (; it != end; ++it) {
			regionCount++;
		}
		
		// 如果区域数量超过限制，合并为一个大区域
		if (regionCount > m_maxRegions) {
			m_dirtyRegion = QRegion(m_dirtyRegion.boundingRect());
			return 1;
		}
		
		return regionCount;
	}

	RenderOptimizer::RenderOptimizer(OptimizationFlags flags)
		: m_enabledFlags(flags) {
	}

	void RenderOptimizer::setOptimization(OptimizationFlags flags, bool enabled) {
		if (enabled) {
			m_enabledFlags = m_enabledFlags | flags;
		} else {
			m_enabledFlags = static_cast<OptimizationFlags>(
				static_cast<int>(m_enabledFlags) & ~static_cast<int>(flags));
		}
	}

	bool RenderOptimizer::isOptimizationEnabled(OptimizationFlags flags) const {
		return (m_enabledFlags & flags) == flags;
	}

	void RenderOptimizer::setViewport(const QRect& viewport) {
		m_viewport = viewport;
		m_dirtyRegionManager.setViewport(viewport);
	}

	std::vector<RoundedRectCmd> RenderOptimizer::optimizeRoundedRects(
		const std::vector<RoundedRectCmd>& commands) const {
		
		std::vector<RoundedRectCmd> optimized;
		optimized.reserve(commands.size());
		
		m_stats.totalCommands += commands.size();
		
		for (const auto& cmd : commands) {
			// 视口剔除
			if (isOptimizationEnabled(OptimizationFlags::ViewportCulling)) {
				if (!isInViewport(cmd.rect)) {
					m_stats.culledCommands++;
					continue;
				}
			}
			
			optimized.push_back(cmd);
		}
		
		// 深度排序
		if (isOptimizationEnabled(OptimizationFlags::DepthSorting)) {
			optimized = sortByDepth(optimized);
		}
		
		return optimized;
	}

	std::unordered_map<int, std::vector<ImageCmd>> RenderOptimizer::optimizeImages(
		const std::vector<ImageCmd>& commands) const {
		
		std::unordered_map<int, std::vector<ImageCmd>> batches;
		
		m_stats.totalCommands += commands.size();
		
		for (const auto& cmd : commands) {
			// 视口剔除
			if (isOptimizationEnabled(OptimizationFlags::ViewportCulling)) {
				if (!isInViewport(cmd.dstRect)) {
					m_stats.culledCommands++;
					continue;
				}
			}
			
			// 纹理批次分组
			if (isOptimizationEnabled(OptimizationFlags::TextureBatching)) {
				batches[cmd.textureId].push_back(cmd);
			} else {
				// 不启用批次化时，每个命令单独成批次
				batches[cmd.textureId * 1000 + batches.size()].push_back(cmd);
			}
		}
		
		// 统计批次化效果
		if (isOptimizationEnabled(OptimizationFlags::TextureBatching)) {
			int totalImageCommands = 0;
			for (const auto& [textureId, images] : batches) {
				totalImageCommands += images.size();
			}
			m_stats.batchedCommands += totalImageCommands;
		}
		
		return batches;
	}

	FrameData RenderOptimizer::optimizeFrameData(const FrameData& frameData) const {
		FrameData optimized;
		
		// 重置统计
		m_stats = OptimizationStats{};
		
		// 优化圆角矩形
		optimized.roundedRects = optimizeRoundedRects(frameData.roundedRects);
		
		// 优化图像命令
		auto imageBatches = optimizeImages(frameData.images);
		
		// 将批次重新展开为线性列表（保持兼容性）
		for (const auto& [textureId, images] : imageBatches) {
			for (const auto& img : images) {
				optimized.images.push_back(img);
			}
		}
		
		// 更新统计信息
		if (m_stats.totalCommands > 0) {
			m_stats.cullingRatio = static_cast<float>(m_stats.culledCommands) / m_stats.totalCommands;
			m_stats.batchingRatio = static_cast<float>(m_stats.batchedCommands) / m_stats.totalCommands;
		}
		
		// 计算脏区域数量
		const QRegion& dirtyRegion = m_dirtyRegionManager.getDirtyRegion();
		int regionCount = 0;
		for (auto it = dirtyRegion.begin(); it != dirtyRegion.end(); ++it) {
			regionCount++;
		}
		m_stats.dirtyRegions = regionCount;
		
		return optimized;
	}

	RenderOptimizer::OptimizationStats RenderOptimizer::getStats() const {
		return m_stats;
	}

	void RenderOptimizer::resetStats() {
		m_stats = OptimizationStats{};
	}

	bool RenderOptimizer::isInViewport(const QRectF& rect) const {
		if (m_viewport.isEmpty()) {
			return true; // 无视口限制时，所有对象都可见
		}
		
		// 简单的矩形相交检测
		return rect.intersects(QRectF(m_viewport));
	}

	template<typename T>
	std::vector<T> RenderOptimizer::sortByDepth(const std::vector<T>& commands) const {
		std::vector<T> sorted = commands;
		
		// 简单的深度排序：按Y坐标排序（远到近）
		std::sort(sorted.begin(), sorted.end(), [](const T& a, const T& b) {
			if constexpr (std::is_same_v<T, RoundedRectCmd>) {
				return a.rect.y() < b.rect.y();
			} else if constexpr (std::is_same_v<T, ImageCmd>) {
				return a.dstRect.y() < b.dstRect.y();
			}
			return false;
		});
		
		return sorted;
	}

	// 显式实例化模板
	template std::vector<RoundedRectCmd> RenderOptimizer::sortByDepth<RoundedRectCmd>(
		const std::vector<RoundedRectCmd>& commands) const;
	template std::vector<ImageCmd> RenderOptimizer::sortByDepth<ImageCmd>(
		const std::vector<ImageCmd>& commands) const;

}