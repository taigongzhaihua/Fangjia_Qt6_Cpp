# GitHub Actions CI/CD 使用指南

本指南介绍如何使用已实施的 GitHub Actions CI/CD 系统来验证项目编译运行和自动修复错误。

## 系统概览

我们已经实施了一套完整的 GitHub Actions 工作流系统，包含：

### 🔄 自动触发的工作流

1. **基础 CI** - 每次 push 和 PR 时自动运行
2. **综合 CI/CD** - 重要分支的综合验证
3. **AI PR 验证** - 自动检测并验证 AI 生成的 PR

### ⚡ 手动触发的工作流

1. **综合验证** - 完整的多平台测试
2. **自动修复** - 检测并修复构建问题

## 快速开始

### 1. 检查工作流状态

访问仓库的 Actions 标签页查看工作流运行状态：
```
https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions
```

### 2. 本地验证

在推送代码前，使用本地验证脚本：

```bash
# 完整验证
./scripts/validate-ci.sh

# 仅构建测试
./scripts/validate-ci.sh --build-only

# 清理构建文件
./scripts/validate-ci.sh --clean

# 查看帮助
./scripts/validate-ci.sh --help
```

### 3. 添加状态标识

在 README.md 中添加构建状态标识：

```markdown
[![CI Status](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Basic%20CI%20Build/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/ci.yml)
[![Comprehensive CI](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Comprehensive%20CI%2FCD%20Pipeline/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/comprehensive-ci.yml)
```

## 使用场景

### 🚀 日常开发

**推送代码时：**
- 基础 CI 会自动运行，验证构建和测试
- 如果失败，检查 Actions 日志查看详细错误信息
- 使用本地验证脚本在推送前验证

**创建 PR 时：**
- AI PR 验证会自动检测 PR 类型
- 提供详细的验证反馈和建议
- 自动评估代码质量

### 🔧 问题修复

**当构建失败时：**

1. **查看失败日志**
   ```bash
   # 访问 GitHub Actions 页面查看详细错误
   ```

2. **触发自动修复**
   - 前往 Actions → "Automatic Build Fix"
   - 点击 "Run workflow"
   - 选择修复类型：`build_errors`, `test_failures`, `dependency_issues`, `all`
   - 启用 "Create PR with fixes"

3. **手动修复**
   ```bash
   # 下载失败的构建日志
   # 本地复现问题
   cmake -B build -S .
   cmake --build build
   ```

### 🤖 AI 协作

**对于 AI 生成的 PR：**

- 系统会自动检测 AI PR（基于用户名、标题、内容）
- 执行增强验证，包括：
  - 快速构建检查
  - 代码质量分析
  - 安全检查
  - 自动反馈生成

**AI PR 最佳实践：**
- 在 PR 描述中标明 "AI-generated"
- 使用描述性的 PR 标题
- 确保代码遵循项目规范

## 工作流详解

### 基础 CI (`ci.yml`)

**触发条件：**
- Push 到 master/main 分支
- 所有 PR 的创建和更新

**执行内容：**
- 智能变更检测（跳过不相关更改）
- Windows 和 Linux 平台构建
- 测试执行和验证
- 可执行文件检查

### 综合 CI/CD (`comprehensive-ci.yml`)

**触发条件：**
- Push 到主要分支
- 手动触发
- PR（可选）

**执行内容：**
- 多平台构建矩阵（Windows/Linux/macOS）
- Debug 和 Release 配置
- 安全分析和依赖检查
- 性能测试
- 自动修复 PR 创建

### AI PR 验证 (`ai-pr-validation.yml`)

**触发条件：**
- PR 创建/更新
- 手动触发

**特殊功能：**
- AI PR 自动检测
- 增强质量检查
- 详细反馈生成
- 代码规范验证

### 自动修复 (`auto-fix.yml`)

**触发条件：**
- 手动触发
- 每日定时执行 (02:00 UTC)

**修复类型：**
- 构建配置问题
- 依赖路径问题
- C++ 标准兼容性
- 测试环境配置

## 故障排除

### 常见问题

#### 1. Qt6 找不到
```bash
# 自动修复会添加这些路径
export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6
# 或
export Qt6_DIR=/opt/Qt/6.5.0/gcc_64/lib/cmake/Qt6
```

#### 2. 构建超时
```bash
# 检查是否有死循环或资源不足
# 优化并行构建设置
cmake --build build --parallel 2  # 减少并行数
```

#### 3. 测试失败
```bash
# 确保正确的测试环境
export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.gl=false"
xvfb-run -a ./build/tests/FangJia_Tests
```

### 调试技巧

#### 1. 下载构建产物
- 在 Actions 运行页面下载 artifacts
- 包含可执行文件、日志、测试结果

#### 2. 启用调试模式
```yaml
# 在工作流中添加
- name: Debug Info
  run: |
    echo "Debug information"
    env
    find . -name "*.log"
```

#### 3. 本地复现
```bash
# 使用相同的环境变量和命令
export QT_QPA_PLATFORM=offscreen
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## 高级配置

### 自定义工作流

#### 修改构建矩阵
```yaml
strategy:
  matrix:
    include:
      - os: ubuntu-22.04
        qt-version: '6.5.0'
      - os: windows-2022
        qt-version: '6.6.3'
```

#### 添加新的修复类型
在 `auto-fix.yml` 中添加新的修复逻辑：

```yaml
- name: Fix new issue type
  run: |
    echo "Fixing new issue..."
    # 添加修复逻辑
```

### 集成外部工具

#### 添加代码覆盖率
```yaml
- name: Code Coverage
  run: |
    # 安装 gcov/lcov
    # 运行带覆盖率的测试
    # 上传到 codecov
```

#### 添加静态分析
```yaml
- name: Static Analysis
  run: |
    # 运行 clang-tidy, cppcheck 等
```

## 维护和监控

### 定期检查

1. **每周检查工作流状态**
2. **更新依赖版本**（Qt, CMake）
3. **清理旧的构建产物**
4. **审查自动修复 PR**

### 性能优化

1. **缓存优化**
2. **并行构建调优**
3. **测试时间监控**
4. **构建矩阵优化**

## 支持和贡献

如需添加新功能或修复问题：

1. 查看 `.github/README.md` 了解架构
2. 修改相应的工作流文件
3. 使用本地验证脚本测试
4. 创建 PR 并等待 AI 验证

---

*此系统旨在提高开发效率，确保代码质量，并支持 AI 协作开发。*