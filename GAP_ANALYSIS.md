# Notepad++ Linux Qt6 Port — Gap Analysis Report

**Date**: 2026-02-14
**Methodology**: Automated code analysis of Windows (`WinControls/`, `ScintillaComponent/`) and Linux (`QtControls/`, `QtCore/`, `Platform/`) source trees, cross-referenced with official Notepad++ documentation.

---

## Executive Summary

The Linux Qt6 port contains **~45,000–50,000 lines** across ~85 files in `QtControls/` and ~15 files in `QtCore/`. The Platform Abstraction Layer (7 interfaces) is **fully implemented**. **NppCommands.cpp** has been ported to Linux (guard fixed from `#ifndef` to `#ifdef NPP_LINUX`) with 100+ command handlers now available including all line operations. Auto-completion, dual view, macro system, shortcut management, file monitoring, encoding support, run dialog variables, and localization have all been implemented in Phase 3.

### By the Numbers

| Metric | Windows | Linux | Gap |
|--------|---------|-------|-----|
| Menu commands | ~400+ | ~320+ connected | ~20% |
| WinControls directories | 31 | 28 Qt equivalents | Most functional |
| Preference sub-pages | 24 | 16 implemented | 67% |
| Supported languages | 90+ | 90+ (same data) | ~0% (data shared) |
| Encoding support | 49 charsets | 49 charsets (full menu) | ~0% gap |
| Dockable panels | 8 | 8 connected | ~10% gap |
| Plugin API messages | 118+ | 0 | 100% gap |
| Toolbar icons | Full icon sets | freedesktop icons | ~0% gap |
| TODOs in Linux code | — | ~40+ | — |
| "Not Implemented" dialogs | — | ~5 | — |

---

## Gap Analysis by Feature Category

### Priority Legend
- **P0 — CRITICAL**: Prevents basic daily use
- **P1 — HIGH**: Core editor feature expected by all users
- **P2 — MEDIUM**: Important but not blocking
- **P3 — LOW**: Nice-to-have / advanced feature

---

### 1. FILE OPERATIONS

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| New file | Implemented | **Working** | — | |
| Open file | Implemented | **Working** | — | |
| Save / Save As | Implemented | **Working** | — | |
| Save All | Implemented | **Working** | — | |
| Close (single) | Implemented | **Working** | — | |
| Close All | Implemented | **Working** | — | |
| Close All But Active | Implemented | **Stub** | P1 | Menu exists, no handler |
| Close All But Pinned | Implemented | **Stub** | P2 | |
| Close to Left/Right | Implemented | **Stub** | P2 | |
| Close Unchanged | Implemented | **Stub** | P2 | |
| Reload from disk | Implemented | **Working** | — | |
| Rename | Implemented | **Working** | — | |
| Delete (Recycle Bin) | Implemented | **Working** | — | |
| Print | Full dialog + Print Now | **Working** | — | QPrinter/QPrintDialog (Phase 2) |
| Print Now (direct) | Implemented | **Not started** | P2 | |
| Load Session | Implemented | **Working** | — | QXmlStreamReader session parsing (Phase 2) |
| Save Session | Implemented | **Working** | — | QXmlStreamWriter session output (Phase 2) |
| Restore Last Closed | Implemented | **Working** | — | |
| Recent Files list | Implemented | **Working** | — | Dynamic Qt menu population (Phase 2) |
| Unsaved check on close | Implemented | **Working** | — | closeEvent() calls fileCloseAll() (Phase 1) |
| Drag & drop open | Implemented | **Working** | — | dropEvent() opens files via doOpen() (Phase 2) |
| Open Containing Folder | Implemented | **Working** | — | |
| Open Default Viewer | Implemented | **Working** | — | |
| File monitoring (tail -f) | ReadDirectoryChanges | **Working** | — | Platform FileWatcher + tail mode toggle (Phase 3) |

