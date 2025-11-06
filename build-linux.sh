#!/bin/bash
# Build script for Linux

set -e

echo "Building Dear ImGui Redis Assistant for Linux..."

# Get version
VERSION="1.0.0"

# Create build directory
mkdir -p build/linux
cd build/linux

# Configure with CMake
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++" \
    -DBUILD_SHARED_LIBS=OFF

# Build
cmake --build . --config Release -j$(nproc)

# Create release directory
RELEASE_DIR="../../releases/v${VERSION}/linux"
mkdir -p "${RELEASE_DIR}"

# Copy executable
cp bin/DearImGuiRedisAssistant "${RELEASE_DIR}/"

# Copy any required files
echo "v${VERSION}" > "${RELEASE_DIR}/VERSION"

echo "Build complete! Executable in: ${RELEASE_DIR}"
ls -lh "${RELEASE_DIR}/"
