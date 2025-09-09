/*
 * 文件名：RenderPipeline.hpp
 * 职责：多阶段渲染管线，支持渲染队列管理和批次优化
 * 依赖：RenderData.hpp、Qt6 Core
 * 线程：线程安全的命令提交，执行阶段需在渲染线程
 * 备注：实现分阶段渲染，支持批次合并和视口剔除优化
 */

#pragma once
#include "RenderData.hpp"
#include <qrect.h>
#include <qhash.h>
#include <vector>
#include <unordered_map>

namespace Render {

	/// 渲染管线阶段枚举
	/// 
	/// 不同阶段有不同的渲染优先级和批次策略：
	/// - Background: 背景元素，通常较少状态切换
	/// - Content: 主要内容，需要批次优化
	/// - Overlay: 覆盖层，可能包含半透明元素
	/// - Debug: 调试信息，通常在开发期间启用
	enum class Stage {
		Background = 0,    ///< 背景和基础几何
		Content = 1,       ///< 文本和图标
		Overlay = 2,       ///< 覆盖层和特效
		Debug = 3          ///< 调试信息
	};

	/// 渲染管线：多阶段命令队列与批次优化系统
	/// 
	/// 功能：
	/// - 分阶段收集和执行渲染命令
	/// - 自动批次合并相同纹理的绘制调用
	/// - 视口剔除优化
	/// - 渲染状态管理和切换最小化
	class RenderPipeline {
	public:
		RenderPipeline() = default;
		~RenderPipeline() = default;

		/// 功能：添加渲染命令到指定阶段
		/// 参数：stage — 渲染阶段
		/// 参数：cmd — 圆角矩形命令
		void addRoundedRect(Stage stage, const RoundedRectCmd& cmd);
		
		/// 功能：添加图像渲染命令到指定阶段
		/// 参数：stage — 渲染阶段  
		/// 参数：cmd — 图像渲染命令
		void addImage(Stage stage, const ImageCmd& cmd);

		/// 功能：从 FrameData 批量添加命令
		/// 参数：stage — 目标渲染阶段
		/// 参数：frameData — 包含命令的帧数据
		void addFrameData(Stage stage, const FrameData& frameData);

		/// 功能：执行指定阶段的所有命令
		/// 参数：stage — 要执行的阶段
		/// 返回：执行的命令总数
		int executeStage(Stage stage);

		/// 功能：执行所有阶段的命令
		/// 返回：执行的命令总数
		int executeAll();

		/// 功能：清空所有阶段的命令队列
		void clear();

		/// 功能：启用/禁用批次合并优化
		/// 参数：enabled — true启用批次合并
		void enableBatching(bool enabled) { m_batchingEnabled = enabled; }

		/// 功能：启用/禁用视口剔除
		/// 参数：enabled — true启用剔除
		void enableCulling(bool enabled) { m_cullingEnabled = enabled; }

		/// 功能：设置渲染视口用于剔除优化
		/// 参数：viewport — 视口矩形
		void setViewport(const QRect& viewport) { m_viewport = viewport; }

		/// 功能：获取指定阶段的统计信息
		/// 参数：stage — 目标阶段
		/// 返回：该阶段的命令数量
		[[nodiscard]] int getStageCommandCount(Stage stage) const;

		/// 功能：检查管线是否为空
		/// 返回：true表示所有阶段都没有命令
		[[nodiscard]] bool empty() const;

		/// 功能：获取指定阶段的圆角矩形命令（用于批次渲染）
		/// 参数：stage — 目标阶段
		/// 返回：该阶段的圆角矩形命令列表
		[[nodiscard]] const std::vector<RoundedRectCmd>& getRoundedRects(Stage stage) const;

		/// 功能：获取指定阶段的图像批次（用于批次渲染）
		/// 参数：stage — 目标阶段
		/// 返回：该阶段的图像批次映射
		[[nodiscard]] const std::unordered_map<int, std::vector<ImageCmd>>& getImageBatches(Stage stage) const;

	private:
		/// 纹理批次：相同纹理的图像命令集合
		struct TextureBatch {
			int textureId;
			std::vector<ImageCmd> images;
		};

		/// 阶段数据：包含该阶段的所有渲染命令
		struct StageData {
			std::vector<RoundedRectCmd> roundedRects;
			std::unordered_map<int, std::vector<ImageCmd>> imageBatches; // textureId -> commands
		};

		// 四个渲染阶段的命令队列
		StageData m_stages[4];

		// 渲染配置
		QRect m_viewport;
		bool m_batchingEnabled = true;
		bool m_cullingEnabled = true;

		/// 功能：检查命令是否在视口内（用于剔除）
		/// 参数：rect — 要检查的矩形
		/// 返回：true表示在视口内或与视口相交
		[[nodiscard]] bool isInViewport(const QRectF& rect) const;

		/// 功能：执行单个阶段的圆角矩形命令
		/// 参数：stage — 阶段索引
		/// 返回：执行的命令数
		int executeRoundedRects(int stage);

		/// 功能：执行单个阶段的图像命令（支持批次合并）
		/// 参数：stage — 阶段索引  
		/// 返回：执行的命令数
		int executeImages(int stage);
	};

}