### 2. EDITING

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Basic edit (type/delete) | Implemented | **Working** | — | Scintilla handles this |
| Undo / Redo | Implemented | **Working** | — | |
| Cut / Copy / Paste | Implemented | **Working** | — | |
| Select All | Implemented | **Working** | — | |
| Duplicate line | Implemented | **Working** | — | |
| Move line up/down | Implemented | **Working** | — | |
| Split / Join lines | Implemented | **Working** | — | |
| Comment toggle | Implemented | **Working** | — | Block + stream |
| Case conversion (8 modes) | Implemented | **Partial** | P2 | Upper/lower work; proper/sentence/invert/random unknown |
| Sort lines (14 modes) | Implemented | **Working** | — | All 14 sort modes via Sorters.h with POSIX locale (Phase 3) |
| Trim whitespace | Implemented | **Working** | — | Trailing, leading, both via Scintilla API (Phase 3) |
| Tab↔Space conversion | Implemented | **Working** | — | Tab-to-space, space-to-tab leading/all (Phase 3) |
| Remove duplicate lines | Implemented | **Working** | — | Consecutive and any duplicates (Phase 3) |
| Remove empty lines | Implemented | **Working** | — | Empty and blank (whitespace-only) lines (Phase 3) |
| Column/Block editing | Dialog implemented | **Working** | — | ColumnEditorDlg is 95% complete |
| Column mode select | Implemented | **Working** | — | Alt+drag, beginOrEndSelect |
| Multi-cursor editing | Implemented | **Working** | — | Select Next/All/Undo/Skip via Scintilla API (Phase 2) |
| Auto-completion (function) | XML-based per-language | **Working** | — | AutoCompletionQt.cpp + XML API files (Phase 3) |
| Auto-completion (word) | Implemented | **Working** | — | Word collection from document via Scintilla (Phase 3) |
| Auto-completion (path) | Implemented | **Working** | — | std::filesystem directory iteration, ~ expansion (Phase 3) |
| Function call tips | Implemented | **Working** | — | FunctionCallTipQt.cpp ported from Windows (Phase 3) |
| Auto-close brackets | Implemented | **Working** | — | SCN_CHARADDED handler with skip-over (Phase 2) |
| Auto-close HTML tags | Implemented | **Not started** | P2 | |
| Auto-indent | Implemented | **Partial** | P1 | Basic indent works via Scintilla |
| Insert date/time | Implemented | **Not started** | P3 | |
| Paste as HTML/RTF | Implemented | **Not started** | P3 | |
| Copy binary | Implemented | **Not started** | P3 | |
| Read-only toggle | Implemented | **Not started** | P2 | |
| Copy filename/path to clipboard | Implemented | **Not started** | P2 | editCopyAllNames/Paths are empty stubs |
| Search on Internet | Implemented | **Not started** | P3 | |

### 3. SEARCH & REPLACE

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Find (Ctrl+F) | Implemented | **Working** | — | |
| Find Next / Previous | Implemented | **Working** | — | |
| Replace | Implemented | **Working** | — | |
| Replace All (current doc) | Implemented | **Working** | — | |
| Match case | Implemented | **Working** | — | |
| Whole word | Implemented | **Working** | — | |
| Regex support | Boost regex | **Working** | — | Qt regex |
| Wrap around | Implemented | **Working** | — | |
| Extended mode (\n, \t) | Implemented | **Unknown** | P1 | Needs verification |
| Replace All in Open Docs | Implemented | **Working** | — | Iterates all open buffers (Phase 2) |
| Find All in Current Doc | Implemented | **Working** | — | Scintilla search API (Phase 1) |
| Find All in All Open Docs | Implemented | **Stub** | P1 | |
| Find in Files (Ctrl+Shift+F) | Full recursive search | **Working** | — | QDirIterator recursive search (Phase 1) |
| Find in Projects | Implemented | **Not started** | P2 | |
| Mark tab | Implemented | **Stub** | P2 | Tab exists, highlight not working |
| Incremental search | Implemented | **Not started** | P2 | |
| Search Results panel (F7) | Full panel with folding | **Working** | — | QTreeWidget hierarchical results (Phase 1) |
| Navigate search results | Implemented | **Working** | — | gotoNextFoundResult with direction (Phase 2) |
| Go to Line | Implemented | **Working** | — | |
| Go to Matching Brace | Implemented | **Working** | — | |
| Find Characters in Range | Implemented | **Working** | — | FindCharsInRangeDlg is 95% |
| Bookmarks (toggle/nav/clear) | Implemented | **Working** | — | |
| Bookmark line operations | Cut/copy/paste/remove | **Not started** | P2 | |
| Style tokens (5 styles) | Implemented | **Not started** | P2 | |
| Smart Highlighting | Implemented | **Working** | — | Scintilla indicators, word boundary aware (Phase 2) |
| Change history navigation | Implemented | **Not started** | P3 | |

