#!/bin/bash

# Build script for EVM WebAssembly module using Emscripten

set -e

echo "Building EVM WebAssembly module..."

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten not found. Please install Emscripten SDK."
    echo "Visit: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake for Emscripten
echo "Configuring CMake for Emscripten..."
emcmake cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-fPIC" \
    ..

# Build
echo "Building..."
cmake --build . --config Release

# Copy output to web directory
echo "Copying output files..."
cp evm.js ../../../web/public/evm.js
cp evm.wasm ../../../web/public/evm.wasm

echo "Build complete!"
echo "Output files:"
echo "  - web/public/evm.js"
echo "  - web/public/evm.wasm"
