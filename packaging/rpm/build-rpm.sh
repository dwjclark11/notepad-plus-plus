#!/bin/bash
# Build script for Notepad++ RPM package
# This script creates .rpm packages for Notepad++

set -e

# Configuration
PACKAGE_NAME="notepad-plus-plus"
VERSION="8.6.0"
RELEASE="1"
BUILD_DIR="build-rpm"
SOURCE_DIR="../.."
RPM_TOPDIR="$(pwd)/$BUILD_DIR/rpmbuild"

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

    if ! command -v rpmbuild &> /dev/null; then
        missing_deps+=("rpm-build")
    fi

    if ! command -v rpmspec &> /dev/null; then
        missing_deps+=("rpmdevtools")
    fi

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install them and try again"
        print_error "For example: sudo dnf install ${missing_deps[*]}"
        exit 1
    fi

    print_info "All required dependencies found"
}

# Setup RPM build environment
setup_rpmbuild() {
    print_info "Setting up RPM build environment..."

    # Create RPM directory structure
    mkdir -p "$RPM_TOPDIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

    print_info "RPM build environment created"
}

# Prepare sources
prepare_sources() {
    print_info "Preparing sources..."

    # Create source tarball
    cd "$BUILD_DIR"

    local tarball_name="${PACKAGE_NAME}-${VERSION}.tar.gz"
    local source_dir="${PACKAGE_NAME}-${VERSION}"

    # Create source directory
    mkdir -p "$source_dir"

    # Copy source files
    cp -r "$SOURCE_DIR"/* "$source_dir/" 2>/dev/null || true

    # Remove unnecessary files
    rm -rf "$source_dir/.git"
    rm -rf "$source_dir/build*"
    rm -rf "$source_dir/packaging"

    # Create tarball
    tar czf "rpmbuild/SOURCES/$tarball_name" "$source_dir"

    # Copy additional source files
    cp ../common/notepad-plus-plus.desktop rpmbuild/SOURCES/
    cp ../common/notepad-plus-plus-mime.xml rpmbuild/SOURCES/
    cp ../common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml rpmbuild/SOURCES/

    # Copy icon if exists
    if [ -f ../common/notepad-plus-plus.svg ]; then
        cp ../common/notepad-plus-plus.svg rpmbuild/SOURCES/
    fi

    # Copy spec file
    cp ../rpm/notepad-plus-plus.spec rpmbuild/SPECS/

    # Clean up
    rm -rf "$source_dir"

    cd ..
    print_info "Sources prepared"
}

# Build RPM package
build_rpm() {
    print_info "Building RPM package..."

    # Build the package
    rpmbuild \
        --define "_topdir $RPM_TOPDIR" \
        --define "_builddir $RPM_TOPDIR/BUILD" \
        --define "_rpmdir $RPM_TOPDIR/RPMS" \
        --define "_sourcedir $RPM_TOPDIR/SOURCES" \
        --define "_specdir $RPM_TOPDIR/SPECS" \
        --define "_srcrpmdir $RPM_TOPDIR/SRPMS" \
        -ba "$RPM_TOPDIR/SPECS/notepad-plus-plus.spec" 2>&1 || {
        print_error "RPM build failed"
        exit 1
    }

    print_info "RPM build completed"
}

# Build binary only
build_binary() {
    print_info "Building binary RPM package..."

    # Build the package
    rpmbuild \
        --define "_topdir $RPM_TOPDIR" \
        --define "_builddir $RPM_TOPDIR/BUILD" \
        --define "_rpmdir $RPM_TOPDIR/RPMS" \
        --define "_sourcedir $RPM_TOPDIR/SOURCES" \
        --define "_specdir $RPM_TOPDIR/SPECS" \
        --define "_srcrpmdir $RPM_TOPDIR/SRPMS" \
        -bb "$RPM_TOPDIR/SPECS/notepad-plus-plus.spec" 2>&1 || {
        print_error "RPM build failed"
        exit 1
    }

    print_info "RPM build completed"
}

# Copy results
copy_results() {
    print_info "Copying build results..."

    mkdir -p "output"

    # Copy RPM files
    find "$RPM_TOPDIR/RPMS" -name "*.rpm" -exec cp {} output/ \;
    find "$RPM_TOPDIR/SRPMS" -name "*.rpm" -exec cp {} output/ \; 2>/dev/null || true

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

    sudo rpm -ivh output/${PACKAGE_NAME}-${VERSION}-${RELEASE}.*.rpm 2>/dev/null || {
        print_warn "Installation may have failed, trying upgrade..."
        sudo rpm -Uvh output/${PACKAGE_NAME}-${VERSION}-${RELEASE}.*.rpm
    }

    print_info "Installation completed"
}

# Show usage
show_help() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build       Build binary packages (default)"
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
            setup_rpmbuild
            prepare_sources
            build_binary
            copy_results
            print_info "Build completed successfully!"
            print_info "Packages are in output/"
            ;;
        full)
            check_dependencies
            setup_rpmbuild
            prepare_sources
            build_rpm
            copy_results
            print_info "Full build completed successfully!"
            ;;
        install)
            check_dependencies
            setup_rpmbuild
            prepare_sources
            build_binary
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
