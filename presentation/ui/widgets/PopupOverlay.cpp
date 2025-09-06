/*
 * PopupOverlay.cpp - 弹出窗口实现
 */

#include "PopupOverlay.h"
#include "Renderer.h"
#include <GL/gl.h>
#include <memory>
#include <qapplication.h>
#include <qcolor.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qopenglwindow.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qsurfaceformat.h>
#include <qtimer.h>
#include <qtmetamacros.h>

#include <qwindow.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <utility>

PopupOverlay::PopupOverlay(QWindow* parent)
	: QOpenGLWindow(NoPartialUpdate, parent)
{
	// 设置OpenGL格式以支持透明度
	QSurfaceFormat format = QSurfaceFormat::defaultFormat();
	format.setAlphaBufferSize(8); // 确保有alpha通道
	format.setSamples(4); // 抗锯齿
	setFormat(format);

	// 设置窗口属性
	setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

	// 设置透明背景支持
	setBackgroundColor(QColor(Qt::transparent));

	// 设置渲染定时器
	m_renderTimer.setSingleShot(false);
	m_renderTimer.setInterval(16); // ~60 FPS
	connect(&m_renderTimer, &QTimer::timeout, this, &PopupOverlay::onRenderTick);

	// Mark that we need content layout update when initialized
	m_needsContentLayoutUpdate = true;
}

PopupOverlay::~PopupOverlay()
{
	// Remove global event filter if installed
	if (m_installEventFilter && QApplication::instance()) {
		QApplication::instance()->removeEventFilter(this);
	}

	// 确保在正确的OpenGL上下文中释放资源
	if (m_initialized) {
		makeCurrent();
		m_renderer.releaseGL();
		m_iconCache.releaseAll(this);
	}
}

void PopupOverlay::setContent(std::unique_ptr<IUiComponent> content)
{
	m_content = std::move(content);
	m_needsContentLayoutUpdate = true; // Mark for layout update

	// Apply current theme to new content
	if (m_content) {
		m_content->onThemeChanged(m_isDarkTheme);
	}

	// 如果窗口已初始化，立即更新布局
	if (m_initialized) {
		updateContentLayout();
	}
}

void PopupOverlay::showAt(const QPoint& globalPos, const QSize& size)
{
	// Calculate expanded size to accommodate shadows
	int shadowMargin = static_cast<int>(m_shadowSize);
	QSize expandedSize(size.width() + 2 * shadowMargin, size.height() + 2 * shadowMargin);

	// Adjust position to account for shadow margin
	QPoint adjustedPos(globalPos.x() - shadowMargin, globalPos.y() - shadowMargin);

	// Set the actual content rect within the expanded window
	m_actualContentRect = QRect(shadowMargin, shadowMargin, size.width(), size.height());

	// 设置窗口位置和大小 (using expanded size)
	setGeometry(adjustedPos.x(), adjustedPos.y(), expandedSize.width(), expandedSize.height());

	// Install global event filter for click-outside detection
	if (!m_installEventFilter && QApplication::instance()) {
		QApplication::instance()->installEventFilter(this);
		m_installEventFilter = true;
	}

	// 显示窗口
	show();

	// Apply current theme to content when showing
	if (m_content) {
		m_content->onThemeChanged(m_isDarkTheme);
	}

	// Ensure window gets focus to receive key events
	requestActivate();

	// 开始渲染循环
	if (!m_renderTimer.isActive()) {
		m_renderTimer.start();
	}

	// 触发可见性回调
	if (m_onVisibilityChanged) {
		m_onVisibilityChanged(true);
	}
}

void PopupOverlay::hidePopup()
{
	// Remove global event filter
	if (m_installEventFilter && QApplication::instance()) {
		QApplication::instance()->removeEventFilter(this);
		m_installEventFilter = false;
	}

	// 停止渲染循环
	m_renderTimer.stop();

	// 隐藏窗口
	hide();

	// 触发可见性回调
	if (m_onVisibilityChanged) {
		m_onVisibilityChanged(false);
	}

	// 发送隐藏信号
	emit popupHidden();
}

void PopupOverlay::initializeGL()
{
	initializeOpenGLFunctions();

	// 启用混合以支持透明度
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 初始化渲染器
	m_renderer.initializeGL(this);

	m_initialized = true;

	// 如果有内容且需要更新，立即更新布局
	if (m_content && m_needsContentLayoutUpdate) {
		updateContentLayout();
	}
}

