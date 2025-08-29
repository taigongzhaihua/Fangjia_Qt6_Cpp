#pragma once

// 主题感知接口：允许组件响应主题变化
class IThemeAware {
public:
    virtual ~IThemeAware() = default;
    
    // 当主题改变时被调用
    // isDark: true表示深色主题，false表示浅色主题
    virtual void onThemeChanged(bool isDark) = 0;
    
    // 应用主题（可选的具体实现）
    virtual void applyTheme(bool isDark) {}
};