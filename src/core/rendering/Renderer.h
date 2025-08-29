/*
 * 文件名：Renderer.h
 * 职责：OpenGL渲染器，负责着色器程序管理、几何数据绑定和帧数据绘制。
 * 依赖：Qt6 OpenGL、IconCache、RenderData定义。
 * 线程：仅在拥有OpenGL上下文的线程中使用（通常为UI线程）。
 * 备注：坐标系转换：逻辑像素 -> 设备像素 -> NDC；剪裁区域采用左上原点，需转换为OpenGL底左原点。
 */

#pragma once
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qopenglshaderprogram.h>
#include <qopenglvertexarrayobject.h>

#include "IconCache.h"
#include "RenderData.hpp"

/// OpenGL渲染器：管理着色器资源与绘制命令执行
/// OpenGL渲染器：管理着色器资源与绘制命令执行
/// 
/// 功能：
/// - OpenGL着色器程序与缓冲对象生命周期管理
/// - 圆角矩形绘制（顶点着色器 + 片段着色器）
/// - 纹理绘制（图标、文本，支持着色）
/// - 剪裁区域管理（逻辑像素坐标系转换）
/// 
/// 坐标系说明：
/// - 输入：逻辑像素坐标（左上原点）
/// - 计算：乘以DPR（设备像素比）得到设备像素
/// - 输出：NDC坐标（-1到1，OpenGL标准）
/// - 剪裁：需要从左上原点转换为OpenGL的底左原点
class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	/// 功能：初始化OpenGL资源（着色器程序、VAO、VBO）
	/// 参数：gl — 当前OpenGL上下文的函数表指针
	/// 说明：必须在有效的OpenGL上下文中调用
	void initializeGL(QOpenGLFunctions* gl);
	
	/// 功能：释放所有OpenGL资源
	/// 说明：在OpenGL上下文销毁前调用，确保资源正确清理
	void releaseGL();

	/// 功能：更新渲染视口尺寸
	/// 参数：fbWpx — 帧缓冲宽度（设备像素）
	/// 参数：fbHpx — 帧缓冲高度（设备像素）
	void resize(int fbWpx, int fbHpx);

	/// 功能：绘制一帧
	/// 参数：fd — 包含所有绘制命令的帧数据
	/// 参数：iconCache — 图标纹理缓存
	/// 参数：devicePixelRatio — DPR（设备像素比），用于逻辑像素到设备像素转换
	void drawFrame(const Render::FrameData& fd, const IconCache& iconCache, float devicePixelRatio);

private:
	void drawRoundedRect(const Render::RoundedRectCmd& cmd);
	void drawImage(const Render::ImageCmd& img, const IconCache& iconCache);

	/// 功能：设置剪裁区域
	/// 参数：clipLogical — 逻辑像素矩形（左上原点），宽高<=0时禁用剪裁
	/// 说明：自动转换为OpenGL剪裁坐标（底左原点，设备像素）
	void applyClip(const QRectF& clipLogical);
	void restoreClip();

private:
	// OpenGL着色器资源（圆角矩形）
	QOpenGLShaderProgram* m_progRect{ nullptr };
	QOpenGLVertexArrayObject m_vao;
	unsigned int m_vbo{ 0 };
	int m_locViewportSize{ -1 };
	int m_locRectPx{ -1 };
	int m_locRadius{ -1 };
	int m_locColor{ -1 };

	// OpenGL着色器资源（纹理绘制）
	QOpenGLShaderProgram* m_progTex{ nullptr };
	int m_texLocViewportSize{ -1 };
	int m_texLocDstRect{ -1 };
	int m_texLocSrcRect{ -1 };
	int m_texLocTexSize{ -1 };
	int m_texLocTint{ -1 };
	int m_texLocSampler{ -1 };

	// 渲染状态
	int m_fbWpx{ 0 };     // 帧缓冲宽度（设备像素）
	int m_fbHpx{ 0 };     // 帧缓冲高度（设备像素）
	float m_currentDpr{ 1.0f };  // 当前帧的DPR（设备像素比）

	// OpenGL函数表
	QOpenGLFunctions* m_gl{ nullptr };

	// 剪裁状态管理
	bool  m_clipActive{ false };
	QRect m_clipPx{ 0,0,0,0 };  // 当前剪裁矩形（设备像素，左上原点 -> OpenGL底左原点）
};