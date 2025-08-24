#pragma once
#include "UiPage.h"

class ExplorePage : public UiPage
{
public:
    ExplorePage();
    ~ExplorePage() override = default;
    
protected:
    void initializeContent() override;
};