# Notepad++ Linux Port Architecture

This document describes the architecture of the Notepad++ Linux port, explaining how the codebase is organized and how platform abstraction enables a single codebase to support both Windows and Linux.

## Table of Contents

- [Overview](#overview)
- [Architecture Layers](#architecture-layers)
- [Platform Abstraction Layer](#platform-abstraction-layer)
- [QtControls Framework](#qtcontrols-framework)
- [Windows Code Preservation](#windows-code-preservation)
- [Adding New Platform Features](#adding-new-platform-features)
- [Build System](#build-system)

## Overview

The Notepad++ Linux port maintains source compatibility with the Windows version through a comprehensive platform abstraction strategy. The core application logic remains unchanged while platform-specific implementations are isolated behind abstract interfaces.

### Design Goals

1. **Minimal Windows Code Changes**: The existing Windows codebase should require few or no modifications
2. **Native Linux Experience**: The Linux port should feel like a native application
3. **Maintainability**: Changes to core logic benefit both platforms
4. **Extensibility**: New features can be added with platform-specific implementations

## Architecture Layers

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│         (Notepad_plus.cpp, NppCommands.cpp, etc.)           │
├─────────────────────────────────────────────────────────────┤
│                    UI Control Layer                         │
│    ┌──────────────┐              ┌──────────────┐          │
│    │   Windows    │              │    Linux     │          │
│    │  (Win32 API) │              │  (Qt6/QWidget)│          │
│    └──────────────┘              └──────────────┘          │
├─────────────────────────────────────────────────────────────┤
│                  Platform Abstraction Layer                 │
│    ┌──────────────┐              ┌──────────────┐          │
│    │   Windows    │              │    Linux     │          │
│    │Implementation│              │Implementation│          │
│    └──────────────┘              └──────────────┘          │
├─────────────────────────────────────────────────────────────┤
│                     System Services                         │
│         (File System, Clipboard, Threading, etc.)           │
└─────────────────────────────────────────────────────────────┘
```

## Platform Abstraction Layer

The Platform Abstraction Layer (PAL) provides cross-platform interfaces for system services. Each interface has platform-specific implementations in separate directories.

### Directory Structure

```
PowerEditor/src/Platform/
├── FileSystem.h          # Interface definition
├── Settings.h            # Configuration interface
├── Process.h             # Process management
├── FileWatcher.h         # File monitoring
├── Clipboard.h           # Clipboard operations
├── Threading.h           # Thread utilities
├── Dialogs.h             # Common dialogs
├── Windows/              # Windows implementations
│   ├── FileSystem.cpp
│   ├── Settings.cpp
│   ├── Process.cpp
│   ├── FileWatcher.cpp
│   ├── Clipboard.cpp
│   ├── Threading.cpp
│   └── Dialogs.cpp
└── Linux/                # Linux implementations
    ├── FileSystem.cpp
    ├── Settings.cpp
    ├── Process.cpp
    ├── FileWatcher.cpp
    ├── Clipboard.cpp
    ├── Threading.cpp
    └── Dialogs.cpp
```

### Interface Design Pattern

Each platform interface follows the singleton pattern with virtual methods:

```cpp
// Platform/FileSystem.h
class IFileSystem {
public:
    static IFileSystem& getInstance();

    virtual bool fileExists(const std::wstring& path) = 0;
    virtual bool copyFile(const std::wstring& src, const std::wstring& dest) = 0;
    // ... more methods
};
```

Platform-specific implementations:

```cpp
// Platform/Windows/FileSystem.cpp
class FileSystemWin32 : public IFileSystem {
    bool fileExists(const std::wstring& path) override {
        return ::PathFileExistsW(path.c_str()) == TRUE;
    }
    // ...
};

// Platform/Linux/FileSystem.cpp
class FileSystemLinux : public IFileSystem {
    bool fileExists(const std::wstring& path) override {
        return std::filesystem::exists(toPath(path));
    }
    // ...
};
```

### Available Interfaces

| Interface | Purpose | Windows Implementation | Linux Implementation |
|-----------|---------|----------------------|---------------------|
| `IFileSystem` | File operations | Win32 API (`CreateFileW`, etc.) | C++17 `std::filesystem` + POSIX |
| `ISettings` | Configuration storage | Registry + INI files | XDG directories + INI files |
| `IProcess` | Process management | `CreateProcess` | `fork()` + `exec()` |
| `IFileWatcher` | File change monitoring | `ReadDirectoryChangesW` | `inotify` |
| `IClipboard` | Clipboard operations | Win32 Clipboard API | Qt Clipboard |
| `IThreading` | Thread utilities | Win32 Threads | `std::thread` + pthreads |
| `IDialogs` | Common dialogs | Win32 Common Dialogs | Qt Dialogs |

## QtControls Framework

The QtControls framework provides Qt6-based implementations of Windows UI controls used by Notepad++.

### Design Philosophy

QtControls mirrors the Windows control hierarchy to minimize changes in the application layer:

```
Windows Hierarchy                    QtControls Hierarchy
─────────────────                    ─────────────────────
HWND (Window Handle)        →        QWidget / QWindow
    │
    ├── StaticDialog         →        QtControls::StaticDialog
    │   ├── GoToLineDlg      →        QtControls::GoToLineDlg
    │   ├── FindReplaceDlg   →        QtControls::FindReplaceDlg
    │   └── ...
    │
    ├── ToolBar              →        QtControls::ToolBar
    ├── StatusBar            →        QtControls::StatusBar
    ├── TabBar               →        QtControls::TabBar
    ├── TreeView             →        QtControls::TreeView
    └── ...
```

### Directory Structure

```
PowerEditor/src/QtControls/
├── Window.h / Window.cpp                 # Main window wrapper
├── StaticDialog/
│   ├── StaticDialog.h / StaticDialog.cpp # Base dialog class
│   └── ...
├── ToolBar/
│   ├── ToolBar.h / ToolBar.cpp
│   └── ...
├── StatusBar/
│   ├── StatusBar.h / StatusBar.cpp
│   └── ...
├── TabBar/
│   ├── TabBar.h / TabBar.cpp
│   └── ...
├── Splitter/
│   ├── Splitter.h / Splitter.cpp
│   └── ...
├── GoToLine/
│   ├── GoToLineDlg.h / GoToLineDlg.cpp
│   └── ...
├── FindReplace/
│   ├── FindReplaceDlg.h / FindReplaceDlg.cpp
│   └── ...
├── RunDlg/
│   ├── RunDlg.h / RunDlg.cpp
│   └── ...
├── AboutDlg/
│   ├── AboutDlg.h / AboutDlg.cpp
│   └── ...
├── TreeView/
│   ├── TreeView.h / TreeView.cpp
│   └── ...
├── ListView/
│   ├── ListView.h / ListView.cpp
│   └── ...
├── ShortcutMapper/
│   ├── ShortcutMapper.h / ShortcutMapper.cpp
│   └── ...
├── WordStyleDlg/
│   ├── WordStyleDlg.h / WordStyleDlg.cpp
│   └── ...
├── UserDefineDialog/
│   ├── UserDefineDialog.h / UserDefineDialog.cpp
│   └── ...
├── Preference/
│   ├── preferenceDlg.h / preferenceDlg.cpp
│   └── ...
├── ClipboardHistory/
│   ├── ClipboardHistoryPanel.h / ClipboardHistoryPanel.cpp
│   └── ...
├── DocumentMap/
│   ├── DocumentMap.h / DocumentMap.cpp
│   └── ...
├── FileBrowser/
│   ├── FileBrowser.h / FileBrowser.cpp
│   └── ...
├── FunctionList/
│   ├── FunctionListPanel.h / FunctionListPanel.cpp
│   └── ...
├── ProjectPanel/
│   ├── ProjectPanel.h / ProjectPanel.cpp
│   └── ...
├── MainWindow/
│   ├── Notepad_plus_Window.h / Notepad_plus_Window.cpp
│   └── ...
└── CMakeLists.txt
```

### Key Classes

#### Window

The base window class wraps `QWidget` and provides a Win32-compatible interface:

```cpp
class Window {
public:
    void init(HINSTANCE hInst, HWND parent);
    HWND getHSelf() const;
    void display(bool toShow = true) const;
    void redraw() const;
    // ... Win32-compatible methods

protected:
    QWidget* _widget;  // Qt widget handle
};
```

#### StaticDialog

Base class for all dialogs, wraps `QDialog`:

```cpp
class StaticDialog : public Window {
public:
    void create(ID dialogID, bool isRTL = false);
    virtual void destroy();

protected:
    QDialog* _dialog;
    // Message handling compatible with Windows
    virtual INT_PTR CALLBACK dlgProc(...);
};
```

### Scintilla Integration

The editor component uses Scintilla via Qt:

```
┌─────────────────────────────────────┐
│        ScintillaEditView            │
│    (PowerEditor/src/ScintillaComponent)│
├─────────────────────────────────────┤
│      ScintillaQt (Qt wrapper)       │
│    (scintilla/qt/ScintillaEditBase) │
├─────────────────────────────────────┤
│         Qt6 Widgets                 │
└─────────────────────────────────────┘
```

## Windows Code Preservation

The Linux port preserves the original Windows code through several strategies:

### 1. Conditional Compilation

```cpp
#ifdef NPP_LINUX
    // Linux-specific code
    #include "Platform/Linux/FileSystem.cpp"
#else
    // Windows code (unchanged)
    #include "Platform/Windows/FileSystem.cpp"
#endif
```

### 2. CMake-Based Source Selection

The build system selects appropriate source files:

```cmake
IF(WIN32)
    list(APPEND src_files
        ./Platform/Windows/FileSystem.cpp
        ./Platform/Windows/Settings.cpp
        # ... Windows files
    )
ELSEIF(UNIX AND NOT APPLE)
    list(APPEND src_files
        ./Platform/Linux/FileSystem.cpp
        ./Platform/Linux/Settings.cpp
        # ... Linux files
    )

    # Remove Windows-specific files
    list(REMOVE_ITEM src_files
        ./DarkMode/DarkMode.cpp
        ./MISC/Exception/Win32Exception.cpp
        ./winmain.cpp
    )
ENDIF()
```

### 3. Type Aliases

Win32 types are aliased to Linux equivalents:

```cpp
// Platform abstraction header
#ifdef NPP_LINUX
    using HWND = QWidget*;
    using HINSTANCE = void*;
    using WPARAM = uintptr_t;
    using LPARAM = intptr_t;
    using LRESULT = intptr_t;
    using UINT = unsigned int;
    using DWORD = unsigned long;
    #define CALLBACK
#endif
```

### 4. Message Mapping

Windows message handling is mapped to Qt signals/slots:

```cpp
// Windows
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_COMMAND:
            // Handle command
            break;
        case WM_NOTIFY:
            // Handle notification
            break;
    }
}

// Linux (Qt)
class MainWindow : public QMainWindow {
    Q_OBJECT
public slots:
    void onActionTriggered();      // Maps to WM_COMMAND
    void onNotificationReceived(); // Maps to WM_NOTIFY
};
```

## Adding New Platform Features

### Step 1: Define the Interface

Add the interface method to the appropriate header:

```cpp
// Platform/FileSystem.h
class IFileSystem {
public:
    // Existing methods...

    // New method
    virtual bool createSymbolicLink(const std::wstring& target,
                                     const std::wstring& linkPath) = 0;
};
```

### Step 2: Implement for Windows

```cpp
// Platform/Windows/FileSystem.cpp
class FileSystemWin32 : public IFileSystem {
    // Existing implementations...

    bool createSymbolicLink(const std::wstring& target,
                           const std::wstring& linkPath) override {
        return CreateSymbolicLinkW(linkPath.c_str(), target.c_str(),
                                   SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE) != FALSE;
    }
};
```

### Step 3: Implement for Linux

```cpp
// Platform/Linux/FileSystem.cpp
class FileSystemLinux : public IFileSystem {
    // Existing implementations...

    bool createSymbolicLink(const std::wstring& target,
                           const std::wstring& linkPath) override {
        return symlink(wstringToUtf8(target).c_str(),
                      wstringToUtf8(linkPath).c_str()) == 0;
    }
};
```

### Step 4: Use in Application Code

```cpp
// Application code (platform-neutral)
void SomeFunction() {
    auto& fs = Platform::IFileSystem::getInstance();
    fs.createSymbolicLink(target, link);
}
```

### Adding New UI Controls

To add a new dialog or control:

1. **Create the Qt implementation** in `QtControls/`:

```cpp
// QtControls/NewDialog/NewDialog.h
namespace QtControls {
class NewDialog : public StaticDialog {
    Q_OBJECT
public:
    void create(...) override;

private slots:
    void onButtonClicked();

private:
    Ui::NewDialog* _ui;
};
}
```

2. **Keep Windows version unchanged** in `WinControls/`

3. **Update CMakeLists.txt** to include the new source file for Linux builds

4. **Use conditional compilation** if the control is used in shared code:

```cpp
#ifdef NPP_LINUX
    #include "QtControls/NewDialog/NewDialog.h"
    using NewDialog = QtControls::NewDialog;
#else
    #include "WinControls/NewDialog/NewDialog.h"
#endif
```

## Build System

The CMake build system handles platform detection and configuration:

### Platform Detection

```cmake
IF(WIN32)
    set(NPP_PLATFORM_WINDOWS TRUE)
ELSEIF(UNIX AND NOT APPLE)
    set(NPP_PLATFORM_LINUX TRUE)
ENDIF()
```

### Qt6 Integration

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
```

### Compiler Flags

```cmake
# Linux-specific
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(defs -DNPP_LINUX -D_USE_64BIT_TIME_T -DTIXML_USE_STL -DQT_NO_KEYWORDS)
```

### Library Linking

```cmake
IF(UNIX AND NOT APPLE)
    TARGET_LINK_LIBRARIES(notepad++
        ${LEXILLA_STATIC_LIBRARY}
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Network
        pthread
        dl
    )
ENDIF()
```

## Best Practices

1. **Keep interfaces minimal**: Only expose what's necessary
2. **Use standard C++**: Prefer `std::` types over platform-specific types in interfaces
3. **Handle encoding properly**: Convert between UTF-8 (Linux native) and UTF-16 (Windows native) at platform boundaries
4. **Test on both platforms**: Changes to shared code must be tested on Windows and Linux
5. **Document platform differences**: Comment when behavior differs between platforms
6. **Use RAII**: Ensure resources are properly managed in both implementations

## Future Enhancements

- **macOS Support**: The abstraction layer is designed to support macOS in the future
- **Wayland Optimizations**: Native Wayland support without XWayland
- **Plugin API**: Standardized plugin interface for cross-platform plugins
- **Native File Dialogs**: Platform-native file dialogs via xdg-desktop-portal
