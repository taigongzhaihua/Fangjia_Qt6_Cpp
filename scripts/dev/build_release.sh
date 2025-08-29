#!/usr/bin/env bash
set -euo pipefail
BUILD_DIR="build-release"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j