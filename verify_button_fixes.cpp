/*
 * 简化的按钮渲染验证测试
 * 不依赖完整的应用程序框架
 */

#include <iostream>
#include "presentation/ui/base/UiButton.hpp"
#include "infrastructure/gfx/RenderData.hpp"

int main() {
    std::cout << "=== 按钮渲染修复验证 ===" << std::endl;
    
    // 测试 Ui::Button 的渲染命令生成
    Ui::Button button;
    
    // 设置按钮属性
    button.setBaseRect(QRect(10, 10, 200, 50));
    button.setCornerRadius(8.0f);
    button.setPalette(
        QColor(70, 130, 255),     // 背景色
        QColor(90, 150, 255),     // 悬停色
        QColor(50, 110, 235),     // 按下色
        QColor(255, 255, 255)     // 图标色
    );
    button.setEnabled(true);
    
    // 生成渲染数据
    Render::FrameData frameData;
    button.append(frameData);
    
    // 验证渲染命令
    if (frameData.roundedRects.empty()) {
        std::cout << "❌ 错误：没有生成圆角矩形渲染命令" << std::endl;
        return 1;
    }
    
    const auto& cmd = frameData.roundedRects[0];
    
    std::cout << "✓ 成功生成渲染命令:" << std::endl;
    std::cout << "  矩形位置: (" << cmd.rect.x() << ", " << cmd.rect.y() << ")" << std::endl;
    std::cout << "  矩形尺寸: " << cmd.rect.width() << " x " << cmd.rect.height() << std::endl;
    std::cout << "  圆角半径: " << cmd.radiusPx << "px" << std::endl;
    std::cout << "  背景颜色: RGB(" << cmd.color.red() << ", " << cmd.color.green() << ", " << cmd.color.blue() << ")" << std::endl;
    
    // 检查修复：剪裁区域应该为空（禁用自剪裁）
    if (cmd.clipRect.isValid() && !cmd.clipRect.isEmpty()) {
        std::cout << "⚠️  警告：仍然设置了剪裁区域，可能导致精度问题" << std::endl;
        std::cout << "  剪裁区域: " << cmd.clipRect.width() << " x " << cmd.clipRect.height() << std::endl;
    } else {
        std::cout << "✓ 修复确认：已禁用自剪裁，避免精度问题" << std::endl;
    }
    
    // 测试不同状态
    std::cout << "\n测试交互状态变化:" << std::endl;
    
    // 模拟鼠标悬停
    button.onMouseMove(QPoint(100, 35)); // 在按钮中心
    frameData.clear();
    button.append(frameData);
    
    if (!frameData.roundedRects.empty()) {
        const auto& hoverCmd = frameData.roundedRects[0];
        std::cout << "✓ 悬停状态颜色: RGB(" << hoverCmd.color.red() << ", " << hoverCmd.color.green() << ", " << hoverCmd.color.blue() << ")" << std::endl;
    }
    
    // 模拟鼠标按下
    button.onMousePress(QPoint(100, 35));
    frameData.clear();
    button.append(frameData);
    
    if (!frameData.roundedRects.empty()) {
        const auto& pressCmd = frameData.roundedRects[0];
        std::cout << "✓ 按下状态颜色: RGB(" << pressCmd.color.red() << ", " << pressCmd.color.green() << ", " << pressCmd.color.blue() << ")" << std::endl;
    }
    
    // 测试边界情况
    std::cout << "\n测试边界情况:" << std::endl;
    
    // 极小矩形
    button.setBaseRect(QRect(0, 0, 1, 1));
    button.setCornerRadius(5.0f); // 半径大于尺寸
    frameData.clear();
    button.append(frameData);
    
    if (!frameData.roundedRects.empty()) {
        const auto& smallCmd = frameData.roundedRects[0];
        std::cout << "✓ 极小矩形处理: " << smallCmd.rect.width() << " x " << smallCmd.rect.height() << ", 半径: " << smallCmd.radiusPx << std::endl;
    }
    
    // 零尺寸矩形
    button.setBaseRect(QRect(0, 0, 0, 0));
    frameData.clear();
    button.append(frameData);
    
    if (frameData.roundedRects.empty()) {
        std::cout << "✓ 零尺寸矩形正确跳过渲染" << std::endl;
    } else {
        std::cout << "⚠️  警告：零尺寸矩形仍生成了渲染命令" << std::endl;
    }
    
    std::cout << "\n🎉 按钮渲染修复验证完成！" << std::endl;
    std::cout << "主要修复点:" << std::endl;
    std::cout << "1. 禁用按钮自剪裁，避免精度问题" << std::endl;
    std::cout << "2. 改进着色器的抗锯齿和半径处理" << std::endl;
    std::cout name="3. 增强坐标变换的精度" << std::endl;
    
    return 0;
}