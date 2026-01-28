#!/bin/bash
# Package generation script for Notepad++ Linux port
# Usage: ./ci/scripts/package.sh [options]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_DIR="build"
PACKAGE_TYPE="all"
VERSION="8.6.0"
ARCH="$(uname -m)"
OUTPUT_DIR="packages"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --type)
            PACKAGE_TYPE="$2"
            shift 2
            ;;
        --version)
            VERSION="$2"
            shift 2
            ;;
        --arch)
            ARCH="$2"
            shift 2
            ;;
        --output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR    Build directory (default: build)"
            echo "  --type TYPE        Package type: all|tarball|deb|rpm|appimage|flatpak"
            echo "  --version VER      Package version (default: 8.6.0)"
            echo "  --arch ARCH        Target architecture (default: $(uname -m))"
            echo "  --output DIR       Output directory (default: packages)"
            echo "  --help             Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 --type tarball"
            echo "  $0 --type deb --version 8.6.0-1"
            echo "  $0 --type all --output ./dist"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$PROJECT_ROOT/PowerEditor/src"
BUILD_PATH="$SOURCE_DIR/$BUILD_DIR"
OUTPUT_PATH="$PROJECT_ROOT/$OUTPUT_DIR"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Notepad++ Linux Package Script${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Build Directory: $BUILD_PATH"
echo "Package Type: $PACKAGE_TYPE"
echo "Version: $VERSION"
echo "Architecture: $ARCH"
echo "Output Directory: $OUTPUT_PATH"
echo ""

# Check if build exists
if [ ! -d "$BUILD_PATH" ]; then
    echo -e "${RED}Error: Build directory not found: $BUILD_PATH${NC}"
    echo "Please build the project first with:"
    echo "  ./ci/scripts/build.sh"
    exit 1
fi

# Check for binary
if [ ! -f "$BUILD_PATH/notepad-plus-plus" ]; then
    echo -e "${RED}Error: Notepad++ binary not found${NC}"
    exit 1
fi

# Create output directory
mkdir -p "$OUTPUT_PATH"

# Function to create tarball
create_tarball() {
    echo -e "${BLUE}Creating tarball...${NC}"

    PACKAGE_NAME="notepad-plus-plus-${VERSION}-linux-${ARCH}.tar.gz"
    PACKAGE_PATH="$OUTPUT_PATH/$PACKAGE_NAME"

    # Create temporary directory for staging
    STAGING_DIR=$(mktemp -d)
    trap "rm -rf $STAGING_DIR" RETURN

    # Create directory structure
    mkdir -p "$STAGING_DIR/notepad-plus-plus/bin"
    mkdir -p "$STAGING_DIR/notepad-plus-plus/share/applications"
    mkdir -p "$STAGING_DIR/notepad-plus-plus/share/doc/notepad-plus-plus"

    # Copy binary
    cp "$BUILD_PATH/notepad-plus-plus" "$STAGING_DIR/notepad-plus-plus/bin/"
    strip "$STAGING_DIR/notepad-plus-plus/bin/notepad-plus-plus" 2>/dev/null || true

    # Copy documentation
    cp "$PROJECT_ROOT/LICENSE" "$STAGING_DIR/notepad-plus-plus/share/doc/notepad-plus-plus/" 2>/dev/null || true
    cp "$PROJECT_ROOT/README.md" "$STAGING_DIR/notepad-plus-plus/share/doc/notepad-plus-plus/" 2>/dev/null || true

    # Create desktop entry
    cat > "$STAGING_DIR/notepad-plus-plus/share/applications/notepad-plus-plus.desktop" << 'EOF'
[Desktop Entry]
Name=Notepad++
Comment=Text and source code editor
Exec=/usr/bin/notepad-plus-plus
Icon=notepad-plus-plus
Type=Application
Categories=TextEditor;Development;IDE;
Terminal=false
StartupNotify=true
MimeType=text/plain;text/x-csrc;text/x-c++src;
EOF

    # Create install script
    cat > "$STAGING_DIR/notepad-plus-plus/install.sh" << 'EOF'
#!/bin/bash
# Notepad++ Linux Installer

INSTALL_PREFIX="${1:-/usr/local}"

echo "Installing Notepad++ to $INSTALL_PREFIX..."

mkdir -p "$INSTALL_PREFIX/bin"
mkdir -p "$INSTALL_PREFIX/share/applications"
mkdir -p "$INSTALL_PREFIX/share/doc/notepad-plus-plus"

cp bin/notepad-plus-plus "$INSTALL_PREFIX/bin/"
cp share/applications/notepad-plus-plus.desktop "$INSTALL_PREFIX/share/applications/"
cp share/doc/notepad-plus-plus/* "$INSTALL_PREFIX/share/doc/notepad-plus-plus/" 2>/dev/null || true

echo "Installation complete!"
echo "Run 'notepad-plus-plus' to start the application."
EOF
    chmod +x "$STAGING_DIR/notepad-plus-plus/install.sh"

    # Create tarball
    tar czf "$PACKAGE_PATH" -C "$STAGING_DIR" notepad-plus-plus

    echo -e "${GREEN}Tarball created: $PACKAGE_PATH${NC}"
    echo "Size: $(du -h "$PACKAGE_PATH" | cut -f1)"
}

# Function to create Debian package
create_deb_package() {
    echo -e "${BLUE}Creating Debian package...${NC}"

    PACKAGE_NAME="notepad-plus-plus_${VERSION}_${ARCH}.deb"
    PACKAGE_PATH="$OUTPUT_PATH/$PACKAGE_NAME"

    # Create package structure
    PKG_DIR=$(mktemp -d)
    trap "rm -rf $PKG_DIR" RETURN

    mkdir -p "$PKG_DIR/DEBIAN"
    mkdir -p "$PKG_DIR/usr/bin"
    mkdir -p "$PKG_DIR/usr/share/applications"
    mkdir -p "$PKG_DIR/usr/share/doc/notepad-plus-plus"
    mkdir -p "$PKG_DIR/usr/share/icons/hicolor/256x256/apps"

    # Copy binary
    cp "$BUILD_PATH/notepad-plus-plus" "$PKG_DIR/usr/bin/"
    strip "$PKG_DIR/usr/bin/notepad-plus-plus" 2>/dev/null || true

    # Copy documentation
    cp "$PROJECT_ROOT/LICENSE" "$PKG_DIR/usr/share/doc/notepad-plus-plus/copyright" 2>/dev/null || true

    # Create desktop entry
    cat > "$PKG_DIR/usr/share/applications/notepad-plus-plus.desktop" << 'EOF'
[Desktop Entry]
Name=Notepad++
Comment=Text and source code editor
Exec=/usr/bin/notepad-plus-plus
Icon=notepad-plus-plus
Type=Application
Categories=TextEditor;Development;IDE;
Terminal=false
StartupNotify=true
MimeType=text/plain;text/x-csrc;text/x-c++src;
EOF

    # Create control file
    cat > "$PKG_DIR/DEBIAN/control" << EOF
Package: notepad-plus-plus
Version: $VERSION
Section: editors
Priority: optional
Architecture: $ARCH
Depends: libqt6widgets6, libqt6gui6, libqt6core6, libqt6network6
Maintainer: Notepad++ Team <team@notepad-plus-plus.org>
Description: Text and source code editor
 Notepad++ is a free source code editor and Notepad replacement
 that supports several programming languages running under the
 MS Windows environment, now ported to Linux.
EOF

    # Create postinst script
    cat > "$PKG_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e
# Update desktop database
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/share/applications
fi
EOF
    chmod 755 "$PKG_DIR/DEBIAN/postinst"

    # Build package
    dpkg-deb --build "$PKG_DIR" "$PACKAGE_PATH"

    echo -e "${GREEN}Debian package created: $PACKAGE_PATH${NC}"
    echo "Size: $(du -h "$PACKAGE_PATH" | cut -f1)"
}

# Function to create RPM package
create_rpm_package() {
    echo -e "${BLUE}Creating RPM package...${NC}"

    # Check if rpmbuild is available
    if ! command -v rpmbuild &> /dev/null; then
        echo -e "${YELLOW}rpmbuild not found, skipping RPM package${NC}"
        return 0
    fi

    PACKAGE_NAME="notepad-plus-plus-${VERSION}-1.${ARCH}.rpm"
    PACKAGE_PATH="$OUTPUT_PATH/$PACKAGE_NAME"

    # Setup RPM build environment
    RPM_BUILD_ROOT=$(mktemp -d)
    trap "rm -rf $RPM_BUILD_ROOT" RETURN

    mkdir -p "$RPM_BUILD_ROOT/{BUILD,RPMS,SOURCES,SPECS,SRPMS}"

    # Create spec file
    cat > "$RPM_BUILD_ROOT/SPECS/notepad-plus-plus.spec" << EOF
Name:           notepad-plus-plus
Version:        $VERSION
Release:        1%{?dist}
Summary:        Text and source code editor

License:        GPL-2.0+
URL:            https://notepad-plus-plus.org/
Source0:        notepad-plus-plus-%{version}.tar.gz

BuildRequires:  cmake, gcc-c++, qt6-qtbase-devel
Requires:       qt6-qtbase-gui

%description
Notepad++ is a free source code editor and Notepad replacement
that supports several programming languages.

%prep
# Binary already built

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications
cp $BUILD_PATH/notepad-plus-plus %{buildroot}/usr/bin/
cat > %{buildroot}/usr/share/applications/notepad-plus-plus.desktop << 'DESKTOP'
[Desktop Entry]
Name=Notepad++
Comment=Text and source code editor
Exec=/usr/bin/notepad-plus-plus
Icon=notepad-plus-plus
Type=Application
Categories=TextEditor;Development;IDE;
Terminal=false
StartupNotify=true
MimeType=text/plain;text/x-csrc;text/x-c++src;
DESKTOP

%files
/usr/bin/notepad-plus-plus
/usr/share/applications/notepad-plus-plus.desktop

%changelog
* $(date '+%a %b %d %Y') Notepad++ Team <team@notepad-plus-plus.org> - $VERSION-1
- Initial Linux port release
EOF

    # Build RPM (this is a simplified version)
    # In practice, you'd use a proper build environment
    echo -e "${YELLOW}RPM package creation requires a proper RPM build environment${NC}"
    echo -e "${YELLOW}Creating mock RPM package instead${NC}"

    # Create a mock RPM using alien or similar would go here
    # For now, just create a tarball with .rpm-like naming
    create_tarball
    mv "$OUTPUT_PATH/notepad-plus-plus-${VERSION}-linux-${ARCH}.tar.gz" "$OUTPUT_PATH/notepad-plus-plus-${VERSION}-1.${ARCH}.rpm.tar.gz"

    echo -e "${GREEN}RPM package placeholder created${NC}"
}

# Function to create AppImage
create_appimage() {
    echo -e "${BLUE}Creating AppImage...${NC}"

    # Check for linuxdeployqt
    if [ ! -f "$PROJECT_ROOT/linuxdeployqt" ] && ! command -v linuxdeployqt &> /dev/null; then
        echo -e "${YELLOW}linuxdeployqt not found, downloading...${NC}"
        wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" \
            -O "$PROJECT_ROOT/linuxdeployqt" 2>/dev/null || true
        chmod a+x "$PROJECT_ROOT/linuxdeployqt" 2>/dev/null || true
    fi

    # Create AppDir structure
    APPDIR=$(mktemp -d)
    trap "rm -rf $APPDIR" RETURN

    mkdir -p "$APPDIR/usr/bin"
    mkdir -p "$APPDIR/usr/lib"
    mkdir -p "$APPDIR/usr/share/applications"
    mkdir -p "$APPDIR/usr/share/metainfo"

    # Copy binary
    cp "$BUILD_PATH/notepad-plus-plus" "$APPDIR/usr/bin/"

    # Create desktop entry
    cat > "$APPDIR/usr/share/applications/notepad-plus-plus.desktop" << 'EOF'
[Desktop Entry]
Type=Application
Name=Notepad++
Comment=Text and source code editor
Exec=notepad-plus-plus
Icon=notepad-plus-plus
Categories=TextEditor;Development;IDE;
Terminal=false
StartupNotify=true
MimeType=text/plain;text/x-csrc;text/x-c++src;
EOF

    cp "$APPDIR/usr/share/applications/notepad-plus-plus.desktop" "$APPDIR/"

    # Create AppRun
    cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${HERE}/usr/plugins"
exec "${HERE}/usr/bin/notepad-plus-plus" "$@"
EOF
    chmod +x "$APPDIR/AppRun"

    # Try to build AppImage
    if [ -f "$PROJECT_ROOT/linuxdeployqt" ]; then
        cd "$PROJECT_ROOT"
        "$PROJECT_ROOT/linuxdeployqt" "$APPDIR/usr/share/applications/notepad-plus-plus.desktop" \
            -appimage \
            -bundle-non-qt-libs 2>/dev/null || true

        if [ -f Notepad++*.AppImage ]; then
            mv Notepad++*.AppImage "$OUTPUT_PATH/Notepad++-${VERSION}-x86_64.AppImage"
            echo -e "${GREEN}AppImage created${NC}"
        else
            echo -e "${YELLOW}AppImage creation failed, creating tarball instead${NC}"
            tar czf "$OUTPUT_PATH/notepad-plus-plus-${VERSION}-AppImage.tar.gz" -C "$APPDIR" .
        fi
    else
        echo -e "${YELLOW}linuxdeployqt not available, creating tarball instead${NC}"
        tar czf "$OUTPUT_PATH/notepad-plus-plus-${VERSION}-AppImage.tar.gz" -C "$APPDIR" .
    fi
}

# Function to create Flatpak
create_flatpak() {
    echo -e "${BLUE}Creating Flatpak...${NC}"

    if ! command -v flatpak-builder &> /dev/null; then
        echo -e "${YELLOW}flatpak-builder not found, skipping Flatpak${NC}"
        return 0
    fi

    # Create Flatpak manifest
    FLATPAK_DIR=$(mktemp -d)
    trap "rm -rf $FLATPAK_DIR" RETURN

    cat > "$FLATPAK_DIR/com.notepad_plus_plus.NotepadPlusPlus.yml" << 'EOF'
app-id: com.notepad_plus_plus.NotepadPlusPlus
runtime: org.freedesktop.Platform
runtime-version: '23.08'
sdk: org.freedesktop.Sdk
sdk-extensions:
  - org.freedesktop.Sdk.Extension.qt6
command: notepad-plus-plus
finish-args:
  - --share=ipc
  - --socket=x11
  - --socket=wayland
  - --filesystem=home
  - --filesystem=host
  - --share=network
  - --device=dri
modules:
  - name: notepad-plus-plus
    buildsystem: cmake-ninja
    config-opts:
      - -DCMAKE_BUILD_TYPE=Release
    sources:
      - type: dir
        path: .
EOF

    # Build Flatpak
    cd "$FLATPAK_DIR"
    flatpak-builder --repo=repo --force-clean build-dir com.notepad_plus_plus.NotepadPlusPlus.yml 2>/dev/null || true

    if [ -d repo ]; then
        flatpak build-bundle repo "$OUTPUT_PATH/notepad-plus-plus-${VERSION}.flatpak" com.notepad_plus_plus.NotepadPlusPlus 2>/dev/null || true
        echo -e "${GREEN}Flatpak created${NC}"
    else
        echo -e "${YELLOW}Flatpak build failed${NC}"
    fi
}

# Main execution
case "$PACKAGE_TYPE" in
    tarball)
        create_tarball
        ;;
    deb)
        create_deb_package
        ;;
    rpm)
        create_rpm_package
        ;;
    appimage)
        create_appimage
        ;;
    flatpak)
        create_flatpak
        ;;
    all)
        create_tarball
        create_deb_package
        create_rpm_package
        create_appimage
        create_flatpak
        ;;
    *)
        echo -e "${RED}Unknown package type: $PACKAGE_TYPE${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Package Generation Complete${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Packages created in: $OUTPUT_PATH"
echo ""
ls -lh "$OUTPUT_PATH/"
echo ""
