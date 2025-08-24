#pragma once
#include "FormulaViewModel.h"
#include "UiBoxLayout.h"  // 继承通用布局控件
#include "UiFormulaDetail.h"
#include "UiTreeList.h"
#include <memory>
#include <qpoint.h>

// 方剂视图：继承自通用布局控件
class UiFormulaView final : public UiBoxLayout
{
public:
	UiFormulaView();
	~UiFormulaView() override;

	// 设置主题
	void setDarkTheme(bool dark);

	// 重写以处理分割条拖动等特殊逻辑
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;

private:
	void initializeLayout();
	void onFormulaSelected(int index) const;
	void updatePalettes();

private:
	std::unique_ptr<FormulaViewModel> m_formulaVm;
	std::unique_ptr<UiTreeList> m_treeList;
	std::unique_ptr<UiFormulaDetail> m_detailView;

	// 分割条
	std::unique_ptr<UiBoxLayout> m_splitter;
	bool m_isDark{ false };

	// 分割条拖动
	bool m_draggingSplitter{ false };
	int m_dragStartX{ 0 };
	float m_splitRatio{ 0.35f };
};