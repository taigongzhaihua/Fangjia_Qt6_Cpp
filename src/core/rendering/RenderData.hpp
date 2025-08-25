#pragma once
#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

#include <qcolor.h>
#include <qrect.h>

namespace Render {

	// 一条绘制圆角矩形的命令
	struct RoundedRectCmd {
		QRectF rect;      // 逻辑像素矩形（后续乘以 DPR，再传给 shader 的像素空间）
		float  radiusPx;  // 圆角半径（逻辑像素）
		QColor color;     // 颜色（含 alpha）

		// 新增：可选的裁剪矩形（逻辑像素；宽高<=0表示不启用）
		QRectF clipRect;
	};

	// 一条绘制纹理图像的命令
	// - dstRect: 目标矩形（逻辑像素）
	// - textureId: GL 纹理句柄（由 IconLoader 提供）
	// - srcRectPx: 源区域（像素坐标，默认用整张纹理：{0,0,w,h}）
	// - tint: 颜色调制（常用作着色或透明度控制；白底图可被 tint 成任意颜色）
	struct ImageCmd {
		QRectF dstRect;
		int    textureId{ 0 };
		QRectF srcRectPx;     // 以像素为单位
		QColor tint{ 255,255,255,255 };

		// 新增：可选的裁剪矩形（逻辑像素；宽高<=0表示不启用）
		QRectF clipRect;
	};

	// 一帧的图形数据（可扩展更多图元类型）
	struct FrameData {
		std::vector<RoundedRectCmd> roundedRects;
		std::vector<ImageCmd>       images;

		void clear() {
			roundedRects.clear();
			images.clear();
		}
		bool empty() const {
			return roundedRects.empty() && images.empty();
		}
	};

	// 线程安全的数据总线：生产者提交最新帧，消费者在渲染线程取快照
	class DataBus {
	public:
		// 提交一帧数据（覆盖旧的等待消费的数据）
		void submit(const FrameData& fd) {
			std::scoped_lock lk(m_mtx);
			m_front = fd;
			m_hasNew.store(true, std::memory_order_release);
		}

		// 消费最新帧。如果返回 true，out 中包含最新数据快照
		bool consume(FrameData& out) {
			if (!m_hasNew.load(std::memory_order_acquire)) {
				return false;
			}
			std::scoped_lock lk(m_mtx);
			out = std::move(m_front);
			m_hasNew.store(false, std::memory_order_release);
			return true;
		}

	private:
		std::mutex m_mtx;
		FrameData m_front;                  // 等待消费的前台数据
		std::atomic<bool> m_hasNew{ false };
	};

}