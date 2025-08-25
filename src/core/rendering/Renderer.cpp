#include "Renderer.h"

#include "IconLoader.h"
#include "RenderData.hpp"

#include <algorithm>
#include <cmath>
#include <gl/GL.h>
#include <qcolor.h>
#include <qopenglext.h>
#include <qopenglfunctions.h>
#include <qopenglshaderprogram.h>
#include <qrect.h>
#include <qsize.h>
#include <qvectornd.h>

namespace {
	inline void rectPxToNdcVerts(const QRectF& rPx, const int vpWpx, const int vpHpx, float out[12]) {
		const float xL = static_cast<float>(rPx.left());
		const float xR = static_cast<float>(rPx.left() + rPx.width());
		const float yT = static_cast<float>(rPx.top());
		const float yB = static_cast<float>(rPx.top() + rPx.height());
		const float ndcL = xL / static_cast<float>(vpWpx) * 2.f - 1.f;
		const float ndcR = xR / static_cast<float>(vpWpx) * 2.f - 1.f;
		const float ndcT = 1.f - yT / static_cast<float>(vpHpx) * 2.f;
		const float ndcB = 1.f - yB / static_cast<float>(vpHpx) * 2.f;
		out[0] = ndcL; out[1] = ndcT; out[2] = ndcR; out[3] = ndcT; out[4] = ndcR; out[5] = ndcB;
		out[6] = ndcL; out[7] = ndcT; out[8] = ndcR; out[9] = ndcB; out[10] = ndcL; out[11] = ndcB;
	}

	inline QRect clipLogicalToPxTopLeft(const QRectF& logical, float dpr, int fbWpx, int fbHpx) {
		if (logical.width() <= 0.0 || logical.height() <= 0.0) return QRect();
		const int x = std::clamp(static_cast<int>(std::floor(logical.left() * dpr)), 0, fbWpx);
		const int yTop = static_cast<int>(std::floor(logical.top() * dpr));
		const int hPx = static_cast<int>(std::ceil(logical.height() * dpr));
		const int y = std::clamp(yTop, 0, fbHpx);
		const int w = std::clamp(static_cast<int>(std::ceil(logical.width() * dpr)), 0, fbWpx - x);
		const int h = std::clamp(hPx, 0, fbHpx - y);
		return QRect(x, y, w, h);
	}

	// OpenGL glScissor 以左下角为原点，我们这里统一使用“左上角原点”的像素坐标，因此需要转换
	inline void glScissorTopLeft(QOpenGLFunctions* gl, const QRect& clipTopLeftPx, int fbHpx) {
		const int x = clipTopLeftPx.x();
		const int y = std::max(0, fbHpx - (clipTopLeftPx.y() + clipTopLeftPx.height()));
		const int w = clipTopLeftPx.width();
		const int h = clipTopLeftPx.height();
		gl->glEnable(GL_SCISSOR_TEST);
		gl->glScissor(x, y, w, h);
	}
}

void Renderer::initializeGL(QOpenGLFunctions* gl)
{
	m_gl = gl;

	if (!m_progRect) {
		static auto vs1 = R"(#version 330 core
layout(location=0) in vec2 aPos;
void main(){ gl_Position = vec4(aPos, 0.0, 1.0); })";

		static auto fs1 = R"(
#version 330 core
out vec4 FragColor;
uniform vec2 uViewportSize;
uniform vec4 uRectPx;
uniform float uRadius;
uniform vec4 uColor;

float sdRoundRect(vec2 p, vec2 halfSize, float r){
    vec2 q = abs(p) - (halfSize - vec2(r));
    float outside = length(max(q, 0.0));
    float inside = min(max(q.x, q.y), 0.0);
    return outside + inside - r;
}

void main(){
    vec2 fragPx = vec2(gl_FragCoord.x, uViewportSize.y - gl_FragCoord.y);
    vec2 rectCenter = uRectPx.xy + 0.5 * uRectPx.zw;
    vec2 halfSize   = 0.5 * uRectPx.zw;
    float r = min(uRadius, min(halfSize.x, halfSize.y));
    vec2 p = fragPx - rectCenter;
    float dist = sdRoundRect(p, halfSize, r);
    float aa = fwidth(dist);
    float alpha = 1.0 - smoothstep(0.0, aa, dist);
    FragColor = vec4(uColor.rgb, uColor.a * alpha);
})";

		m_progRect = new QOpenGLShaderProgram();
		m_progRect->addShaderFromSourceCode(QOpenGLShader::Vertex, vs1);
		m_progRect->addShaderFromSourceCode(QOpenGLShader::Fragment, fs1);
		m_progRect->link();

		m_locViewportSize = m_progRect->uniformLocation("uViewportSize");
		m_locRectPx = m_progRect->uniformLocation("uRectPx");
		m_locRadius = m_progRect->uniformLocation("uRadius");
		m_locColor = m_progRect->uniformLocation("uColor");

		m_vao.create();
		m_vao.bind();

		m_gl->glGenBuffers(1, &m_vbo);
		m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW);

		m_gl->glEnableVertexAttribArray(0);
		m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

		m_vao.release();
	}

	if (!m_progTex) {
		static auto vs2 = R"(
#version 330 core
layout(location=0) in vec2 aPos;
void main(){ gl_Position = vec4(aPos, 0.0, 1.0); })";

		static auto fs2 = R"(
