# Windows-Only Files Excluded from Linux Build

This document tracks all Windows-specific files that have been excluded from the Linux build. These files need Linux/Qt alternatives to be implemented.

## Core Application Files

| File | Windows API Used | Qt Alternative Status |
|------|------------------|----------------------|
| `Notepad_plus.cpp` | shlwapi.h, Win32 APIs | ❌ Missing - Core application logic |
| `NppBigSwitch.cpp` | shlwapi.h, Win32 message handling | ❌ Missing - Message dispatcher |
| `NppNotification.cpp` | WinControls, Windows notifications | ❌ Missing - Notification handling |
| `NppCommands.cpp` (original) | Windows command handlers | ✅ Has Qt alternative: `QtCore/NppCommands.cpp` |
| `NppIO.cpp` (original) | Windows file I/O | ✅ Has Qt alternative: `QtCore/NppIO.cpp` |

## Settings and Parameters

| File | Windows API Used | Qt Alternative Status |
|------|------------------|----------------------|
| `Parameters.cpp` | keys.h (winuser.h), Windows registry | ❌ Missing - Settings management |
| `localization.cpp` | Windows locale APIs | ❌ Missing - UI localization |

## UI/Dialog Files

| File | Windows API Used | Qt Alternative Status |
|------|------------------|----------------------|
| `lastRecentFileList.cpp` | CreatePopupMenu, RemoveMenu, MF_* flags | ❌ Missing - Recent files menu |
| `lesDlgs.cpp` | WM_CTLCOLORDLG, WM_PRINTCLIENT | ❌ Missing - Dialog utilities |
| `NppDarkMode.cpp` | Windows dark mode APIs | ❌ Missing - Dark mode support |
| `dpiManagerV2.cpp` | Windows DPI APIs | ❌ Missing - DPI awareness |

## MISC/Common Files

| File | Windows API Used | Qt Alternative Status |
|------|------------------|----------------------|
| `MISC/Common/Common.cpp` | windows.h, Win32 APIs | ⚠️ Partial - LinuxTypes.h provides stubs |
| `MISC/Common/FileInterface.cpp` | shlwapi.h, FindFirstStreamW | ❌ Missing - File operations |
| `MISC/Common/SortLocale.cpp` | LCMAP_SORTKEY, LCMapStringEx, MultiByteToWideChar | ❌ Missing - Locale-aware sorting |

## MISC/Other Files

| File | Windows API Used | Qt Alternative Status |
|------|------------------|----------------------|
| `MISC/md5/md5Dlgs.cpp` | shlwapi.h | ❌ Missing - MD5 dialog |
| `MISC/PluginsManager/PluginsManager.cpp` | Windows DLL loading | ❌ Missing - Plugin management |
| `MISC/Process/Processus.cpp` | Windows process APIs | ✅ Has Qt alternative: `Platform/Linux/Process.cpp` |
| `MISC/RegExt/regExtDlg.cpp` | Windows registry | ❌ Missing - File associations |
| `MISC/sha512/sha512.cpp` | Windows CryptoAPI | ❌ Missing - SHA-512 support |

## ScintillaComponent Files

| File | Windows API Used | Qt Alternative Status |
|------|------------------|----------------------|
| `ScintillaComponent/Buffer.cpp` (original) | Windows-specific | ✅ Has Qt alternative: `QtCore/Buffer.cpp` |
| `ScintillaComponent/ScintillaEditView.cpp` | Windows Scintilla integration | ❌ Missing - Editor view (header used) |
| `ScintillaComponent/DocTabView.cpp` | Windows tab control | ❌ Missing - Document tabs |
| `ScintillaComponent/FindReplaceDlg.cpp` | Windows dialogs | ⚠️ Partial - Qt version exists but incomplete |
| `ScintillaComponent/Printer.cpp` | Windows printing | ❌ Missing - Print support |
| `ScintillaComponent/AutoCompletion.cpp` | Windows-specific UI | ❌ Missing - Auto-completion |
| `ScintillaComponent/GoToLineDlg.cpp` | Windows dialog | ⚠️ Partial - Qt version exists |
| `ScintillaComponent/columnEditor.cpp` | Windows dialog | ❌ Missing - Column editor |

## WinControls (All Excluded)

All files in `WinControls/` directory are Windows-specific and excluded. Qt alternatives are in `QtControls/`.

| Component | Windows Files | Qt Alternative |
|-----------|--------------|----------------|
| StaticDialog | `WinControls/StaticDialog/*` | ✅ `QtControls/StaticDialog/*` |
| ToolBar | `WinControls/ToolBar/*` | ⚠️ Partial |
| TreeView | `WinControls/TreeView/*` | ✅ `QtControls/TreeView/*` |
| ListView | `WinControls/ListView/*` | ✅ `QtControls/ListView/*` |
| Docking | `WinControls/DockingWnd/*` | ⚠️ Partial |
| Preferences | `WinControls/Preference/*` | ⚠️ Partial |
| ProjectPanel | `WinControls/ProjectPanel/*` | ⚠️ Partial |
| FunctionList | `WinControls/FunctionList/*` | ⚠️ Partial |
| FileBrowser | `WinControls/FileBrowser/*` | ⚠️ Partial |
| Clipboard | `WinControls/ClipboardHistory/*` | ⚠️ Partial |
| DocumentMap | `WinControls/DocumentMap/*` | ⚠️ Partial |

## Files Needing Immediate Attention

These are high-priority files that need Qt alternatives for basic functionality:

1. **Parameters.cpp** - Settings management is core functionality
2. **Notepad_plus.cpp** - Main application logic
3. **ScintillaEditView.cpp** - Editor functionality
4. **localization.cpp** - UI text localization

## Build-Only Exclusions (Can be Stubbed)

These files can have minimal stubs for building:

- `MISC/md5/md5Dlgs.cpp` - Only needed for MD5 dialog
- `MISC/Common/SortLocale.cpp` - Only needed for locale sorting
- `NppDarkMode.cpp` - Can be stubbed (Qt has native dark mode)
- `dpiManagerV2.cpp` - Can be stubbed (Qt handles DPI)

## Notes

- Files marked ❌ are completely missing and need new implementations
- Files marked ⚠️ Partial exist but may be incomplete or need more work
- Files marked ✅ have working Qt alternatives
- The Qt alternatives are in `QtControls/`, `QtCore/`, and `Platform/Linux/` directories

## Last Updated

2026-01-29 - After parallel agent fixes for Linux build
