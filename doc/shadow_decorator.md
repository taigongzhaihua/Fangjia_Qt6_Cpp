# Shadow Decorator API

## Overview / 概述

The Shadow Decorator provides a universal, attachable shadow system that can be applied to any widget in the declarative UI framework. It uses a layered rendering approach to approximate soft shadows without requiring new GPU effects or external dependencies.

阴影装饰器提供了通用的、可附加的阴影系统，可以应用于声明式UI框架中的任何组件。它使用分层渲染方法来近似软阴影效果，无需新的GPU效果或外部依赖。

## API Reference / API参考

### Widget Shadow Method / Widget阴影方法

```cpp
std::shared_ptr<Widget> shadow(QColor color, float blurPx, QPoint offset, float spreadPx = 0.0f);
```

**Parameters / 参数:**
- `color`: Shadow color (recommended: semi-transparent black like `QColor(0,0,0,160)`) / 阴影颜色（推荐：半透明黑色，如 `QColor(0,0,0,160)`）
- `blurPx`: Blur radius in pixels (controls softness, range 0-50) / 模糊半径（像素，控制柔化程度，范围0-50）
- `offset`: Shadow offset as QPoint(x, y) / 阴影偏移，格式为QPoint(x, y)
- `spreadPx`: Additional expansion before layering (optional) / 分层前的额外扩展（可选）

### Card Elevation / Card高度

```cpp
std::shared_ptr<Card> elevation(float e);
```

Card elevation now automatically generates appropriate shadows based on Material Design principles:
Card elevation 现在基于 Material Design 原则自动生成适当的阴影：

- **Shadow opacity** / 阴影透明度: `30 + elevation * 10` (range 30-120, improved transparency)
- **Blur radius** / 模糊半径: `elevation * 2` (range 2-24px)  
- **Y offset** / Y偏移: `elevation * 0.5` (range 1-8px)
- **Spread** / 扩展: `elevation * 0.25` (range 0-4px)

## Usage Examples / 使用示例

### Basic Shadow / 基础阴影

```cpp
// Add shadow to any widget / 为任意组件添加阴影
UI::text("Hello World")
    ->shadow(QColor(0,0,0,160), 8.0f, QPoint(2, 4), 1.0f);

// Colored shadow / 彩色阴影
UI::button("Click Me")
    ->shadow(QColor(255,0,0,100), 12.0f, QPoint(0, 6));
```

### Card Elevation / Card高度

```cpp
// Low elevation card / 低高度卡片
UI::card(UI::text("Content"))
    ->elevation(2);

// High elevation card / 高高度卡片  
UI::card(UI::text("Important"))
    ->elevation(8);
```

### Combined Effects / 组合效果

```cpp
// Card with custom styling and shadow / 带自定义样式和阴影的卡片
UI::card(UI::text("Custom Card"))
    ->elevation(4)
    ->padding(20)
    ->background(QColor(240,240,255), 12.0f);
```

## Implementation Details / 实现细节

### Shadow Rendering / 阴影渲染

The shadow system uses a layered approach to approximate Gaussian blur:
阴影系统使用分层方法来近似高斯模糊：

1. **Layer count** / 层数计算: `clamp(round(blurPx), 8, 64)` (improved from 4-16 to 8-64)
2. **Alpha falloff** / 透明度衰减: `baseAlpha * exp(-2.5 * t)` where `t ∈ (0,1]` (smoother exponential decay)
3. **Rect inflation** / 矩形膨胀: `expand = spreadPx + t * blurPx`
4. **Radius expansion** / 半径扩展: `radius = baseRadius + expand`
5. **Shadow clipping** / 阴影裁剪: Extended clip region allows shadows to extend beyond control bounds

### Performance / 性能

- **Bounded draw calls** / 有界绘制调用: Maximum 64 layers per shadow (improved from 16)
- **No additional textures** / 无额外纹理: Uses existing RoundedRectCmd
- **Extended shadow clipping** / 扩展阴影裁剪: Shadows can extend beyond control bounds
- **Layout neutral** / 布局中性: Shadows don't affect measurement or arrangement

## Technical Notes / 技术说明

### Rendering Order / 渲染顺序

1. **Shadow layers** / 阴影层 (multiple RoundedRectCmd with decreasing alpha)
2. **Border** / 边框 (if enabled)
3. **Background** / 背景 (if enabled)
4. **Content** / 内容 (child components)

### Integration / 集成

- **Universal** / 通用性: Works with any Widget-derived component
- **Chainable** / 可链式调用: Integrates with existing decoration APIs
- **Theme aware** / 主题感知: Respects component opacity and theme changes
- **No breaking changes** / 无破坏性变更: Extends existing DecoratedBox functionality

## Browser Compatibility / 浏览器兼容性

This feature is implemented purely in the C++/Qt backend and has no browser dependencies.
此功能完全在C++/Qt后端实现，无浏览器依赖。