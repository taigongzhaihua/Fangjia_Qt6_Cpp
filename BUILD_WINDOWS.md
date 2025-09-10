# Windows 构建指南 (VS2022)

## 系统要求

- **操作系统**: Windows 10/11 (64位)
- **编译器**: Visual Studio 2022 (v143) 或更新版本
- **Qt版本**: Qt 6.4.0 或更新版本 (推荐 6.6+)
- **CMake版本**: 3.16 或更新版本

## 前置条件

### 1. 安装 Visual Studio 2022

确保安装了以下工作负载：
- **使用 C++ 的桌面开发**
- **CMake 工具**

### 2. 安装 Qt6

推荐通过 Qt 官方在线安装器安装：

1. 下载 [Qt 在线安装器](https://www.qt.io/download-qt-installer)
2. 安装时选择：
   - **Qt 6.8.0** (或最新版本)
   - **MSVC 2022 64-bit** 编译器版本
   - **Qt 5 Compatibility Module** (如需要)
   - **Additional Libraries**: Qt Svg, Qt OpenGL

常见安装路径：
```
C:\Qt\6.8.0\msvc2022_64
C:\Qt\6.7.3\msvc2022_64
C:\Qt\6.6.3\msvc2022_64
```

### 3. 验证安装

打开 **Developer Command Prompt for VS 2022**，运行：
```cmd
cmake --version
```

## 构建方法

### 方法一：使用 Visual Studio 2022 IDE

1. **打开项目**
   - 启动 Visual Studio 2022
   - 选择 "打开本地文件夹"
   - 导航到项目根目录并打开

2. **配置 CMake**
   - VS 会自动检测 CMakeLists.txt
   - 在顶部工具栏选择配置（建议 x64-Debug 或 x64-Debug-Qt6）
   - 如果 Qt 未自动检测，设置 CMAKE_PREFIX_PATH

3. **构建项目**
   - 选择 "生成" → "全部生成"
   - 或者使用快捷键 `Ctrl+Shift+B`

4. **运行程序**
   - 在解决方案资源管理器中右键 FangJia.exe
   - 选择 "设为启动项"
   - 按 F5 运行

### 方法二：使用命令行

1. **打开 Developer Command Prompt for VS 2022**

2. **导航到项目目录**
   ```cmd
   cd C:\path\to\Fangjia_Qt6_Cpp
   ```

3. **设置 Qt 路径** (如果未在系统PATH中)
   ```cmd
   set CMAKE_PREFIX_PATH=C:\Qt\6.8.0\msvc2022_64
   ```

4. **配置项目**
   ```cmd
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```

5. **构建项目**
   ```cmd
   cmake --build build --config Debug
   ```

6. **运行程序**
   ```cmd
   .\build\Debug\FangJia.exe
   ```

### 方法三：使用 Ninja (推荐用于CI/CD)

1. **打开 Developer Command Prompt for VS 2022**

2. **设置环境**
   ```cmd
   set CMAKE_PREFIX_PATH=C:\Qt\6.8.0\msvc2022_64
   ```

3. **配置和构建**
   ```cmd
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ```

## 常见问题及解决方案

### 问题1：找不到 Qt6
```
CMake Error: Could not find Qt6
```

**解决方案：**
- 确保 Qt6 已正确安装
- 设置 CMAKE_PREFIX_PATH 环境变量：
  ```cmd
  set CMAKE_PREFIX_PATH=C:\Qt\6.8.0\msvc2022_64
  ```
- 或在 CMake 配置时指定：
  ```cmd
  cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.8.0\msvc2022_64 ...
  ```

### 问题2：C++20 编译错误
```
error: This version of Visual Studio does not support C++20
```

**解决方案：**
- 更新到 Visual Studio 2022 17.0 或更新版本
- 项目已配置为使用 C++20，VS2022 完全支持

### 问题3：Windows.h 冲突
```
error: 'min' is not a member of 'std'
```

**解决方案：**
项目已配置 NOMINMAX 宏，如仍有问题，请检查：
- 确保在包含 Windows.h 之前定义了 NOMINMAX
- 使用 `std::min` 而不是 `min`

### 问题4：缺少 dwmapi.lib
```
LINK : fatal error LNK1104: cannot open file 'dwmapi.lib'
```

**解决方案：**
- 确保使用 Visual Studio 2022 的 MSVC 编译器
- 检查 Windows SDK 是否正确安装

### 问题5：编码问题
```
error: source file encoding not supported
```

**解决方案：**
项目已配置 UTF-8 编码支持。如仍有问题：
- 确保源文件保存为 UTF-8 编码
- 在 VS 中：文件 → 高级保存选项 → 编码：UTF-8

## 性能优化建议

### Debug 构建优化
- 使用 `/JMC` (Just My Code) 提高调试体验
- 启用 Edit and Continue 支持热重载

### Release 构建优化
- 自动启用 `/O2` 全速优化
- 使用 `/GL` 和 `/LTCG` 进行链接时优化
- 生成 PDB 文件用于崩溃分析

## 项目结构

```
├── apps/fangjia/          # 主应用程序
├── domain/                # 业务逻辑层
├── data/                  # 数据访问层
├── infrastructure/        # 基础设施层
├── presentation/          # 表现层 (UI)
├── resources/             # Qt 资源文件
└── tests/                 # 单元测试
```

## 开发工具推荐

- **Qt Creator**: 专为 Qt 开发优化的 IDE
- **Qt Visual Studio Tools**: VS 插件，提供 Qt 项目模板和工具
- **vcpkg**: C++ 包管理器，用于管理第三方依赖

## 更多信息

- **项目文档**: [README.md](README.md)
- **架构文档**: [doc/](doc/)
- **中文文档**: [doc.zh-cn/](doc.zh-cn/)

如有问题，请参考项目文档或提交 Issue。