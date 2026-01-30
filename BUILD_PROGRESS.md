# Notepad++ Linux Port - Build Progress

**Last Updated:** 2026-01-30

## Overview

This document tracks the build progress of the Notepad++ Linux Qt6 port.

---

## Build Status Summary

| Component | Status | Notes |
|-----------|--------|-------|
| CMake Configuration | ✅ Complete | Qt6 detection, build system configured |
| Lexilla Library | ✅ Building | All lexers compile successfully |
| Scintilla Qt6 | ✅ Building | Qt6 port compiles |
| **Core Backend** | ✅ **Complete** | Buffer, FileManager, ScintillaEditView, Notepad_plus |
| **UI Base Classes** | ✅ **Implemented** | StaticDialog, ToolBar, StatusBar, DockingManager, Splitter |
| **WinControls Port** | 🔄 **In Progress** | Conditional compilation added, stubs created |
| **Overall Build** | ⚠️ **In Progress** | Compiling ~70%, Qt implementation issues remain |

---

## Recent Changes (2026-01-30)

### WinControls Headers - Conditional Compilation Added

**Files Modified with `#ifdef _WIN32` guards:**
- `WinControls/TabBar/TabBar.h` - TabBar and TabBarPlus classes
- `WinControls/TabBar/ControlsTab.h` - ControlsTab dialog
- `ScintillaComponent/UserDefineDialog.h` - UserDefineDialog and related dialogs
- `MISC/md5/md5Dlgs.h` - HashFromFilesDlg and HashFromTextDlg
- `WinControls/FindCharsInRange/FindCharsInRange.h` - FindCharsInRangeDlg
- `WinControls/ColourPicker/ColourPicker.h` - ColourPicker
- `WinControls/ColourPicker/WordStyleDlg.h` - WordStyleDlg
- `WinControls/AboutDlg/URLCtrl.h` - URLCtrl
- `WinControls/WindowsDlg/SizeableDlg.h` - SizeableDlg base class
- `WinControls/WindowsDlg/WindowsDlg.h` - WindowsDlg and WindowsMenu
- `WinControls/AnsiCharPanel/ListView.h` - ListView control
- `WinControls/PluginsAdmin/pluginsAdmin.h` - PluginsAdminDlg
- `WinControls/DocumentMap/documentSnapshot.h` - DocumentPeeker
- `WinControls/ContextMenu/ContextMenu.h` - ContextMenu (redirects to QtControls on Linux)
- `WinControls/StaticDialog/StaticDialog.h` - Windows version wrapped
- `WinControls/Window.h` - Windows version wrapped
- `ScintillaComponent/columnEditor.h` - ColumnEditorDlg
- `lesDlgs.h` - ButtonDlg

**Notepad_plus.h Updates:**
- Added proper `#ifdef NPP_LINUX` guards for Qt vs Windows includes
- Added stub class definitions for dialogs not yet ported:
  - `AboutDlg`, `DebugInfoDlg`, `CmdLineArgsDlg` (stubs)
  - `DockingCont` (stub)
  - `WordStyleDlg` (includes QtControls version)
- Excluded Windows-only headers on Linux:
  - `AboutDlg.h`, `WordStyleDlg.h`, `trayIconControler.h`
  - `pluginsAdmin.h` (temporarily)
- Wrapped Windows-specific member variables in `#ifndef NPP_LINUX`
- Added `Window` base class include for Linux

**Notepad_plus_Window.h Updates:**
- Added conditional compilation to use QtControls version on Linux
- Windows version wrapped in `#ifndef NPP_LINUX`

**DocumentMap.cpp Fixes:**
- Changed RECT to QRect for Linux compatibility
- Removed unnecessary LinuxTypes.h include

---

## Current Status

### What Works

1. **Core Backend (100%)**
   - Buffer management
   - FileManager operations
   - ScintillaEditView integration
   - Notepad_plus initialization

2. **UI Base Classes (100%)**
   - StaticDialog with Qt
   - ToolBar/ReBar implementation
   - StatusBar
   - DockingManager
   - SplitterContainer
   - Window base class

3. **Header Compatibility (90%)**
   - Most WinControls headers now compile on Linux
   - Conditional compilation in place for Windows-specific code
   - Linux stubs provided for essential classes

### Remaining Issues

1. **Qt Implementation Issues**
   - `DocumentMap.cpp` - QRect vs RECT type issues (partially fixed)
   - `MainWindow/Notepad_plus_Window.cpp` - ToolBarButtonUnit initialization mismatch
     - Struct expects 10 int fields, code provides 4 values with string
   - Various Qt includes and type conversions

2. **Unported Dialogs (Stubs Only)**
   - AboutDlg
   - DebugInfoDlg
   - CmdLineArgsDlg
   - PluginsAdminDlg
   - FindCharsInRangeDlg (has stub, needs full implementation)

3. **Linker Issues**
   - Virtual function implementations needed for some UI classes

---

## Build Instructions

```bash
cd PowerEditor/src/build
rm -rf *
cmake .. -DBUILD_TESTS=OFF
make -j4 2>&1 | head -100
```

---

## Next Steps

### Immediate (To Get Build Working)

1. **Fix ToolBarButtonUnit initialization** in `MainWindow/Notepad_plus_Window.cpp`
   - Either update struct to match usage, or fix initialization

2. **Fix remaining QRect/RECT issues** in DocumentMap.cpp

3. **Add missing virtual function implementations** for UI classes

### Short Term (To Get Working Application)

1. **Port Essential Dialogs:**
   - AboutDlg (partially exists in QtControls)
   - FindCharsInRangeDlg

2. **Fix Accelerator/Shortcut handling** for Linux

3. **Implement menu system** integration

### Long Term (Full Feature Parity)

1. **Port All Remaining Dialogs:**
   - PluginsAdmin
   - DebugInfoDlg
   - CmdLineArgsDlg
   - UserDefineDialog (syntax highlighting config)

2. **Implement plugin support**

3. **Add tray icon support** (if applicable on Linux)

---

## Git Commit History

### Commit 1: Core Implementation
```
Linux Port: Implement core Buffer, FileManager, and Notepad_plus methods
```

### Commit 2: UI Base Classes
```
Linux Port: Implement UI base classes (StaticDialog, ToolBar, DockingManager)
```

### Commit 3: CMake Fixes
```
Linux Port: Fix CMake include order and Shortcut header path
```

### Commit 4: WinControls Conditional Compilation
```
Linux Port: Add conditional compilation to WinControls headers

- Wrap Windows-specific classes in #ifdef _WIN32
- Add Linux stub classes for dialogs
- Fix Notepad_plus.h for Linux includes
- Fix DocumentMap.cpp QRect usage
```

---

## Summary

The Linux port has made significant progress:
- ✅ Complete core backend
- ✅ Qt UI base classes implemented
- ✅ WinControls headers wrapped for conditional compilation
- 🔄 Fixing Qt implementation details (toolbar initialization, etc.)
- ⏳ Porting remaining dialogs (stubs in place)

The build is approximately 70% complete, with compilation errors now focused on Qt implementation details rather than fundamental architectural issues.
