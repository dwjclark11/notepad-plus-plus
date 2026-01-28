#!/bin/bash
# Build script for Notepad++ Arch Linux package
# This script creates Arch Linux packages for Notepad++

set -e

# Configuration
PACKAGE_NAME="notepad-plus-plus"
VERSION="8.6.0"
PKGREL="1"
BUILD_DIR="build-arch"
SOURCE_DIR="../.."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

# Check for required tools
check_dependencies() {
    print_info "Checking dependencies..."

    local missing_deps=()

    if ! command -v makepkg &> /dev/null; then
        missing_deps+=("pacman")
    fi

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install them and try again"
        print_error "For example: sudo pacman -S ${missing_deps[*]}"
        exit 1
    fi

    print_info "All required dependencies found"
}

# Prepare build directory
prepare_build() {
    print_info "Preparing build directory..."

    # Create build directory
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"

    # Copy PKGBUILD
    cp arch/PKGBUILD "$BUILD_DIR/"

    # Copy .SRCINFO
    cp arch/.SRCINFO "$BUILD_DIR/" 2>/dev/null || true

    # Copy common files
    cp common/notepad-plus-plus.desktop "$BUILD_DIR/"
    cp common/notepad-plus-plus-mime.xml "$BUILD_DIR/"
    cp common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml "$BUILD_DIR/"

    # Copy icon if exists
    if [ -f common/notepad-plus-plus.svg ]; then
        cp common/notepad-plus-plus.svg "$BUILD_DIR/"
    fi

    print_info "Build directory prepared"
}

# Download sources
download_sources() {
    print_info "Downloading sources..."

    cd "$BUILD_DIR"

    # Use makepkg to download sources
    makepkg -g >> PKGBUILD || true

    cd ..
    print_info "Sources downloaded"
}

# Build package
build_package() {
    print_info "Building Arch package..."

    cd "$BUILD_DIR"

    # Build the package
    makepkg -sf --noconfirm

    cd ..
    print_info "Package build completed"
}

# Build only (don't install dependencies)
build_only() {
    print_info "Building Arch package (no dependency check)..."

    cd "$BUILD_DIR"

    # Build the package without installing dependencies
    makepkg -sf --noconfirm -d

    cd ..
    print_info "Package build completed"
}

# Copy results
copy_results() {
    print_info "Copying build results..."

    mkdir -p "output"

    # Copy package files
    cp "$BUILD_DIR"/*.pkg.tar.* "output/" 2>/dev/null || true

    print_info "Results copied to output/"
}

# Clean up
cleanup() {
    print_info "Cleaning up..."
    rm -rf "$BUILD_DIR"
    print_info "Cleanup completed"
}

# Install locally
install_local() {
    print_info "Installing package locally..."

    cd "$BUILD_DIR"

    # Install the package
    sudo pacman -U --noconfirm *.pkg.tar.*

    cd ..
    print_info "Installation completed"
}

# Update .SRCINFO
update_srcinfo() {
    print_info "Updating .SRCINFO..."

    cd "$BUILD_DIR"

    # Generate .SRCINFO
    makepkg --printsrcinfo > .SRCINFO

    cd ..

    # Copy back to arch directory
    cp "$BUILD_DIR/.SRCINFO" arch/

    print_info ".SRCINFO updated"
}

# Show usage
show_help() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build       Build the package (default)"
    echo "  build-only  Build without dependency checking"
    echo "  install     Build and install locally"
    echo "  srcinfo     Update .SRCINFO file"
    echo "  clean       Clean build files"
    echo ""
    echo "Options:"
    echo "  --help, -h  Show this help message"
    echo ""
}

# Main function
main() {
    local command="${1:-build}"

    case "$command" in
        build)
            check_dependencies
            prepare_build
            download_sources
            build_package
            copy_results
            print_info "Build completed successfully!"
            print_info "Packages are in output/"
            ;;
        build-only)
            check_dependencies
            prepare_build
            download_sources
            build_only
            copy_results
            print_info "Build completed successfully!"
            ;;
        install)
            check_dependencies
            prepare_build
            download_sources
            build_package
            copy_results
            install_local
            print_info "Installation completed successfully!"
            ;;
        srcinfo)
            prepare_build
            update_srcinfo
            print_info ".SRCINFO updated!"
            ;;
        clean)
            cleanup
            rm -rf output
            print_info "Clean completed!"
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown command: $command"
            show_help
            exit 1
            ;;
    esac
}

# Run main function
main "$@"
