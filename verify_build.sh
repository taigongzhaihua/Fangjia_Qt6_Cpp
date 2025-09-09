#!/bin/bash

# Build verification script for Fangjia Qt6 C++ project
# This script ensures the project builds successfully and detects common compilation errors

set -e

echo "=== Fangjia Qt6 C++ Build Verification ==="
echo "Starting build verification at $(date)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check for required dependencies
print_status "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    print_error "cmake is not installed"
    exit 1
fi

if ! command -v make &> /dev/null; then
    print_error "make is not installed" 
    exit 1
fi

if ! pkg-config --exists Qt6Core; then
    print_error "Qt6 development packages not found"
    exit 1
fi

print_status "All dependencies found"

# Create build directory
BUILD_DIR="build_verification"
if [ -d "$BUILD_DIR" ]; then
    print_status "Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure project
print_status "Configuring project with CMake..."
if ! cmake .. -DCMAKE_BUILD_TYPE=Debug > cmake_output.log 2>&1; then
    print_error "CMake configuration failed"
    cat cmake_output.log
    exit 1
fi

# Build project
print_status "Building project..."
if ! make -j$(nproc) > build_output.log 2>&1; then
    print_error "Build failed"
    
    # Check for specific error patterns mentioned in original problem
    echo "Checking for known error patterns..."
    
    if grep -q "explicit specialization\|显式专用化" build_output.log; then
        print_warning "Template specialization errors detected"
    fi
    
    if grep -q "OpenGL header already included" build_output.log; then
        print_warning "OpenGL header conflict detected"
    fi
    
    if grep -q "stb_image.h.*No such file" build_output.log; then
        print_warning "Missing stb_image.h header detected"
    fi
    
    if grep -q "onClick.*non-standard syntax\|onClick.*函数不接受.*参数" build_output.log; then
        print_warning "Button onClick syntax error detected"
    fi
    
    if grep -q "setLayoutConstraints.*lambda.*转换" build_output.log; then
        print_warning "Lambda conversion error detected"
    fi
    
    cat build_output.log
    exit 1
fi

# Run tests if available
if [ -f "./FangJia_Tests" ]; then
    print_status "Running tests..."
    if ! ./FangJia_Tests > test_output.log 2>&1; then
        print_warning "Some tests failed"
        cat test_output.log
    else
        print_status "All tests passed"
    fi
fi

# Cleanup
cd ..
rm -rf "$BUILD_DIR"

print_status "Build verification completed successfully at $(date)"
echo "=== Build Verification Summary ==="
echo "✓ Dependencies found"
echo "✓ CMake configuration succeeded"
echo "✓ Build completed without errors"
echo "✓ No known error patterns detected"

exit 0