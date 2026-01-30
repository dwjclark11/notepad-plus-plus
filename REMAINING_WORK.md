# Notepad++ Linux Port - Remaining Work

## Current Status (as of 2026-01-30)

The Linux Qt6 port has made significant progress. All core Buffer and FileManager methods are now implemented, ScintillaEditView integration is complete, and Notepad_plus core initialization is done. The remaining work is primarily UI class implementations that are causing linker errors.

## Build Status

- **CMake Configuration**: ✓ Complete
- **Lexilla Library**: ✓ Building
- **Scintilla Qt6**: ✓ Building
- **Buffer/FileManager Core**: ✓ Complete
- **ScintillaEditView Integration**: ✓ Complete
- **Notepad_plus Core**: ✓ Compiling (constructor, destructor, loadSession, loadLastSession)
- **NppDarkMode**: ✓ Stubs implemented
- **Main Executable**: ⚠️ Linker errors for UI classes

## Recently Completed (2026-01-30)

### 1. Buffer.cpp Scintilla API Fixes
Fixed all ScintillaEditView API calls to use proper `execute(SCI_*, ...)` messages instead of mock methods.

### 2. FileManager Additional Methods
| Method | Status | Description |
|--------|--------|-------------|
| `bufferFromDocument(Document, bool)` | ✓ **IMPLEMENTED** | Creates Buffer from existing Scintilla document |
| `addBufferReference(BufferID, ScintillaEditView*)` | ✓ **IMPLEMENTED** | Tracks buffer usage by views |
| `saveBuffer(BufferID, wchar_t*, bool)` | ✓ **IMPLEMENTED** | Saves buffer to file with encoding |

### 3. Notepad_plus Core
| Method | Status | Description |
|--------|--------|-------------|
| `Notepad_plus()` constructor | ✓ **IMPLEMENTED** | Initializes core, plugins, auto-complete |
| `~Notepad_plus()` destructor | ✓ **IMPLEMENTED** | Cleanup resources, panels, settings |
| `loadLastSession()` | ✓ **IMPLEMENTED** | Loads files from previous session |
| `loadSession(Session&, bool, wchar_t*)` | ✓ **IMPLEMENTED** | Full session restoration |

### 4. NppDarkMode Linux Stubs
Implemented stub functions for Windows-specific dark mode functionality:
- `isWindowsModeEnabled()`, `getThemeName()`
- Color functions returning default dark mode colors
- Windows-specific UI subclassing as no-ops

### 5. Global Variables
- `g_nppStartTimePoint` - Startup time tracking
- `g_pluginsLoadingTime` - Plugin load time tracking

## Remaining Linker Errors

The following UI classes need implementation (vtable errors indicate missing virtual function implementations):

### High Priority (Blocking)
| Class | Error Type | Location |
|-------|-----------|----------|
| `StaticDialog` | vtable + destructor | Base class for dialogs |
| `StatusBar` | vtable + destructor | Main window status bar |
| `ToolBar` | vtable + destructor | Main toolbar |
| `ReBar` | vtable + destructor | Toolbar container |
| `DockingManager` | destructor | Panel docking |
| `Splitter` / `SplitterContainer` | vtable + destructor | Editor view splitting |
| `ContextMenu` | `destroy()` method | Right-click menus |

### Medium Priority (Dialog Classes)
| Class | Error Type | Description |
|-------|-----------|-------------|
| `AboutDlg` | vtable + destructor | About dialog |
| `DebugInfoDlg` | vtable + destructor | Debug info dialog |
| `CmdLineArgsDlg` | vtable + destructor | Command line args dialog |
| `HashFromFilesDlg` | vtable + destructor | Hash from files dialog |
| `HashFromTextDlg` | vtable + destructor | Hash from text dialog |
| `ColumnEditorDlg` | vtable + destructor | Column editor dialog |
| `WordStyleDlg` | vtable + destructor | Style configuration dialog |
| `FindCharsInRangeDlg` | vtable + destructor | Find special chars dialog |
| `PluginsAdminDlg` | vtable + destructor | Plugin manager dialog |
| `DocumentPeeker` | vtable + destructor | Document peeker |
| `ButtonDlg` | vtable + destructor | Generic button dialog |
| `URLCtrl` | vtable + destructor | URL hyperlink control |
| `ListView` | vtable + `destroy()` | List view control |
| `PluginViewList` | `destroy()` | Plugin list view |
| `TabBar` | vtable + destructor | Tab bar component |

### Methods
| Method | Class | Description |
|--------|-------|-------------|
| `showView(int)` | `Notepad_plus` | Show/hide main/sub editor views |
| `destroy()` | `ContextMenu` | Cleanup context menu |
| `destroy()` | `ListView` | Cleanup list view |
| `setDpiWithSystem()` | `DPIManagerV2` | DPI awareness |
| `getDpiForSystem()` | `DPIManagerV2` | Get system DPI |

## Build Instructions

```bash
cd PowerEditor/src/build
cmake .. -DBUILD_TESTS=OFF
make -j4 2>&1 | tail -50
```

## Testing Plan

Once linker errors are resolved:

1. **Basic Launch Test**
   ```bash
   ./notepad-plus-plus
   # Should show main window with menus and toolbar
   ```

2. **File Open Test**
   ```bash
   ./notepad-plus-plus /path/to/test.txt
   # Should open and display file content
   ```

3. **File Save Test**
   - Edit a file
   - Save with Ctrl+S
   - Verify changes written to disk

4. **Multiple Files Test**
   - Open multiple files
   - Switch between tabs
   - Close individual tabs

## Implementation Strategy

For each missing UI class:

1. Look at Windows implementation in corresponding file
2. Create Qt version in `QtControls/` with same interface
3. Implement virtual functions with Qt equivalents
4. Ensure proper QObject parent-child relationships
5. Test individually before moving to next class

## Files Requiring Attention

### Core Implementation Files (✓ Complete)
- ~~`QtCore/Buffer.h` / `Buffer.cpp`~~ - ✓ Buffer and FileManager
- ~~`QtCore/NppIO.cpp`~~ - ✓ File I/O operations
- ~~`QtControls/Notepad_plus.cpp`~~ - ✓ Main application logic

### UI Classes Needing Implementation
- `QtControls/StaticDialog/` - Base dialog class
- `QtControls/StatusBar/` - Status bar
- `QtControls/ToolBar/` - Toolbar
- `QtControls/ReBar/` - Toolbar container
- `QtControls/DockingManager/` - Docking system
- `QtControls/Splitter/` - View splitter
- `QtControls/ContextMenu/` - Context menus
- Various dialog classes in `QtControls/`

## Notes

- The core backend (Buffer, FileManager, ScintillaEditView) is now complete
- Remaining work is primarily UI widget implementations
- Qt's widget system can provide equivalents for all Windows controls
- Consider implementing base classes first (StaticDialog, StatusBar, ToolBar)
- Dialogs can be stubbed initially with minimal functionality

## Last Updated

2026-01-30 (Updated after major Buffer/Notepad_plus implementation push)

---

**Next Milestone:** Implement UI base classes to resolve linker errors and achieve a working executable.
