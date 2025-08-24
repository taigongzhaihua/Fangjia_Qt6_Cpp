#pragma once
#include "UiPage.h"

class HomePage : public UiPage
{
public:
    HomePage();
    ~HomePage() override = default;
    
protected:
    void initializeContent() override;
};