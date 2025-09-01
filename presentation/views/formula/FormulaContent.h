/*
 * 文件名：FormulaContent.h
 * 职责：方剂内容的声明式UI组件，使用Grid布局管理树形列表和详情面板。
 * 依赖：UI::Widget、UI::Grid、UI::BindingHost、FormulaViewModel。
 * 线程：仅在UI线程使用。
 * 备注：采用声明式设计，分离UI与数据逻辑，支持外部ViewModel注入。
 */

#pragma once

#include "Widget.h"
#include <memory>

// 前向声明
class FormulaViewModel;
class UiTreeList;

namespace UI {
    class RebuildHost;
}

/// 方剂内容声明式组件：提供左侧树形列表和右侧详情面板的布局
/// 
/// 设计原则：
/// - 纯UI组件：不拥有数据，接受外部FormulaViewModel*作为数据源
/// - 声明式布局：使用UI::Grid进行三列布局（树列表+分隔线+详情面板）
/// - 响应式更新：通过UI::BindingHost自动响应ViewModel信号变化
/// - 主题适配：支持动态主题切换，组件自动适配颜色方案
/// 
/// 布局结构：
/// - 左侧(35%)：UiTreeList绑定到FormulaViewModel的树形数据
/// - 中间(1px)：半透明分隔线，提供视觉区域划分
/// - 右侧(65%)：ScrollView包装的详情面板，响应选中项变化
class FormulaContent : public UI::Widget {
public:
    /// 构造函数：接受外部FormulaViewModel指针
    /// 参数：vm — 方剂数据视图模型指针（非拥有，生命周期由调用方管理）
    /// 说明：FormulaContent不会删除或管理vm的生命周期
    explicit FormulaContent(FormulaViewModel* vm);

    /// 析构函数：清理内部UI组件
    ~FormulaContent() override = default;

    /// 构建UI组件树
    /// 返回：完整的UI组件层次结构
    /// 说明：创建Grid布局，配置左树+分隔线+右详情的三列结构
    std::unique_ptr<IUiComponent> build() const override;

private:
    /// 创建左侧树形列表组件
    /// 返回：配置好ModelFns绑定的UiTreeList组件指针
    std::unique_ptr<UiTreeList> createTreeList() const;

    /// 创建右侧详情面板内容
    /// 返回：包含方剂详情的UI组件树
    /// 说明：使用BindingHost监听ViewModel信号，自动重建内容
    UI::WidgetPtr createDetailsPanel() const;

    /// 创建详情内容组件（用于BindingHost的Builder函数）
    /// 返回：根据当前选中方剂生成的详情UI
    UI::WidgetPtr buildDetailsContent() const;

private:
    FormulaViewModel* m_viewModel;   // 外部注入的数据模型（非拥有）
};