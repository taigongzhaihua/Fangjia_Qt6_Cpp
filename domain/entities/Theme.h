#pragma once
#include <cstdint>

namespace domain {
namespace entities {

/// Pure C++ theme mode enumeration for domain layer
/// No Qt dependencies - uses standard C++ types only
enum class ThemeMode : uint8_t {
    Light,          // 强制亮色主题
    Dark,           // 强制暗色主题  
    FollowSystem    // 跟随系统配色方案
};

/// Convert ThemeMode enum to string representation
inline const char* themeModeToString(ThemeMode mode) {
    switch (mode) {
        case ThemeMode::Light: return "light";
        case ThemeMode::Dark: return "dark";
        case ThemeMode::FollowSystem: return "system";
    }
    return "system"; // Default fallback
}

/// Convert string to ThemeMode enum
inline ThemeMode stringToThemeMode(const char* str) {
    if (!str) return ThemeMode::FollowSystem;
    
    // Simple string comparison
    const char* s = str;
    if (s[0] == 'l' && s[1] == 'i' && s[2] == 'g' && s[3] == 'h' && s[4] == 't' && s[5] == '\0') {
        return ThemeMode::Light;
    }
    if (s[0] == 'd' && s[1] == 'a' && s[2] == 'r' && s[3] == 'k' && s[4] == '\0') {
        return ThemeMode::Dark;
    }
    return ThemeMode::FollowSystem; // Default for "system" and unknown values
}

} // namespace entities
} // namespace domain