[English](../../doc/presentation/components/nav-rail.md) | **简体中文**

# NavRail 组件

## 概览

NavRail 组件提供具有平滑动画、数据提供者集成和主题感知样式的垂直导航。它支持图标/文本组合、活动状态指示器，以及与应用程序状态变化同步的可自定义动画。

## 核心特性

- **垂直导航**: 基于图标或文本+图标的导航项目
- **活动指示器**: 显示当前选择的平滑动画指示器
- **数据提供者模式**: 与外部数据源集成以实现动态内容
- **动画同步**: 与其他 UI 组件的协调动画
- **主题集成**: 明/暗主题的自动颜色适应
- **响应式设计**: 自适应宽度和项目大小

## 基本用法

### 简单导航轨道

```cpp
auto navRail = std::make_unique<UiNavRail>();

// 添加导航项目
navRail->addItem({"home", "主页", ":/icons/home.svg"});
navRail->addItem({"data", "数据", ":/icons/database.svg"});
navRail->addItem({"settings", "设置", ":/icons/settings.svg"});

// 设置活动项目
navRail->setActiveItem("home");

// 配置外观
navRail->setWidth(200);
navRail->setItemHeight(48);
navRail->setItemSpacing(4);
```

### 事件处理

```cpp
// 处理项目选择
navRail->onItemClick = [](const QString& itemId) {
    qDebug() << "选中项目:" << itemId;
    // 处理导航逻辑
};

// 处理活动项目变化（包括编程变化）
navRail->onActiveChanged = [](const QString& itemId) {
    qDebug() << "活动项目变更为:" << itemId;
    // 更新应用程序状态
};
```

## DataProvider 集成

### 使用外部数据源

NavRail 可以由外部数据提供者驱动以实现动态内容：

```cpp
class NavigationDataProvider {
public:
    struct NavItem {
        QString id;
        QString title;
        QString iconPath;
        bool isEnabled = true;
        bool isVisible = true;
        int sortOrder = 0;
        QVariantMap metadata;
    };
    
    virtual QVector<NavItem> getNavigationItems() const = 0;
    virtual QString getActiveItemId() const = 0;
    virtual void setActiveItemId(const QString& id) = 0;
    
signals:
    void itemsChanged();
    void activeItemChanged(const QString& id);
};

// 使用数据提供者配置 NavRail
navRail->setDataProvider(dataProvider);
navRail->setAutoUpdate(true); // 数据变化时自动更新
```

## 动画配置

### 指示器动画

```cpp
// 配置活动指示器动画
navRail->setAnimationDuration(300);
navRail->setAnimationEasing(QEasingCurve::OutCubic);
navRail->setIndicatorColor(QColor(13, 110, 253));
navRail->setIndicatorWidth(4);

// 自定义动画行为
navRail->setAnimationMode(UiNavRail::AnimationMode::Smooth);
```

### 主题适应

```cpp
void MyNavRail::onThemeChanged(bool isDark) {
    const auto& palette = currentPalette();
    
    // 更新指示器颜色
    setIndicatorColor(palette.primaryAccent);
    
    // 更新项目颜色
    setItemTextColor(palette.primaryText);
    setItemIconColor(palette.secondaryText);
    setActiveItemTextColor(palette.primaryAccent);
    setActiveItemIconColor(palette.primaryAccent);
    
    // 更新背景
    setBackgroundColor(palette.navigation.background);
    setItemBackgroundColor(palette.navigation.itemBackground);
    setActiveItemBackgroundColor(palette.navigation.activeBackground);
}
```

## 相关文档

- [UI 框架概览](../ui-framework/overview.md)
- [主题与渲染](../ui-framework/theme-and-rendering.md)
- [数据绑定](../binding.md)