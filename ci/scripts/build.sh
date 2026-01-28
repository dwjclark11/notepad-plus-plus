#!/bin/bash
# Build script for Notepad++ Linux port
# Usage: ./ci/scripts/build.sh [Release|Debug] [options]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"
JOBS=$(nproc)
CLEAN_BUILD=false
VERBOSE=false
ENABLE_COVERAGE=false
COMPILER="gcc"

# Parse arguments
shift || true
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE=true
            BUILD_TYPE="Debug"
            BUILD_DIR="build-coverage"
            shift
            ;;
        --clang)
            COMPILER="clang"
            shift
            ;;
        --gcc)
            COMPILER="gcc"
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [Release|Debug] [options]"
            echo ""
            echo "Options:"
            echo "  --clean       Clean build directory before building"
            echo "  --verbose     Verbose build output"
            echo "  --coverage    Build with coverage instrumentation"
            echo "  --clang       Use Clang compiler"
            echo "  --gcc         Use GCC compiler (default)"
            echo "  -j, --jobs N  Number of parallel jobs (default: $(nproc))"
            echo "  --help        Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Set compiler
if [ "$COMPILER" == "clang" ]; then
    export CC=clang
    export CXX=clang++
else
    export CC=gcc
    export CXX=g++
fi

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$PROJECT_ROOT/PowerEditor/src"
BUILD_PATH="$SOURCE_DIR/$BUILD_DIR"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Notepad++ Linux Build Script${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Build Type: $BUILD_TYPE"
echo "Compiler: $CC / $CXX"
echo "Build Directory: $BUILD_PATH"
echo "Parallel Jobs: $JOBS"
echo "Clean Build: $CLEAN_BUILD"
echo "Coverage: $ENABLE_COVERAGE"
echo ""

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

# Check for required tools
for tool in cmake ninja-build $CC $CXX; do
    if ! command -v "$tool" &> /dev/null; then
        # Handle ninja-build vs ninja naming
        if [ "$tool" == "ninja-build" ]; then
            if command -v ninja &> /dev/null; then
                continue
            fi
        fi
        echo -e "${RED}Error: $tool is not installed${NC}"
        exit 1
    fi
done

# Check for Qt6
if ! pkg-config --exists Qt6Core Qt6Widgets Qt6Gui 2>/dev/null; then
    echo -e "${RED}Error: Qt6 development packages not found${NC}"
    echo "Please install Qt6 development packages:"
    echo "  Ubuntu/Debian: sudo apt-get install qt6-base-dev"
    echo "  Fedora: sudo dnf install qt6-qtbase-devel"
    echo "  Arch: sudo pacman -S qt6-base"
    exit 1
fi

echo -e "${GREEN}All dependencies found!${NC}"
echo ""

# Clean build directory if requested
if [ "$CLEAN_BUILD" == true ] && [ -d "$BUILD_PATH" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_PATH"
fi

# Create build directory
mkdir -p "$BUILD_PATH"
cd "$BUILD_PATH"

# Configure CMake
echo -e "${YELLOW}Configuring CMake...${NC}"

CMAKE_ARGS=(
    -GNinja
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_CXX_STANDARD=20
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

# Add coverage flags if enabled
if [ "$ENABLE_COVERAGE" == true ]; then
    CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage")
    CMAKE_ARGS+=(-DCMAKE_C_FLAGS="--coverage -fprofile-arcs -ftest-coverage")
    CMAKE_ARGS+=(-DCMAKE_EXE_LINKER_FLAGS="--coverage")
fi

# Add verbose flag if enabled
if [ "$VERBOSE" == true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

# Run CMake
if ! cmake "${CMAKE_ARGS[@]}" ..; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

echo -e "${GREEN}CMake configuration complete!${NC}"
echo ""

# Build
echo -e "${YELLOW}Building Notepad++...${NC}"

NINJA_ARGS=(-j"$JOBS")
if [ "$VERBOSE" == true ]; then
    NINJA_ARGS+=(-v)
fi

if ! ninja "${NINJA_ARGS[@]}"; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build complete!${NC}"
echo ""

# Print build info
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Build Summary${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Binary location: $BUILD_PATH/notepad-plus-plus"

if [ -f "$BUILD_PATH/notepad-plus-plus" ]; then
    echo "Binary size: $(du -h "$BUILD_PATH/notepad-plus-plus" | cut -f1)"
    echo ""
    echo -e "${GREEN}Build successful!${NC}"
else
    echo -e "${RED}Binary not found!${NC}"
    exit 1
fi

# Generate compile_commands.json symlink for clangd
if [ -f "$BUILD_PATH/compile_commands.json" ]; then
    ln -sf "$BUILD_PATH/compile_commands.json" "$SOURCE_DIR/compile_commands.json"
fi

echo ""
echo "To run Notepad++:"
echo "  $BUILD_PATH/notepad-plus-plus"
echo ""

exit 0
