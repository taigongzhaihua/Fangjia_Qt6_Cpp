#pragma once
#include "UiPage.h"
#include <memory>

class HomePage : public UiPage
{
public:
    HomePage();
    ~HomePage() override;

protected:
    void initializeContent() override;
    void applyPageTheme(bool isDark) override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};