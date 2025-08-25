#include "Theme.h"

namespace UI {

// 当前主题存储
static ThemeData g_currentTheme;

const ThemeData& Theme::of() {
    return g_currentTheme;
}

} // namespace UI