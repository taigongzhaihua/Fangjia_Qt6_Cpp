#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <IconLoader.h>
#include <memory>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>

class FormulaViewModel;
class UiTreeList;

namespace UI { class RebuildHost; }

class UiFormulaView final : public IUiComponent, public IUiContent {
public:
	UiFormulaView();
	~UiFormulaView() override;

	// 主题切换（供 DataPage 调用）
	void setDarkTheme(bool dark);

	// IUiContent
	void setViewportRect(const QRect& r) override;

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override;

	void onThemeChanged(bool isDark) override;

private:
	// 构建整棵根 UI（Row：左树 + 竖线 + 右详情）
	void rebuildRoot();
	// 根据主题应用调色到树
	void applyPalettes();

private:
	// VM 与视图
	std::unique_ptr<FormulaViewModel> m_vm;
	std::unique_ptr<UiTreeList>       m_tree;
	// 将 VM 适配为 UiTreeList::Model
	class VmTreeAdapter;
	std::unique_ptr<VmTreeAdapter>    m_adapter;

	// 右侧详情重建宿主（在选中变化/主题变化时重建）
	std::unique_ptr<UI::RebuildHost>  m_detailHost;

	// 根组件（声明式 Widget build() 产物）
	std::unique_ptr<IUiComponent>     m_root;

	// 视口/上下文缓存（用于转发）
	QRect m_viewport;
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
	QSize m_lastWinSize;

	// 状态
	bool  m_isDark{ false };
	float m_leftRatio{ 0.35f }; // 左右分栏比例：左 35% / 右 65%
};