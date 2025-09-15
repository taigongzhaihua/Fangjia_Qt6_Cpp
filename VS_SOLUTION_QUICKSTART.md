# Visual Studio 解决方案快速指南
# Visual Studio Solution Quick Guide

## 快速开始 / Quick Start

### 方法 1: 自动生成脚本 (推荐)
```powershell
# PowerShell
.\scripts\generate_vs_solution.ps1

# 命令提示符
.\scripts\generate_vs_solution.bat
```

### 方法 2: Visual Studio 直接打开
1. 打开 Visual Studio 2019/2022
2. 文件 → 打开 → 文件夹
3. 选择项目根目录
4. 选择配置 (如 VS2022-x64-Debug)
5. 生成 → 全部生成

### 方法 3: 手动生成
```cmd
# Visual Studio 2022
cmake -S . -B build_vs2022 -G "Visual Studio 17 2022" -A x64
start build_vs2022\Fangjia_Qt6_Cpp.sln

# Visual Studio 2019  
cmake -S . -B build_vs2019 -G "Visual Studio 16 2019" -A x64
start build_vs2019\Fangjia_Qt6_Cpp.sln
```

## 前置要求 / Prerequisites

✅ Visual Studio 2019/2022 + C++ 工具  
✅ Qt 6.5+ (MSVC 版本)  
✅ CMake 3.16+  
✅ 环境变量: Qt6_DIR, PATH  

## 可用配置 / Available Configurations

- `x64-Debug` (Ninja)
- `x64-Release` (Ninja) 
- `VS2019-x64-Debug` (Visual Studio 2019)
- `VS2019-x64-Release` (Visual Studio 2019)
- `VS2022-x64-Debug` (Visual Studio 2022)
- `VS2022-x64-Release` (Visual Studio 2022)

## 故障排除 / Troubleshooting

| 问题 | 解决方案 |
|------|----------|
| 找不到 Qt | 设置 `Qt6_DIR` 环境变量 |
| CMake 错误 | 检查 Visual Studio 版本匹配 |
| 链接错误 | 确保 Qt 库路径正确 |
| 运行时缺少 DLL | 添加 Qt bin 目录到 PATH |

详细文档: [doc.zh-cn/build/windows.md](doc.zh-cn/build/windows.md)