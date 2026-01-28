#!/bin/bash
# Build script for Notepad++ Debian package
# This script creates .deb packages for Notepad++

set -e

# Configuration
PACKAGE_NAME="notepad-plus-plus"
VERSION="8.6.0"
DEBIAN_REVISION="1"
FULL_VERSION="${VERSION}-${DEBIAN_REVISION}"
BUILD_DIR="build-deb"
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

    if ! command -v dpkg-buildpackage &> /dev/null; then
        missing_deps+=("dpkg-dev")
    fi

    if ! command -v debhelper &> /dev/null; then
        missing_deps+=("debhelper")
    fi

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install them and try again"
        print_error "For example: sudo apt install ${missing_deps[*]}"
        exit 1
    fi

    print_info "All required dependencies found"
}

# Prepare source package
prepare_source() {
    print_info "Preparing source package..."

    # Create build directory
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"

    # Create orig tarball
    cd "$BUILD_DIR"

    # Create source directory structure
    mkdir -p "${PACKAGE_NAME}-${VERSION}"

    # Copy source files
    cp -r "$SOURCE_DIR"/* "${PACKAGE_NAME}-${VERSION}/" 2>/dev/null || true

    # Remove unnecessary files
    rm -rf "${PACKAGE_NAME}-${VERSION}/.git"
    rm -rf "${PACKAGE_NAME}-${VERSION}/build*"
    rm -rf "${PACKAGE_NAME}-${VERSION}/packaging"

    # Create debian directory
    mkdir -p "${PACKAGE_NAME}-${VERSION}/debian"

    # Copy debian files
    cp ../debian/* "${PACKAGE_NAME}-${VERSION}/debian/" 2>/dev/null || true
    cp -r ../debian/source "${PACKAGE_NAME}-${VERSION}/debian/" 2>/dev/null || true

    # Copy common files
    cp ../common/notepad-plus-plus.desktop "${PACKAGE_NAME}-${VERSION}/debian/"
    cp ../common/notepad-plus-plus-mime.xml "${PACKAGE_NAME}-${VERSION}/debian/"
    cp ../common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml "${PACKAGE_NAME}-${VERSION}/debian/"

    # Create orig tarball
    tar czf "${PACKAGE_NAME}_${VERSION}.orig.tar.gz" "${PACKAGE_NAME}-${VERSION}"

    cd ..
    print_info "Source package prepared"
}

# Build the package
build_package() {
    print_info "Building Debian package..."

    cd "$BUILD_DIR/${PACKAGE_NAME}-${VERSION}"

    # Build the package
    dpkg-buildpackage -us -uc -b

    cd ../..
    print_info "Package build completed"
}

# Build source package
build_source_package() {
    print_info "Building source package..."

    cd "$BUILD_DIR/${PACKAGE_NAME}-${VERSION}"

    # Build source package
    dpkg-buildpackage -us -uc -S

    cd ../..
    print_info "Source package build completed"
}

# Copy results
copy_results() {
    print_info "Copying build results..."

    mkdir -p "output"

    # Copy .deb files
    cp "$BUILD_DIR"/*.deb "output/" 2>/dev/null || true
    cp "$BUILD_DIR"/*.changes "output/" 2>/dev/null || true
    cp "$BUILD_DIR"/*.buildinfo "output/" 2>/dev/null || true
    cp "$BUILD_DIR"/*.dsc "output/" 2>/dev/null || true
    cp "$BUILD_DIR"/*.tar.gz "output/" 2>/dev/null || true

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

    sudo dpkg -i output/${PACKAGE_NAME}_${FULL_VERSION}_*.deb 2>/dev/null || {
        print_warn "Installation may have failed, trying to fix dependencies..."
        sudo apt-get install -f -y
    }

    print_info "Installation completed"
}

# Show usage
show_help() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build       Build binary packages (default)"
    echo "  source      Build source package"
    echo "  full        Build both binary and source packages"
    echo "  install     Build and install locally"
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
            prepare_source
            build_package
            copy_results
            print_info "Build completed successfully!"
            print_info "Packages are in output/"
            ;;
        source)
            check_dependencies
            prepare_source
            build_source_package
            copy_results
            print_info "Source package build completed!"
            ;;
        full)
            check_dependencies
            prepare_source
            build_source_package
            build_package
            copy_results
            print_info "Full build completed successfully!"
            ;;
        install)
            check_dependencies
            prepare_source
            build_package
            copy_results
            install_local
            print_info "Installation completed successfully!"
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
