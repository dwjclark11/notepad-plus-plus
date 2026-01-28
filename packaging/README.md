# Notepad++ Linux Packaging

This directory contains packaging configurations for various Linux distributions.

## Supported Package Formats

- **AppImage** - Universal Linux package that runs on most distributions
- **Flatpak** - Sandboxed application for Flathub and other repositories
- **Debian (.deb)** - For Debian, Ubuntu, Linux Mint, and derivatives
- **RPM (.rpm)** - For Fedora, RHEL, CentOS, openSUSE, and derivatives
- **Arch Linux (PKGBUILD)** - For Arch Linux, Manjaro, and derivatives

## Quick Start

### Universal Install Script

```bash
cd packaging
./install.sh
```

This will build and install Notepad++ to `/usr/local` by default. You can customize the installation:

```bash
# Install to /usr
sudo ./install.sh --prefix=/usr

# Build with debug symbols
./install.sh --debug

# Uninstall
sudo ./install.sh uninstall
```

## Package Format Specific Instructions

### AppImage

The AppImage is a self-contained executable that works on most Linux distributions without installation.

```bash
cd packaging/appimage
./build-appimage.sh
```

Output: `notepad-plus-plus-8.6.0-x86_64.AppImage`

Usage:
```bash
# Make executable and run
chmod +x notepad-plus-plus-8.6.0-x86_64.AppImage
./notepad-plus-plus-8.6.0-x86_64.AppImage

# Install desktop integration
./notepad-plus-plus-8.6.0-x86_64.AppImage --install-desktop-file
```

### Flatpak

Flatpak packages are sandboxed and distributed through Flathub or other repositories.

```bash
cd packaging/flatpak
./build-flatpak.sh
```

Commands:
- `build` - Build the Flatpak package
- `bundle` - Create a single-file bundle
- `install` - Build and install locally
- `run` - Build, install, and run
- `export` - Export for Flathub submission

### Debian Package

For Debian, Ubuntu, Linux Mint, and derivatives.

```bash
cd packaging/debian
./build-deb.sh
```

Commands:
- `build` - Build binary packages
- `source` - Build source package
- `full` - Build both binary and source packages
- `install` - Build and install locally

Output files will be in `output/` directory:
- `notepad-plus-plus_8.6.0-1_amd64.deb`
- `notepad-plus-plus-l10n_8.6.0-1_all.deb`
- `notepad-plus-plus-plugins_8.6.0-1_amd64.deb`

Install manually:
```bash
sudo dpkg -i output/*.deb
sudo apt-get install -f  # Fix any dependency issues
```

### RPM Package

For Fedora, RHEL, CentOS, openSUSE, and derivatives.

```bash
cd packaging/rpm
./build-rpm.sh
```

Commands:
- `build` - Build binary packages
- `full` - Build both binary and source packages
- `install` - Build and install locally

Output files will be in `output/` directory:
- `notepad-plus-plus-8.6.0-1.x86_64.rpm`
- `notepad-plus-plus-lang-8.6.0-1.x86_64.rpm`
- `notepad-plus-plus-plugins-8.6.0-1.x86_64.rpm`

Install manually:
```bash
sudo rpm -ivh output/*.rpm
# or
sudo dnf install output/*.rpm
```

### Arch Linux Package

For Arch Linux, Manjaro, and derivatives.

```bash
cd packaging/arch
./build-arch.sh
```

Commands:
- `build` - Build the package
- `build-only` - Build without dependency checking
- `install` - Build and install locally
- `srcinfo` - Update .SRCINFO file

Install manually:
```bash
makepkg -si
# or
sudo pacman -U notepad-plus-plus-8.6.0-1-x86_64.pkg.tar.zst
```

## Common Files

The `packaging/common/` directory contains shared resources:

- `notepad-plus-plus.desktop` - XDG desktop entry
- `notepad-plus-plus-mime.xml` - MIME type associations
- `org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml` - AppStream metadata
- `notepad-plus-plus.svg` - Application icon (to be added)

## Directory Structure

```
packaging/
├── common/                          # Shared packaging resources
│   ├── notepad-plus-plus.desktop
│   ├── notepad-plus-plus-mime.xml
│   └── org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml
├── appimage/                        # AppImage packaging
│   ├── AppRun
│   └── build-appimage.sh
├── flatpak/                         # Flatpak packaging
│   ├── org.notepad-plus-plus.NotepadPlusPlus.yml
│   └── build-flatpak.sh
├── debian/                          # Debian packaging
│   ├── debian/
│   │   ├── control
│   │   ├── rules
│   │   ├── changelog
│   │   ├── copyright
│   │   ├── postinst
│   │   ├── prerm
│   │   └── ...
│   └── build-deb.sh
├── rpm/                             # RPM packaging
│   ├── notepad-plus-plus.spec
│   └── build-rpm.sh
├── arch/                            # Arch Linux packaging
│   ├── PKGBUILD
│   ├── .SRCINFO
│   └── build-arch.sh
├── install.sh                       # Universal install script
└── README.md                        # This file
```

## Dependencies

### Build Dependencies

- CMake >= 3.10
- GCC >= 11 or Clang >= 14
- Qt6 >= 6.2 (Qt6Core, Qt6Widgets, Qt6Gui, Qt6Network)

### Runtime Dependencies

- Qt6 >= 6.2
- Standard C++ library supporting C++20

## Distribution-Specific Notes

### Debian/Ubuntu

Required packages for building:
```bash
sudo apt install build-essential cmake debhelper qt6-base-dev
```

### Fedora/RHEL/CentOS

Required packages for building:
```bash
sudo dnf install gcc-c++ cmake rpm-build qt6-qtbase-devel
```

### Arch Linux

Required packages for building:
```bash
sudo pacman -S base-devel cmake qt6-base
```

### openSUSE

Required packages for building:
```bash
sudo zypper install gcc-c++ cmake rpm-build qt6-base-devel
```

## Creating a Release

To create packages for all supported formats:

```bash
cd packaging

# Build all packages
./build-all.sh

# Or build individually
cd appimage && ./build-appimage.sh && cd ..
cd flatpak && ./build-flatpak.sh bundle && cd ..
cd debian && ./build-deb.sh && cd ..
cd rpm && ./build-rpm.sh && cd ..
cd arch && ./build-arch.sh && cd ..
```

All packages will be created in their respective `output/` directories.

## Contributing

When modifying packaging files:

1. Test changes on the target distribution
2. Ensure the package follows distribution guidelines
3. Update version numbers in all relevant files
4. Test installation and removal
5. Verify desktop integration works correctly

## Troubleshooting

### Qt6 Not Found

If CMake cannot find Qt6, ensure:
- Qt6 development packages are installed
- `Qt6_DIR` or `CMAKE_PREFIX_PATH` is set correctly

Example:
```bash
export CMAKE_PREFIX_PATH=/usr/lib/qt6/cmake
```

### Missing Icons

If the application icon doesn't appear, try:
```bash
sudo gtk-update-icon-cache -f /usr/share/icons/hicolor
sudo update-desktop-database /usr/share/applications
```

### MIME Types Not Working

Update the MIME database:
```bash
sudo update-mime-database /usr/share/mime
```

## License

The packaging files are released under the same license as Notepad++ (GPL v2 or later).
