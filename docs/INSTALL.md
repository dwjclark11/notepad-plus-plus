# Installing Notepad++ on Linux

This guide covers various methods to install Notepad++ on Linux systems.

## Table of Contents

- [Installation Methods](#installation-methods)
- [Package Manager Installation](#package-manager-installation)
- [Universal Packages](#universal-packages)
- [Building from Source](#building-from-source)
- [Post-Installation Configuration](#post-installation-configuration)
- [Troubleshooting](#troubleshooting)

## Installation Methods

### Quick Comparison

| Method | Difficulty | Auto-Updates | Best For |
|--------|------------|--------------|----------|
| Package Manager | Easy | Yes | Regular users |
| Flatpak | Easy | Yes | Cross-distribution compatibility |
| AppImage | Easy | No | Portable usage |
| Build from Source | Hard | No | Developers, bleeding edge |

## Package Manager Installation

### Ubuntu / Debian (Coming Soon)

```bash
# Add Notepad++ PPA (when available)
sudo add-apt-repository ppa:notepad-plus-plus/stable
sudo apt update
sudo apt install notepad-plus-plus
```

### Fedora / RHEL

```bash
# Using DNF (when available in repositories)
sudo dnf install notepad-plus-plus

# Or using COPR
sudo dnf copr enable notepad-plus-plus/stable
sudo dnf install notepad-plus-plus
```

### Arch Linux (AUR)

```bash
# Using yay
yay -S notepad-plus-plus

# Using paru
paru -S notepad-plus-plus

# Manual build from AUR
git clone https://aur.archlinux.org/notepad-plus-plus.git
cd notepad-plus-plus
makepkg -si
```

### openSUSE

```bash
# Using Zypper (when available)
sudo zypper install notepad-plus-plus

# Or from OBS (Open Build Service)
sudo zypper addrepo https://download.opensuse.org/repositories/editors/openSUSE_Tumbleweed/editors.repo
sudo zypper refresh
sudo zypper install notepad-plus-plus
```

## Universal Packages

### Flatpak

Flatpak provides a sandboxed, distribution-independent installation.

```bash
# Install Flatpak if not already installed
# Ubuntu/Debian
sudo apt install flatpak

# Fedora
sudo dnf install flatpak

# Add Flathub repository
sudo flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Install Notepad++ (when available)
flatpak install flathub com.notepad-plus-plus.NotepadPlusPlus

# Run
flatpak run com.notepad-plus-plus.NotepadPlusPlus
```

#### Flatpak Permissions

```bash
# Grant access to host filesystem
flatpak override com.notepad-plus-plus.NotepadPlusPlus --filesystem=host

# Grant access to specific directories
flatpak override com.notepad-plus-plus.NotepadPlusPlus --filesystem=~/Documents
flatpak override com.notepad-plus-plus.NotepadPlusPlus --filesystem=~/Projects

# View current permissions
flatpak info --show-permissions com.notepad-plus-plus.NotepadPlusPlus
```

### AppImage

AppImage provides a portable, single-file executable.

```bash
# Download the latest AppImage
wget https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/vX.X.X/NotepadPlusPlus-X.X.X-x86_64.AppImage

# Make executable
chmod +x NotepadPlusPlus-X.X.X-x86_64.AppImage

# Run
./NotepadPlusPlus-X.X.X-x86_64.AppImage

# Optional: Move to applications directory
mkdir -p ~/.local/bin
mv NotepadPlusPlus-X.X.X-x86_64.AppImage ~/.local/bin/notepad-plus-plus

# Create desktop entry
mkdir -p ~/.local/share/applications
cat > ~/.local/share/applications/notepad-plus-plus.desktop << 'EOF'
[Desktop Entry]
Name=Notepad++
Comment=Text and source code editor
Exec=/home/USERNAME/.local/bin/notepad-plus-plus
Icon=notepad-plus-plus
Type=Application
Categories=TextEditor;Development;IDE;
MimeType=text/plain;text/x-c++src;text/x-csrc;
EOF

# Update desktop database
update-desktop-database ~/.local/share/applications
```

## Building from Source

See [BUILD.md](BUILD.md) for detailed build instructions.

Quick start:

```bash
# Clone repository
git clone https://github.com/notepad-plus-plus/notepad-plus-plus.git
cd notepad-plus-plus

# Install dependencies (Ubuntu/Debian example)
sudo apt install build-essential cmake qt6-base-dev

# Build
mkdir build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install locally
sudo make install
```

## Post-Installation Configuration

### Desktop Integration

#### Create Desktop Entry

```bash
# System-wide installation
sudo tee /usr/share/applications/notepad-plus-plus.desktop > /dev/null << 'EOF'
[Desktop Entry]
Version=1.0
Type=Application
Name=Notepad++
GenericName=Text Editor
Comment=Free source code editor and Notepad replacement
Exec=notepad-plus-plus %F
Icon=notepad-plus-plus
Terminal=false
MimeType=text/plain;text/x-c++src;text/x-csrc;text/x-chdr;text/x-java;text/x-python;application/xml;text/x-php;
Categories=TextEditor;Development;IDE;
Keywords=text;editor;source;code;programming;
StartupNotify=true
StartupWMClass=notepad-plus-plus
EOF

# User-specific installation
mkdir -p ~/.local/share/applications
cp /usr/share/applications/notepad-plus-plus.desktop ~/.local/share/applications/
```

#### Icon Installation

```bash
# System-wide
sudo cp /usr/share/notepad-plus-plus/notepad-plus-plus.png /usr/share/pixmaps/

# User-specific
mkdir -p ~/.local/share/icons/hicolor/256x256/apps
cp /path/to/notepad-plus-plus.png ~/.local/share/icons/hicolor/256x256/apps/
```

### Default Application

#### Set as Default Text Editor

```bash
# For GNOME
xdg-mime default notepad-plus-plus.desktop text/plain

# For KDE Plasma
# System Settings > Default Applications > Text Editor

# Command line method
xdg-settings set default-url-scheme-handler text/plain notepad-plus-plus.desktop
```

#### File Associations

```bash
# Associate file types
xdg-mime default notepad-plus-plus.desktop text/x-c++src
xdg-mime default notepad-plus-plus.desktop text/x-python
xdg-mime default notepad-plus-plus.desktop application/xml
```

### Configuration Files

Notepad++ on Linux follows the XDG Base Directory specification:

| Purpose | Location |
|---------|----------|
| User Configuration | `~/.config/notepad++/` |
| User Data | `~/.local/share/notepad++/` |
| Cache | `~/.cache/notepad++/` |
| Temporary Files | `/tmp/` or `$TMPDIR` |

#### Migrating from Windows

```bash
# Copy Windows config (from Wine or dual-boot)
mkdir -p ~/.config/notepad++
cp /path/to/windows/config.xml ~/.config/notepad++/
cp -r /path/to/windows/userDefineLangs ~/.config/notepad++/
cp -r /path/to/windows/themes ~/.config/notepad++/

# Convert line endings if needed
dos2unix ~/.config/notepad++/*.xml
```

### Environment Variables

```bash
# Add to ~/.bashrc or ~/.zshrc

# Notepad++ specific
export NPP_CONFIG_DIR="$HOME/.config/notepad++"
export NPP_PLUGIN_DIR="$HOME/.local/share/notepad++/plugins"

# Qt-specific for better integration
export QT_QPA_PLATFORM=xcb          # Force X11 backend
export QT_AUTO_SCREEN_SCALE_FACTOR=1 # Auto HiDPI scaling
export QT_STYLE_OVERRIDE=fusion      # Consistent styling

# Add to PATH if installed in non-standard location
export PATH="$HOME/.local/bin:$PATH"
```

## Troubleshooting

### Application Won't Start

#### Check Dependencies

```bash
# Check for missing libraries
ldd $(which notepad-plus-plus) | grep "not found"

# Install missing Qt dependencies
sudo apt install libqt6widgets6 libqt6gui6 libqt6network6
```

#### Check Configuration

```bash
# Reset configuration
mv ~/.config/notepad++ ~/.config/notepad++.backup

# Run with debug output
QT_DEBUG_PLUGINS=1 notepad-plus-plus
```

#### Wayland Issues

```bash
# Force X11 backend
QT_QPA_PLATFORM=xcb notepad-plus-plus

# Or set permanently
echo "export QT_QPA_PLATFORM=xcb" >> ~/.bashrc
```

### Font Rendering Issues

```bash
# Improve font rendering
export QT_QPA_PLATFORM=xcb
export QT_QPA_PLATFORMTHEME=gtk2

# Or use Qt5ct for configuration
sudo apt install qt5ct qt6ct
export QT_QPA_PLATFORMTHEME=qt6ct
```

### Plugin Issues

#### Plugin Directory

```bash
# Create plugin directory
mkdir -p ~/.local/share/notepad++/plugins

# Set correct permissions
chmod 755 ~/.local/share/notepad++/plugins
```

#### Compatible Plugins

Not all Windows plugins work on Linux. Check the plugin compatibility list:

| Plugin | Linux Support | Notes |
|--------|---------------|-------|
| NppExport | Partial | Basic functionality works |
| Converter | Full | Native implementation |
| mimeTools | Full | Native implementation |
| Compare | Testing | In development |
| XML Tools | Testing | In development |

### Performance Issues

#### Slow Startup

```bash
# Preload Qt libraries
sudo apt install preload

# Disable unnecessary plugins
# Edit ~/.config/notepad++/config.xml
```

#### High Memory Usage

```bash
# Limit file monitoring
# Settings > Preferences > MISC. > File Status Auto-Detection

# Disable session snapshot
# Settings > Preferences > Backup
```

### Uninstallation

#### Package Manager

```bash
# Ubuntu/Debian
sudo apt remove notepad-plus-plus
sudo apt autoremove

# Fedora
sudo dnf remove notepad-plus-plus

# Arch
sudo pacman -R notepad-plus-plus
```

#### Flatpak

```bash
flatpak uninstall com.notepad-plus-plus.NotepadPlusPlus
flatpak uninstall --unused
```

#### Manual Installation

```bash
# Remove binary
sudo rm /usr/local/bin/notepad-plus-plus

# Remove desktop entry
sudo rm /usr/share/applications/notepad-plus-plus.desktop

# Remove configuration (optional)
rm -rf ~/.config/notepad++
rm -rf ~/.local/share/notepad++
rm -rf ~/.cache/notepad++
```

## Getting Help

If you encounter installation issues:

1. Check the [FAQ](FAQ.md)
2. Search [GitHub Issues](https://github.com/notepad-plus-plus/notepad-plus-plus/issues)
3. Join the [Notepad++ Community Forum](https://community.notepad-plus-plus.org/)
4. Create a new issue with:
   - Installation method used
   - Distribution and version
   - Error messages
   - Steps to reproduce
