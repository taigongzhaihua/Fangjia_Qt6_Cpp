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

### 使用 Qt Creator
1. 在 Qt Creator 中打开 `CMakeLists.txt`
2. 使用 Qt 6 工具包配置项目
3. 构建 → 构建全部

### 命令行构建

```cmd
# 打开 Qt 开发者命令提示符
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019"
cmake --build build --config Debug

# 发布版本构建
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019"  
cmake --build build --config Release
```

## 运行应用程序

构建成功后：
```cmd
# 调试版本
.\build\apps\fangjia\Debug\FangJia.exe

# 发布版本  
.\build\apps\fangjia\Release\FangJia.exe
```

## 常见问题

### 找不到 Qt
- 确保 `Qt6_DIR` 环境变量指向 Qt 安装目录
- 检查 Qt bin 目录是否在 PATH 中
- 验证 Qt 版本兼容性 (6.5+)

### Visual Studio 错误
- 安装最新的 Visual Studio C++ 构建工具
- 确保安装了 Windows SDK
- 检查 CMake 生成器是否与 Visual Studio 版本匹配

### OpenGL 问题
- 更新显卡驱动程序
- 验证 OpenGL 3.3+ 支持
- 检查 Windows 显示设置中的硬件加速

## 平台特定功能

### Windows Chrome 集成
应用程序包含 Windows 10/11 的自定义窗口装饰：
- 原生窗口装饰
- DPI 感知
- 主题集成
- 流畅的窗口动画

实现细节请参阅 [Windows 平台集成](../infrastructure/platform-windows.md)。