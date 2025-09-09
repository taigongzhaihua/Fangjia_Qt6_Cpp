/*
 * 文件名：Renderer.h
 * 职责：增强的OpenGL渲染器，支持多阶段渲染管线、批次优化和纹理管理。
 * 依赖：Qt6 OpenGL、TextureManager、RenderPipeline、RenderData定义。
 * 线程：仅在拥有OpenGL上下文的线程中使用（通常为UI线程）。
 * 备注：支持渲染管线阶段、纹理批次合并、视口剔除等性能优化功能。
 */

#pragma once
#include <qopenglfunctions.h>
#include <qopenglshaderprogram.h>
#include <qopenglvertexarrayobject.h>

#include "IconCache.h"
#include "RenderData.hpp"
#include "RenderPipeline.hpp"
#include "TextureManager.hpp"

/// 增强的OpenGL渲染器：支持多阶段渲染管线和批次优化
/// 
/// 功能：
/// - 多阶段渲染管线（Background、Content、Overlay、Debug）
/// - OpenGL着色器程序与缓冲对象生命周期管理
/// - 圆角矩形绘制（顶点着色器 + 片段着色器）
/// - 纹理绘制（图标、文本，支持着色和批次合并）
/// - 剪裁区域管理（逻辑像素坐标系转换）
/// - 高级纹理管理和资源优化
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

	/// 功能：初始化OpenGL资源（着色器程序、VAO、VBO、纹理管理器）
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

	/// 功能：绘制一帧（传统兼容接口）
	/// 参数：fd — 包含所有绘制命令的帧数据
	/// 参数：iconCache — 图标纹理缓存
	/// 参数：devicePixelRatio — DPR（设备像素比）
	void drawFrame(const Render::FrameData& fd, const IconCache& iconCache, float devicePixelRatio);

	/// 功能：使用渲染管线绘制一帧（新接口）
	/// 参数：pipeline — 包含分阶段渲染命令的管线
	/// 参数：devicePixelRatio — DPR（设备像素比）
	/// 返回：渲染的命令总数
	int drawPipeline(Render::RenderPipeline& pipeline, float devicePixelRatio);

	/// 功能：获取纹理管理器实例
	/// 返回：纹理管理器的引用
	Render::TextureManager& getTextureManager() { return *m_textureManager; }

	/// 功能：启用/禁用渲染批次优化
	/// 参数：enabled — true启用批次优化
	void enableBatching(bool enabled) { m_batchingEnabled = enabled; }

	/// 功能：启用/禁用视口剔除
	/// 参数：enabled — true启用剔除
	void enableCulling(bool enabled) { m_cullingEnabled = enabled; }

	/// 功能：设置渲染视口用于剔除优化
	/// 参数：viewport — 视口矩形
	void setViewport(const QRect& viewport) { m_viewport = viewport; }

private:
	// 基础渲染方法
	void drawRoundedRect(const Render::RoundedRectCmd& cmd);
	void drawImage(const Render::ImageCmd& img, const IconCache& iconCache);

	// 批次渲染方法
	void drawRoundedRectsBatch(const std::vector<Render::RoundedRectCmd>& rects);
	void drawImagesBatch(int textureId, const std::vector<Render::ImageCmd>& images);

	// 管线阶段渲染
	int drawPipelineStage(Render::RenderPipeline& pipeline, Render::Stage stage);

	/// 功能：设置剪裁区域
	/// 参数：clipLogical — 逻辑像素矩形（左上原点），宽高<=0时禁用剪裁
	/// 说明：自动转换为OpenGL剪裁坐标（底左原点，设备像素）
	void applyClip(const QRectF& clipLogical);
	void restoreClip();

	/// 功能：检查矩形是否在视口内（用于剔除）
	[[nodiscard]] bool isInViewport(const QRectF& rect) const;

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

	// 高级渲染功能
	std::unique_ptr<Render::TextureManager> m_textureManager;
	QRect m_viewport;
	bool m_batchingEnabled = true;
	bool m_cullingEnabled = true;
};