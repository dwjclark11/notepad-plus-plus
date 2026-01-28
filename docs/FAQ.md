# Notepad++ Linux Port - Frequently Asked Questions

This FAQ addresses common questions about the Notepad++ Linux port.

## Table of Contents

- [General Questions](#general-questions)
- [Installation](#installation)
- [Building from Source](#building-from-source)
- [Features and Compatibility](#features-and-compatibility)
- [Plugins](#plugins)
- [Performance](#performance)
- [Troubleshooting](#troubleshooting)
- [Known Issues](#known-issues)
- [Contributing](#contributing)

## General Questions

### Q: Is this an official Notepad++ release?

**A:** The Linux port is developed as part of the official Notepad++ project. It aims to provide the same functionality as the Windows version while being a native Linux application.

### Q: Is this a Wine wrapper or emulator?

**A:** No. This is a native Linux application built with Qt6. It uses a Platform Abstraction Layer to share core logic with the Windows version while providing native Linux UI and system integration.

### Q: What Linux distributions are supported?

**A:** The Linux port is tested on:
- Ubuntu 22.04 and later
- Fedora 39 and later
- Arch Linux / Manjaro
- openSUSE Tumbleweed and Leap 15.5+
- Debian 12+

Other distributions may work but are not officially tested.

### Q: What desktop environments are supported?

**A:** Notepad++ works on all major desktop environments:
- KDE Plasma (with enhanced integration)
- GNOME
- XFCE
- LXQt
- Cinnamon
- MATE

### Q: Is there a macOS version?

**A:** Not currently. The Platform Abstraction Layer is designed to support macOS in the future, but there are no immediate plans for a macOS release.

## Installation

### Q: How do I install Notepad++ on Linux?

**A:** Several methods are available:

1. **Package Manager** (when available in repositories):
   ```bash
   # Ubuntu/Debian
   sudo apt install notepad-plus-plus

   # Fedora
   sudo dnf install notepad-plus-plus

   # Arch Linux (AUR)
   yay -S notepad-plus-plus
   ```

2. **Flatpak**:
   ```bash
   flatpak install flathub com.notepad-plus-plus.NotepadPlusPlus
   ```

3. **AppImage**: Download from releases page

4. **Build from Source**: See [BUILD.md](BUILD.md)

### Q: Where are configuration files stored?

**A:** Notepad++ follows the XDG Base Directory specification:
- Configuration: `~/.config/notepad++/`
- User data: `~/.local/share/notepad++/`
- Cache: `~/.cache/notepad++/`

### Q: Can I use my Windows configuration on Linux?

**A:** Yes, you can copy configuration files from Windows:
```bash
mkdir -p ~/.config/notepad++
cp /path/to/windows/config.xml ~/.config/notepad++/
cp -r /path/to/windows/userDefineLangs ~/.config/notepad++/
```

Note: Some Windows-specific settings may not apply on Linux.

### Q: How do I uninstall Notepad++?

**A:** Depends on installation method:

```bash
# Package manager
sudo apt remove notepad-plus-plus  # Ubuntu/Debian
sudo dnf remove notepad-plus-plus  # Fedora

# Flatpak
flatpak uninstall com.notepad-plus-plus.NotepadPlusPlus

# Manual installation
sudo rm /usr/local/bin/notepad-plus-plus
rm -rf ~/.config/notepad++
rm -rf ~/.local/share/notepad++
```

## Building from Source

### Q: What are the build requirements?

**A:** Minimum requirements:
- CMake 3.16+
- Qt6 (Core, Widgets, Gui, Network)
- GCC 11+ or Clang 14+
- C++20 support

See [BUILD.md](BUILD.md) for distribution-specific requirements.

### Q: The build fails with "Qt6 not found"

**A:** Install Qt6 development packages:

```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-base-dev-tools

# Fedora
sudo dnf install qt6-qtbase-devel

# Arch Linux
sudo pacman -S qt6-base
```

Then specify Qt6 location:
```bash
cmake -DQt6_DIR=/usr/lib/cmake/Qt6 ..
```

### Q: Can I build with Qt5 instead of Qt6?

**A:** No. The Linux port requires Qt6 for modern C++ support and better Wayland integration.

### Q: How do I create a debug build?

**A:**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Q: Can I cross-compile for ARM64?

**A:** Yes, with a proper toolchain file:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm64-linux-gnu.cmake ..
```

## Features and Compatibility

### Q: What features are available compared to Windows?

**A:** Core features are available:
- Text editing with syntax highlighting
- Multi-document interface
- Find/Replace with regex
- Macros
- Plugins (limited compatibility)
- Session management
- File monitoring

Some Windows-specific features are not available:
- Windows shell extensions
- Some Windows-only plugins
- Windows-specific file associations

### Q: Does it support my programming language?

**A:** Notepad++ supports 80+ programming languages through Lexilla. All languages supported on Windows are available on Linux.

### Q: Can I use Windows themes on Linux?

**A:** Most themes work on Linux. Themes are stored in `~/.config/notepad++/themes/`. Some Windows-specific visual styles may not render identically.

### Q: Is printing supported?

**A:** Basic printing support is available. Advanced print settings may differ from Windows.

### Q: Does it support drag and drop?

**A:** Yes, drag and drop works within the application and from file managers. Some edge cases with remote files may have issues.

### Q: Can I open files from SFTP/SMB shares?

**A:** Yes, if your desktop environment provides KIO/gvfs integration. You can also use the File Browser panel to browse remote locations.

## Plugins

### Q: Can I use Windows plugins on Linux?

**A:** Not directly. Plugins must be:
1. Recompiled for Linux
2. Ported to use the Platform Abstraction Layer

Some popular plugins are being ported.

### Q: Which plugins are available?

**A:** Currently available:
- Converter (built-in)
- mimeTools (built-in)
- NppExport (partial)

In development:
- Compare
- XML Tools

### Q: How do I install plugins?

**A:** Plugins are installed to `~/.local/share/notepad++/plugins/`:
```bash
mkdir -p ~/.local/share/notepad++/plugins
cp plugin.so ~/.local/share/notepad++/plugins/
```

### Q: Can I develop plugins for Linux?

**A:** Yes! See [PORTING.md](PORTING.md) for the plugin API. The interface is designed to be compatible with Windows plugins where possible.

## Performance

### Q: Is the Linux version slower than Windows?

**A:** Performance is comparable. Some operations may be faster on Linux due to better file system caching. Startup time is optimized through lazy initialization.

### Q: How can I improve startup time?

**A:**
1. Disable unnecessary plugins
2. Reduce the number of files in session
3. Disable file status auto-detection for network drives
4. Use a SSD for configuration directory

### Q: Large files are slow to open

**A:** For files >100MB:
1. Disable word wrap (View > Word Wrap)
2. Disable document map
3. Increase "Large file restriction" threshold in preferences
4. Consider using "Find in Files" instead of opening

### Q: High memory usage

**A:** To reduce memory usage:
1. Disable session snapshot (Settings > Preferences > Backup)
2. Close unused documents
3. Reduce undo levels (Settings > Preferences > MISC)
4. Disable unnecessary plugins

## Troubleshooting

### Q: Notepad++ won't start

**A:** Try these steps:

1. Check for missing libraries:
   ```bash
   ldd $(which notepad-plus-plus) | grep "not found"
   ```

2. Reset configuration:
   ```bash
   mv ~/.config/notepad++ ~/.config/notepad++.backup
   ```

3. Run with debug output:
   ```bash
   QT_DEBUG_PLUGINS=1 notepad-plus-plus
   ```

4. Check for Wayland issues:
   ```bash
   QT_QPA_PLATFORM=xcb notepad-plus-plus
   ```

### Q: Fonts look wrong / blurry

**A:**
```bash
# Force font antialiasing
export QT_QPA_PLATFORM=xcb

# Or use Qt5ct/Qt6ct for configuration
sudo apt install qt6ct
export QT_QPA_PLATFORMTHEME=qt6ct
```

### Q: HiDPI scaling issues

**A:**
```bash
# Auto-detect scaling
export QT_AUTO_SCREEN_SCALE_FACTOR=1

# Or set specific scale
export QT_SCALE_FACTOR=2
```

### Q: File changes not detected

**A:** Check file watcher settings:
1. Settings > Preferences > MISC.
2. Enable "File Status Auto-Detection"
3. Adjust polling interval if needed

For network drives, you may need to enable polling mode.

### Q: Cannot save files (permission denied)

**A:**
1. Check file permissions
2. If editing system files, use:
   ```bash
   pkexec notepad-plus-plus /etc/config-file
   ```
3. Or save to temporary location and copy with sudo

### Q: Keyboard shortcuts don't work

**A:**
1. Check for conflicts with desktop environment
2. Reset shortcuts: `~/.config/notepad++/shortcuts.xml`
3. Some shortcuts may be intercepted by the window manager

### Q: Crash on startup with Wayland

**A:** Force X11 backend:
```bash
QT_QPA_PLATFORM=xcb notepad-plus-plus
```

Add to `~/.bashrc` for permanent fix:
```bash
export QT_QPA_PLATFORM=xcb
```

## Known Issues

### Q: What are the current known issues?

**A:** See [CHANGELOG.md](CHANGELOG.md) for the latest known issues. Common ones include:

- Some Unicode characters in file names may not display correctly
- Drag and drop between some applications may not work
- Some plugins are not yet compatible
- Print preview may differ from Windows

### Q: Where can I report bugs?

**A:** Report bugs on GitHub: https://github.com/notepad-plus-plus/notepad-plus-plus/issues

Include:
- Linux distribution and version
- Desktop environment
- Qt version (`qmake6 --version`)
- Steps to reproduce
- Debug info from `? > Debug Info...`

### Q: Is there a workaround for [specific issue]?

**A:** Check the [GitHub Issues](https://github.com/notepad-plus-plus/notepad-plus-plus/issues) page for workarounds. Common workarounds:

- **Wayland issues**: Use `QT_QPA_PLATFORM=xcb`
- **Font issues**: Install `qt6ct` and configure fonts
- **Plugin crashes**: Disable problematic plugins

## Contributing

### Q: How can I contribute?

**A:** See [CONTRIBUTING.md](CONTRIBUTING.md). Ways to contribute:
- Report bugs
- Submit pull requests
- Improve documentation
- Test on different distributions
- Port plugins

### Q: What skills are needed?

**A:** Depending on contribution type:
- **C++**: For core development
- **Qt**: For UI development
- **CMake**: For build system
- **Technical writing**: For documentation

### Q: Where do I start?

**A:** Good first issues are tagged "good first issue" on GitHub. You can also:
1. Build the project
2. Run the tests
3. Look at the [PORTING.md](PORTING.md) guide
4. Join the community forum

### Q: How do I port a Windows feature?

**A:** See [PORTING.md](PORTING.md) for the complete guide. The general process:
1. Understand the Windows implementation
2. Add PAL interface if needed
3. Implement QtControls if UI-related
4. Test on both platforms

### Q: Can I donate to the project?

**A:** Notepad++ is free software. You can support it by:
- Contributing code or documentation
- Helping other users
- Spreading the word

## Additional Resources

- [User Manual](USER_MANUAL.md)
- [Build Instructions](BUILD.md)
- [Installation Guide](INSTALL.md)
- [Architecture Documentation](ARCHITECTURE.md)
- [Porting Guide](PORTING.md)
- [Notepad++ Website](https://notepad-plus-plus.org/)
- [Community Forum](https://community.notepad-plus-plus.org/)

## Still Have Questions?

If your question isn't answered here:

1. Search [existing GitHub issues](https://github.com/notepad-plus-plus/notepad-plus-plus/issues)
2. Ask on the [Community Forum](https://community.notepad-plus-plus.org/)
3. Join GitHub Discussions
4. Create a new issue with your question
