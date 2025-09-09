#include "RenderPipeline.hpp"
#include <algorithm>

namespace Render {

	void RenderPipeline::addRoundedRect(Stage stage, const RoundedRectCmd& cmd) {
		int stageIndex = static_cast<int>(stage);
		if (stageIndex >= 0 && stageIndex < 4) {
			m_stages[stageIndex].roundedRects.push_back(cmd);
		}
	}

	void RenderPipeline::addImage(Stage stage, const ImageCmd& cmd) {
		int stageIndex = static_cast<int>(stage);
		if (stageIndex >= 0 && stageIndex < 4 && cmd.textureId > 0) {
			m_stages[stageIndex].imageBatches[cmd.textureId].push_back(cmd);
		}
	}

	void RenderPipeline::addFrameData(Stage stage, const FrameData& frameData) {
		// 添加所有圆角矩形命令
		for (const auto& rect : frameData.roundedRects) {
			addRoundedRect(stage, rect);
		}
		
		// 添加所有图像命令
		for (const auto& img : frameData.images) {
			addImage(stage, img);
		}
	}

	int RenderPipeline::executeStage(Stage stage) {
		int stageIndex = static_cast<int>(stage);
		if (stageIndex < 0 || stageIndex >= 4) return 0;
		
		int commandCount = 0;
		commandCount += executeRoundedRects(stageIndex);
		commandCount += executeImages(stageIndex);
		
		return commandCount;
	}

	int RenderPipeline::executeAll() {
		int totalCommands = 0;
		
		// 按阶段顺序执行
		totalCommands += executeStage(Stage::Background);
		totalCommands += executeStage(Stage::Content);
		totalCommands += executeStage(Stage::Overlay);
		totalCommands += executeStage(Stage::Debug);
		
		return totalCommands;
	}

	void RenderPipeline::clear() {
		for (int i = 0; i < 4; ++i) {
			m_stages[i].roundedRects.clear();
			m_stages[i].imageBatches.clear();
		}
	}

	int RenderPipeline::getStageCommandCount(Stage stage) const {
		int stageIndex = static_cast<int>(stage);
		if (stageIndex < 0 || stageIndex >= 4) return 0;
		
		const auto& stageData = m_stages[stageIndex];
		int count = stageData.roundedRects.size();
		
		// 计算图像命令数量
		for (const auto& [textureId, commands] : stageData.imageBatches) {
			count += commands.size();
		}
		
		return count;
	}

	bool RenderPipeline::empty() const {
		for (int i = 0; i < 4; ++i) {
			if (!m_stages[i].roundedRects.empty() || !m_stages[i].imageBatches.empty()) {
				return false;
			}
		}
		return true;
	}

	const std::vector<RoundedRectCmd>& RenderPipeline::getRoundedRects(Stage stage) const {
		int stageIndex = static_cast<int>(stage);
		if (stageIndex >= 0 && stageIndex < 4) {
			return m_stages[stageIndex].roundedRects;
		}
		static const std::vector<RoundedRectCmd> empty;
		return empty;
	}

	const std::unordered_map<int, std::vector<ImageCmd>>& RenderPipeline::getImageBatches(Stage stage) const {
		int stageIndex = static_cast<int>(stage);
		if (stageIndex >= 0 && stageIndex < 4) {
			return m_stages[stageIndex].imageBatches;
		}
		static const std::unordered_map<int, std::vector<ImageCmd>> empty;
		return empty;
	}

	bool RenderPipeline::isInViewport(const QRectF& rect) const {
		if (!m_cullingEnabled || m_viewport.isEmpty()) {
			return true; // 禁用剔除或无视口设置时，所有对象都可见
		}
		
		// 简单的矩形相交检测
		return rect.intersects(QRectF(m_viewport));
	}

	int RenderPipeline::executeRoundedRects(int stage) {
		if (stage < 0 || stage >= 4) return 0;
		
		auto& rects = m_stages[stage].roundedRects;
		if (rects.empty()) return 0;
		
		// 视口剔除过滤
		std::vector<Render::RoundedRectCmd> visibleRects;
		for (const auto& rect : rects) {
			if (isInViewport(rect.rect)) {
				visibleRects.push_back(rect);
			}
		}
		
		// TODO: 这里需要调用实际的渲染器
		// 现在我们返回可见的命令数量，实际渲染将由Renderer类处理
		return visibleRects.size();
	}

	int RenderPipeline::executeImages(int stage) {
		if (stage < 0 || stage >= 4) return 0;
		
		auto& imageBatches = m_stages[stage].imageBatches;
		int executedCount = 0;
		
		for (const auto& [textureId, commands] : imageBatches) {
			// 视口剔除过滤
			std::vector<Render::ImageCmd> visibleImages;
			for (const auto& cmd : commands) {
				if (isInViewport(cmd.dstRect)) {
					visibleImages.push_back(cmd);
				}
			}
			
			// TODO: 这里需要调用实际的渲染器
			// renderer->drawImagesBatch(textureId, visibleImages);
			executedCount += visibleImages.size();
		}
		
		return executedCount;
	}

}