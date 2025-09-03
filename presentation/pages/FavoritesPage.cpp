#include "FavoritesPage.h"

FavoritesPage::FavoritesPage()
{
	setTitle("收藏");
	FavoritesPage::initializeContent();
}

void FavoritesPage::initializeContent()
{
	// 收藏页暂时没有特定内容
	setContent(nullptr);
}