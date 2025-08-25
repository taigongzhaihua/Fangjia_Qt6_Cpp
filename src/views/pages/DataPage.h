#pragma once
#include "TabViewModel.h"
#include "UiPage.h"
#include <memory>

class DataPage : public UiPage
{
public:
	DataPage();
	~DataPage() override;

	TabViewModel* tabViewModel() const;

protected:
	void initializeContent() override;
	void applyPageTheme(bool isDark) override;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};