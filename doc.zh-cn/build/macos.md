[English](../../doc/build/macos.md) | **简体中文**

# macOS 构建指南

## 前置要求

### 必需软件
- **Xcode 12 或更高版本** 包含命令行工具
- **Qt 6.5 或更高版本** 
- **CMake 3.16 或更高版本**

### 安装步骤

#### 1. 安装 Xcode
```bash
# 从 App Store 安装或从 Apple Developer 下载
xcode-select --install  # 安装命令行工具
```

#### 2. 安装 Qt
**选项 A：使用 Homebrew**
```bash
brew install qt@6
brew link qt@6 --force

# 添加到 PATH
echo 'export PATH="/usr/local/opt/qt@6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

**选项 B：官方 Qt 安装程序**
1. 从 [Qt 官方网站](https://www.qt.io/download) 下载
2. 安装到 `/Applications/Qt`
3. 添加到 PATH：`export PATH=/Applications/Qt/6.5.0/macos/bin:$PATH`

#### 3. 安装 CMake
```bash
# 使用 Homebrew
brew install cmake

# 或从 cmake.org 下载
```

## 构建配置

### 标准构建
```bash
# 为本机架构配置
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# 使用所有核心构建
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 通用二进制构建
```bash
# 为 Intel 和 Apple Silicon 配置
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15

cmake --build build -j$(sysctl -n hw.ncpu)
```

### Xcode 项目生成
```bash
# 生成 Xcode 项目
cmake -S . -B build -G Xcode

# 在 Xcode 中打开
open build/Fangjia.xcodeproj
```

## 运行应用程序

```bash
# 从终端
./build/apps/fangjia/fangjia

# 或双击应用包（如果已创建）
open ./build/apps/fangjia/fangjia.app
```

## 创建应用包

### 基本应用包
```bash
# 创建应用包结构
mkdir -p fangjia.app/Contents/MacOS
mkdir -p fangjia.app/Contents/Resources

# 复制可执行文件
cp ./build/apps/fangjia/fangjia fangjia.app/Contents/MacOS/

# 创建 Info.plist
cat > fangjia.app/Contents/Info.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>fangjia</string>
    <key>CFBundleIdentifier</key>
    <string>com.fangjia.qt6cpp</string>
    <key>CFBundleName</key>
    <string>Fangjia Qt6 C++</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
</dict>
</plist>
EOF
```

### 部署 Qt 依赖
```bash
# 使用 macdeployqt 打包 Qt 库
macdeployqt fangjia.app

# 验证应用包
otool -L fangjia.app/Contents/MacOS/fangjia
```

## 常见问题

### 找不到 Qt
```bash
# 设置 Qt 环境
export Qt6_DIR=/usr/local/opt/qt@6/lib/cmake/Qt6
# 或使用官方安装程序
export Qt6_DIR=/Applications/Qt/6.5.0/macos/lib/cmake/Qt6
```

### Xcode 构建错误
- 确保安装了 Xcode 命令行工具
- 更新到最新的 macOS SDK
- 检查部署目标兼容性

### 代码签名（用于分发）
```bash
# 签名应用程序
codesign --force --verify --verbose --sign "Developer ID Application: Your Name" fangjia.app

# 验证签名
codesign --verify --verbose=2 fangjia.app
spctl --assess --verbose=2 fangjia.app
```

## 平台特定功能

### macOS 集成
应用程序包含原生 macOS 功能：
- 菜单栏集成
- 深色模式支持
- Retina 显示器优化
- Touch Bar 支持（如果可用）

### 性能优化
```bash
# 使用 Instruments 分析
instruments -t "Time Profiler" ./build/apps/fangjia/fangjia

# 内存分析
instruments -t "Allocations" ./build/apps/fangjia/fangjia
```