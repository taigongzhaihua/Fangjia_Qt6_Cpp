[English](../../doc/build/linux.md) | **简体中文**

# Linux 构建指南

## 前置要求

### 软件包安装

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake git

# Qt 6 软件包
sudo apt install qt6-base-dev qt6-base-dev-tools \
                 libqt6opengl6-dev libqt6svg6-dev \
                 qmake6 qt6-qmltooling-plugins

# 附加依赖
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

#### Fedora/RHEL
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Qt 6 软件包
sudo dnf install qt6-qtbase-devel qt6-qtsvg-devel \
                 qt6-qttools-devel mesa-libGL-devel \
                 mesa-libGLU-devel
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake git
sudo pacman -S qt6-base qt6-svg qt6-tools mesa
```

### 替代方案：手动 Qt 安装

1. 从 [Qt 官方网站](https://www.qt.io/download) 下载 Qt 安装程序
2. 使安装程序可执行：`chmod +x qt-online-installer-linux-x64-*.run`
3. 运行安装程序：`./qt-online-installer-linux-x64-*.run`
4. 将 Qt 添加到 PATH：`export PATH=/opt/Qt/6.5.0/gcc_64/bin:$PATH`

## 构建配置

### 标准构建
```bash
# 配置
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# 构建（使用所有可用核心）
cmake --build build -j$(nproc)

# 发布版本构建
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 带调试符号的开发构建
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build -j$(nproc)
```

## 运行应用程序

```bash
# 从构建目录
./build/apps/fangjia/fangjia

# 或使用调试信息
gdb ./build/apps/fangjia/fangjia
```

## 常见问题

### 找不到 Qt
```bash
# 设置 Qt 环境
export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6
# 或如果使用手动 Qt 安装
export Qt6_DIR=/opt/Qt/6.5.0/gcc_64/lib/cmake/Qt6
```

### 缺少依赖
```bash
# 检查缺少的库
ldd ./build/apps/fangjia/fangjia

# 根据需要安装缺少的软件包
```

### OpenGL 问题
```bash
# 检查 OpenGL 支持
glxinfo | grep "OpenGL version"

# 如果需要，安装 Mesa 驱动程序
sudo apt install mesa-utils libgl1-mesa-glx
```

## 开发环境

### VS Code 设置
1. 安装 C++ 扩展
2. 安装 CMake 扩展
3. 配置 `settings.json`：
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    ]
}
```

## 平台特定说明

### Wayland 支持
应用程序同时支持 X11 和 Wayland：
```bash
# 强制 X11（如果需要）
export QT_QPA_PLATFORM=xcb

# 强制 Wayland
export QT_QPA_PLATFORM=wayland
```

### 高 DPI 支持
```bash
# 启用自动 DPI 缩放
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_SCALE_FACTOR=1.5  # 如果需要手动缩放
```