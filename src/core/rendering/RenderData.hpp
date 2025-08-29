/*
 * 文件名：RenderData.hpp
 * 职责：渲染系统的核心数据结构定义，包含绘制命令和帧数据容器。
 * 依赖：Qt6 Core（QColor、QRect）、标准库容器。
 * 线程：数据结构线程安全，可在多线程间传递。
 * 备注：定义了逻辑像素坐标系统，支持剪裁区域，采用命令模式收集绘制指令。
 */

#pragma once
#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

#include <qcolor.h>
#include <qrect.h>

namespace Render {

	/// 圆角矩形绘制命令
	/// 
	/// 坐标系说明：
	/// - rect使用逻辑像素坐标（左上原点）
	/// - 渲染时乘以DPR转换为设备像素
	/// - 着色器接收设备像素坐标进行绘制
	struct RoundedRectCmd {
		QRectF rect;      // 目标矩形（逻辑像素坐标）
		float  radiusPx;  // 圆角半径（逻辑像素）
		QColor color;     // 填充颜色（包含alpha透明度）

		// 剪裁区域（逻辑像素坐标；宽高<=0表示不启用剪裁）
		QRectF clipRect;
	};

	/// 纹理图像绘制命令
	/// 
	/// 功能：
	/// - 支持纹理的任意区域绘制到任意目标矩形
	/// - 白底图标可通过tint着色为任意颜色（白膜策略）
	/// - 支持透明度调制和颜色混合
	struct ImageCmd {
		QRectF dstRect;       // 目标矩形（逻辑像素坐标）
		int    textureId{ 0 }; // OpenGL纹理句柄
		QRectF srcRectPx;     // 源纹理区域（设备像素坐标）
		QColor tint{ 255,255,255,255 }; // 着色调制（白色=不变，其他=着色）

		// 剪裁区域（逻辑像素坐标；宽高<=0表示不启用剪裁）
		QRectF clipRect;
	};

	/// 帧渲染数据容器：收集一帧内的所有绘制命令
	/// 
	/// 设计理念：
	/// - 命令模式：UI组件生成绘制命令，渲染器批量执行
	/// - 类型扩展：可轻松添加新的图元类型（线条、贝塞尔曲线等）
	/// - 内存管理：使用vector确保内存局部性和高效遍历
	struct FrameData {
		std::vector<RoundedRectCmd> roundedRects;  // 圆角矩形绘制命令列表
		std::vector<ImageCmd>       images;        // 纹理图像绘制命令列表

		/// 功能：清空所有绘制命令
		/// 说明：准备下一帧的命令收集
		void clear() {
			roundedRects.clear();
			images.clear();
		}
		
		/// 功能：检查是否包含绘制命令
		/// 返回：true表示无任何绘制内容
		bool empty() const {
			return roundedRects.empty() && images.empty();
		}
	};

}