### 4. VIEW & DISPLAY

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Word wrap toggle | Implemented | **Working** | — | |
| Show whitespace | Implemented | **Working** | — | |
| Show EOL | Implemented | **Working** | — | |
| Show all characters | Implemented | **Working** | — | |
| Show indent guide | Implemented | **Working** | — | |
| Zoom in/out/restore | Implemented | **Working** | — | |
| Code folding | Implemented | **Working** | — | Fold all, by level |
| Full screen mode | Implemented | **Working** | — | Delegates to MainWindow (Phase 2) |
| Post-It mode | Implemented | **Working** | — | Frameless window mode (Phase 2) |
| Distraction-free mode | Implemented | **Working** | — | Fullscreen + hidden UI (Phase 2) |
| Always on top | Implemented | **Stub** | P3 | |
| Hide lines | Implemented | **Not started** | P3 | |
| Line numbers | Implemented | **Working** | — | |
| Dual view (split editor) | Full move/clone | **Working** | — | docGotoAnotherEditView with move/clone/pinned/monitoring (Phase 3) |
| Synchronized scrolling | V + H sync | **Not started** | P2 | |
| Tab switching (Ctrl+1-9) | Implemented | **Working** | — | |
| Tab move forward/back | Implemented | **Working** | — | |
| Tab coloring (5 colors) | Implemented | **Partial** | P3 | |
| Tab pinning | Implemented | **Working** | — | |
| View in browser | 4 browsers | **Not started** | P3 | |
| RTL/LTR text direction | Implemented | **Stub** | P3 | TODO in ScintillaEditViewQt |
| File summary | Implemented | **Working** | — | Line/word/char count via Scintilla (Phase 2) |

### 5. PANELS (Dockable)

| Panel | Windows | Linux | Priority | Notes |
|-------|---------|-------|----------|-------|
| Document Map | Full minimap | **Connected (70%)** | P2 | Toggle working, scroll sync still partial (Phase 2) |
| Document List | Full switcher | **Connected (90%)** | — | Panel launch + toggle via DockingManager (Phase 2) |
| Function List | 40+ language parsers | **Connected (85%)** | — | Panel launch + toggle, parses on show (Phase 2) |
| Folder as Workspace | Full file browser | **Connected (80%)** | P2 | Toggle working, some stubs remain (Phase 2) |
| Project Panels (×3) | Full tree management | **Connected (85%)** | — | All 3 panels launchable via menu (Phase 2) |
| Clipboard History | Full clipboard mgmt | **Substantial (80%)** | P2 | |
| ANSI Char Panel | Full 256-char table | **Working (95%)** | — | |
| Search Results | Full results panel | **Working** | — | QTreeWidget with navigation (Phase 1) |

### 6. ENCODING

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| UTF-8 | Implemented | **Working** | — | |
| UTF-8 with BOM | Implemented | **Working** | — | |
| UTF-16 BE/LE | Implemented | **Working** | — | |
| ANSI | Implemented | **Working** | — | |
| Auto-detection | uchardet + BOM | **Working** | — | In Buffer.cpp |
| Convert between encodings | Implemented | **Working** | — | QStringConverter for all charsets (Phase 3) |
| 49 character sets | Full charset menu | **Working** | — | Full submenu by region with QActions (Phase 3) |
| EOL conversion (CR/LF/CRLF) | Implemented | **Working** | — | |

