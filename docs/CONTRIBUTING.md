# 贡献指南 / Contributing Guide

欢迎为 Fangjia Qt6 C++ 项目贡献代码！本文档介绍了代码风格、构建流程和质量门禁规则。

## 代码风格与格式化

### 自动格式化

项目使用 **clang-format** 统一代码风格：

```bash
# 格式化所有源文件（需要 clang-format）
cmake --build build --target format

# 检查代码格式是否符合规范
cmake --build build --target format-check
```

### 代码风格规范

- **基础风格**: 基于 LLVM，适配项目习惯
- **缩进**: 4 个空格（不使用 Tab）
- **行宽**: 120 列
- **指针/引用**: 左对齐 (`int* ptr`, `int& ref`)
- **Include 顺序**: 自动归组和排序

## 静态代码检查

### clang-tidy

项目启用保守但实用的 clang-tidy 规则集：

- `bugprone-*`: 检测潜在错误
- `performance-*`: 性能相关建议
- `modernize-*`: C++ 现代化建议（排除侵入性命名规则）
- `readability-*`: 可读性检查
- `cppcoreguidelines-*`: 有限的核心指南子集

### 本地运行静态检查

```bash
# 构建时启用 clang-tidy（需要安装 clang-tidy）
cmake -B build -G Ninja -DENABLE_CLANG_TIDY=ON
cmake --build build
```

### 抑制误报

可以在代码中使用注释抑制特定规则：

```cpp
// 抑制下一行的所有检查
// NOLINTNEXTLINE
auto result = some_legacy_function();

// 抑制当前行的特定规则
auto ptr = (int*)malloc(size);  // NOLINT(cppcoreguidelines-no-malloc)

// 抑制下一行的特定规则
// NOLINTNEXTLINE(modernize-use-auto)
std::vector<int> vec;
```

## 构建与测试

### 本地开发构建

```bash
# 基础构建（警告不作为错误，更友好）
cmake -B build -G Ninja
cmake --build build

# 运行测试
cd build && ctest --output-on-failure
```

### CI 模式构建

```bash
# 严格模式（警告作为错误，启用静态检查）
cmake -B build -G Ninja \
  -DFANGJIA_WARNINGS_AS_ERRORS=ON \
  -DENABLE_CLANG_TIDY=ON
cmake --build build
```

### 依赖安装

#### Ubuntu/Debian

```bash
# 基础依赖
sudo apt-get install cmake ninja-build qt6-base-dev qt6-svg-dev

# 静态检查工具
sudo apt-get install clang-tidy clang-format

# 测试环境
sudo apt-get install libgl1-mesa-dev
export QT_QPA_PLATFORM=offscreen  # 无头环境测试
```

#### Windows

```bash
# 使用 Chocolatey
choco install cmake ninja qt6

# 或使用 vcpkg
vcpkg install qt6-base qt6-svg
```

## CI 质量门禁

### 自动检查项目

GitHub Actions CI 会自动执行：

1. **跨平台构建**: Ubuntu + Windows
2. **编译警告检查**: 警告视为错误
3. **静态代码分析**: Linux 上运行 clang-tidy
4. **单元测试**: 所有平台
5. **代码格式检查**: Linux 上验证 clang-format

### 门禁规则

- ✅ **所有平台构建成功**
- ✅ **无编译警告** （`-Werror` / `/WX`）
- ✅ **静态检查通过** （Linux 上的 clang-tidy）
- ✅ **测试全部通过**
- ✅ **代码格式规范** （Linux 上的 clang-format）

### 失败处理

如果 CI 失败：

1. **构建错误**: 检查编译错误日志
2. **警告错误**: 修复或合理抑制警告
3. **静态检查**: 修复或使用 `NOLINT` 抑制误报
4. **测试失败**: 查看 test-results 构件中的详细日志
5. **格式错误**: 运行 `cmake --build build --target format`

## 开发工作流

### 提交前检查清单

- [ ] 代码已格式化 (`format` 目标)
- [ ] 本地构建成功
- [ ] 测试通过
- [ ] 新功能有对应测试
- [ ] 遵循现有代码结构

### Pull Request

1. 创建功能分支
2. 小量多次提交，清晰的提交信息
3. 确保 CI 通过
4. 描述清楚变更内容和测试情况

## 工具配置

### VS Code

推荐安装扩展：

- **C/C++**: Microsoft
- **CMake Tools**: Microsoft  
- **clang-format**: xaver
- **Clang-Tidy**: notskm

### CLion/Qt Creator

IDE 通常内置 CMake 和 clang-format 支持，配置项目根目录的 `.clang-format` 即可。

## 常见问题

### Q: 本地 clang-tidy 检查过于严格怎么办？

A: 本地开发时可以不启用 `ENABLE_CLANG_TIDY`，只在 CI 上运行。或针对特定代码使用 `NOLINT` 抑制。

### Q: 格式化改动了很多文件怎么办？

A: 建议在单独的提交中进行格式化，不要与功能修改混合。

### Q: Windows 上如何安装 clang-format？

A: 可以通过 LLVM 官方安装包，或使用 `choco install llvm`。

### Q: 如何添加新的警告规则？

A: 修改 `CMakeLists.txt` 中的 `FANGJIA_WARNING_FLAGS` 或 `.clang-tidy` 配置文件。

---

感谢您的贡献！如有问题请在 GitHub Issues 中讨论。