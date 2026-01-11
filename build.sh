#!/bin/bash
# ShareY Build Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${1:-Release}"

echo "╔══════════════════════════════════════════╗"
echo "║          ShareY Build Script             ║"
echo "╚══════════════════════════════════════════╝"
echo ""
echo "Build type: ${BUILD_TYPE}"
echo ""

# Check dependencies
echo "Checking dependencies..."

check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "❌ $1 is not installed"
        return 1
    else
        echo "✓ $1 found"
        return 0
    fi
}

MISSING=0
check_command cmake || MISSING=1
check_command ninja || check_command make || MISSING=1
check_command pkg-config || MISSING=1

if [ $MISSING -eq 1 ]; then
    echo ""
    echo "Please install missing dependencies:"
    echo "  Debian/Ubuntu: sudo apt install build-essential cmake ninja-build"
    echo "  Arch Linux:    sudo pacman -S base-devel cmake ninja"
    echo "  Fedora:        sudo dnf install cmake ninja-build gcc-c++"
    exit 1
fi

# Check Qt6
if ! pkg-config --exists Qt6Core 2>/dev/null; then
    echo "⚠ Qt6 not found via pkg-config, CMake will try to find it..."
fi

# Check XCB
if ! pkg-config --exists xcb 2>/dev/null; then
    echo "❌ libxcb not found"
    echo "  Debian/Ubuntu: sudo apt install libxcb1-dev libxcb-util-dev libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-xfixes0-dev"
    echo "  Arch Linux:    sudo pacman -S libxcb xcb-util xcb-util-keysyms"
    exit 1
else
    echo "✓ libxcb found"
fi

echo ""

# Convert SVG icon to PNG if possible
if command -v convert &> /dev/null || command -v rsvg-convert &> /dev/null; then
    echo "Converting icon to PNG..."
    ICON_SVG="${SCRIPT_DIR}/resources/icons/shareY.svg"
    ICON_PNG="${SCRIPT_DIR}/resources/icons/shareY.png"
    
    if [ -f "$ICON_SVG" ] && [ ! -f "$ICON_PNG" ]; then
        if command -v rsvg-convert &> /dev/null; then
            rsvg-convert -w 256 -h 256 "$ICON_SVG" -o "$ICON_PNG"
        elif command -v convert &> /dev/null; then
            convert -background none -resize 256x256 "$ICON_SVG" "$ICON_PNG"
        fi
        echo "✓ Icon converted"
    fi
fi

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure
echo ""
echo "Configuring..."

GENERATOR=""
if command -v ninja &> /dev/null; then
    GENERATOR="-G Ninja"
fi

cmake .. ${GENERATOR} \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
echo ""
echo "Building..."

if command -v ninja &> /dev/null; then
    ninja
else
    make -j$(nproc)
fi

echo ""
echo "╔══════════════════════════════════════════╗"
echo "║           Build Successful! ✓            ║"
echo "╚══════════════════════════════════════════╝"
echo ""
echo "Run the application:"
echo "  ${BUILD_DIR}/shareY"
echo ""
echo "Install (optional):"
echo "  sudo cmake --install ${BUILD_DIR}"
echo ""
