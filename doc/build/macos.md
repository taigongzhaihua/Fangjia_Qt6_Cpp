**English** | [简体中文](../../doc.zh-cn/build/macos.md)

# macOS Build Guide

## Prerequisites

### Required Software
- **Xcode 12 or later** with Command Line Tools
- **Qt 6.5 or later** 
- **CMake 3.16 or later**

### Installation Steps

#### 1. Install Xcode
```bash
# Install from App Store or download from Apple Developer
xcode-select --install  # Install command line tools
```

#### 2. Install Qt
**Option A: Using Homebrew**
```bash
brew install qt@6
brew link qt@6 --force

# Add to PATH
echo 'export PATH="/usr/local/opt/qt@6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

**Option B: Official Qt Installer**
1. Download from [Qt official website](https://www.qt.io/download)
2. Install to `/Applications/Qt`
3. Add to PATH: `export PATH=/Applications/Qt/6.5.0/macos/bin:$PATH`

#### 3. Install CMake
```bash
# Using Homebrew
brew install cmake

# Or download from cmake.org
```

## Build Configuration

### Standard Build
```bash
# Configure for native architecture
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build using all cores
cmake --build build -j$(sysctl -n hw.ncpu)
```

### Universal Binary Build
```bash
# Configure for both Intel and Apple Silicon
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15

cmake --build build -j$(sysctl -n hw.ncpu)
```

### Xcode Project Generation
```bash
# Generate Xcode project
cmake -S . -B build -G Xcode

# Open in Xcode
open build/Fangjia.xcodeproj
```

## Running the Application

```bash
# From terminal
./build/apps/fangjia/fangjia

# Or double-click the bundle (if created)
open ./build/apps/fangjia/fangjia.app
```

## Creating App Bundle

### Basic Bundle
```bash
# Create bundle structure
mkdir -p fangjia.app/Contents/MacOS
mkdir -p fangjia.app/Contents/Resources

# Copy executable
cp ./build/apps/fangjia/fangjia fangjia.app/Contents/MacOS/

# Create Info.plist
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

### Deploy Qt Dependencies
```bash
# Use macdeployqt to bundle Qt libraries
macdeployqt fangjia.app

# Verify bundle
otool -L fangjia.app/Contents/MacOS/fangjia
```

## Common Issues

### Qt Not Found
```bash
# Set Qt environment
export Qt6_DIR=/usr/local/opt/qt@6/lib/cmake/Qt6
# Or for official installer
export Qt6_DIR=/Applications/Qt/6.5.0/macos/lib/cmake/Qt6
```

### Xcode Build Errors
- Ensure Xcode Command Line Tools are installed
- Update to latest macOS SDK
- Check deployment target compatibility

### Code Signing (for distribution)
```bash
# Sign the application
codesign --force --verify --verbose --sign "Developer ID Application: Your Name" fangjia.app

# Verify signature
codesign --verify --verbose=2 fangjia.app
spctl --assess --verbose=2 fangjia.app
```

### Universal Binary Issues
- Ensure all dependencies support target architectures
- Test on both Intel and Apple Silicon Macs
- Use `lipo -info` to verify architecture support

## Development Environment

### Xcode Configuration
1. Open project in Xcode: `open build/Fangjia.xcodeproj`
2. Set scheme to Debug
3. Configure signing team (for device testing)

### VS Code Setup
Install extensions:
- C/C++ (Microsoft)
- CMake Tools
- Qt tools

Configure `.vscode/settings.json`:
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_BUILD_TYPE=Debug"
    ],
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

## Platform-Specific Features

### macOS Integration
The application includes native macOS features:
- Menu bar integration
- Dark mode support
- Retina display optimization
- Touch Bar support (if available)

### Performance Optimization
```bash
# Profile with Instruments
instruments -t "Time Profiler" ./build/apps/fangjia/fangjia

# Memory analysis
instruments -t "Allocations" ./build/apps/fangjia/fangjia
```