#version 330 core
out vec4 FragColor;
uniform vec2  uViewportSize;
uniform vec4  uDstRectPx;
uniform vec4  uSrcRectPx;
uniform vec2  uTexSizePx;
uniform vec4  uTint;
uniform sampler2D uTex;

void main(){
    vec2 fragPx = vec2(gl_FragCoord.x, uViewportSize.y - gl_FragCoord.y);
    vec2 dst0   = uDstRectPx.xy;
    vec2 dstSz  = uDstRectPx.zw;
    vec2 t      = (fragPx - dst0) / dstSz;  // 0..1
    vec2 srcPx  = uSrcRectPx.xy + t * uSrcRectPx.zw;
    vec2 uv     = srcPx / uTexSizePx;

    vec4 texel = texture(uTex, uv);
    FragColor  = texel * uTint;
})";

		m_progTex = new QOpenGLShaderProgram();
		m_progTex->addShaderFromSourceCode(QOpenGLShader::Vertex, vs2);
		m_progTex->addShaderFromSourceCode(QOpenGLShader::Fragment, fs2);
		m_progTex->link();

		m_texLocViewportSize = m_progTex->uniformLocation("uViewportSize");
		m_texLocDstRect = m_progTex->uniformLocation("uDstRectPx");
		m_texLocSrcRect = m_progTex->uniformLocation("uSrcRectPx");
		m_texLocTexSize = m_progTex->uniformLocation("uTexSizePx");
		m_texLocTint = m_progTex->uniformLocation("uTint");
		m_texLocSampler = m_progTex->uniformLocation("uTex");
	}
}

