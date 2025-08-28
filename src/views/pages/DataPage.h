#pragma once
#include "TabViewModel.h"
#include "UiPage.h"
#include <memory>

// 前向声明
class AppConfig;

class DataPage : public UiPage
{
public:
	explicit DataPage(AppConfig* config);
	~DataPage() override;

	TabViewModel* tabViewModel() const;

protected:
	void initializeContent() override;
	void applyPageTheme(bool isDark) override;

	// 页面生命周期钩子
	void onAppear() override;
	void onDisappear() override;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};