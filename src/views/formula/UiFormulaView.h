#pragma once

#include "UiPanel.h"
#include <memory>

class FormulaViewModel;
class UiTreeList;

namespace UI { class RebuildHost; }

// 使用 UiPanel 作为“水平顺序容器”，按子项实际尺寸依次排布
class UiFormulaView final : public UiPanel {
public:
	UiFormulaView();
	~UiFormulaView() override;

	// 供 DataPage 调用
	void setDarkTheme(bool dark);

	// IUiContent
	void setViewportRect(const QRect& r) override;

	// IUiComponent
	void onThemeChanged(bool isDark) override;

private:
	// 初始化/重建子项（左树 + 分割条 + 右详情）
	void buildChildren();
	// 根据主题应用树的调色板
	void applyPalettes() const;

	// 内部宽度提示包装器（给 Panel 提供主轴方向的期望尺寸）
	class WidthHint;

private:
	// VM 与视图
	std::unique_ptr<FormulaViewModel> m_vm;
	std::unique_ptr<UiTreeList>       m_tree;
	class VmTreeAdapter;
	std::unique_ptr<VmTreeAdapter>    m_adapter;

	// 右侧详情（可重建宿主）
	std::unique_ptr<UI::RebuildHost>  m_detailHost;
	std::unique_ptr<class UiScrollView> m_detailScroll; // 新增：详情区域滚动容器

	// 中间分割条
	class VSplitter;
	std::unique_ptr<VSplitter>        m_splitter;

	// 包装器：为 Panel 提供“期望宽度”
	std::unique_ptr<WidthHint>        m_treeWrap;
	std::unique_ptr<WidthHint>        m_detailWrap;

	// 状态
	bool  m_isDark{ false };
	float m_leftRatio{ 0.35f }; // 左右分栏比例：左 35% / 右 65%（用于计算期望宽度）
};