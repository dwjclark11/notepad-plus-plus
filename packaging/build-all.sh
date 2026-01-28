#!/bin/bash
# Master build script for all Notepad++ Linux packages
# This script builds packages for all supported formats

set -e

# Configuration
VERSION="8.6.0"
OUTPUT_DIR="$(pwd)/output"

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

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

# Create output directory
setup_output() {
    print_step "Setting up output directory..."
    mkdir -p "$OUTPUT_DIR"
    print_info "Output directory: $OUTPUT_DIR"
}

# Build AppImage
build_appimage() {
    print_header "Building AppImage"

    if [ ! -d "appimage" ]; then
        print_warn "AppImage directory not found, skipping..."
        return 0
    fi

    cd appimage

    if [ -f "build-appimage.sh" ]; then
        ./build-appimage.sh || {
            print_error "AppImage build failed"
            return 1
        }

        # Copy results
        if ls *.AppImage 1> /dev/null 2>&1; then
            cp *.AppImage "$OUTPUT_DIR/"
            print_info "AppImage built successfully"
        else
            print_warn "No AppImage file found"
        fi
    else
        print_warn "build-appimage.sh not found"
    fi

    cd ..
}

# Build Flatpak
build_flatpak() {
    print_header "Building Flatpak"

    if [ ! -d "flatpak" ]; then
        print_warn "Flatpak directory not found, skipping..."
        return 0
    fi

    cd flatpak

    if [ -f "build-flatpak.sh" ]; then
        ./build-flatpak.sh bundle || {
            print_error "Flatpak build failed"
            return 1
        }

        # Copy results
        if ls *.flatpak 1> /dev/null 2>&1; then
            cp *.flatpak "$OUTPUT_DIR/"
            print_info "Flatpak built successfully"
        else
            print_warn "No Flatpak file found"
        fi
    else
        print_warn "build-flatpak.sh not found"
    fi

    cd ..
}

# Build Debian package
build_debian() {
    print_header "Building Debian Package"

    if [ ! -d "debian" ]; then
        print_warn "Debian directory not found, skipping..."
        return 0
    fi

    cd debian

    if [ -f "build-deb.sh" ]; then
        ./build-deb.sh || {
            print_error "Debian build failed"
            return 1
        }

        # Copy results
        if [ -d "output" ] && ls output/*.deb 1> /dev/null 2>&1; then
            cp output/*.deb "$OUTPUT_DIR/"
            print_info "Debian packages built successfully"
        else
            print_warn "No Debian packages found"
        fi
    else
        print_warn "build-deb.sh not found"
    fi

    cd ..
}

# Build RPM package
build_rpm() {
    print_header "Building RPM Package"

    if [ ! -d "rpm" ]; then
        print_warn "RPM directory not found, skipping..."
        return 0
    fi

    cd rpm

    if [ -f "build-rpm.sh" ]; then
        ./build-rpm.sh || {
            print_error "RPM build failed"
            return 1
        }

        # Copy results
        if [ -d "output" ] && ls output/*.rpm 1> /dev/null 2>&1; then
            cp output/*.rpm "$OUTPUT_DIR/"
            print_info "RPM packages built successfully"
        else
            print_warn "No RPM packages found"
        fi
    else
        print_warn "build-rpm.sh not found"
    fi

    cd ..
}

# Build Arch package
build_arch() {
    print_header "Building Arch Linux Package"

    if [ ! -d "arch" ]; then
        print_warn "Arch directory not found, skipping..."
        return 0
    fi

    cd arch

    if [ -f "build-arch.sh" ]; then
        ./build-arch.sh || {
            print_error "Arch build failed"
            return 1
        }

        # Copy results
        if [ -d "output" ] && ls output/*.pkg.tar.* 1> /dev/null 2>&1; then
            cp output/*.pkg.tar.* "$OUTPUT_DIR/"
            print_info "Arch packages built successfully"
        else
            print_warn "No Arch packages found"
        fi
    else
        print_warn "build-arch.sh not found"
    fi

    cd ..
}

# Clean all build directories
clean_all() {
    print_step "Cleaning all build directories..."

    for dir in appimage flatpak debian rpm arch; do
        if [ -d "$dir" ]; then
            cd "$dir"
            if [ -f "build-*.sh" ]; then
                ./build-*.sh clean 2>/dev/null || true
            fi
            rm -rf build* output *.AppImage *.flatpak 2>/dev/null || true
            cd ..
        fi
    done

    rm -rf "$OUTPUT_DIR"

    print_info "Cleanup completed"
}

# List available formats
list_formats() {
    echo "Available package formats:"
    echo ""
    echo "  appimage    - Universal Linux package"
    echo "  flatpak     - Sandboxed application"
    echo "  debian      - Debian/Ubuntu package"
    echo "  rpm         - Fedora/RHEL/openSUSE package"
    echo "  arch        - Arch Linux package"
    echo "  all         - Build all formats (default)"
    echo ""
}

# Show usage
show_help() {
    echo "Usage: $0 [command] [format]"
    echo ""
    echo "Commands:"
    echo "  build       Build packages (default)"
    echo "  clean       Clean all build files"
    echo "  list        List available formats"
    echo "  help        Show this help message"
    echo ""
    echo "Formats:"
    echo "  all         Build all formats (default)"
    echo "  appimage    - Build AppImage only"
    echo "  flatpak     - Build Flatpak only"
    echo "  debian      - Build Debian package only"
    echo "  rpm         - Build RPM package only"
    echo "  arch        - Build Arch package only"
    echo ""
    echo "Examples:"
    echo "  $0                      # Build all packages"
    echo "  $0 build appimage       # Build AppImage only"
    echo "  $0 build debian,rpm     # Build Debian and RPM"
    echo "  $0 clean                # Clean all build files"
    echo ""
}

# Print summary
print_summary() {
    print_header "Build Summary"

    echo "Version: $VERSION"
    echo "Output directory: $OUTPUT_DIR"
    echo ""

    if [ -d "$OUTPUT_DIR" ]; then
        echo "Generated packages:"
        ls -lh "$OUTPUT_DIR/" 2>/dev/null || echo "  (none)"
    else
        echo "No packages generated"
    fi

    echo ""
    print_info "Build process completed!"
}

# Main function
main() {
    local command="${1:-build}"
    local formats="${2:-all}"

    case "$command" in
        build|"")
            setup_output

            if [ "$formats" = "all" ]; then
                build_appimage
                build_flatpak
                build_debian
                build_rpm
                build_arch
            else
                # Parse comma-separated formats
                IFS=',' read -ra FORMAT_ARRAY <<< "$formats"
                for format in "${FORMAT_ARRAY[@]}"; do
                    case "$format" in
                        appimage) build_appimage ;;
                        flatpak) build_flatpak ;;
                        debian) build_debian ;;
                        rpm) build_rpm ;;
                        arch) build_arch ;;
                        *)
                            print_error "Unknown format: $format"
                            list_formats
                            exit 1
                            ;;
                    esac
                done
            fi

            print_summary
            ;;
        clean)
            clean_all
            ;;
        list)
            list_formats
            ;;
        help|--help|-h)
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
