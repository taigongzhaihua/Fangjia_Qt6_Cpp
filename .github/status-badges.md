# GitHub Actions 状态标识配置

本文档描述如何在项目 README 中添加 GitHub Actions 状态标识。

## 状态标识

在项目的主 README.md 文件中添加以下标识：

### 基础 CI 状态
```markdown
![CI Status](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Basic%20CI%20Build/badge.svg)
```

### 综合 CI 状态
```markdown
![Comprehensive CI](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Comprehensive%20CI%2FCD%20Pipeline/badge.svg)
```

### AI PR 验证状态
```markdown
![AI PR Validation](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/AI%20PR%20Validation/badge.svg)
```

### 完整状态标识组合
```markdown
[![CI Status](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Basic%20CI%20Build/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/ci.yml)
[![Comprehensive CI](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Comprehensive%20CI%2FCD%20Pipeline/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/comprehensive-ci.yml)
[![AI PR Validation](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/AI%20PR%20Validation/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/ai-pr-validation.yml)
```

## 使用示例

可以在 README.md 的开头部分添加：

```markdown
# Fangjia Qt6 C++ 房价计算器

[![CI Status](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Basic%20CI%20Build/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/ci.yml)
[![Comprehensive CI](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/workflows/Comprehensive%20CI%2FCD%20Pipeline/badge.svg)](https://github.com/taigongzhaihua/Fangjia_Qt6_Cpp/actions/workflows/comprehensive-ci.yml)

一个基于 Qt6 和 C++ 的现代化房价计算器应用程序。

## 特性

- 🔧 自动化 CI/CD 流水线
- 🤖 AI PR 智能验证
- 🛠️ 自动构建错误修复
- 📊 综合质量检查
```

## 状态标识说明

- **绿色 (passing)**: 所有检查通过
- **红色 (failing)**: 某些检查失败
- **黄色 (pending)**: 检查正在进行中
- **灰色 (no status)**: 尚未运行或跳过

## 自定义标识

你也可以创建自定义状态标识：

```markdown
![Build](https://img.shields.io/github/actions/workflow/status/taigongzhaihua/Fangjia_Qt6_Cpp/ci.yml?label=Build)
![Tests](https://img.shields.io/github/actions/workflow/status/taigongzhaihua/Fangjia_Qt6_Cpp/comprehensive-ci.yml?label=Tests)
![Version](https://img.shields.io/github/v/tag/taigongzhaihua/Fangjia_Qt6_Cpp?label=Version)
```