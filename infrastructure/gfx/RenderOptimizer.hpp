/*
 * 文件名：RenderOptimizer.hpp
 * 职责：渲染性能优化器，提供脏区域更新、视口剔除和渲染批次优化
 * 依赖：Qt6 Core、RenderData.hpp
 * 线程：线程安全的优化策略配置，实际优化在渲染线程执行
 * 备注：实现各种渲染性能优化策略，可独立启用/禁用各个优化功能
 */

#pragma once
#include "RenderData.hpp"
#include <qrect.h>
#include <qregion.h>
#include <vector>
#include <unordered_set>

namespace Render {

	/// 脏区域管理器：跟踪需要重绘的屏幕区域
	/// 
	/// 功能：
	/// - 收集和合并脏区域
	/// - 最小化重绘范围
	/// - 支持增量更新策略
	class DirtyRegionManager {
	public:
		DirtyRegionManager() = default;
		~DirtyRegionManager() = default;

		/// 功能：标记区域为脏（需要重绘）
		/// 参数：region — 脏区域矩形
		void markDirty(const QRect& region);

		/// 功能：批量标记多个区域为脏
		/// 参数：regions — 脏区域列表
		void markDirty(const std::vector<QRect>& regions);

		/// 功能：获取所有脏区域
		/// 返回：当前的脏区域集合
		[[nodiscard]] QRegion getDirtyRegion() const;

		/// 功能：检查是否有脏区域
		/// 返回：true表示有区域需要重绘
		[[nodiscard]] bool hasDirtyRegions() const;

		/// 功能：清空所有脏区域
		void clearDirtyRegions();

		/// 功能：设置视口尺寸（用于区域裁剪）
		/// 参数：viewport — 视口矩形
		void setViewport(const QRect& viewport);

		/// 功能：优化脏区域（合并相邻区域）
		/// 返回：优化后的脏区域数量
		int optimizeRegions();

	private:
		QRegion m_dirtyRegion;          ///< 累积的脏区域
		QRect m_viewport;               ///< 当前视口
		int m_maxRegions{50};           ///< 最大区域数量限制
	};

	/// 渲染性能优化器：集成各种优化策略
	/// 
	/// 功能：
	/// - 视口剔除优化
	/// - 脏区域更新
	/// - 渲染批次合并
	/// - 性能统计和监控
	class RenderOptimizer {
	public:
		/// 优化策略标志
		enum class OptimizationFlags {
			None = 0,
			ViewportCulling = 1 << 0,     ///< 视口剔除
			DirtyRegions = 1 << 1,        ///< 脏区域更新
			TextureBatching = 1 << 2,     ///< 纹理批次合并
			DepthSorting = 1 << 3,        ///< 深度排序优化
			All = ViewportCulling | DirtyRegions | TextureBatching | DepthSorting
		};

		/// 优化统计信息
		struct OptimizationStats {
			int totalCommands{0};         ///< 原始命令总数
			int culledCommands{0};        ///< 被剔除的命令数
			int batchedCommands{0};       ///< 批次合并的命令数
			int dirtyRegions{0};          ///< 脏区域数量
			float cullingRatio{0.0f};     ///< 剔除比例
			float batchingRatio{0.0f};    ///< 批次化比例
		};

		explicit RenderOptimizer(OptimizationFlags flags = OptimizationFlags::All);
		~RenderOptimizer() = default;

		/// 功能：启用/禁用优化策略
		/// 参数：flags — 优化策略标志
		/// 参数：enabled — 是否启用
		void setOptimization(OptimizationFlags flags, bool enabled);

		/// 功能：检查优化策略是否启用
		/// 参数：flags — 要检查的优化策略
		/// 返回：true表示已启用
		[[nodiscard]] bool isOptimizationEnabled(OptimizationFlags flags) const;

		/// 功能：设置渲染视口
		/// 参数：viewport — 视口矩形
		void setViewport(const QRect& viewport);

		/// 功能：优化圆角矩形命令列表
		/// 参数：commands — 输入命令列表
		/// 返回：优化后的命令列表
		[[nodiscard]] std::vector<RoundedRectCmd> optimizeRoundedRects(
			const std::vector<RoundedRectCmd>& commands) const;

		/// 功能：优化图像命令列表
		/// 参数：commands — 输入命令列表
		/// 返回：按纹理分组的优化命令映射
		[[nodiscard]] std::unordered_map<int, std::vector<ImageCmd>> optimizeImages(
			const std::vector<ImageCmd>& commands) const;

		/// 功能：优化整个帧数据
		/// 参数：frameData — 输入帧数据
		/// 返回：优化后的帧数据
		[[nodiscard]] FrameData optimizeFrameData(const FrameData& frameData) const;

		/// 功能：获取脏区域管理器
		/// 返回：脏区域管理器的引用
		DirtyRegionManager& getDirtyRegionManager() { return m_dirtyRegionManager; }

		/// 功能：获取优化统计信息
		/// 返回：当前的优化统计数据
		[[nodiscard]] OptimizationStats getStats() const;

		/// 功能：重置统计计数器
		void resetStats();

	private:
		OptimizationFlags m_enabledFlags;      ///< 启用的优化策略
		QRect m_viewport;                      ///< 当前视口
		DirtyRegionManager m_dirtyRegionManager;  ///< 脏区域管理器
		mutable OptimizationStats m_stats;     ///< 优化统计信息

		/// 功能：检查矩形是否在视口内
		/// 参数：rect — 要检查的矩形
		/// 返回：true表示在视口内或与视口相交
		[[nodiscard]] bool isInViewport(const QRectF& rect) const;

		/// 功能：按深度排序命令（Z-order）
		/// 参数：commands — 输入命令列表
		/// 返回：排序后的命令列表
		template<typename T>
		[[nodiscard]] std::vector<T> sortByDepth(const std::vector<T>& commands) const;
	};

	// 位运算操作符重载
	inline RenderOptimizer::OptimizationFlags operator|(
		RenderOptimizer::OptimizationFlags a, RenderOptimizer::OptimizationFlags b) {
		return static_cast<RenderOptimizer::OptimizationFlags>(
			static_cast<int>(a) | static_cast<int>(b));
	}

	inline RenderOptimizer::OptimizationFlags operator&(
		RenderOptimizer::OptimizationFlags a, RenderOptimizer::OptimizationFlags b) {
		return static_cast<RenderOptimizer::OptimizationFlags>(
			static_cast<int>(a) & static_cast<int>(b));
	}

}