#!/bin/bash
# Entrypoint script for Notepad++ build containers

set -e

# Print environment information
echo "=========================================="
echo "Notepad++ Linux Build Environment"
echo "=========================================="
echo ""
echo "Compiler: $(gcc --version | head -1)"
echo "CMake: $(cmake --version | head -1)"
echo "Qt6: $(qmake6 --version 2>/dev/null || echo 'qmake6 not found')"
echo ""
echo "Available commands:"
echo "  build       - Build Notepad++"
echo "  test        - Run tests"
echo "  package     - Create packages"
echo "  shell       - Start interactive shell"
echo "  format      - Run clang-format"
echo "  tidy        - Run clang-tidy"
echo "  cppcheck    - Run cppcheck"
echo "  coverage    - Generate coverage report"
echo ""

# Function to build Notepad++
build_notepad_plus_plus() {
    echo "Building Notepad++..."

    BUILD_TYPE="${1:-Release}"
    BUILD_DIR="build"

    cd /workspace/PowerEditor/src

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    cmake .. \
        -GNinja \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    ninja -j$(nproc)

    echo "Build complete!"
    echo "Binary location: /workspace/PowerEditor/src/$BUILD_DIR/notepad-plus-plus"
}

# Function to run tests
run_tests() {
    echo "Running tests..."

    cd /workspace/PowerEditor/src/build

    # Run ctest if available
    if [ -f CTestTestfile.cmake ]; then
        ctest --output-on-failure
    else
        echo "No tests configured"
    fi
}

# Function to create packages
create_packages() {
    echo "Creating packages..."

    cd /workspace/PowerEditor/src/build

    # Create tarball
    tar czf notepad-plus-plus-linux-x86_64.tar.gz notepad-plus-plus

    echo "Package created: notepad-plus-plus-linux-x86_64.tar.gz"
}

# Function to run clang-format
run_clang_format() {
    echo "Running clang-format..."

    cd /workspace

    if [ -f .clang-format ]; then
        find PowerEditor/src -type f \( -name "*.cpp" -o -name "*.h" \) \
            ! -path "*/TinyXml/*" \
            ! -path "*/pugixml/*" \
            ! -path "*/uchardet/*" \
            ! -path "*/json/*" \
            ! -path "*/lexilla/*" \
            ! -path "*/scintilla/*" \
            ! -path "*/boostregex/*" \
            -exec clang-format -i {} \;
        echo "clang-format complete!"
    else
        echo "No .clang-format file found"
    fi
}

# Function to run clang-tidy
run_clang_tidy() {
    echo "Running clang-tidy..."

    cd /workspace/PowerEditor/src

    if [ -f build/compile_commands.json ]; then
        find . -type f -name "*.cpp" \
            ! -path "*/TinyXml/*" \
            ! -path "*/pugixml/*" \
            ! -path "*/uchardet/*" \
            ! -path "*/json/*" \
            ! -path "*/lexilla/*" \
            ! -path "*/scintilla/*" \
            ! -path "*/boostregex/*" \
            ! -path "*/build/*" \
            -exec clang-tidy -p build {} \;
    else
        echo "No compile_commands.json found. Run build first."
    fi
}

# Function to run cppcheck
run_cppcheck() {
    echo "Running cppcheck..."

    cd /workspace/PowerEditor/src

    cppcheck \
        --enable=all \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --error-exitcode=0 \
        --std=c++20 \
        -I. \
        -I../../scintilla/include \
        -I../../lexilla/include \
        -iTinyXml \
        -ipugixml \
        -iuchardet \
        -ijson \
        -ibuild \
        . 2>&1 | tee cppcheck-report.txt

    echo "cppcheck complete! Report: cppcheck-report.txt"
}

# Function to generate coverage report
generate_coverage() {
    echo "Generating coverage report..."

    cd /workspace/PowerEditor/src

    mkdir -p build-coverage
    cd build-coverage

    cmake .. \
        -GNinja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="--coverage" \
        -DCMAKE_C_FLAGS="--coverage"

    ninja -j$(nproc)

    # Generate reports
    gcovr -r .. --html --html-details -o coverage-report.html
    gcovr -r .. --xml -o coverage-report.xml

    echo "Coverage report generated!"
    echo "HTML: build-coverage/coverage-report.html"
    echo "XML: build-coverage/coverage-report.xml"
}

# Main entrypoint logic
case "${1:-shell}" in
    build)
        build_notepad_plus_plus "${2:-Release}"
        ;;
    test)
        run_tests
        ;;
    package)
        create_packages
        ;;
    format)
        run_clang_format
        ;;
    tidy)
        run_clang_tidy
        ;;
    cppcheck)
        run_cppcheck
        ;;
    coverage)
        generate_coverage
        ;;
    shell|bash|sh)
        exec bash
        ;;
    *)
        # If the command is a file, execute it
        if [ -f "$1" ]; then
            exec "$@"
        else
            echo "Unknown command: $1"
            echo "Usage: $0 {build|test|package|format|tidy|cppcheck|coverage|shell}"
            exit 1
        fi
        ;;
esac
