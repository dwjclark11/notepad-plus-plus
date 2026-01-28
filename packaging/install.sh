#!/bin/bash
# Install script for Notepad++ Linux packaging
# This script installs Notepad++ from source with proper system integration

set -e

# Configuration
APP_NAME="notepad-plus-plus"
VERSION="8.6.0"
PREFIX="${PREFIX:-/usr/local}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
SOURCE_DIR="$(dirname "$(readlink -f "$0")")/.."
BUILD_DIR="build-install"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Check for required tools
check_dependencies() {
    print_step "Checking dependencies..."

    local missing_deps=()

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi

    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi

    # Check for Qt6
    if ! pkg-config --exists Qt6Core 2>/dev/null; then
        if ! [ -d "/usr/include/qt6" ] && ! [ -d "/usr/lib/qt6" ]; then
            missing_deps+=("qt6-base-dev" "or" "qt6-qtbase-devel")
        fi
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install them and try again"
        print_error ""
        print_error "On Debian/Ubuntu: sudo apt install build-essential cmake qt6-base-dev"
        print_error "On Fedora: sudo dnf install gcc-c++ cmake qt6-qtbase-devel"
        print_error "On Arch: sudo pacman -S base-devel cmake qt6-base"
        exit 1
    fi

    print_info "All required dependencies found"
}

# Build Notepad++
build_notepad() {
    print_step "Building Notepad++..."

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure with CMake
    print_info "Configuring with CMake..."
    cmake "$SOURCE_DIR/PowerEditor/src" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_CXX_STANDARD=20

    # Build
    print_info "Compiling..."
    make -j$(nproc)

    cd ..
    print_info "Build completed"
}

