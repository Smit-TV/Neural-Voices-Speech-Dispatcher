#!/usr/bin/env bash
set -e

BUILD_DIR="build"
BUILD_TYPE="Release"   # or Debug

if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory"
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

echo "Preparing..."
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo "Building..."
cmake --build . -j$(nproc)

echo "Done!"