void PopupOverlay::resizeGL(int w, int h)
{
	// 设置视口
	glViewport(0, 0, w, h);

	// 更新内容矩形 - use full window size for m_contentRect (for OpenGL viewport)
	m_contentRect = QRect(0, 0, w, h);

	// 更新渲染器视口
	m_renderer.resize(w, h);

	// Mark that content layout needs update
	m_needsContentLayoutUpdate = true;

	// 更新内容布局
	updateContentLayout();
}

void PopupOverlay::paintGL()
{
	// 清除背景为透明
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!m_content) {
		return;
	}

	// 渲染背景
	renderBackground();

	// 渲染内容
	renderContent();
}

void PopupOverlay::mousePressEvent(QMouseEvent* event)
{
	// Convert click position to content coordinates
	QPoint contentPos = event->pos();

	// Check if click is within actual content area (excluding shadow margins)
	if (m_actualContentRect.isValid() && !m_actualContentRect.contains(contentPos)) {
		// Click is outside content area (in shadow area) - hide popup
		hidePopup();
		event->accept();
		return;
	}

	// Adjust position to content-relative coordinates
	if (m_actualContentRect.isValid()) {
		contentPos -= m_actualContentRect.topLeft();
	}

	if (m_content) {
		m_content->onMousePress(contentPos);
	}

	// 点击内容区域内应该保持弹出窗口打开，不关闭
	event->accept();
}

void PopupOverlay::mouseMoveEvent(QMouseEvent* event)
{
	if (m_content) {
		// Convert to content-relative coordinates
		QPoint contentPos = event->pos();
		if (m_actualContentRect.isValid()) {
			contentPos -= m_actualContentRect.topLeft();
		}
		m_content->onMouseMove(contentPos);
	}
	event->accept();
}

void PopupOverlay::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_content) {
		// Convert to content-relative coordinates
		QPoint contentPos = event->pos();
		if (m_actualContentRect.isValid()) {
			contentPos -= m_actualContentRect.topLeft();
		}
		m_content->onMouseRelease(contentPos);
	}
	event->accept();
}

void PopupOverlay::keyPressEvent(QKeyEvent* event)
{
	// ESC键隐藏弹出窗口
	if (event->key() == Qt::Key_Escape) {
		hidePopup();
		event->accept();
		return;
	}

	QOpenGLWindow::keyPressEvent(event);
}

void PopupOverlay::focusOutEvent(QFocusEvent* event)
{
	// 失去焦点时隐藏弹出窗口
	Q_UNUSED(event)
		hidePopup();
}

void PopupOverlay::hideEvent(QHideEvent* event)
{
	Q_UNUSED(event)

		// Remove global event filter when hidden
		if (m_installEventFilter && QApplication::instance()) {
			QApplication::instance()->removeEventFilter(this);
			m_installEventFilter = false;
		}

	// 停止渲染
	m_renderTimer.stop();

	// 触发可见性回调
	if (m_onVisibilityChanged) {
		m_onVisibilityChanged(false);
	}
}

void PopupOverlay::onRenderTick()
{
	// 更新内容动画状态
	bool needsUpdate = false;

	// Check if content layout needs update (persistent UI root maintenance)
	if (m_needsContentLayoutUpdate && m_initialized) {
		updateContentLayout();
	}

	if (m_content) {
		needsUpdate = m_content->tick();
	}

	// 如果需要更新，触发重绘
	if (needsUpdate || m_needsContentLayoutUpdate) {
		update();
	}
}

void PopupOverlay::updateContentLayout()
{
	if (!m_content || !m_initialized) {
		return;
	}

	// 更新资源上下文 - maintain persistent UI root
	m_content->updateResourceContext(m_iconCache, this, devicePixelRatio());

	// Use actual content size for layout, not expanded window size
	QSize contentSize = m_actualContentRect.isValid() ? m_actualContentRect.size() : size();
	m_content->updateLayout(contentSize);

	// 设置内容视口（如果支持的话） - use actual content rect
	if (auto* contentInterface = dynamic_cast<IUiContent*>(m_content.get())) {
		QRect viewportRect = m_actualContentRect.isValid() ?
			QRect(0, 0, m_actualContentRect.width(), m_actualContentRect.height()) :
			QRect(0, 0, width(), height());
		contentInterface->setViewportRect(viewportRect);
	}

	// Clear the update flag
	m_needsContentLayoutUpdate = false;
}

