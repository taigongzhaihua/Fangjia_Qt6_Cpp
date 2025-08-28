# 贡献指南（Contributing）

感谢你对 Fangjia 的贡献！本项目采用现代 C++/Qt6 + 自绘 UI 架构，欢迎通过 Issue/PR 参与改进。

开发环境
- Qt 6.5+（Core/Gui/Widgets/OpenGL/Svg）
- CMake 3.16+
- C++23 编译器（MSVC/Clang/GCC 任一）

构建步骤
```bash
# Windows (MSVC)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug

# Linux/macOS (Ninja)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

代码风格（建议）
- 头文件优先使用前向声明；尽量减少包含面。
- 使用 enum class、std::unique_ptr/std::shared_ptr；
- 避免宏（除平台宏）；
- 命名：类名驼峰，函数/变量小驼峰；常量/枚举使用清晰前缀；
- UI 相关：保持 IUiComponent 生命周期顺序（updateLayout → updateResourceContext → append → tick）。

分支与提交
- 分支：feature/xxx、fix/xxx、docs/xxx；
- 提交信息：类型: 简短描述（如 docs: 添加绑定指南）。
- PR 说明：包含动机/变更点/验证方式/影响范围。

评审清单（Reviewer）
- 是否遵守分层依赖（Features 不依赖彼此；不向上依赖）；
- VM 是否仅暴露状态与命令，持久化是否统一走 AppConfig；
- UI 是否通过绑定或增量属性更新，是否避免过度重建；
- 渲染路径是否遵循 IconCache/Renderer 约束（GL 在线程/上下文中创建与释放）。

CI/工具
- 里程碑 J 暂未启用 CI。可本地使用 clang-tidy/clang-format 辅助检查（可选）。

问题反馈
- 请在 Issue 中给出复现步骤、期望与实际行为、平台/Qt 版本信息。