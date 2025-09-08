#include "IconCache.h"
#include "RenderData.hpp"
#include "UiPushButton.h"

#include <algorithm>
#include <qfontmetrics.h>
#include <qopenglfunctions.h>

#include "RenderUtils.hpp"
#include <ILayoutable.hpp>
#include <qbytearray.h>
#include <qcolor.h>
#include <qfont.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qnumeric.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qglobal.h>

UiPushButton::UiPushButton() {
	// 设置默认圆角半径
	m_button.setCornerRadius(m_cornerRadius);

	// 设置默认启用状态
	m_button.setEnabled(!m_disabled);

	// 初始化图标绘制器
	setupIconPainter();
}

// === 属性配置实现 ===

void UiPushButton::setIconPath(const QString& path) {
	m_iconPath = path;
	m_useThemeIconPaths = false;
	setupIconPainter();
}

void UiPushButton::setIconThemePaths(const QString& lightPath, const QString& darkPath) {
	m_iconLightPath = lightPath;
	m_iconDarkPath = darkPath;
	m_useThemeIconPaths = true;
	setupIconPainter();
}

// === IUiContent 接口实现 ===

void UiPushButton::setViewportRect(const QRect& r) {
	m_bounds = r;
	// 更新内部按钮的基础矩形
	m_button.setBaseRect(r);
}

// === ILayoutable 接口实现 ===

QSize UiPushButton::measure(const SizeConstraints& cs) {
	const QFont font = getFont();
	const QFontMetrics fm(font);
	const QMargins padding = getPadding();
	const int iconSize = getIconSize();

	// 计算文本尺寸
	int textWidth = 0;
	int textHeight = 0;
	if (!m_text.isEmpty()) {
		textWidth = fm.horizontalAdvance(m_text);
		textHeight = fm.height();
	}

	// 计算总内容宽度
	int contentWidth = textWidth;
	if (!getCurrentIconPath().isEmpty()) {
		contentWidth += iconSize;
		if (!m_text.isEmpty()) {
			contentWidth += 8; // 图标和文本之间的间距
		}
	}

	// 加上内边距
	const int totalWidth = padding.left() + contentWidth + padding.right();
	const int totalHeight = padding.top() + std::max(textHeight, iconSize) + padding.bottom();

	// 应用约束
	const int finalWidth = std::clamp(totalWidth, cs.minW, cs.maxW);
	const int finalHeight = std::clamp(totalHeight, cs.minH, cs.maxH);

	return QSize(finalWidth, finalHeight);
}

void UiPushButton::arrange(const QRect& finalRect) {
	m_bounds = finalRect;
	// 更新内部按钮的基础矩形
	m_button.setBaseRect(finalRect);
}

// === IUiComponent 接口实现 ===

void UiPushButton::updateLayout(const QSize& windowSize) {
	// 按钮布局由父容器管理，这里不需要特殊处理
	Q_UNUSED(windowSize);
}

void UiPushButton::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) {
	m_cache = &cache;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);

	// 重新设置图标绘制器以使用新的资源上下文
	setupIconPainter();
}

void UiPushButton::append(Render::FrameData& fd) const {
	// 记录初始命令数量，用于父级剪裁
	const int rr0 = static_cast<int>(fd.roundedRects.size());
	const int im0 = static_cast<int>(fd.images.size());

	// 委托给内部按钮进行背景和图标绘制
	m_button.append(fd);

	// 绘制焦点环（在按钮内容之后，这样焦点环在最上层）
	if (m_focused && !m_disabled) {
		const QRectF buttonRect = m_button.visualRectF();
		const float focusRingWidth = 2.0f;
		const QRectF focusRect = buttonRect.adjusted(-focusRingWidth, -focusRingWidth, focusRingWidth, focusRingWidth);

		// 焦点环颜色：根据主题选择
		QColor focusColor = m_isDarkTheme ? QColor(120, 170, 255, 120) : QColor(70, 130, 255, 120);

		// 添加焦点环（绘制一个外部矩形作为焦点指示）
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = focusRect,
			.radiusPx = m_cornerRadius + focusRingWidth,
			.color = focusColor,
			.clipRect = focusRect
			});
	}

	// 应用父级剪裁到新增的命令
	RenderUtils::applyParentClip(fd, rr0, im0, QRectF(m_bounds));
}