# Install files
install_files() {
    print_step "Installing Notepad++..."

    cd "$BUILD_DIR"

    # Install binary
    install -Dm755 notepad-plus-plus "$PREFIX/bin/notepad-plus-plus"

    # Create symlink
    ln -sf notepad-plus-plus "$PREFIX/bin/notepad++" 2>/dev/null || true

    cd ..

    # Install desktop file
    install -Dm644 packaging/common/notepad-plus-plus.desktop \
        "$PREFIX/share/applications/notepad-plus-plus.desktop"

    # Install icons
    install -d "$PREFIX/share/icons/hicolor/scalable/apps"
    if [ -f packaging/common/notepad-plus-plus.svg ]; then
        install -Dm644 packaging/common/notepad-plus-plus.svg \
            "$PREFIX/share/icons/hicolor/scalable/apps/notepad-plus-plus.svg"
    fi

    # Install metainfo
    install -Dm644 packaging/common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml \
        "$PREFIX/share/metainfo/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml"

    # Install MIME type
    install -Dm644 packaging/common/notepad-plus-plus-mime.xml \
        "$PREFIX/share/mime/packages/notepad-plus-plus.xml"

    # Install localization files
    install -d "$PREFIX/share/notepad-plus-plus/localization"
    if [ -d "$SOURCE_DIR/PowerEditor/installer/nativeLang" ]; then
        cp -r "$SOURCE_DIR/PowerEditor/installer/nativeLang"/* \
            "$PREFIX/share/notepad-plus-plus/localization/" 2>/dev/null || true
    fi

    # Install themes
    install -d "$PREFIX/share/notepad-plus-plus/themes"
    if [ -d "$SOURCE_DIR/PowerEditor/installer/themes" ]; then
        cp -r "$SOURCE_DIR/PowerEditor/installer/themes"/* \
            "$PREFIX/share/notepad-plus-plus/themes/" 2>/dev/null || true
    fi

    # Install documentation
    install -Dm644 LICENSE "$PREFIX/share/doc/notepad-plus-plus/LICENSE"
    install -Dm644 README.md "$PREFIX/share/doc/notepad-plus-plus/README.md"

    print_info "Installation completed"
}

# Update system databases
update_databases() {
    print_step "Updating system databases..."

    # Update MIME database
    if command -v update-mime-database >/dev/null 2>&1; then
        update-mime-database "$PREFIX/share/mime" 2>/dev/null || true
        print_info "MIME database updated"
    fi

    # Update desktop database
    if command -v update-desktop-database >/dev/null 2>&1; then
        update-desktop-database "$PREFIX/share/applications" 2>/dev/null || true
        print_info "Desktop database updated"
    fi

    # Update icon cache
    if command -v gtk-update-icon-cache >/dev/null 2>&1; then
        gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" 2>/dev/null || true
        print_info "Icon cache updated"
    fi
    if command -v update-icon-caches >/dev/null 2>&1; then
        update-icon-caches "$PREFIX/share/icons/hicolor" 2>/dev/null || true
    fi
}

# Uninstall
uninstall() {
    print_step "Uninstalling Notepad++..."

    # Remove binary
    rm -f "$PREFIX/bin/notepad-plus-plus"
    rm -f "$PREFIX/bin/notepad++"

    # Remove desktop file
    rm -f "$PREFIX/share/applications/notepad-plus-plus.desktop"

    # Remove icons
    rm -f "$PREFIX/share/icons/hicolor/scalable/apps/notepad-plus-plus.svg"

    # Remove metainfo
    rm -f "$PREFIX/share/metainfo/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml"

    # Remove MIME type
    rm -f "$PREFIX/share/mime/packages/notepad-plus-plus.xml"

    # Remove data directories
    rm -rf "$PREFIX/share/notepad-plus-plus"

    # Remove documentation
    rm -rf "$PREFIX/share/doc/notepad-plus-plus"

    # Update databases
    update_databases

    print_info "Uninstallation completed"
}

# Clean build files
clean() {
    print_step "Cleaning build files..."
    rm -rf "$BUILD_DIR"
    print_info "Clean completed"
}

# Show version
show_version() {
    echo "Notepad++ $VERSION"
    echo "Copyright (C) 2024 Don Ho"
    echo "License GPLv2+: GNU GPL version 2 or later"
}

# Show usage
show_help() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  install     Build and install Notepad++ (default)"
    echo "  uninstall   Remove Notepad++ from the system"
    echo "  build       Build only (don't install)"
    echo "  clean       Remove build files"
    echo "  version     Show version information"
    echo ""
    echo "Options:"
    echo "  --prefix=PATH    Set installation prefix (default: /usr/local)"
    echo "  --debug          Build with debug symbols"
    echo "  --release        Build with optimizations (default)"
    echo "  --help, -h       Show this help message"
    echo ""
    echo "Environment variables:"
    echo "  PREFIX           Installation prefix"
    echo "  BUILD_TYPE       CMake build type (Debug, Release, RelWithDebInfo, MinSizeRel)"
    echo ""
    echo "Examples:"
    echo "  $0                          # Build and install to /usr/local"
    echo "  $0 --prefix=/usr            # Build and install to /usr"
    echo "  PREFIX=/usr sudo $0         # Same as above"
    echo "  $0 uninstall                # Remove Notepad++"
    echo ""
}

# Main function
main() {
    local command="install"

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            install|uninstall|build|clean|version)
                command="$1"
                shift
                ;;
            --prefix=*)
                PREFIX="${1#*=}"
                shift
                ;;
            --debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            --release)
                BUILD_TYPE="Release"
                shift
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # Execute command
    case "$command" in
        install)
            check_dependencies
            build_notepad
            install_files
            update_databases
            print_info "Notepad++ $VERSION installed successfully to $PREFIX"
            print_info "Run 'notepad-plus-plus' or 'notepad++' to start the application"
            ;;
        uninstall)
            uninstall
            print_info "Notepad++ has been uninstalled"
            ;;
        build)
            check_dependencies
            build_notepad
            print_info "Build completed successfully!"
            print_info "Binary location: $BUILD_DIR/notepad-plus-plus"
            ;;
        clean)
            clean
            ;;
        version)
            show_version
            ;;
    esac
}

# Run main function
main "$@"
