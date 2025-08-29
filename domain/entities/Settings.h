#pragma once
#include <string>

namespace domain {
namespace entities {

/// Pure C++ representation of application settings
/// No Qt dependencies - uses standard C++ types only
struct Settings {
    // Theme configuration
    std::string themeMode;  // "system", "light", "dark"
    
    // Navigation configuration
    bool navExpanded = true;
    int navSelectedIndex = 0;
    
    // Window configuration  
    struct WindowGeometry {
        int x = 0;
        int y = 0; 
        int width = 1200;
        int height = 760;
    } windowGeometry;
    
    std::string windowState; // Base64 encoded window state
    
    // Recent usage tracking
    std::string recentTab;
    std::string recentFormula;
    
    // Default constructor with sensible defaults
    Settings() : themeMode("system") {}
};

} // namespace entities
} // namespace domain