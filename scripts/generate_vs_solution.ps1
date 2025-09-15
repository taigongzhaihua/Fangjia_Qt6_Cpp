# Generate Visual Studio Solution for Fangjia Qt6 C++ Project
# 为房价 Qt6 C++ 项目生成 Visual Studio 解决方案

param(
    [Parameter(HelpMessage="Visual Studio version (2019 or 2022)")]
    [ValidateSet("2019", "2022")]
    [string]$VSVersion = "2022",
    
    [Parameter(HelpMessage="Target architecture (x64 or x86)")]
    [ValidateSet("x64", "x86")]
    [string]$Architecture = "x64",
    
    [Parameter(HelpMessage="Clean build directory before generation")]
    [switch]$Clean
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Configuration
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = "$ProjectRoot\build_vs$VSVersion"

# Visual Studio generator mapping
$Generators = @{
    "2019" = "Visual Studio 16 2019"
    "2022" = "Visual Studio 17 2022"
}

$Generator = $Generators[$VSVersion]

Write-Host "=======================================" -ForegroundColor Green
Write-Host "Fangjia Qt6 C++ - Visual Studio Solution Generator" -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Green
Write-Host "Visual Studio Version: $VSVersion" -ForegroundColor Yellow
Write-Host "Architecture: $Architecture" -ForegroundColor Yellow
Write-Host "Generator: $Generator" -ForegroundColor Yellow
Write-Host "Build Directory: $BuildDir" -ForegroundColor Yellow
Write-Host ""

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor Cyan

# Check CMake
try {
    $cmakeVersion = cmake --version | Select-Object -First 1
    Write-Host "✓ CMake found: $cmakeVersion" -ForegroundColor Green
} catch {
    Write-Error "CMake not found. Please install CMake 3.16 or higher."
}

# Check Qt
if (-not $env:Qt6_DIR -and -not (Get-Command "qmake" -ErrorAction SilentlyContinue)) {
    Write-Warning "Qt6 not found in PATH or Qt6_DIR not set. Please ensure Qt 6.5+ is installed."
    Write-Host "Set Qt6_DIR environment variable, example:" -ForegroundColor Yellow
    Write-Host "  set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6" -ForegroundColor Yellow
}

# Clean build directory if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Cyan
    Remove-Item $BuildDir -Recurse -Force
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Change to project root
Push-Location $ProjectRoot

try {
    Write-Host "Generating Visual Studio solution..." -ForegroundColor Cyan
    
    # Run CMake configuration
    $cmakeArgs = @(
        "-S", ".",
        "-B", $BuildDir,
        "-G", $Generator,
        "-A", $Architecture
    )
    
    Write-Host "Running: cmake $($cmakeArgs -join ' ')" -ForegroundColor Gray
    & cmake @cmakeArgs
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "✓ Visual Studio solution generated successfully!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Solution file location:" -ForegroundColor Cyan
        Write-Host "  $BuildDir\Fangjia_Qt6_Cpp.sln" -ForegroundColor White
        Write-Host ""
        Write-Host "To open the solution:" -ForegroundColor Cyan
        Write-Host "  start `"$BuildDir\Fangjia_Qt6_Cpp.sln`"" -ForegroundColor White
        Write-Host ""
        Write-Host "To build from command line:" -ForegroundColor Cyan
        Write-Host "  cmake --build `"$BuildDir`" --config Debug" -ForegroundColor White
        Write-Host "  cmake --build `"$BuildDir`" --config Release" -ForegroundColor White
        
        # Ask if user wants to open the solution
        Write-Host ""
        $openSolution = Read-Host "Open Visual Studio solution now? (y/N)"
        if ($openSolution -eq "y" -or $openSolution -eq "Y") {
            Start-Process "$BuildDir\Fangjia_Qt6_Cpp.sln"
        }
    } else {
        Write-Error "Failed to generate Visual Studio solution. Check the error messages above."
    }
    
} catch {
    Write-Error "An error occurred: $($_.Exception.Message)"
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "For more information, see: doc.zh-cn/build/windows.md" -ForegroundColor Gray