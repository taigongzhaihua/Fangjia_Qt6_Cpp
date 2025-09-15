@echo off
rem Generate Visual Studio Solution for Fangjia Qt6 C++ Project
rem 为房价 Qt6 C++ 项目生成 Visual Studio 解决方案

setlocal enabledelayedexpansion

echo =======================================
echo Fangjia Qt6 C++ - Visual Studio Solution Generator
echo =======================================
echo.

rem Check for Visual Studio version parameter
set VS_VERSION=2022
if not "%1"=="" set VS_VERSION=%1

rem Set generator based on VS version
if "%VS_VERSION%"=="2019" (
    set GENERATOR=Visual Studio 16 2019
    set BUILD_DIR=build_vs2019
) else if "%VS_VERSION%"=="2022" (
    set GENERATOR=Visual Studio 17 2022
    set BUILD_DIR=build_vs2022
) else (
    echo Error: Unsupported Visual Studio version: %VS_VERSION%
    echo Supported versions: 2019, 2022
    goto :error
)

echo Visual Studio Version: %VS_VERSION%
echo Generator: %GENERATOR%
echo Build Directory: %BUILD_DIR%
echo.

rem Check prerequisites
echo Checking prerequisites...

rem Check CMake
cmake --version >nul 2>&1
if !errorlevel! neq 0 (
    echo Error: CMake not found. Please install CMake 3.16 or higher.
    goto :error
)
echo [OK] CMake found

rem Check Qt (optional warning)
if "%Qt6_DIR%"=="" (
    echo Warning: Qt6_DIR environment variable not set.
    echo Please ensure Qt 6.5+ is installed and Qt6_DIR is configured.
    echo Example: set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
    echo.
)

rem Clean and create build directory
if exist %BUILD_DIR% (
    echo Cleaning existing build directory...
    rmdir /s /q %BUILD_DIR%
)
mkdir %BUILD_DIR%

rem Generate solution
echo.
echo Generating Visual Studio solution...
echo Running: cmake -S . -B %BUILD_DIR% -G "%GENERATOR%" -A x64

cmake -S . -B %BUILD_DIR% -G "%GENERATOR%" -A x64

if !errorlevel! equ 0 (
    echo.
    echo [SUCCESS] Visual Studio solution generated successfully!
    echo.
    echo Solution file location:
    echo   %BUILD_DIR%\Fangjia_Qt6_Cpp.sln
    echo.
    echo To open the solution:
    echo   start "%BUILD_DIR%\Fangjia_Qt6_Cpp.sln"
    echo.
    echo To build from command line:
    echo   cmake --build "%BUILD_DIR%" --config Debug
    echo   cmake --build "%BUILD_DIR%" --config Release
    echo.
    
    set /p OPEN_SOLUTION="Open Visual Studio solution now? (y/N): "
    if /i "!OPEN_SOLUTION!"=="y" (
        start "" "%BUILD_DIR%\Fangjia_Qt6_Cpp.sln"
    )
) else (
    echo.
    echo [ERROR] Failed to generate Visual Studio solution.
    echo Check the error messages above for details.
    goto :error
)

echo.
echo For more information, see: doc.zh-cn/build/windows.md
goto :end

:error
echo.
echo Generation failed. Please check the requirements:
echo 1. Visual Studio 2019/2022 with C++ components
echo 2. Qt 6.5+ with MSVC compiler support
echo 3. CMake 3.16+
echo 4. Proper environment variables (Qt6_DIR, PATH)
exit /b 1

:end
pause