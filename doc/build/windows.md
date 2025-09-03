**English** | [简体中文](../../doc.zh-cn/build/windows.md)

# Windows Build Guide

## Prerequisites

### Required Software
- **Visual Studio 2019 or later** with C++ components
- **Qt 6.5 or later** with following components:
  - Qt6 Core
  - Qt6 Gui  
  - Qt6 Widgets
  - Qt6 OpenGL
  - Qt6 Svg
- **CMake 3.16 or later**

### Qt Installation

1. Download Qt installer from [Qt official website](https://www.qt.io/download)
2. Install Qt 6.5+ with MSVC compiler support
3. Add Qt `bin` directory to system PATH
4. Set `Qt6_DIR` environment variable to Qt installation directory

## Build Configuration

### Using Qt Creator
1. Open `CMakeLists.txt` in Qt Creator
2. Configure project with Qt 6 kit
3. Build → Build All

### Command Line Build

```cmd
# Open Qt Developer Command Prompt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019"
cmake --build build --config Debug

# For Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019"  
cmake --build build --config Release
```

## Running the Application

After successful build:
```cmd
# Debug build
.\build\apps\fangjia\Debug\FangJia.exe

# Release build  
.\build\apps\fangjia\Release\FangJia.exe
```

## Common Issues

### Qt Not Found
- Ensure `Qt6_DIR` environment variable points to Qt installation
- Check that Qt bin directory is in PATH
- Verify Qt version compatibility (6.5+)

### Visual Studio Errors
- Install latest Visual Studio C++ build tools
- Ensure Windows SDK is installed
- Check CMake generator matches Visual Studio version

### OpenGL Issues
- Update graphics drivers
- Verify OpenGL 3.3+ support
- Check Windows display settings for hardware acceleration

## Platform-Specific Features

### Windows Chrome Integration
The application includes custom window chrome for Windows 10/11:
- Native window decoration
- DPI awareness
- Theme integration
- Smooth window animations

See [Windows Platform Integration](../infrastructure/platform-windows.md) for implementation details.