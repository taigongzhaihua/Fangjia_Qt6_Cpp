[English](../../doc/build/windows.md) | **简体中文**

# Windows 构建指南

## 前置要求

### 必需软件
- **Visual Studio 2019 或更高版本** 包含 C++ 组件
- **Qt 6.5 或更高版本** 包含以下组件：
  - Qt6 Core
  - Qt6 Gui  
  - Qt6 Widgets
  - Qt6 OpenGL
  - Qt6 Svg
- **CMake 3.16 或更高版本**

### Qt 安装

1. 从 [Qt 官方网站](https://www.qt.io/download) 下载 Qt 安装程序
2. 安装 Qt 6.5+ 并支持 MSVC 编译器
3. 将 Qt `bin` 目录添加到系统 PATH
4. 设置 `Qt6_DIR` 环境变量指向 Qt 安装目录

## 构建配置

### 使用 Visual Studio 解决方案 (推荐)

#### 方法 1: 使用 Visual Studio 2019/2022 直接打开
1. 启动 Visual Studio 2019 或 2022
2. 选择 "打开本地文件夹" 或 "Open a local folder"
3. 选择项目根目录（包含 `CMakeLists.txt` 的目录）
4. Visual Studio 会自动检测 CMake 项目并加载 `CMakeSettings.json` 配置
5. 在工具栏中选择所需的配置（如 `VS2022-x64-Debug`）
6. 构建 → 全部生成 (Ctrl+Shift+B)

#### 方法 2: 使用自动化脚本 (推荐)
项目提供了便捷的脚本来生成 Visual Studio 解决方案：

**PowerShell 脚本 (推荐):**
```powershell
# 生成 Visual Studio 2022 解决方案
.\scripts\generate_vs_solution.ps1

# 生成 Visual Studio 2019 解决方案
.\scripts\generate_vs_solution.ps1 -VSVersion 2019

# 清理并重新生成
.\scripts\generate_vs_solution.ps1 -Clean
```

**批处理脚本:**
```cmd
# 生成 Visual Studio 2022 解决方案
.\scripts\generate_vs_solution.bat

# 生成 Visual Studio 2019 解决方案  
.\scripts\generate_vs_solution.bat 2019
```

#### 方法 3: 生成 .sln 解决方案文件
```cmd
# Visual Studio 2019
cmake -S . -B build_vs2019 -G "Visual Studio 16 2019" -A x64
start build_vs2019\Fangjia_Qt6_Cpp.sln

# Visual Studio 2022  
cmake -S . -B build_vs2022 -G "Visual Studio 17 2022" -A x64
start build_vs2022\Fangjia_Qt6_Cpp.sln
```

#### 方法 4: 命令行构建（使用 VS 生成器）
```cmd
# Visual Studio 2019 - 调试版本
cmake -S . -B build_vs2019 -G "Visual Studio 16 2019" -A x64
cmake --build build_vs2019 --config Debug

# Visual Studio 2019 - 发布版本
cmake -S . -B build_vs2019 -G "Visual Studio 16 2019" -A x64
cmake --build build_vs2019 --config Release

# Visual Studio 2022 - 调试版本
cmake -S . -B build_vs2022 -G "Visual Studio 17 2022" -A x64
cmake --build build_vs2022 --config Debug

# Visual Studio 2022 - 发布版本
cmake -S . -B build_vs2022 -G "Visual Studio 17 2022" -A x64
cmake --build build_vs2022 --config Release
```

### 使用 Qt Creator
1. 在 Qt Creator 中打开 `CMakeLists.txt`
2. 使用 Qt 6 工具包配置项目
3. 构建 → 构建全部

### 使用 Ninja 构建 (高性能)
```cmd
# 打开 Qt 开发者命令提示符
cmake -S . -B build_ninja -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_ninja

# 发布版本构建
cmake -S . -B build_ninja -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build_ninja
```

## 运行应用程序

### 从 Visual Studio 运行
1. 在 Visual Studio 中设置 `FangJia` 为启动项目
2. 按 F5 开始调试，或 Ctrl+F5 开始执行（不调试）

### 从命令行运行
构建成功后：

```cmd
# Visual Studio 构建 - 调试版本
.\build_vs2019\apps\fangjia\Debug\FangJia.exe
# 或
.\build_vs2022\apps\fangjia\Debug\FangJia.exe

# Visual Studio 构建 - 发布版本  
.\build_vs2019\apps\fangjia\Release\FangJia.exe
# 或
.\build_vs2022\apps\fangjia\Release\FangJia.exe

# Ninja 构建
.\build_ninja\apps\fangjia\FangJia.exe
```

### 调试配置
在 Visual Studio 中调试时，确保：
1. 工作目录设置正确（通常是项目根目录）
2. 环境变量 `PATH` 包含 Qt 的 bin 目录
3. 如需要，在项目属性中设置额外的库路径

## 常见问题

### 找不到 Qt
- 确保 `Qt6_DIR` 环境变量指向 Qt 安装目录
  ```cmd
  set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
  ```
- 检查 Qt bin 目录是否在 PATH 中
  ```cmd
  set PATH=%PATH%;C:\Qt\6.5.0\msvc2019_64\bin
  ```
- 验证 Qt 版本兼容性 (6.5+)

### Visual Studio 解决方案问题

#### 无法生成解决方案文件
- 确保 Visual Studio 版本与 CMake 生成器匹配：
  - Visual Studio 2019: 使用 `"Visual Studio 16 2019"`
  - Visual Studio 2022: 使用 `"Visual Studio 17 2022"`
- 检查是否安装了所需的 Visual Studio 组件：
  - MSVC v142/v143 编译器工具集
  - Windows 10/11 SDK
  - CMake 工具

#### Visual Studio 中无法识别项目
- 确保 Visual Studio 安装了 "用于 Visual Studio 的 CMake 工具"
- 重新生成 CMake 缓存：删除 `out` 文件夹后重新配置
- 检查 CMakeSettings.json 格式是否正确

#### 编译错误
- 确保使用正确的平台架构 (x64)
- 检查 C++ 标准支持：项目使用 C++23，需要最新的编译器
- 验证 MSVC 工具集版本：
  ```cmd
  # 在 Visual Studio 开发者命令提示符中
  cl.exe
  ```

#### 链接错误
- 确保 Qt 库路径正确配置
- 检查是否缺少必需的 Windows 库（如 Dwmapi.lib）
- 验证所有依赖项都已正确构建

### CMake 配置问题
- 清理构建目录重新配置：
  ```cmd
  rmdir /s build_vs2019
  cmake -S . -B build_vs2019 -G "Visual Studio 16 2019" -A x64
  ```
- 检查 CMake 输出中的警告和错误信息
- 确保 boost-ext/di 依赖项正确下载

### 运行时问题
- 缺少 DLL 文件：确保 Qt DLL 在 PATH 中或复制到执行目录
- OpenGL 错误：更新显卡驱动程序，验证 OpenGL 3.3+ 支持
- 高 DPI 显示问题：在项目属性中启用 DPI 感知

## Visual Studio 集成功能

### CMake 支持
- 项目使用现代 CMake (3.16+) 配置
- 支持 Visual Studio 的 CMake 工具
- 包含预配置的 `CMakeSettings.json`
- 自动生成 IntelliSense 配置

### 调试和开发功能
- **热重载支持**: 项目配置支持 MSVC 的 Edit and Continue 功能
- **C++23 标准**: 使用最新 C++ 特性和标准库
- **Qt 集成**: 自动 MOC、UIC、RCC 处理
- **多配置支持**: Debug、Release、RelWithDebInfo

### 项目结构
- 分层架构设计便于在 Visual Studio 中导航
- 清晰的模块划分 (Infrastructure, Domain, Data, Presentation)
- 独立的测试项目便于单元测试

### IntelliSense 和代码分析
- 通过 CMake 自动配置包含路径
- 支持 Qt 特定的代码提示
- 依赖注入库 (Boost.DI) 的智能提示

## 平台特定功能
### Windows Chrome 集成
应用程序包含 Windows 10/11 的自定义窗口装饰：
- DPI 感知
- 主题集成
- 流畅的窗口动画

实现细节请参阅 [Windows 平台集成](../infrastructure/platform-windows.md)。