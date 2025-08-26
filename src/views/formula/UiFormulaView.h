#pragma once

#include <memory>
#pragma once
#include "UiBoxLayout.h"

class FormulaViewModel;
class UiTreeList;

namespace UI { class RebuildHost; }

// 直接继承 UiBoxLayout，把自己作为“水平分栏容器”
class UiFormulaView final : public UiBoxLayout {
public:
	UiFormulaView();
	~UiFormulaView() override;

	// 供 DataPage 调用
	void setDarkTheme(bool dark);

	// IUiComponent
	void onThemeChanged(bool isDark) override;

private:
	// 初始化/重建子项（左树 + 分割条 + 右详情）
	void buildChildren();
	// 根据主题应用树的调色板
	void applyPalettes();

private:
	// VM 与视图
	std::unique_ptr<FormulaViewModel> m_vm;
	std::unique_ptr<UiTreeList>       m_tree;
	class VmTreeAdapter;
	std::unique_ptr<VmTreeAdapter>    m_adapter;

	// 右侧详情（可重建宿主）
	std::unique_ptr<UI::RebuildHost>  m_detailHost;

	// 中间分割条
	class VSplitter;
	std::unique_ptr<VSplitter>        m_splitter;

	// 状态
	bool  m_isDark{ false };
	float m_leftRatio{ 0.35f }; // 左右分栏比例：左 35% / 右 65%
};