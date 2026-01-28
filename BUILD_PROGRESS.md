# Notepad++ Linux Build Progress

## Current Status (as of last build attempt)

### Successfully Fixed
1. **Platform type definitions** - Added to `Common.h`:
   - Basic Windows types: HWND, UINT, WPARAM, LPARAM, DWORD, BOOL, etc.
   - Structures: RECT, POINT, SIZE, SYSTEMTIME, FILETIME
   - Constants: TRUE, FALSE, CP_ACP, CP_UTF8
   - Menu and file flags: MF_ENABLED, TPM_LEFTALIGN, GENERIC_READ, etc.
   - Helper functions: DestroyIcon(), DestroyWindow(), TrackPopupMenu(), etc.

2. **Headers with platform guards added**:
   - `Common.h` - Complete platform abstraction
   - `Docking.h` - Windows types and structures
   - `Window.h` - Basic window types
   - `NppConstants.h` - Platform types and unix macro handling
   - `NppDarkMode.h` - COLORREF type
   - `Notepad_plus_msgs.h` - Platform types
   - `dpiManagerV2.h` - Windows message constants
   - `FileInterface.h` - HANDLE type
   - `ScintillaEditView.h` - Platform guards around UserDefineDialog
   - `UserDefineDialog.h` - Basic types
   - `TabBar.h` - Platform guards
   - `Splitter.h` - Platform guards
   - `SplitterContainer.h` - Platform guards

3. **CMakeLists.txt improvements**:
   - QtControls include directories now inserted at the beginning for priority
   - Linux-specific source files configured

### Remaining Issues

The build is failing due to headers that still include `<windows.h>` without platform guards:

1. **WordStyleDlg.h** - Includes windows.h directly
2. **DockingCont.h** - Has additional issues:
   - Missing StaticDialog base class (needs proper include)
   - Missing `_hSelf` member
   - Missing DRAWITEMSTRUCT (added to Common.h but still causing issues)

3. **shortcut.h** - Missing types:
   - UCHAR
   - VK_* virtual key constants (VK_DOWN, VK_UP, etc.)

4. **Platform namespace conflict**:
   - `FileSystem.h` defines `namespace Platform`
   - `Notepad_plus_msgs.h` defines `enum Platform`
   - These conflict on Linux

5. **CP_ACP/CP_UTF8 redefinition**:
   - Both `FileSystem.h` and `Parameters.h` define these constants
   - The guard macros aren't working correctly across translation units

### Build Output Summary

```
Scintilla Qt6 library: ✓ Built successfully
Lexilla library: ✓ Built successfully
Notepad++ executable: ✗ Compilation errors

Current error count: Multiple cascading errors in:
- DockingCont.h (expected class-name before '{' token)
- shortcut.h (UCHAR not declared, VK_* constants not declared)
- WordStyleDlg.h (windows.h not found)
```

### Next Steps Required

To complete the build, the following work is needed:

1. Add remaining platform guards to:
   - `WinControls/ColourPicker/WordStyleDlg.h`
   - `WinControls/shortcut/shortcut.h`
   - All other WinControls headers that include windows.h

2. Fix the Platform namespace/enum conflict:
   - Rename one of them or use namespace qualification

3. Fix CP_ACP/CP_UTF8 constant redefinition:
   - Use consistent guard macros across all headers

4. Add missing type definitions to Common.h:
   - UCHAR (unsigned char)
   - VK_* virtual key code constants

5. Fix DockingCont.h issues:
   - Ensure StaticDialog base class is properly defined
   - Add missing member variables

### Recommendation

This is a significant porting effort. The QtControls files were designed to replace WinControls for Linux, but the include chain from ScintillaEditView.h → Buffer.h → Parameters.h brings in many WinControls headers.

Consider either:
1. Creating minimal stub headers for Linux that replace the WinControls functionality
2. Expanding the platform guards in all WinControls headers systematically
3. Creating a separate CMake target for Linux that excludes WinControls entirely
