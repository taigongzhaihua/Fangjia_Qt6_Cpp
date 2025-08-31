#pragma once

#include "UiComponent.hpp"
#include "IconCache.h"
#include <memory>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>

class FormulaViewModel;
class UiTreeList;

namespace UI { 
	class RebuildHost; 
	class Widget;
	using WidgetPtr = std::shared_ptr<Widget>;
}

namespace Render {
	struct FrameData;
}

// Simplified UiFormulaView as thin IUiComponent wrapper using declarative UI::Grid
class UiFormulaView final : public IUiComponent {
public:
	UiFormulaView();
	~UiFormulaView() override;

	// 供 DataPage 调用
	void setDarkTheme(bool dark);

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
	bool tick() override;
	QRect bounds() const override;
	void onThemeChanged(bool isDark) override;

private:
	// 初始化/重建主UI组件
	void buildUI();
	// 根据主题应用树的调色板
	void applyPalettes() const;

private:
	// VM 与视图
	std::unique_ptr<FormulaViewModel> m_vm;
	std::unique_ptr<UiTreeList> m_tree;

	// 右侧详情（声明式绑定宿主）
	UI::WidgetPtr m_detailBindingHost;

	// 主UI组件（Grid布局）
	std::unique_ptr<IUiComponent> m_mainUI;

	// 状态
	bool m_isDark{ false };
};