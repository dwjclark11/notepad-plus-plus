# Notepad++ Linux Port Changelog

All notable changes to the Notepad++ Linux port will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial Linux port with Qt6-based UI framework
- Platform Abstraction Layer (PAL) for cross-platform compatibility
- QtControls framework providing Linux implementations of Windows UI controls
- CMake build system supporting both Windows and Linux
- XDG Base Directory specification compliance
- Native file operations with inotify-based file watching
- KDE Plasma integration with native styling
- Support for multiple Linux distributions (Ubuntu, Fedora, Arch, openSUSE)

### Platform Abstraction Layer
- IFileSystem interface with full Linux implementation
- ISettings interface for XDG-compliant configuration storage
- IProcess interface using fork/exec
- IFileWatcher interface using inotify
- IClipboard interface using Qt clipboard
- IThreading interface using std::thread and pthreads
- IDialogs interface using Qt dialogs

### QtControls Implemented
- Window base class wrapping QWidget
- StaticDialog base class wrapping QDialog
- ToolBar implementation
- StatusBar implementation
- TabBar implementation
- Splitter implementation
- GoToLineDlg implementation
- RunDlg implementation
- AboutDlg implementation
- TreeView implementation
- FindReplaceDlg implementation
- ShortcutMapper implementation

### Changed
- Modified CMakeLists.txt to support Linux builds
- Updated source file selection based on platform
- Adapted Scintilla integration for Qt6

### Deprecated
- N/A (initial release)

### Removed
- Windows-specific code from Linux builds (DarkMode, Win32Exception, etc.)

### Fixed
- N/A (initial release)

### Security
- N/A (initial release)

## [0.1.0-alpha] - 2024-XX-XX

### Added
- Initial alpha release of Linux port
- Basic text editing functionality
- File operations (open, save, close)
- Syntax highlighting via Lexilla
- Multi-document interface with tabs
- Find and Replace dialog
- Go To Line dialog
- About dialog
- Preferences dialog (partial)
- Keyboard shortcuts
- Session management

### Known Issues
- Some plugins not yet compatible
- Limited theme support
- Print functionality not implemented
- Drag and drop between applications may have issues
- Some Unicode edge cases in file names

## Future Releases (Planned)

### [0.2.0] - Target: Q2 2024

#### Added
- Full plugin API compatibility layer
- Additional QtControls implementations:
  - WordStyleDlg
  - UserDefineDialog
  - Preference dialog (complete)
  - ListView
  - ClipboardHistory panel
  - DocumentMap panel
  - FileBrowser panel
  - FunctionList panel
  - ProjectPanel
- Print support
- Macro recording and playback
- Column mode editing enhancements

#### Fixed
- Unicode file name handling
- Drag and drop improvements
- Performance optimizations for large files

### [0.3.0] - Target: Q3 2024

#### Added
- Plugin manager for Linux
- Native Wayland support (without XWayland)
- xdg-desktop-portal integration for sandboxed environments
- Additional language definitions
- Custom theme support

#### Changed
- Improved startup performance
- Reduced memory footprint

### [1.0.0] - Target: Q4 2024

#### Added
- Feature parity with Windows version (core functionality)
- Complete plugin compatibility layer
- Comprehensive test suite
- Full documentation

#### Changed
- Stabilized API for plugin developers
- Performance optimizations

### [1.1.0] - Future

#### Added
- Native GTK integration option
- Additional desktop environment integrations
- Cloud sync integration
- Collaborative editing features

## Version History Details

### Build System Evolution

#### CMake Configuration
- **v0.1.0**: Basic CMake support with Qt6 detection
- **v0.2.0**: Added CPack configuration for packaging
- **v0.3.0**: Added support for ccache and unity builds

#### Compiler Support
- **v0.1.0**: GCC 11+, Clang 14+
- **v0.2.0**: Added MSVC compatibility for cross-platform development

### Platform Support

#### Linux Distributions

| Distribution | Version | Status | Notes |
|--------------|---------|--------|-------|
| Ubuntu | 22.04+ | Supported | Primary development platform |
| Ubuntu | 24.04 LTS | Supported | Recommended |
| Fedora | 39+ | Supported | Tested regularly |
| Arch Linux | Rolling | Supported | Community tested |
| Manjaro | Rolling | Supported | Community tested |
| openSUSE | Tumbleweed | Supported | Tested |
| openSUSE | Leap 15.5+ | Supported | Tested |
| Debian | 12+ | Supported | Tested |
| CentOS Stream | 9+ | Testing | Limited testing |

#### Desktop Environments

| Environment | Status | Integration Level |
|-------------|--------|-------------------|
| KDE Plasma | Supported | Full integration |
| GNOME | Supported | Standard support |
| XFCE | Supported | Standard support |
| LXQt | Supported | Standard support |
| Cinnamon | Supported | Standard support |
| MATE | Supported | Standard support |
| i3/Sway | Testing | Basic support |

