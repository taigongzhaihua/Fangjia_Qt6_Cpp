#pragma once
#include "UiPage.h"

class FavoritesPage : public UiPage
{
public:
    FavoritesPage();
    ~FavoritesPage() override = default;
    
protected:
    void initializeContent() override;
};