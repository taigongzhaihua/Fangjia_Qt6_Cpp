#pragma once
#include "TabViewModel.h"
#include "UiPage.h"
#include "UiTabView.h"
#include <memory>

class DataPage : public UiPage
{
public:
	DataPage();
	~DataPage() override;

	TabViewModel* tabViewModel() { return &m_tabsVm; }

protected:
	void initializeContent() override;
	void applyPageTheme(bool isDark) override;

private:
	TabViewModel m_tabsVm;
	UiTabView m_tabView;
	std::unique_ptr<class UiFormulaView> m_formulaView;
};