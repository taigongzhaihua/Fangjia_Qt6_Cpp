# Fangjia Qt6 C++ Windows 构建脚本 (PowerShell)
# 适用于 Visual Studio 2022 / Windows 10/11

param(
    [string]$BuildType = "Debug",
    [string]$QtPath = "",
    [switch]$Clean = $false,
    [switch]$Run = $false,
    [switch]$Help = $false
)

if ($Help) {
    Write-Host @"
Fangjia Qt6 C++ Windows 构建脚本

用法:
    .\build_windows.ps1 [选项]

选项:
    -BuildType <Debug|Release>  构建类型 (默认: Debug)
    -QtPath <路径>             Qt6 安装路径
    -Clean                     清理构建目录
    -Run                       构建完成后运行程序
    -Help                      显示此帮助信息

示例:
    .\build_windows.ps1 -BuildType Release -Run
    .\build_windows.ps1 -QtPath "C:\Qt\6.8.0\msvc2022_64" -Clean
"@
    exit 0
}

Write-Host "===========================================" -ForegroundColor Cyan
Write-Host "Fangjia Qt6 C++ Windows 构建脚本 (PowerShell)" -ForegroundColor Cyan
Write-Host "===========================================" -ForegroundColor Cyan

# 检查 CMake
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -ne 0) { throw }
    Write-Host "✓ 发现 CMake: $($cmakeVersion[0])" -ForegroundColor Green
} catch {
    Write-Host "✗ 错误: 未找到 CMake" -ForegroundColor Red
    Write-Host "请安装 CMake 并确保在 PATH 中" -ForegroundColor Yellow
    exit 1
}

# 检查 Visual Studio 环境
if (-not $env:VSINSTALLDIR) {
    Write-Host "⚠ 警告: 未检测到 Visual Studio 环境" -ForegroundColor Yellow
    Write-Host "建议在 'Developer PowerShell for VS 2022' 中运行" -ForegroundColor Yellow
}

# 搜索 Qt6 安装路径
if (-not $QtPath -and -not $env:CMAKE_PREFIX_PATH) {
    Write-Host "正在搜索 Qt6 安装路径..." -ForegroundColor Yellow
    
    $qtPaths = @(
        "C:\Qt\6.8.0\msvc2022_64",
        "C:\Qt\6.7.3\msvc2022_64", 
        "C:\Qt\6.6.3\msvc2022_64",
        "C:\Qt\6.5.3\msvc2022_64",
        "C:\Qt\6.4.2\msvc2022_64"
    )
    
    foreach ($path in $qtPaths) {
        if (Test-Path "$path\lib\cmake\Qt6") {
            $QtPath = $path
            Write-Host "✓ 发现 Qt6: $QtPath" -ForegroundColor Green
            break
        }
    }
    
    if (-not $QtPath) {
        Write-Host "⚠ 未找到 Qt6 安装路径" -ForegroundColor Yellow
        $QtPath = Read-Host "请输入 Qt6 安装路径（或回车跳过）"
    }
}

# 使用环境变量中的路径
if (-not $QtPath -and $env:CMAKE_PREFIX_PATH) {
    $QtPath = $env:CMAKE_PREFIX_PATH
}

# 显示配置信息
Write-Host ""
Write-Host "构建配置:" -ForegroundColor Cyan
Write-Host "- 编译器: MSVC (Visual Studio 2022)"
Write-Host "- C++ 标准: C++20" 
Write-Host "- 构建类型: $BuildType"
Write-Host "- Qt 路径: $(if($QtPath) { $QtPath } else { '未设置' })"
Write-Host "- 源目录: $(Get-Location)"
Write-Host "- 构建目录: $(Join-Path (Get-Location) 'build')"
Write-Host ""

# 清理构建目录
if ($Clean -and (Test-Path "build")) {
    Write-Host "清理构建目录..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
}

Write-Host "===========================================" -ForegroundColor Cyan
Write-Host "开始构建 ($BuildType)" -ForegroundColor Cyan  
Write-Host "===========================================" -ForegroundColor Cyan

try {
    # 配置项目
    Write-Host ""
    Write-Host "[1/3] 配置 CMake..." -ForegroundColor Green
    
    $cmakeArgs = @(
        "-S", ".",
        "-B", "build", 
        "-G", "Visual Studio 17 2022",
        "-A", "x64",
        "-DCMAKE_BUILD_TYPE=$BuildType"
    )
    
    if ($QtPath) {
        $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtPath"
    }
    
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "CMake 配置失败" }
    
    # 构建项目
    Write-Host ""
    Write-Host "[2/3] 构建项目..." -ForegroundColor Green
    & cmake --build build --config $BuildType --parallel
    if ($LASTEXITCODE -ne 0) { throw "构建失败" }
    
    # 运行测试
    Write-Host ""
    Write-Host "[3/3] 运行测试..." -ForegroundColor Green
    Push-Location build
    & ctest --config $BuildType --output-on-failure
    $testResult = $LASTEXITCODE
    Pop-Location
    
    Write-Host ""
    Write-Host "===========================================" -ForegroundColor Cyan
    Write-Host "构建完成！" -ForegroundColor Green
    Write-Host "===========================================" -ForegroundColor Cyan
    
    if ($testResult -ne 0) {
        Write-Host "⚠ 警告: 部分测试失败，但构建成功" -ForegroundColor Yellow
    } else {
        Write-Host "✓ 所有测试通过！" -ForegroundColor Green
    }
    
    Write-Host ""
    Write-Host "可执行文件位置:" -ForegroundColor Cyan
    Write-Host "- 主程序: build\$BuildType\FangJia.exe"
    Write-Host "- 测试: build\tests\$BuildType\FangJia_Tests.exe" 
    Write-Host "- 示例: build\examples\$BuildType\mvvm_binding_example.exe"
    Write-Host ""
    
    # 运行程序
    if ($Run) {
        $exePath = "build\$BuildType\FangJia.exe"
        if (Test-Path $exePath) {
            Write-Host "启动 FangJia..." -ForegroundColor Green
            Start-Process $exePath
        } else {
            Write-Host "✗ 未找到可执行文件: $exePath" -ForegroundColor Red
        }
    }
    
} catch {
    Write-Host ""
    Write-Host "✗ 错误: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "运行主程序: .\build\$BuildType\FangJia.exe" -ForegroundColor Cyan