#!/bin/bash
# Build script for Notepad++ Flatpak
# This script creates a Flatpak package for Notepad++

set -e

# Configuration
APP_ID="org.notepad-plus-plus.NotepadPlusPlus"
APP_VERSION="8.6.0"
BUILD_DIR="build-flatpak"
REPO_DIR="repo"

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

    if ! command -v flatpak &> /dev/null; then
        missing_deps+=("flatpak")
    fi

    if ! command -v flatpak-builder &> /dev/null; then
        missing_deps+=("flatpak-builder")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install them and try again"
        print_error "For example: sudo apt install flatpak flatpak-builder"
        exit 1
    fi

    # Check for required runtimes
    print_info "Checking Flatpak runtimes..."

    if ! flatpak list --runtime | grep -q "org.kde.Platform.*6.6"; then
        print_warn "KDE Platform 6.6 not found. Installing..."
        flatpak install -y flathub org.kde.Platform//6.6 org.kde.Sdk//6.6
    fi

    print_info "All required dependencies found"
}

# Build Flatpak
build_flatpak() {
    print_info "Building Flatpak package..."

    # Create build directory
    mkdir -p "$BUILD_DIR"
    mkdir -p "$REPO_DIR"

    # Build the Flatpak
    flatpak-builder \
        --force-clean \
        --repo="$REPO_DIR" \
        "$BUILD_DIR" \
        "${APP_ID}.yml"

    print_info "Flatpak build completed"
}

# Create single-file bundle
create_bundle() {
    print_info "Creating Flatpak bundle..."

    local bundle_name="${APP_ID}-${APP_VERSION}.flatpak"

    # Build bundle from repo
    flatpak build-bundle \
        "$REPO_DIR" \
        "$bundle_name" \
        "$APP_ID" \
        --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

    print_info "Flatpak bundle created: $bundle_name"
}

# Install locally for testing
install_local() {
    print_info "Installing Flatpak locally..."

    # Add local repo if not exists
    flatpak remote-add --user --no-gpg-verify notepad-local "$REPO_DIR" 2>/dev/null || true

    # Install or update
    flatpak install -y --user notepad-local "$APP_ID"

    print_info "Flatpak installed locally"
}

# Run the application
run_app() {
    print_info "Running Notepad++..."
    flatpak run "$APP_ID"
}

# Clean up
cleanup() {
    print_info "Cleaning up..."
    rm -rf "$BUILD_DIR"
    rm -rf "$REPO_DIR"
    print_info "Cleanup completed"
}

# Export to Flathub format
export_flathub() {
    print_info "Exporting for Flathub..."

    # Create flathub directory structure
    mkdir -p "flathub"

    # Copy manifest
    cp "${APP_ID}.yml" "flathub/"

    # Copy metainfo
    cp ../common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml "flathub/"

    # Copy desktop file
    cp ../common/notepad-plus-plus.desktop "flathub/${APP_ID}.desktop"

    # Copy icon
    if [ -f ../common/notepad-plus-plus.svg ]; then
        cp ../common/notepad-plus-plus.svg "flathub/${APP_ID}.svg"
    fi

    print_info "Flathub export completed in flathub/ directory"
}

# Show usage
show_help() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build       Build the Flatpak package (default)"
    echo "  bundle      Build and create single-file bundle"
    echo "  install     Build and install locally"
    echo "  run         Build, install, and run"
    echo "  export      Export for Flathub submission"
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
            build_flatpak
            print_info "Build completed successfully!"
            ;;
        bundle)
            check_dependencies
            build_flatpak
            create_bundle
            print_info "Bundle created successfully!"
            ;;
        install)
            check_dependencies
            build_flatpak
            install_local
            print_info "Installation completed successfully!"
            ;;
        run)
            check_dependencies
            build_flatpak
            install_local
            run_app
            ;;
        export)
            export_flathub
            print_info "Export completed!"
            ;;
        clean)
            cleanup
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
