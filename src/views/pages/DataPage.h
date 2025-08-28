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

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};