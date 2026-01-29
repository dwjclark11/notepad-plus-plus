# Notepad++ Linux Build Progress

## Current Status (as of 2026-01-28)

### Build System Status

| Component | Status | Notes |
|-----------|--------|-------|
| Scintilla Qt6 library | ✓ Fixed | CMake configuration updated |
| Lexilla library | ✓ Fixed | Building successfully |
| Platform Abstraction | ✓ Fixed | Common.h refactored into platform-specific headers |
| Notepad++ executable | In Progress | 30 remaining errors in Qt/NppCommands integration |

### Summary of Changes

#### 1. Refactored Common.h into Platform-Specific Headers

**Problem:** Common.h was 1,877 lines with complex nested `#ifdef` blocks that were error-prone and difficult to maintain.

**Solution:** Split into three focused headers:

**New Files Created:**
- `PowerEditor/src/MISC/Common/LinuxTypes.h` - Linux platform abstraction (types, constants, stubs)
- `PowerEditor/src/MISC/Common/WindowsTypes.h` - Windows API headers
- `PowerEditor/src/MISC/Common/Common.h` - Only truly common utilities (refactored)

**LinuxTypes.h Contents:**
- All Windows type aliases for Linux (HWND, UINT, WPARAM, DWORD, BOOL, etc.)
- All Windows structures (RECT, POINT, MSG, WINDOWPLACEMENT, MINMAXINFO, etc.)
- All Windows constants (WM_*, SW_*, MB_*, VK_*, etc.)
- All stub functions (SendMessage, ShowWindow, CreateEvent, etc.)
- ControlInfoTip stub class for Linux

**Benefits:**
- Clean separation of platform-specific code
- No complex #ifdef nesting
- Easier maintenance
- Clearer dependencies

#### 2. Headers Updated to Use New Structure

**Core Headers:**
- `Window.h` - Now includes `../MISC/Common/Common.h` instead of defining its own types
- `ToolBar.h` - Uses Common.h which includes appropriate platform header
- `StaticDialog.h` - Uses Common.h for platform types
- `DockingDlgInterface.h` - Uses Common.h

**All platform-specific code now properly isolated in:**
- `LinuxTypes.h` for Linux builds
- `WindowsTypes.h` for Windows builds

#### 3. Previously Fixed Issues (Preserved)

**CMakeLists.txt Improvements:**
- Qt6 integration with proper components (Core, Widgets, Gui, Network, PrintSupport)
- OpenSSL integration for Linux
- Platform Linux files included in build
- Proper include directories

**Name Conflicts Resolved:**
- `enum Platform` → `enum class Platform` (scoped enum)
- CP_ACP/CP_UTF8 redefinition fixed with guard macros

**Qt6 Platform Files:**
- Settings.cpp - Qt6 native APIs
- Process.cpp - Linux process implementation
- FileWatcher.cpp - QFileSystemWatcher-based implementation
- Clipboard.cpp - QClipboard-based implementation
- Threading.cpp - QThread-based implementation

### Remaining Issues

The following errors remain in QtCore/NppCommands.cpp (30 errors total):

**Missing Constants:**
- `CMD_EDIT_BLOCK_UNCOMMENT`
- `CMD_EDIT_STREAM_UNCOMMENT`
- `CMD_VIEW_SWITCHTO_PROJECT_PANEL_1/2/3`
- `CMD_VIEW_SWITCHTO_FILEBROWSER`
- `CMD_VIEW_SWITCHTO_FUNC_LIST`
- `CMD_VIEW_SWITCHTO_DOCLIST`
- `CMD_VIEW_TAB_START`
- `CMD_VIEW_TAB_END`

**Missing Function:**
- `findMatchingBracePos` - not declared in scope

**Private Method Access:**
- `Notepad_plus::cutMarkedLines()`
- `Notepad_plus::copyMarkedLines()`
- `Notepad_plus::pasteToMarkedLines()`
- `Notepad_plus::deleteMarkedLines(bool)`
- `Notepad_plus::inverseMarks()`
- `Notepad_plus::goToNextIndicator(int, bool)`
- `Notepad_plus::goToPreviousIndicator(int, bool)`
- `Notepad_plus::fullScreenToggle()`
- `Notepad_plus::postItToggle()`
- `Notepad_plus::distractionFreeToggle()`
- `Notepad_plus::otherView()`
- `Notepad_plus::switchEditViewTo(int)`
- `Notepad_plus::activateDoc(size_t)`
- `Notepad_plus::activateNextDoc(bool)`

### Build Instructions

```bash
cd PowerEditor/src/build
cmake .. -DNPP_LINUX=ON
make -j$(nproc)
```

### Files Created/Modified

**New Files:**
- `PowerEditor/src/MISC/Common/LinuxTypes.h` - Linux platform abstraction
- `PowerEditor/src/MISC/Common/WindowsTypes.h` - Windows platform headers

**Modified Files:**
- `PowerEditor/src/MISC/Common/Common.h` - Refactored to only common utilities
- `PowerEditor/src/WinControls/Window.h` - Updated to include Common.h
- `PowerEditor/src/CMakeLists.txt` - Qt6 and Linux build configuration

### Next Steps

1. Fix remaining Qt/NppCommands.cpp integration issues:
   - Add missing CMD_* constants or use IDM_* equivalents
   - Add findMatchingBracePos declaration
   - Make required Notepad_plus methods public or add wrapper methods
2. Complete build verification
3. Test executable on Linux
