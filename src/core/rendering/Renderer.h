#pragma once
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qopenglshaderprogram.h>
#include <qopenglvertexarrayobject.h>

#include "IconCache.h"
#include "RenderData.hpp"

// 负责：GL 资源（Program / VAO / VBO）与 FrameData 绘制
class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	// 初始化/释放 OpenGL 资源（需在当前 GL 上下文中调用）
	// 传入当前上下文的 QOpenGLFunctions，用于调用 GL API
	void initializeGL(QOpenGLFunctions* gl);
	void releaseGL();

	// 视口变更（像素尺寸）
	void resize(int fbWpx, int fbHpx);

	// 绘制一帧（像素清屏颜色由调用者控制）
	void drawFrame(const Render::FrameData& fd, const IconCache& iconCache, float devicePixelRatio);

private:
	void drawRoundedRect(const Render::RoundedRectCmd& cmd);
	void drawImage(const Render::ImageCmd& img, const IconCache& iconCache);

	// 设置/恢复剪裁（逻辑像素矩形；宽高<=0 则禁用）
	void applyClip(const QRectF& clipLogical);
	void restoreClip();

private:
	// GL 资源（圆角矩形）
	QOpenGLShaderProgram* m_progRect{ nullptr };
	QOpenGLVertexArrayObject m_vao;
	unsigned int m_vbo{ 0 };
	int m_locViewportSize{ -1 };
	int m_locRectPx{ -1 };
	int m_locRadius{ -1 };
	int m_locColor{ -1 };

	// GL 资源（纹理）
	QOpenGLShaderProgram* m_progTex{ nullptr };
	int m_texLocViewportSize{ -1 };
	int m_texLocDstRect{ -1 };
	int m_texLocSrcRect{ -1 };
	int m_texLocTexSize{ -1 };
	int m_texLocTint{ -1 };
	int m_texLocSampler{ -1 };

	// 帧缓冲像素尺寸
	int m_fbWpx{ 0 };
	int m_fbHpx{ 0 };

	// 本帧 DPR（用于逻辑像素->像素转换）
	float m_currentDpr{ 1.0f };

	// 当前上下文的 GL 函数表
	QOpenGLFunctions* m_gl{ nullptr };

	// 剪裁状态
	bool  m_clipActive{ false };
	QRect m_clipPx{ 0,0,0,0 }; // 当前启用的剪裁矩形（像素坐标，原点左上）
};