bool UiPushButton::onMousePress(const QPoint& pos) {
	if (m_disabled) return false;
	const bool handled = m_button.onMousePress(pos);
	// Focus will be automatically set by UiRoot when this returns true
	return handled;
}

bool UiPushButton::onMouseMove(const QPoint& pos) {
	if (m_disabled) return false;
	return m_button.onMouseMove(pos);
}

bool UiPushButton::onMouseRelease(const QPoint& pos) {
	if (m_disabled) return false;

	bool clicked = false;
	const bool consumed = m_button.onMouseRelease(pos, clicked);

	// 如果点击成功且有回调，则执行回调
	if (clicked && m_onTap) {
		m_onTap();
	}

	return consumed;
}

bool UiPushButton::tick() {
	// 检查按钮状态是否需要视觉更新
	// 当鼠标悬停状态或按下状态发生变化时，需要重新渲染
	bool currentHovered = m_button.hovered();
	bool currentPressed = m_button.pressed();
	
	if (m_lastHovered != currentHovered || m_lastPressed != currentPressed) {
		m_lastHovered = currentHovered;
		m_lastPressed = currentPressed;
		return true; // 状态发生变化，需要更新
	}
	
	return false; // 状态无变化，无需更新
}

QRect UiPushButton::bounds() const {
	return m_bounds;
}

// === IThemeAware 接口实现 ===

void UiPushButton::applyTheme(bool isDark) {
	m_isDarkTheme = isDark;
	updateButtonPalette();
	setupIconPainter(); // 主题变化可能影响图标路径
}

// === 内部辅助方法实现 ===

QFont UiPushButton::getFont() const {
	QFont font;

	// 根据尺寸设置字体大小
	switch (m_size) {
	case Size::S:
		font.setPixelSize(12);
		break;
	case Size::M:
		font.setPixelSize(14);
		break;
	case Size::L:
		font.setPixelSize(16);
		break;
	}

	// Primary变体使用稍微粗一些的字体
	if (m_variant == Variant::Primary) {
		font.setWeight(QFont::Medium);
	}
	else {
		font.setWeight(QFont::Normal);
	}

	return font;
}

QMargins UiPushButton::getPadding() const {
	if (m_useCustomPadding) {
		return m_customPadding;
	}

	// 根据尺寸提供预设内边距
	switch (m_size) {
	case Size::S:
		return QMargins(12, 6, 12, 6);
	case Size::M:
		return QMargins(16, 8, 16, 8);
	case Size::L:
		return QMargins(20, 12, 20, 12);
	}

	return QMargins(16, 8, 16, 8); // 默认值
}

int UiPushButton::getIconSize() const {
	// 根据尺寸返回图标逻辑像素尺寸
	switch (m_size) {
	case Size::S:
		return 16;
	case Size::M:
		return 20;
	case Size::L:
		return 24;
	}

	return 20; // 默认值
}

