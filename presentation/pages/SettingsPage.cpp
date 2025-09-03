#include "SettingsPage.h"

SettingsPage::SettingsPage()
{
	setTitle("设置");
	SettingsPage::initializeContent();
}

void SettingsPage::initializeContent()
{
	// 设置页暂时没有特定内容
	setContent(nullptr);
}