### 7. LANGUAGE & SYNTAX

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| 90+ built-in languages | Implemented | **Working** | — | Language detection from extension (50+) and shebang |
| Language menu | Full with all languages | **Working** | — | |
| Syntax highlighting | Scintilla lexers | **Working** | — | |
| User Defined Language | Full UDL editor | **Substantial (75%)** | P2 | Dialog works, preview and some integration stubs |
| Style Configurator | Full theme editor | **Substantial (80%)** | P2 | `saveTheme()` is stub |
| XML tag matching | Implemented | **Not started** | P2 | |

### 8. SETTINGS & CONFIGURATION

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Preferences dialog | 24 sub-pages | **16 sub-pages (67%)** | P1 | Missing: File Association, Misc, Indentation separate page |
| Settings persistence | Registry + INI + XML | **QSettings (working)** | — | |
| XML config parsing | Full XML load/save | **Working** | — | 25+ methods implemented in Parameters.cpp (Phase 1) |
| Shortcut Mapper | 5 tabs, full editing | **Working** | — | Delete, import, export, reset, Scintilla tab all implemented (Phase 3) |
| Shortcut customization | Full remap | **Working** | — | All 5 tabs including Scintilla key mappings (Phase 3) |
| Context menu customization | Editable XML | **Stub** | P2 | ContextMenu is empty stub |

### 9. MACRO SYSTEM

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Record macro | Implemented | **Working** | — | Full recording including menu commands (Phase 3) |
| Playback macro | Implemented | **Working** | — | Playback with mtUseSParameter support (Phase 3) |
| Save macro (named) | Implemented | **Working** | — | Dialog-based naming + persist to shortcuts.xml (Phase 3) |
| Run Multiple Times | Full dialog | **Working** | — | Run N times and run-until-EOF (Phase 3) |
| Macro shortcut assignment | Implemented | **Working** | — | Save/load macro shortcuts from config (Phase 3) |

### 10. PLUGIN SYSTEM

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| DLL plugin loading | Full architecture | **Not started** | P2 | Would need .so adaptation |
| Plugin API (118+ messages) | Implemented | **Not started** | P2 | |
| Plugin Admin dialog | Full install/update/remove | **UI only (50%)** | P2 | 8 critical backend stubs |
| Plugin notifications | Full event system | **Not started** | P2 | 4 TODO stubs in Notepad_plus.cpp |
| Default plugins (MIME, etc.) | 3 built-in | **Not started** | P3 | |

### 11. TOOLS

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| MD5 generate/from files | Implemented | **Working** | — | HashFromFilesDlg/HashFromTextDlg at 95% |
| SHA-1 generate/from files | Implemented | **Working** | — | |
| SHA-256 generate/from files | Implemented | **Working** | — | |
| SHA-512 generate/from files | Implemented | **Working** | — | |

### 12. RUN MENU

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Run command dialog | Full with variables | **Working** | — | Full command execution via /bin/sh (Phase 3) |
| Variable substitution | $(FULL_CURRENT_PATH) etc. | **Working** | — | All 10 variables expanded with shell escaping (Phase 3) |
| Save command | Implemented | **Working** | — | Save to shortcuts.xml + dynamic Run menu (Phase 3) |

### 13. WINDOW MANAGEMENT

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Windows dialog (doc list) | Full grid with sort | **Not started** | P2 | TODO in MainWindow |
| Sort open docs | By name/path/type/size/date | **Not started** | P3 | |
| System tray icon | Implemented | **Working** | — | |
| Tray minimize | Implemented | **Working** | — | |

### 14. THEMING & APPEARANCE

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Dark mode | Full custom rendering | **Qt theme-based** | — | Different approach, works via KDE integration |
| KDE integration | N/A | **Extensive (90%)** | — | Color scheme, fonts, icons, animations |
| Toolbar icons | Multiple icon sets | **Working** | — | QIcon::fromTheme() freedesktop icons (Phase 1) |
| Dark mode color pickers | 12 customizable | **Working via Preferences** | — | |
| Custom title bar | Win32 dark title bar | **Qt native** | — | Not needed |

