@echo off
:: Fangjia Qt6 C++ Windows 构建脚本
:: 适用于 Visual Studio 2022 / Windows 10/11

echo ===========================================
echo Fangjia Qt6 C++ Windows 构建脚本
echo ===========================================

:: 检查是否在 Developer Command Prompt 中
if "%VSINSTALLDIR%"=="" (
    echo 错误: 请在 "Developer Command Prompt for VS 2022" 中运行此脚本
    echo 或者设置好 Visual Studio 2022 的环境变量
    pause
    exit /b 1
)

:: 检查 CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 CMake，请确保已安装并在 PATH 中
    pause
    exit /b 1
)

:: 设置默认 Qt 路径（如果环境变量未设置）
if "%CMAKE_PREFIX_PATH%"=="" (
    echo 正在搜索 Qt6 安装路径...
    
    set QT_PATHS=C:\Qt\6.8.0\msvc2022_64;C:\Qt\6.7.3\msvc2022_64;C:\Qt\6.6.3\msvc2022_64;C:\Qt\6.5.3\msvc2022_64
    
    for %%p in (%QT_PATHS%) do (
        if exist "%%p\lib\cmake\Qt6" (
            set CMAKE_PREFIX_PATH=%%p
            echo 发现 Qt6: %%p
            goto :qt_found
        )
    )
    
    echo 警告: 未找到 Qt6 安装路径
    echo 请设置 CMAKE_PREFIX_PATH 环境变量指向 Qt6 安装目录
    echo 示例: set CMAKE_PREFIX_PATH=C:\Qt\6.8.0\msvc2022_64
    echo.
    set /p QT_PATH="请输入 Qt6 安装路径（或回车跳过）: "
    if not "%QT_PATH%"=="" set CMAKE_PREFIX_PATH=%QT_PATH%
    
    :qt_found
)

:: 显示配置信息
echo.
echo 构建配置:
echo - 编译器: MSVC (Visual Studio 2022)
echo - C++ 标准: C++20
echo - Qt 路径: %CMAKE_PREFIX_PATH%
echo - 源目录: %cd%
echo - 构建目录: %cd%\build
echo.

:: 选择构建类型
set BUILD_TYPE=Debug
set /p BUILD_CHOICE="选择构建类型 [1=Debug, 2=Release] (默认: Debug): "
if "%BUILD_CHOICE%"=="2" set BUILD_TYPE=Release

echo.
echo ===========================================
echo 开始构建 (%BUILD_TYPE%)
echo ===========================================

:: 清理旧的构建目录
if exist build (
    echo 清理旧的构建文件...
    rmdir /s /q build
)

:: 配置项目
echo.
echo [1/3] 配置 CMake...
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%"

if errorlevel 1 (
    echo 错误: CMake 配置失败
    pause
    exit /b 1
)

:: 构建项目
echo.
echo [2/3] 构建项目...
cmake --build build --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo 错误: 构建失败
    pause
    exit /b 1
)

:: 运行测试
echo.
echo [3/3] 运行测试...
cd build
ctest --config %BUILD_TYPE% --output-on-failure
cd ..

if errorlevel 1 (
    echo 警告: 部分测试失败，但构建成功
) else (
    echo 所有测试通过！
)

echo.
echo ===========================================
echo 构建完成！
echo ===========================================
echo.
echo 可执行文件位置:
echo - 主程序: build\%BUILD_TYPE%\FangJia.exe
echo - 测试: build\tests\%BUILD_TYPE%\FangJia_Tests.exe
echo - 示例: build\examples\%BUILD_TYPE%\mvvm_binding_example.exe
echo.
echo 运行主程序:
echo cd build\%BUILD_TYPE% ^&^& FangJia.exe
echo.

:: 询问是否立即运行
set /p RUN_NOW="是否立即运行主程序？ [y/N]: "
if /i "%RUN_NOW%"=="y" (
    echo 启动 FangJia...
    cd build\%BUILD_TYPE%
    start FangJia.exe
    cd ..\..
)

pause