param(
  [string]$Configuration = "Debug",
  [string]$BuildDir = "build"
)
$ErrorActionPreference = "Stop"
cmake -S . -B $BuildDir
cmake --build $BuildDir --config $Configuration