### Feature Implementation Status

#### Core Features

| Feature | Status | Version |
|---------|--------|---------|
| Text editing | Complete | 0.1.0 |
| Syntax highlighting | Complete | 0.1.0 |
| Multi-document tabs | Complete | 0.1.0 |
| Find/Replace | Complete | 0.1.0 |
| Regular expressions | Complete | 0.1.0 |
| Session management | Complete | 0.1.0 |
| File watching | Complete | 0.1.0 |
| Print | In Progress | 0.2.0 |
| Macros | In Progress | 0.2.0 |
| Column mode | Complete | 0.1.0 |
| Multi-view | Complete | 0.1.0 |
| Zoom | Complete | 0.1.0 |
| Bookmarks | Complete | 0.1.0 |
| Code folding | Complete | 0.1.0 |

#### Dialogs

| Dialog | Status | Version |
|--------|--------|---------|
| Find/Replace | Complete | 0.1.0 |
| Go To Line | Complete | 0.1.0 |
| Run | Complete | 0.1.0 |
| About | Complete | 0.1.0 |
| Preferences | Partial | 0.1.0 |
| Shortcut Mapper | Complete | 0.1.0 |
| Style Configurator | In Progress | 0.2.0 |
| User Defined Language | In Progress | 0.2.0 |
| Print | Not Started | 0.2.0 |
| Save As | Complete | 0.1.0 |
| Open | Complete | 0.1.0 |

#### Panels

| Panel | Status | Version |
|-------|--------|---------|
| Document Map | In Progress | 0.2.0 |
| Function List | In Progress | 0.2.0 |
| Clipboard History | In Progress | 0.2.0 |
| File Browser | In Progress | 0.2.0 |
| Project Panel | In Progress | 0.2.0 |
| Folder as Workspace | In Progress | 0.2.0 |
| Document List | Complete | 0.1.0 |
| Character Panel | Not Started | 0.3.0 |
| ASCII Insertion Panel | Not Started | 0.3.0 |

#### Plugins

| Plugin | Status | Version |
|--------|--------|---------|
| NppExport | Partial | 0.1.0 |
| Converter | Complete | 0.1.0 |
| mimeTools | Complete | 0.1.0 |
| NppFTP | Not Started | Future |
| Compare | Testing | 0.3.0 |
| XML Tools | Testing | 0.3.0 |
| JSON Viewer | Not Started | Future |
| Markdown Viewer | Not Started | Future |

### API Changes

#### Platform Abstraction Layer

##### v0.1.0
- Initial release of all PAL interfaces
- IFileSystem: Complete file operation abstraction
- ISettings: XDG-compliant configuration
- IProcess: Cross-platform process management
- IFileWatcher: File change monitoring
- IClipboard: Clipboard operations
- IThreading: Thread utilities
- IDialogs: Common dialogs

##### v0.2.0 (Planned)
- IFileSystem: Add async file operations
- ISettings: Add encrypted storage support
- IProcess: Add process group management

### Bug Fixes by Version

#### v0.1.0-alpha
- Fixed file path encoding issues with non-ASCII characters
- Resolved Qt6 signal/slot connection warnings
- Fixed memory leak in file watcher
- Corrected line ending detection on mixed-format files

### Performance Improvements

#### v0.1.0
- Optimized file loading for large files (>100MB)
- Improved syntax highlighting performance
- Reduced startup time through lazy initialization

#### v0.2.0 (Planned)
- Implement file memory mapping for large files
- Optimize text rendering with hardware acceleration
- Improve search performance with indexing

### Security Updates

#### v0.1.0
- Implemented secure temporary file handling
- Added path traversal protection
- Sanitized file name inputs

### Documentation Updates

#### v0.1.0
- Initial architecture documentation
- Build instructions for major distributions
- User manual for Linux-specific features
- Porting guide for developers

#### v0.2.0 (Planned)
- Plugin development guide
- API reference documentation
- Contribution guidelines update

### Deprecation Notices

No deprecations in initial release.

### Breaking Changes

No breaking changes in initial release.

---

## How to Update

### Package Manager

```bash
# Ubuntu/Debian
sudo apt update && sudo apt upgrade notepad-plus-plus

# Fedora
sudo dnf update notepad-plus-plus

# Arch Linux
sudo pacman -Syu notepad-plus-plus
```

### Flatpak

```bash
flatpak update com.notepad-plus-plus.NotepadPlusPlus
```

### AppImage

Download the new AppImage from the releases page and replace the old one.

### From Source

```bash
cd notepad-plus-plus
git pull origin master
cd build
cmake ../PowerEditor/src
make -j$(nproc)
sudo make install
```

---

## Release Notes Archive

Detailed release notes for each version are available on the GitHub releases page:
https://github.com/notepad-plus-plus/notepad-plus-plus/releases
