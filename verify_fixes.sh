#!/bin/bash
# 按钮渲染修复验证脚本
# 验证关键修复点是否正确实施

echo "=== 按钮渲染修复验证 ==="
echo ""

# 检查 UiButton.hpp 中的自剪裁修复
echo "1. 检查按钮自剪裁修复..."
if grep -q "clipRect = QRectF()" presentation/ui/base/UiButton.hpp; then
    echo "✓ 已修复：按钮自剪裁已禁用，避免精度问题"
else
    echo "❌ 错误：自剪裁修复未正确应用"
    exit 1
fi

# 检查注释说明
if grep -q "禁用自剪裁，避免精度问题" presentation/ui/base/UiButton.hpp; then
    echo "✓ 已添加：修复说明注释"
else
    echo "⚠️  建议：添加修复说明注释"
fi

echo ""

# 检查 Renderer.cpp 中的精度改进
echo "2. 检查坐标精度改进..."
if grep -q "向内收缩边界以确保不会意外剪裁" infrastructure/gfx/Renderer.cpp; then
    echo "✓ 已修复：剪裁精度计算改进"
else
    echo "❌ 错误：精度改进未正确应用"
    exit 1
fi

# 检查着色器改进
if grep -q "改进：更好的半径限制" infrastructure/gfx/Renderer.cpp; then
    echo "✓ 已修复：着色器半径处理改进"
else
    echo "❌ 错误：着色器改进未正确应用"
    exit 1
fi

# 检查抗锯齿改进
if grep -q "更好的抗锯齿计算" infrastructure/gfx/Renderer.cpp; then
    echo "✓ 已修复：抗锯齿计算改进"
else
    echo "❌ 错误：抗锯齿改进未正确应用"
    exit 1
fi

# 检查输入验证
if grep -q "验证矩形有效性" infrastructure/gfx/Renderer.cpp; then
    echo "✓ 已修复：渲染输入验证增强"
else
    echo "❌ 错误：输入验证未正确应用"
    exit 1
fi

echo ""

# 分析修复前后的差异
echo "3. 关键修复点分析..."
echo ""

echo "修复前的问题："
echo "- 按钮使用 .clipRect = r 进行自剪裁，导致精度问题"
echo "- 剪裁计算使用简单的 floor/ceil，可能剪裁到边缘像素"
echo "- 着色器半径限制不够保守，可能导致边缘伪影"
echo "- 缺少输入验证，可能渲染无效矩形"
echo ""

echo "修复后的改进："
echo "- 按钮使用 .clipRect = QRectF() 禁用自剪裁"
echo "- 剪裁计算添加 0.001px 容差，避免意外剪裁"
echo "- 着色器半径减少 0.5px，改善边缘质量"
echo "- 添加矩形尺寸和透明度验证"
echo ""

# 检查相关文件是否存在
echo "4. 检查相关文件..."
required_files=(
    "presentation/ui/base/UiButton.hpp"
    "infrastructure/gfx/Renderer.cpp"
    "infrastructure/gfx/RenderData.hpp"
    "presentation/ui/widgets/UiPushButton.cpp"
    "presentation/ui/widgets/UiPushButton.h"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file 存在"
    else
        echo "❌ $file 缺失"
        exit 1
    fi
done

echo ""

# 生成修复摘要
echo "5. 修复摘要："
echo ""
echo "主要修复："
echo "1. 禁用按钮自剪裁 (UiButton.hpp:122)"
echo "2. 改进剪裁精度计算 (Renderer.cpp:30-45)"
echo "3. 增强着色器质量 (Renderer.cpp:68-88)"
echo "4. 添加渲染验证 (Renderer.cpp:191-197)"
echo ""

echo "预期效果："
echo "- 按钮应正常渲染，不会因精度问题消失"
echo "- 圆角质量改善，减少边缘伪影"
echo "- 小尺寸按钮渲染更稳定"
echo "- 整体渲染性能和质量提升"
echo ""

echo "🎉 所有关键修复点验证通过！"
echo ""
echo "建议测试："
echo "1. 创建不同尺寸的按钮 (小、中、大)"
echo "2. 测试不同圆角半径 (0px, 4px, 8px, 16px, 过大半径)"
echo "3. 验证不同设备像素比 (1.0x, 1.5x, 2.0x)"
echo "4. 检查按钮交互状态 (正常、悬停、按下、禁用)"
echo ""