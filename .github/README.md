# GitHub Actions CI/CD 系统

本仓库包含一套完整的 GitHub Actions 工作流，用于验证项目编译运行状态、自动修复错误并生成修复 PR。

## 工作流概览

### 1. 基础 CI (`.github/workflows/ci.yml`)
**触发条件**: Push 到 master/main，PR 创建/更新
- 快速变更检测，避免不必要的构建
- Windows 和 Linux 平台构建验证
- 基础测试执行
- 可执行文件验证

### 2. 综合 CI/CD 流水线 (`.github/workflows/comprehensive-ci.yml`)
**触发条件**: Push 到主要分支，PR，手动触发
- 多平台构建矩阵 (Windows/Linux/macOS)
- Debug/Release 配置验证
- 安全分析和代码质量检查
- 性能测试
- 构建失败自动创建修复 PR

### 3. AI PR 验证 (`.github/workflows/ai-pr-validation.yml`)
**触发条件**: PR 创建/更新（特别针对 AI 生成的 PR）
- AI PR 自动检测
- 快速构建验证
- 综合质量检查
- 自动反馈生成
- AI 代码质量验证

### 4. 自动修复系统 (`.github/workflows/auto-fix.yml`)
**触发条件**: 手动触发，每日定时执行
- 构建问题检测分析
- 自动修复生成
- 修复验证测试
- 自动创建修复 PR

## 使用方法

### 基础使用
所有工作流都会在相关事件发生时自动触发。基础 CI 会在每次 push 和 PR 时运行。

### 手动触发综合验证
```bash
# 在 GitHub Actions 页面手动触发 "Comprehensive CI/CD Pipeline"
# 选择是否在构建失败时创建修复 PR
```

### 触发自动修复
```bash
# 在 GitHub Actions 页面手动触发 "Automatic Build Fix"
# 选择修复类型：build_errors, test_failures, dependency_issues, configuration_problems, all
```

### AI PR 验证
AI 生成的 PR 会自动被检测并进行更严格的验证。系统会：
1. 自动识别 AI 生成的 PR
2. 执行快速构建验证
3. 进行综合质量检查
4. 在 PR 中提供详细反馈

## 支持的平台和配置

### 构建平台
- **Windows**: Windows Server 2022, MSVC 2019, Qt 6.6.3
- **Linux**: Ubuntu Latest, GCC, Qt 6.4.2+
- **macOS**: macOS Latest, Clang, Homebrew Qt6

### 构建配置
- Debug 和 Release 配置
- C++20/23 标准支持
- CMake 3.16+
- Qt6 组件：Core, Gui, OpenGL, Widgets, Svg, Sql

## 特色功能

### 🚀 智能构建优化
- 变更检测，跳过不必要的构建
- 并行构建加速
- 构建缓存优化

### 🔧 自动错误修复
- CMake 配置问题自动修复
- 依赖路径自动配置
- C++ 标准兼容性修复
- 测试环境自动配置

### 🤖 AI PR 支持
- AI 生成 PR 自动识别
- 增强验证流程
- 代码质量检查
- 自动反馈生成

### 🛡️ 安全和质量
- 依赖安全扫描
- 代码质量分析
- 性能测试
- 运行时验证

## 工作流状态监控

### 查看构建状态
在仓库主页可以看到最新的构建状态标识。

### 下载构建产物
每次构建都会上传构建产物，包括：
- 可执行文件
- 测试结果
- 构建日志
- 错误分析报告

### 错误诊断
如果构建失败：
1. 检查 GitHub Actions 标签页的详细日志
2. 下载诊断日志进行本地分析
3. 如果启用，会自动创建修复 PR

## 配置选项

### 环境变量
```yaml
env:
  QT_VERSION: '6.6.3'        # Qt 版本
  BUILD_TYPE: Release        # 默认构建类型
```

### 触发条件自定义
可以通过修改 `on:` 部分来自定义触发条件。

### 平台矩阵自定义
在 `comprehensive-ci.yml` 中的 `strategy.matrix` 部分可以自定义构建平台和配置。

## 故障排除

### 常见问题

#### Qt6 找不到
自动修复系统会尝试添加常见的 Qt6 安装路径。如果仍有问题：
```cmake
set(Qt6_DIR "/your/qt6/path/lib/cmake/Qt6")
```

#### C++ 标准问题
如果 C++23 不支持，自动修复会降级到 C++20：
```cmake
set(CMAKE_CXX_STANDARD 20)
```

#### 测试在 CI 中失败
确保设置了正确的环境变量：
```bash
export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.gl=false"
```

### 手动修复
如果自动修复无法解决问题：
1. 查看生成的 `AUTO_FIX_REPORT.md`
2. 检查 CI 日志中的具体错误信息
3. 在本地复现问题
4. 手动应用修复并测试

## 贡献指南

### 添加新的构建平台
在 `comprehensive-ci.yml` 的矩阵中添加新的配置。

### 改进自动修复
在 `auto-fix.yml` 的修复生成步骤中添加新的修复逻辑。

### 增强 AI PR 验证
在 `ai-pr-validation.yml` 中添加更多的质量检查规则。

## 许可证
与主项目相同的许可证。