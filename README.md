# Fangjia Qt6 C++

一个基于 Qt 6 + OpenGL 的自绘 UI 应用示例工程，包含：
- 自定义渲染管线（Renderer/RenderData/IconCache）
- UI 组件与容器（UiRoot/UiPage/UiPanel/UiGrid/UiTabView/UiNav/UiTopBar 等）
- 主题与配置（ThemeManager/AppConfig）
- 示例业务页面（Home/Data/Explore/Favorites/Settings）

本仓库的中文文档集中在 docs/ 目录，建议从《项目结构说明》《开发指南》《设计架构》开始阅读：
- docs/项目结构说明.md
- docs/开发指南.md
- docs/设计架构.md
- docs/渲染与UI体系.md
- docs/配置与主题.md
- docs/编码规范.md
- docs/贡献指南.md

## 快速开始
1. 准备环境：Qt 6（Core/Gui/Widgets/OpenGL/Svg），CMake ≥ 3.16，C++23 编译器。
2. 配置与构建：
```bash
# Linux/macOS（可使用 scripts/dev/build_debug.sh）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Windows PowerShell（可使用 scripts/dev/build.ps1）
# 先在 Qt 命令行/开发者命令提示符中执行
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```
3. 运行可执行程序（Windows 平台目标名：FangJia）。

## 目录总览
```
├─ src/                 # 源码（核心渲染、UI框架、业务视图、应用）
├─ resources/           # Qt 资源（icons 等）
├─ tests/               # 测试（如有）
├─ examples/            # 示例（如有）
├─ docs/                # 全中文文档（本PR新增）
├─ scripts/             # 开发辅助脚本（本PR新增 scripts/dev/*）
├─ CMakeLists.txt       # 顶层构建脚本
└─ README.md            # 工程说明（指向 docs/）
```

## 许可证
根据项目实际情况补充（例如 MIT/Apache-2.0 等）。