void UiPushButton::updateButtonPalette() {
	QColor bg, bgHover, bgPressed, textColor;

	// 根据变体和主题设置颜色
	if (m_variant == Variant::Primary) {
		if (m_isDarkTheme) {
			bg = QColor(70, 130, 255);          // 蓝色主按钮
			bgHover = QColor(90, 150, 255);     // 悬停时更亮
			bgPressed = QColor(50, 110, 235);   // 按下时更暗
			textColor = QColor(255, 255, 255);  // 白色文字
		}
		else {
			bg = QColor(60, 120, 245);
			bgHover = QColor(80, 140, 255);
			bgPressed = QColor(40, 100, 225);
			textColor = QColor(255, 255, 255);
		}
	}
	else if (m_variant == Variant::Secondary) {
		if (m_isDarkTheme) {
			bg = QColor(60, 65, 70);            // 深灰背景
			bgHover = QColor(80, 85, 90);       // 悬停时稍亮
			bgPressed = QColor(40, 45, 50);     // 按下时更暗
			textColor = QColor(220, 225, 230);  // 浅色文字
		}
		else {
			bg = QColor(240, 242, 245);
			bgHover = QColor(230, 232, 235);
			bgPressed = QColor(220, 222, 225);
			textColor = QColor(60, 65, 70);
		}
	}
	else if (m_variant == Variant::Ghost) {
		if (m_isDarkTheme) {
			bg = QColor(0, 0, 0, 0);            // 透明背景
			bgHover = QColor(255, 255, 255, 20); // 悬停时微弱白色
			bgPressed = QColor(255, 255, 255, 40); // 按下时稍强
			textColor = QColor(220, 225, 230);   // 浅色文字
		}
		else {
			bg = QColor(0, 0, 0, 0);
			bgHover = QColor(0, 0, 0, 20);
			bgPressed = QColor(0, 0, 0, 40);
			textColor = QColor(60, 65, 70);
		}
	}
	else { // Destructive
		if (m_isDarkTheme) {
			bg = QColor(220, 60, 60);           // 红色破坏性按钮
			bgHover = QColor(240, 80, 80);      // 悬停时更亮
			bgPressed = QColor(200, 40, 40);    // 按下时更暗
			textColor = QColor(255, 255, 255);  // 白色文字
		}
		else {
			bg = QColor(210, 50, 50);
			bgHover = QColor(230, 70, 70);
			bgPressed = QColor(190, 30, 30);
			textColor = QColor(255, 255, 255);
		}
	}

	// 禁用状态下降低不透明度
	if (m_disabled) {
		const float disabledOpacity = 0.4f;
		bg.setAlphaF(bg.alphaF() * disabledOpacity);
		bgHover.setAlphaF(bgHover.alphaF() * disabledOpacity);
		bgPressed.setAlphaF(bgPressed.alphaF() * disabledOpacity);
		textColor.setAlphaF(textColor.alphaF() * disabledOpacity);

		// 禁用时不响应悬停和按下
		bgHover = bg;
		bgPressed = bg;
	}

	// 应用到内部按钮
	m_button.setPalette(bg, bgHover, bgPressed, textColor);
	m_button.setEnabled(!m_disabled);
	m_button.setCornerRadius(m_cornerRadius);
}

QString UiPushButton::getCurrentIconPath() const {
	if (m_useThemeIconPaths) {
		return m_isDarkTheme ? m_iconDarkPath : m_iconLightPath;
	}
	return m_iconPath;
}

void UiPushButton::setupIconPainter() {
	createIconAndTextPainter();
}

