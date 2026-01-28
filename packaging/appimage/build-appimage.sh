#!/bin/bash
# Build script for Notepad++ AppImage
# This script creates an AppImage package for Notepad++

set -e

# Configuration
APP_NAME="notepad-plus-plus"
APP_VERSION="8.6.0"
APP_DIR="AppDir"
BUILD_DIR="build-appimage"
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

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi

    if ! command -v linuxdeploy &> /dev/null && ! [ -f "linuxdeploy-x86_64.AppImage" ]; then
        print_warn "linuxdeploy not found, will download it"
    fi

    if ! command -v linuxdeploy-plugin-qt &> /dev/null && ! [ -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
        print_warn "linuxdeploy-plugin-qt not found, will download it"
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install them and try again"
        exit 1
    fi

    print_info "All required dependencies found"
}

# Download linuxdeploy tools
download_linuxdeploy() {
    print_info "Downloading linuxdeploy tools..."

    local linuxdeploy_url="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    local qt_plugin_url="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"

    if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
        wget -q "$linuxdeploy_url" -O linuxdeploy-x86_64.AppImage
        chmod +x linuxdeploy-x86_64.AppImage
    fi

    if [ ! -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
        wget -q "$qt_plugin_url" -O linuxdeploy-plugin-qt-x86_64.AppImage
        chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
    fi

    print_info "linuxdeploy tools downloaded"
}

# Build Notepad++
build_notepad() {
    print_info "Building Notepad++..."

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure with CMake
    print_info "Configuring with CMake..."
    cmake "$SOURCE_DIR/PowerEditor/src" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt6"

    # Build
    print_info "Compiling..."
    make -j$(nproc)

    cd ..
    print_info "Build completed"
}

# Create AppDir structure
create_appdir() {
    print_info "Creating AppDir structure..."

    # Clean and create AppDir
    rm -rf "$APP_DIR"
    mkdir -p "$APP_DIR/usr/bin"
    mkdir -p "$APP_DIR/usr/lib"
    mkdir -p "$APP_DIR/usr/share/applications"
    mkdir -p "$APP_DIR/usr/share/icons/hicolor/scalable/apps"
    mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"
    mkdir -p "$APP_DIR/usr/share/metainfo"
    mkdir -p "$APP_DIR/usr/share/mime/packages"

    # Copy binary
    cp "$BUILD_DIR/notepad-plus-plus" "$APP_DIR/usr/bin/"

    # Copy desktop file
    cp "$SOURCE_DIR/packaging/common/notepad-plus-plus.desktop" "$APP_DIR/"
    cp "$SOURCE_DIR/packaging/common/notepad-plus-plus.desktop" "$APP_DIR/usr/share/applications/"

    # Copy icons (create placeholder if not exists)
    if [ -f "$SOURCE_DIR/packaging/common/notepad-plus-plus.svg" ]; then
        cp "$SOURCE_DIR/packaging/common/notepad-plus-plus.svg" "$APP_DIR/"
        cp "$SOURCE_DIR/packaging/common/notepad-plus-plus.svg" "$APP_DIR/usr/share/icons/hicolor/scalable/apps/"
    fi

    if [ -f "$SOURCE_DIR/packaging/common/notepad-plus-plus.png" ]; then
        cp "$SOURCE_DIR/packaging/common/notepad-plus-plus.png" "$APP_DIR/usr/share/icons/hicolor/256x256/apps/"
    fi

    # Copy metainfo
    cp "$SOURCE_DIR/packaging/common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml" "$APP_DIR/usr/share/metainfo/"

    # Copy MIME type definition
    cp "$SOURCE_DIR/packaging/common/notepad-plus-plus-mime.xml" "$APP_DIR/usr/share/mime/packages/"

    # Copy AppRun script
    cp "$SOURCE_DIR/packaging/appimage/AppRun" "$APP_DIR/"
    chmod +x "$APP_DIR/AppRun"

    # Copy plugins directory structure
    mkdir -p "$APP_DIR/usr/lib/notepad-plus-plus/plugins"

    # Copy localization files if they exist
    if [ -d "$SOURCE_DIR/PowerEditor/installer/nativeLang" ]; then
        mkdir -p "$APP_DIR/usr/share/notepad-plus-plus/localization"
        cp -r "$SOURCE_DIR/PowerEditor/installer/nativeLang"/* "$APP_DIR/usr/share/notepad-plus-plus/localization/" 2>/dev/null || true
    fi

    # Copy themes if they exist
    if [ -d "$SOURCE_DIR/PowerEditor/installer/themes" ]; then
        mkdir -p "$APP_DIR/usr/share/notepad-plus-plus/themes"
        cp -r "$SOURCE_DIR/PowerEditor/installer/themes"/* "$APP_DIR/usr/share/notepad-plus-plus/themes/" 2>/dev/null || true
    fi

    print_info "AppDir structure created"
}

# Create AppImage
create_appimage() {
    print_info "Creating AppImage..."

    # Set environment variables for linuxdeploy
    export QMAKE="/usr/lib/qt6/bin/qmake"
    export QT_PLUGIN_PATH="/usr/lib/qt6/plugins"

    # Run linuxdeploy
    ./linuxdeploy-x86_64.AppImage \
        --appdir "$APP_DIR" \
        --plugin qt \
        --output appimage \
        -d "$APP_DIR/notepad-plus-plus.desktop" \
        -i "$APP_DIR/usr/share/icons/hicolor/scalable/apps/notepad-plus-plus.svg" 2>/dev/null || \
    ./linuxdeploy-x86_64.AppImage \
        --appdir "$APP_DIR" \
        --plugin qt \
        --output appimage \
        -d "$APP_DIR/notepad-plus-plus.desktop"

    # Rename the output file
    local output_name="${APP_NAME}-${APP_VERSION}-x86_64.AppImage"
    mv *.AppImage "$output_name" 2>/dev/null || true

    print_info "AppImage created: $output_name"
}

# Add update information
add_update_info() {
    print_info "Adding update information..."

    local appimage_file="${APP_NAME}-${APP_VERSION}-x86_64.AppImage"

    if [ -f "$appimage_file" ]; then
        # Add zsync update information
        ./linuxdeploy-x86_64.AppImage --appimage-updateinformation \
            "gh-releases-zsync|notepad-plus-plus|notepad-plus-plus|latest|${APP_NAME}-*-x86_64.AppImage.zsync" \
            "$appimage_file" 2>/dev/null || print_warn "Could not add update information"

        # Make executable
        chmod +x "$appimage_file"

        print_info "Update information added"
    fi
}

# Clean up
cleanup() {
    print_info "Cleaning up..."
    rm -rf "$BUILD_DIR"
    rm -rf "$APP_DIR"
    print_info "Cleanup completed"
}

# Main function
main() {
    print_info "Building Notepad++ AppImage v${APP_VERSION}"

    # Parse arguments
    local clean_after=false

    while [[ $# -gt 0 ]]; do
        case $1 in
            --clean)
                clean_after=true
                shift
                ;;
            --help|-h)
                echo "Usage: $0 [options]"
                echo ""
                echo "Options:"
                echo "  --clean     Clean up build files after creating AppImage"
                echo "  --help, -h  Show this help message"
                echo ""
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    # Run build steps
    check_dependencies
    download_linuxdeploy
    build_notepad
    create_appdir
    create_appimage
    add_update_info

    if [ "$clean_after" = true ]; then
        cleanup
    fi

    print_info "AppImage build completed successfully!"
    print_info "Output: ${APP_NAME}-${APP_VERSION}-x86_64.AppImage"
}

# Run main function
main "$@"
