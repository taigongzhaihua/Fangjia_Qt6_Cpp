/*
 * 文件名：UiPushButton.h
 * 职责：可重用的按钮运行时组件，支持文本、图标、主题和交互状态。
 * 依赖：UI组件接口、布局系统、渲染工具。
 * 线程：仅在UI线程使用。
 * 备注：内部使用 Ui::Button 引擎处理背景绘制和交互状态。
 */

#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "ILayoutable.hpp"
#include "UiButton.hpp"
#include "RenderUtils.hpp"

#include <qfont.h>
#include <qmargins.h>
#include <qstring.h>
#include <functional>

/// 可重用按钮运行时组件：实现完整的按钮功能包括文本、图标、状态管理
/// 
/// 功能特性：
/// - 支持纯文本、纯图标或文本+图标组合
/// - 多种预设尺寸（S/M/L）和视觉变体（Primary/Secondary/Ghost）
/// - 完整的交互状态：正常/悬停/按下/禁用
/// - 主题自适应的颜色方案
/// - DPR优化的文本和图标渲染
/// - 可定制的圆角半径和内边距
/// 
/// 使用方式：
/// UiPushButton btn;
/// btn.setText("保存");
/// btn.setVariant(UiPushButton::Variant::Primary);
/// btn.setSize(UiPushButton::Size::M);
/// btn.setOnTap([](){ /* 点击处理 */ });
class UiPushButton final : public IUiComponent, public IUiContent, public ILayoutable {
public:
	/// 按钮视觉变体
	enum class Variant {
		Primary,   // 主要按钮：高对比背景色
		Secondary, // 次要按钮：较低对比背景色  
		Ghost      // 幽灵按钮：透明背景，仅边框/文字
	};

	/// 按钮尺寸预设
	enum class Size {
		S,  // 小尺寸：适用于紧凑界面
		M,  // 中等尺寸：标准用途
		L   // 大尺寸：突出的主要操作
	};

	UiPushButton();
	~UiPushButton() override = default;

	// === 属性配置 ===

	/// 功能：设置按钮文本
	/// 参数：text — 显示的文本内容
	void setText(const QString& text) { m_text = text; }
	
	/// 功能：获取按钮文本
	/// 返回：当前设置的文本内容
	const QString& text() const { return m_text; }

	/// 功能：设置单个图标路径
	/// 参数：path — SVG图标文件路径
	void setIconPath(const QString& path);

	/// 功能：设置主题相关的图标路径
	/// 参数：lightPath — 浅色主题图标路径
	/// 参数：darkPath — 深色主题图标路径
	void setIconThemePaths(const QString& lightPath, const QString& darkPath);

	/// 功能：设置按钮变体样式
	/// 参数：variant — 视觉变体类型
	void setVariant(Variant variant) { m_variant = variant; }
	
	/// 功能：获取按钮变体
	/// 返回：当前的视觉变体
	Variant variant() const { return m_variant; }

	/// 功能：设置按钮尺寸
	/// 参数：size — 尺寸预设
	void setSize(Size size) { m_size = size; }
	
	/// 功能：获取按钮尺寸
	/// 返回：当前的尺寸预设
	Size size() const { return m_size; }

	/// 功能：设置圆角半径
	/// 参数：radius — 圆角半径（逻辑像素）
	void setCornerRadius(float radius) { m_cornerRadius = radius; }

	/// 功能：设置内边距覆盖
	/// 参数：padding — 自定义内边距，空值使用预设
	void setPadding(const QMargins& padding) { m_customPadding = padding; m_useCustomPadding = true; }
	
	/// 功能：清除内边距覆盖，恢复预设
	void clearCustomPadding() { m_useCustomPadding = false; }

	/// 功能：设置禁用状态
	/// 参数：disabled — 是否禁用
	void setDisabled(bool disabled) { m_disabled = disabled; }
	
	/// 功能：获取禁用状态
	/// 返回：是否处于禁用状态
	bool isDisabled() const { return m_disabled; }

	/// 功能：设置点击回调
	/// 参数：callback — 点击时执行的函数
	void setOnTap(std::function<void()> callback) { m_onTap = std::move(callback); }

	// === IUiContent 接口 ===
	void setViewportRect(const QRect& r) override;

	// === ILayoutable 接口 ===
	QSize measure(const SizeConstraints& cs) override;
	void arrange(const QRect& finalRect) override;

	// === IUiComponent 接口 ===
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override;

protected:
	// === IThemeAware 接口 ===
	void applyTheme(bool isDark) override;

private:
	// === 内部辅助方法 ===
	QFont getFont() const;
	QMargins getPadding() const;
	int getIconSize() const;
	void updateButtonPalette();
	void setupIconPainter();
	QString getCurrentIconPath() const;
	void createIconAndTextPainter();

private:
	// === 配置属性 ===
	QString m_text;
	QString m_iconPath;
	QString m_iconLightPath;
	QString m_iconDarkPath;
	bool m_useThemeIconPaths{ false };
	
	Variant m_variant{ Variant::Primary };
	Size m_size{ Size::M };
	float m_cornerRadius{ 8.0f };
	
	QMargins m_customPadding;
	bool m_useCustomPadding{ false };
	bool m_disabled{ false };
	
	std::function<void()> m_onTap;

	// === 运行时状态 ===
	QRect m_bounds;
	bool m_isDarkTheme{ false };
	
	// === 渲染资源 ===
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// === 内部引擎 ===
	Ui::Button m_button; // 负责背景绘制和交互状态管理
};