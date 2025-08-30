#pragma once
#include "TabViewModel.h"
#include "ViewModelBase.hpp"
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>

// Forward declarations
namespace domain::usecases
{
	class GetRecentTabUseCase;
	class SetRecentTabUseCase;
}

// DataPage 的 ViewModel - transitional implementation during refactor
class DataViewModel : public ViewModelBase
{
	Q_OBJECT
		Q_PROPERTY(int selectedTab READ selectedTab NOTIFY selectedTabChanged)

public:
	// Constructor with dependency injection for domain use cases
	DataViewModel(std::shared_ptr<domain::usecases::GetRecentTabUseCase> getRecentTabUseCase,
		std::shared_ptr<domain::usecases::SetRecentTabUseCase> setRecentTabUseCase,
		QObject* parent = nullptr);

	~DataViewModel() override = default;

	// 标签页相关
	[[nodiscard]] TabViewModel* tabs() const { return m_tabViewModel.get(); }

	// Q_PROPERTY 访问器
	int selectedTab() const;

signals:
	void selectedTabChanged(int index);

private slots:
	// 监听 TabViewModel 的选中变更，通过用例更新设置
	void onTabSelectionChanged(int index);

private:
	void initializeTabs();
	void restoreRecentTab();

private:
	std::unique_ptr<TabViewModel> m_tabViewModel;

	// Domain use cases for recent tab management
	std::shared_ptr<domain::usecases::GetRecentTabUseCase> m_getRecentTabUseCase;
	std::shared_ptr<domain::usecases::SetRecentTabUseCase> m_setRecentTabUseCase;
};