void PopupOverlay::renderBackground()
{
	// 使用渲染器绘制圆角背景矩形 with shadow
	// First render shadow layers
	if (m_shadowSize > 0) {
		int numShadowLayers = static_cast<int>(m_shadowSize);
		for (int i = 0; i < numShadowLayers; ++i) {
			float alpha = (1.0f - static_cast<float>(i) / numShadowLayers) * 0.1f; // Exponential falloff
			float offset = static_cast<float>(i);

			Render::RoundedRectCmd shadowCmd;
			shadowCmd.rect = QRectF(
				m_actualContentRect.x() + offset,
				m_actualContentRect.y() + offset,
				m_actualContentRect.width(),
				m_actualContentRect.height()
			);
			shadowCmd.radiusPx = m_cornerRadius;
			shadowCmd.color = QColor(0, 0, 0, static_cast<int>(alpha * 255));
			shadowCmd.clipRect = QRectF();  // 不需要剪裁

			Render::FrameData shadowFrameData;
			shadowFrameData.roundedRects.push_back(shadowCmd);
			m_renderer.drawFrame(shadowFrameData, m_iconCache, devicePixelRatio());
		}
	}

	// Then render the main background
	Render::RoundedRectCmd bgCmd;
	if (m_actualContentRect.isValid()) {
		bgCmd.rect = QRectF(m_actualContentRect);
	}
	else {
		bgCmd.rect = QRectF(0, 0, width(), height());  // fallback to full window
	}
	bgCmd.radiusPx = m_cornerRadius;
	bgCmd.color = m_backgroundColor;
	bgCmd.clipRect = QRectF();  // 不需要剪裁

	Render::FrameData bgFrameData;
	bgFrameData.roundedRects.push_back(bgCmd);

	m_renderer.drawFrame(bgFrameData, m_iconCache, devicePixelRatio());
}

void PopupOverlay::renderContent()
{
	if (!m_content) {
		return;
	}

	// Save current transform state if needed for content positioning
	if (m_actualContentRect.isValid()) {
		// We'll need to translate rendering commands to account for content offset
		// For now, let the content render at its natural position within the actual content rect
	}

	// 创建渲染数据容器
	Render::FrameData frameData;

	// 让内容添加渲染数据
	m_content->append(frameData);

	// Translate all rendering commands to account for content positioning
	if (m_actualContentRect.isValid() && (m_actualContentRect.x() != 0 || m_actualContentRect.y() != 0)) {
		QPointF offset(m_actualContentRect.x(), m_actualContentRect.y());

		// Translate all rounded rect commands
		for (auto& rectCmd : frameData.roundedRects) {
			rectCmd.rect.translate(offset);
			if (rectCmd.clipRect.width() > 0 && rectCmd.clipRect.height() > 0) {
				rectCmd.clipRect.translate(offset);
			}
		}

		// Translate all image commands
		for (auto& imageCmd : frameData.images) {
			imageCmd.dstRect.translate(offset);
			if (imageCmd.clipRect.width() > 0 && imageCmd.clipRect.height() > 0) {
				imageCmd.clipRect.translate(offset);
			}
		}
	}

	// 使用渲染器绘制frameData
	m_renderer.drawFrame(frameData, m_iconCache, devicePixelRatio());
}

void PopupOverlay::forwardThemeChange(bool isDark)
{
	m_isDarkTheme = isDark;
	if (m_content) {
		m_content->onThemeChanged(isDark);
	}
}

bool PopupOverlay::eventFilter(QObject* obj, QEvent* event)
{
	// Only filter mouse press events for click-outside detection
	if (event->type() == QEvent::MouseButtonPress && isVisible()) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

		// Convert global position to window coordinates
		QPoint globalPos = mouseEvent->globalPosition().toPoint();
		QPoint localPos = mapFromGlobal(globalPos);

		// If click is outside this window, hide the popup
		if (!QRect(0, 0, width(), height()).contains(localPos)) {
			hidePopup();
			// Don't consume the event - let other widgets handle it
			return false;
		}
	}

	// Call parent event filter
	return QOpenGLWindow::eventFilter(obj, event);
}