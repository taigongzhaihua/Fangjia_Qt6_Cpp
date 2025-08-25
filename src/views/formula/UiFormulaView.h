#pragma once
#include "UiBoxLayout.h"  // 继承通用布局控件
#include "UiFormulaDetail.h"
#include "UiTreeList.h"
#include <memory>
#include <qcontainerfwd.h>
#include <qpoint.h>

class FormulaViewModel;

// 方剂视图：继承自通用布局控件
class UiFormulaView final : public UiBoxLayout
{
public:
	UiFormulaView();
	~UiFormulaView() override;

	// 设置主题（保留现有接口，内部自动向下应用）
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

	// 将 VM 适配为 UiTreeList::Model
	class VmTreeAdapter : public UiTreeList::Model
	{
	public:
		explicit			 VmTreeAdapter(FormulaViewModel* vm) : m_vm(vm) {}
		QVector<int>		 rootIndices() const override;
		QVector<int>		 childIndices(int nodeId) const override;
		UiTreeList::NodeInfo nodeInfo(int nodeId) const override;
		int					 selectedId() const override;
		void				 setSelectedId(int nodeId) override;
		void				 setExpanded(int nodeId, bool on) override;

	private:
		FormulaViewModel* m_vm{ nullptr };
	};

	std::unique_ptr<UiTreeList> m_treeList;
	std::unique_ptr<UiFormulaDetail> m_detailView;

	// 分割条
	std::unique_ptr<UiBoxLayout> m_splitter;
	bool m_isDark{ false };

	// 适配器实例（指向 VM）
	std::unique_ptr<VmTreeAdapter> m_treeAdapter;

	// 分割条拖动
	bool m_draggingSplitter{ false };
	int m_dragStartX{ 0 };
	float m_splitRatio{ 0.35f };
};
