#pragma once
#include "UiPage.h"

class SettingsPage : public UiPage
{
public:
    SettingsPage();
    ~SettingsPage() override = default;
    
protected:
    void initializeContent() override;
};