void UiPushButton::createIconAndTextPainter() {
	// 创建图标和文本绘制器
	m_button.setIconPainter([this](const QRectF& rect, Render::FrameData& fd, const QColor& iconColor, float opacity) {
		if (!m_cache || !m_gl) return;

		const QMargins padding = getPadding();
		const int iconSize = getIconSize();
		const QString iconPath = getCurrentIconPath();

		// 计算内容区域
		const QRectF contentRect = rect.adjusted(padding.left(), padding.top(), -padding.right(), -padding.bottom());

		float currentX = contentRect.left();
		bool hasIcon = false;

		// 绘制图标
		if (!iconPath.isEmpty()) {
			const QByteArray svgData = RenderUtils::loadSvgCached(iconPath);
			if (!svgData.isEmpty()) {
				const int pixelSize = qRound(iconSize * m_dpr);
				const QString cacheKey = RenderUtils::makeIconCacheKey(iconPath, pixelSize);

				const int texId = m_cache->ensureSvgPx(cacheKey, svgData, QSize(pixelSize, pixelSize), m_gl);
				const QSize texSizePx = m_cache->textureSizePx(texId);

				if (texId > 0 && !texSizePx.isEmpty()) {
					// 计算图标位置（垂直居中）
					const float iconY = contentRect.top() + (contentRect.height() - iconSize) * 0.5f;
					const QRectF iconRect(currentX, iconY, iconSize, iconSize);

					fd.images.push_back(Render::ImageCmd{
						.dstRect = iconRect,
						.textureId = texId,
						.srcRectPx = QRectF(QPointF(0, 0), QSizeF(texSizePx)),
						.tint = iconColor,
						.clipRect = rect // 使用按钮整体区域作为剪裁
						});

					currentX += iconSize + (m_text.isEmpty() ? 0 : 8); // 图标后加间距
					hasIcon = true;
				}
			}
		}

		// 绘制文本
		if (!m_text.isEmpty()) {
			const QFont font = getFont();
			const QFont fontPx = [&]() {
				QFont f = font;
				f.setPixelSize(qRound(font.pixelSize() * m_dpr));
				return f;
				}();

			const QString cacheKey = RenderUtils::makeTextCacheKey(m_text, fontPx.pixelSize(), iconColor);
			const int texId = m_cache->ensureTextPx(cacheKey, fontPx, m_text, iconColor, m_gl);
			const QSize texSizePx = m_cache->textureSizePx(texId);

			if (texId > 0 && !texSizePx.isEmpty()) {
				// 计算文本位置（垂直居中）
				const QFontMetrics fm(font);
				const float textWidth = fm.horizontalAdvance(m_text);
				const float textHeight = fm.height();
				const float textY = contentRect.top() + (contentRect.height() - textHeight) * 0.5f;

				// 计算水平居中位置
				float textX;
				if (hasIcon) {
					// 有图标：在剩余区域内水平居中
					const QRectF remainingRect(currentX, contentRect.top(), contentRect.right() - currentX, contentRect.height());
					textX = remainingRect.left() + (remainingRect.width() - textWidth) * 0.5f;
				}
				else {
					// 仅文本：在整个内容区域内水平居中
					textX = contentRect.left() + (contentRect.width() - textWidth) * 0.5f;
				}

				const QRectF textRect(textX, textY, textWidth, textHeight);

				fd.images.push_back(Render::ImageCmd{
					.dstRect = textRect,
					.textureId = texId,
					.srcRectPx = QRectF(QPointF(0, 0), QSizeF(texSizePx)),
					.tint = iconColor,
					.clipRect = rect // 使用按钮整体区域作为剪裁
					});
			}
		}
		});
}

// === IFocusable 接口实现 ===

bool UiPushButton::isFocused() const {
	return m_focused;
}

void UiPushButton::setFocused(bool focused) {
	if (m_focused != focused) {
		m_focused = focused;
		// 焦点状态改变时，可能需要重新绘制焦点环
		// 实际绘制在append()方法中处理
	}
}

bool UiPushButton::canFocus() const {
	// 只有启用的按钮可以获得焦点
	return !m_disabled;
}

// === IKeyInput 接口实现 ===

bool UiPushButton::onKeyPress(int key, Qt::KeyboardModifiers modifiers) {
	Q_UNUSED(modifiers);

	// 只有有焦点且启用的按钮才响应键盘输入
	if (!m_focused || m_disabled) {
		return false;
	}

	// 空格键和回车键可以激活按钮
	if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
		// 模拟鼠标按下，触发按下状态
		m_button.simulatePress();
		return true;
	}

	return false;
}

bool UiPushButton::onKeyRelease(int key, Qt::KeyboardModifiers modifiers) {
	Q_UNUSED(modifiers);

	// 只有有焦点且启用的按钮才响应键盘输入
	if (!m_focused || m_disabled) {
		return false;
	}

	// 空格键和回车键释放时执行点击回调
	if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
		// 模拟鼠标释放并触发点击
		m_button.simulateRelease();
		if (m_onTap) {
			m_onTap();
		}
		return true;
	}

	return false;
}