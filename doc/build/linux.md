**English** | [简体中文](../../doc.zh-cn/build/linux.md)

# Linux Build Guide

## Prerequisites

### Package Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake git

# Qt 6 packages
sudo apt install qt6-base-dev qt6-base-dev-tools \
                 libqt6opengl6-dev libqt6svg6-dev \
                 qmake6 qt6-qmltooling-plugins

# Additional dependencies
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

#### Fedora/RHEL
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Qt 6 packages
sudo dnf install qt6-qtbase-devel qt6-qtsvg-devel \
                 qt6-qttools-devel mesa-libGL-devel \
                 mesa-libGLU-devel
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake git
sudo pacman -S qt6-base qt6-svg qt6-tools mesa
```

### Alternative: Manual Qt Installation

1. Download Qt installer from [Qt official website](https://www.qt.io/download)
2. Make installer executable: `chmod +x qt-online-installer-linux-x64-*.run`
3. Run installer: `./qt-online-installer-linux-x64-*.run`
4. Add Qt to PATH: `export PATH=/opt/Qt/6.5.0/gcc_64/bin:$PATH`

## Build Configuration

### Standard Build
```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build (using all available cores)
cmake --build build -j$(nproc)

# For Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Development Build with Debug Symbols
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build -j$(nproc)
```

## Running the Application

```bash
# From build directory
./build/apps/fangjia/fangjia

# Or with debug information
gdb ./build/apps/fangjia/fangjia
```

## Common Issues

### Qt Not Found
```bash
# Set Qt environment
export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6
# Or if using manual Qt installation
export Qt6_DIR=/opt/Qt/6.5.0/gcc_64/lib/cmake/Qt6
```

### Missing Dependencies
```bash
# Check missing libraries
ldd ./build/apps/fangjia/fangjia

# Install missing packages as needed
```

### OpenGL Issues
```bash
# Check OpenGL support
glxinfo | grep "OpenGL version"

# Install Mesa drivers if needed
sudo apt install mesa-utils libgl1-mesa-glx
```

## Development Environment

### VS Code Setup
1. Install C++ extension
2. Install CMake extension
3. Configure `settings.json`:
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    ]
}
```

### Debug Configuration
Create `.vscode/launch.json`:
```json
{
    "configurations": [
        {
            "name": "Debug Fangjia",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/apps/fangjia/fangjia",
            "cwd": "${workspaceFolder}",
            "environment": [
                {"name": "QT_QPA_PLATFORM", "value": "xcb"}
            ]
        }
    ]
}
```

## Platform-Specific Notes

### Wayland Support
The application supports both X11 and Wayland:
```bash
# Force X11 (if needed)
export QT_QPA_PLATFORM=xcb

# Force Wayland
export QT_QPA_PLATFORM=wayland
```

### High-DPI Support
```bash
# Enable automatic DPI scaling
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_SCALE_FACTOR=1.5  # Manual scaling if needed
```