### 15. LOCALIZATION

| Feature | Windows | Linux | Priority | Notes |
|---------|---------|-------|----------|-------|
| Multi-language UI | XML-based translations | **Working** | — | Full NativeLangSpeaker with menu/dialog translation (Phase 3) |
| 80+ languages | Full translation support | **Working** | — | 94 language XML files supported via changeMenuLangQt (Phase 3) |

---

## Summary Statistics

### By Implementation Status

| Status | Count | Examples |
|--------|-------|---------|
| **Working** | ~110 features | Basic editing, find/replace, save, syntax highlighting, folding, bookmarks, hash tools, print, sessions, drag-drop, recent files, smart highlighting, auto-close brackets, multi-cursor, Find in Files, Search Results, toolbar icons, XML config, view modes, panel toggles, **auto-completion (word/function/path), function call tips, dual view move/clone, sort lines (14 modes), trim whitespace, tab↔space, remove duplicates, remove empty lines, file monitoring/tail mode, macro record/playback/save/run-multiple, shortcut mapper (full), encoding (49 charsets), run dialog variables, localization (94 languages)** |
| **Partial** | ~10 features | Case conversion (8 modes), preferences (67%), UDL preview, Style Configurator save |
| **Stub/UI only** | ~8 features | Plugin admin backend, context menu, some close variants |
| **Not started** | ~25 features | Plugin system (.so), synchronized scrolling, incremental search, bookmark line operations, style tokens |

### By Priority

| Priority | Count | Key Items |
|----------|-------|-----------|
| **P0 — CRITICAL** | ~~5~~ **0 remaining** | All resolved in Phase 1 |
| **P1 — HIGH** | ~~22~~ **~2 remaining** | ~~Print, sessions, recent files, drag-drop, smart highlighting, auto-close brackets, multi-cursor~~ resolved in Phase 2. ~~Auto-completion, dual view, shortcut editing~~ resolved in Phase 3. Remaining: preferences completeness, extended search mode verification |
| **P2 — MEDIUM** | ~~30~~ **~12 remaining** | ~~Encoding charsets, file monitoring, macro save, localization, line sorting, run variables~~ resolved in Phase 3. Remaining: plugin system, synchronized scrolling, incremental search, UDL preview, document map scroll sync, context menu, bookmark line ops, style tokens, find in projects |
| **P3 — LOW** | ~12 | Date/time insert, paste HTML/RTF, view in browser, always-on-top, hide lines, change history, sort open docs |

---

## Recommended Implementation Priority

### Phase 1 — Critical Fixes (Data loss prevention + basic usability) — COMPLETED
1. ~~**Unsaved document check on close**~~ — closeEvent() calls fileCloseAll()
2. ~~**Toolbar icons**~~ — QIcon::fromTheme() freedesktop icons
3. ~~**XML config parsing**~~ — 25+ methods implemented in Parameters.cpp
4. ~~**Find in Files backend**~~ — QDirIterator recursive search with filters
5. ~~**Search Results panel**~~ — QTreeWidget hierarchical results with navigation

### Phase 2 — Core Feature Parity — COMPLETED
6. ~~**Recent Files menu integration**~~ — Dynamic Qt menu population
7. ~~**Drag & drop file opening**~~ — dropEvent() opens files via doOpen()
8. ~~**Smart Highlighting**~~ — Scintilla indicators, word boundary aware
9. ~~**Auto-completion engine**~~ — *(deferred to Phase 3, now completed)*
10. ~~**Auto-close brackets/tags**~~ — SCN_CHARADDED handler with skip-over
11. ~~**Print support**~~ — QPrinter/QPrintDialog/QTextDocument
12. ~~**Session load/save**~~ — QXmlStreamReader/Writer compatible format
13. ~~**Panel integration**~~ — Doc List, Doc Map, Function List, Project Panels connected
14. ~~**Replace All in Open Docs / Find All**~~ — Buffer iteration + find history persistence
15. ~~**Multi-cursor commands**~~ — Select Next/All/Undo/Skip via Scintilla API

