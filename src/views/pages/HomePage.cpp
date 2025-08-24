#include "HomePage.h"
#include <qstring.h>

HomePage::HomePage()
{
    setTitle("首页");
    initializeContent();
}

void HomePage::initializeContent()
{
    // 首页暂时没有特定内容
    setContent(nullptr);
}