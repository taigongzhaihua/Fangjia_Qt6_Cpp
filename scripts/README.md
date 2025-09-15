# 构建脚本 / Build Scripts

本目录包含用于简化 Fangjia Qt6 C++ 项目构建过程的脚本。

This directory contains scripts to simplify the build process for the Fangjia Qt6 C++ project.

## Visual Studio 解决方案生成 / Visual Studio Solution Generation

### PowerShell 脚本 (推荐 / Recommended)

**文件:** `generate_vs_solution.ps1`

**用法 / Usage:**
```powershell
# 生成 Visual Studio 2022 解决方案 / Generate VS 2022 solution
.\generate_vs_solution.ps1

# 生成 Visual Studio 2019 解决方案 / Generate VS 2019 solution
.\generate_vs_solution.ps1 -VSVersion 2019

# 清理并重新生成 / Clean and regenerate
.\generate_vs_solution.ps1 -Clean

# 指定架构 / Specify architecture
.\generate_vs_solution.ps1 -Architecture x64
```

**功能 / Features:**
- 自动检测前置条件 / Automatic prerequisite checking
- 支持 Visual Studio 2019/2022 / Support for Visual Studio 2019/2022
- 清理选项 / Clean option
- 自动打开解决方案选项 / Auto-open solution option

### 批处理脚本 / Batch Script

**文件:** `generate_vs_solution.bat`

**用法 / Usage:**
```cmd
# 生成 Visual Studio 2022 解决方案 / Generate VS 2022 solution
generate_vs_solution.bat

# 生成 Visual Studio 2019 解决方案 / Generate VS 2019 solution
generate_vs_solution.bat 2019
```

**功能 / Features:**
- 简单易用的命令行界面 / Simple command-line interface
- 自动前置条件检查 / Automatic prerequisite checking
- 支持 Visual Studio 2019/2022 / Support for Visual Studio 2019/2022

## 前置要求 / Prerequisites

使用这些脚本之前，请确保已安装：
Before using these scripts, ensure you have installed:

1. **Visual Studio 2019 或 2022** 包含 C++ 组件 / with C++ components
2. **Qt 6.5+** 支持 MSVC 编译器 / with MSVC compiler support
3. **CMake 3.16+**
4. 正确配置的环境变量 / Properly configured environment variables:
   - `Qt6_DIR` (指向 Qt CMake 目录 / pointing to Qt CMake directory)
   - `PATH` (包含 Qt bin 目录 / including Qt bin directory)

## 故障排除 / Troubleshooting

如果脚本失败，请检查：
If the scripts fail, please check:

1. 所有前置要求都已满足 / All prerequisites are met
2. 环境变量正确设置 / Environment variables are correctly set
3. Visual Studio 版本与所选生成器匹配 / VS version matches selected generator

更多信息请参阅：[Windows 构建指南](../doc.zh-cn/build/windows.md)
For more information, see: [Windows Build Guide](../doc.zh-cn/build/windows.md)