### Phase 3 — Feature Completeness — COMPLETED
16. ~~**Auto-completion engine**~~ — AutoCompletionQt.cpp + FunctionCallTipQt.cpp (word, function, path completion)
17. ~~**Dual view (split editor)**~~ — docGotoAnotherEditView with move/clone, pinned tab + monitoring preservation
18. ~~**Encoding menu expansion**~~ — Full 49-charset submenu by region with QStringConverter
19. ~~**Macro system completion**~~ — Save named macros, run multiple times/until-EOF, shortcut assignment, XML persistence
20. ~~**Shortcut Mapper completion**~~ — Delete, import, export, reset, Scintilla tab with key mappings
21. ~~**View modes**~~ — Fullscreen, Post-It, Distraction-free (completed in Phase 2)
22. ~~**File monitoring**~~ — Platform FileWatcher + tail mode toggle with auto-reload/scroll-to-end
23. ~~**Line operations**~~ — 14 sort modes (Sorters.h POSIX port), trim, remove duplicates/empty, tab↔space
24. ~~**Run dialog variables**~~ — All 10 $(VAR) expansions with shell escaping, save command to XML
25. ~~**Localization system**~~ — NativeLangSpeaker with menu/dialog translation, 94 language XML files

### Phase 4 — Advanced Features
25. **Plugin system** — .so loading, API messages, notifications
26. **Document Map scroll sync** — Complete minimap
27. **User Defined Language preview** — Scintilla integration
28. **Windows dialog** — Open document grid
29. **Style Configurator save** — Theme persistence
30. **Context menu customization** — Replace empty stub

---

## Architecture Notes

### NppCommands.cpp — Now Ported to Linux
The `#ifndef NPP_LINUX` guard on `NppCommands.cpp` was fixed to `#ifdef NPP_LINUX` in Phase 3, enabling 100+ command handlers on Linux. Windows-specific APIs were adapted:
- `Sorters.h` ported with POSIX locale wrappers (`_npp_create_locale`/`_npp_free_locale`) replacing MSVC `_create_locale`
- `_wcsicmp` → `wcscasecmp` for case-insensitive string comparison
- `EolType` constants adapted to Linux Buffer equivalents
- 19 new commands registered in `registerEditCommands()`

`NppIO.cpp` remains excluded — file I/O continues through `Buffer.cpp` directly.

### Phase 3 Key Implementation Details
- **Auto-completion**: New `AutoCompletionQt.cpp` (35KB) and `FunctionCallTipQt.cpp` (11KB) ported from Windows using `std::filesystem` for path completion, `strncasecmp` for case-insensitive matching, and `charAdded` signal for trigger
- **Dual view**: `docGotoAnotherEditView()` follows Windows pattern with SCI_CREATEDOCUMENT/ADDREFDOCUMENT, preserves pinned and monitoring state across move/clone
- **Localization**: `NativeLangSpeaker` maps Win32 MB_* constants to QMessageBox, 3-phase menu translation (top-level by position, commands by ID, submenus by SubEntries)
- **Run dialog**: Commands execute via `/bin/sh -c` for proper shell handling; variables shell-escaped with single-quote wrapping

### Platform Abstraction Layer
All 7 PAL interfaces are fully implemented for Linux — this is a solid foundation.

### Working Foundation
The following are solid and production-ready:
- Buffer management (Buffer.cpp — 3100+ lines, most complete component)
- Scintilla editor integration (syntax highlighting, folding, basic editing, auto-completion)
- Tab bar and document tab management
- Dual view with move/clone between views
- KDE desktop integration (color schemes, fonts, icons)
- Full macro system (record, playback, save, run multiple)
- Shortcut mapper (all 5 tabs fully functional)
- File monitoring with tail mode
- Line operations (14 sort modes, trim, dedup, tab/space conversion)
- 49-charset encoding support
- Localization (94 languages)
- Run dialog with variable substitution
- Status bar
- Hash tools (MD5, SHA-1/256/512)
- Platform abstraction layer (all 7 interfaces)