void Renderer::releaseGL()
{
	if (m_gl && m_vbo) { m_gl->glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
	if (m_progRect) { delete m_progRect; m_progRect = nullptr; }
	if (m_progTex) { delete m_progTex; m_progTex = nullptr; }
	if (m_vao.isCreated()) m_vao.destroy();
}

void Renderer::resize(const int fbWpx, const int fbHpx)
{
	m_fbWpx = fbWpx;
	m_fbHpx = fbHpx;
	if (m_gl) m_gl->glViewport(0, 0, m_fbWpx, m_fbHpx);
}

void Renderer::applyClip(const QRectF& clipLogical)
{
	// 若 clip 未启用，关闭剪裁
	if (clipLogical.width() <= 0.0 || clipLogical.height() <= 0.0) {
		restoreClip();
		return;
	}
	// 计算像素空间并启用 glScissor（左上角原点）
	m_clipPx = clipLogicalToPxTopLeft(clipLogical, m_currentDpr, m_fbWpx, m_fbHpx);
	if (m_clipPx.width() <= 0 || m_clipPx.height() <= 0) {
		restoreClip();
		return;
	}
	m_clipActive = true;
	glScissorTopLeft(m_gl, m_clipPx, m_fbHpx);
}

void Renderer::restoreClip()
{
	if (m_clipActive) {
		m_gl->glDisable(GL_SCISSOR_TEST);
		m_clipActive = false;
		m_clipPx = QRect(0, 0, 0, 0);
	}
}

void Renderer::drawRoundedRect(const Render::RoundedRectCmd& cmd)
{
	if (!m_progRect || !m_gl || m_fbWpx <= 0 || m_fbHpx <= 0) return;

	// 应用裁剪
	applyClip(cmd.clipRect);

	const QRectF rp(cmd.rect.x() * m_currentDpr, cmd.rect.y() * m_currentDpr, cmd.rect.width() * m_currentDpr, cmd.rect.height() * m_currentDpr);
	const float  rr = cmd.radiusPx * m_currentDpr;

	float verts[12];
	rectPxToNdcVerts(rp, m_fbWpx, m_fbHpx, verts);

	m_vao.bind();
	m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

	m_progRect->bind();
	m_progRect->setUniformValue(m_locViewportSize, QVector2D(static_cast<float>(m_fbWpx), static_cast<float>(m_fbHpx)));
	m_progRect->setUniformValue(m_locRectPx, QVector4D(static_cast<float>(rp.x()), static_cast<float>(rp.y()), static_cast<float>(rp.width()), static_cast<float>(rp.height())));
	m_progRect->setUniformValue(m_locRadius, rr);
	m_progRect->setUniformValue(m_locColor, QVector4D(cmd.color.redF(), cmd.color.greenF(), cmd.color.blueF(), cmd.color.alphaF()));
	m_gl->glDrawArrays(GL_TRIANGLES, 0, 6);
	m_progRect->release();
	m_vao.release();

	// 恢复裁剪
	restoreClip();
}

void Renderer::drawImage(const Render::ImageCmd& img, const IconLoader& iconLoader)
{
	if (!m_progTex || !m_gl || img.textureId == 0 || m_fbWpx <= 0 || m_fbHpx <= 0) return;

	// 应用裁剪
	applyClip(img.clipRect);

	const QRectF dstPx(img.dstRect.x() * m_currentDpr, img.dstRect.y() * m_currentDpr, img.dstRect.width() * m_currentDpr, img.dstRect.height() * m_currentDpr);

	float verts[12];
	rectPxToNdcVerts(dstPx, m_fbWpx, m_fbHpx, verts);

	m_vao.bind();
	m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

	m_progTex->bind();
	m_progTex->setUniformValue(m_texLocViewportSize, QVector2D(static_cast<float>(m_fbWpx), static_cast<float>(m_fbHpx)));
	m_progTex->setUniformValue(m_texLocDstRect, QVector4D(static_cast<float>(dstPx.x()), static_cast<float>(dstPx.y()), static_cast<float>(dstPx.width()), static_cast<float>(dstPx.height())));
	m_progTex->setUniformValue(m_texLocSrcRect, QVector4D(static_cast<float>(img.srcRectPx.x()), static_cast<float>(img.srcRectPx.y()),
		static_cast<float>(img.srcRectPx.width()), static_cast<float>(img.srcRectPx.height())));
	const QSize texSz = iconLoader.textureSizePx(img.textureId);
	m_progTex->setUniformValue(m_texLocTexSize, QVector2D(static_cast<float>(texSz.width()), static_cast<float>(texSz.height())));
	m_progTex->setUniformValue(m_texLocTint, QVector4D(img.tint.redF(), img.tint.greenF(), img.tint.blueF(), img.tint.alphaF()));
	m_progTex->setUniformValue(m_texLocSampler, 0);

	m_gl->glActiveTexture(GL_TEXTURE0);
	m_gl->glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(img.textureId));
	m_gl->glDrawArrays(GL_TRIANGLES, 0, 6);
	m_gl->glBindTexture(GL_TEXTURE_2D, 0);

	m_progTex->release();
	m_vao.release();

	// 恢复裁剪
	restoreClip();
}

void Renderer::drawFrame(const Render::FrameData& fd, const IconLoader& iconLoader, const float devicePixelRatio)
{
	m_currentDpr = std::max(0.5f, devicePixelRatio);

	// 圆角矩形
	for (const auto& rr : fd.roundedRects) drawRoundedRect(rr);

	// 纹理
	for (const auto& im : fd.images)       drawImage(im, iconLoader);
}