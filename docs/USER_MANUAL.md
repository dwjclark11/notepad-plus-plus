# Notepad++ Linux User Manual

This manual covers Linux-specific features and usage of Notepad++.

## Table of Contents

- [Getting Started](#getting-started)
- [Linux-Specific Features](#linux-specific-features)
- [KDE Plasma Integration](#kde-plasma-integration)
- [Keyboard Shortcuts](#keyboard-shortcuts)
- [Configuration Locations](#configuration-locations)
- [Command Line Options](#command-line-options)
- [Tips and Tricks](#tips-and-tricks)

## Getting Started

### First Launch

When you first start Notepad++ on Linux:

1. **Configuration Directory**: Notepad++ creates its configuration in `~/.config/notepad++/`
2. **Theme Detection**: Automatically detects dark/light mode based on system settings
3. **Font Configuration**: Uses system monospace font by default

### Basic Interface

The Notepad++ interface on Linux consists of:

- **Menu Bar**: File, Edit, Search, View, Encoding, Language, Settings, Macro, Run, Plugins, Window, ?
- **Tool Bar**: Quick access to common functions
- **Tab Bar**: Multiple document interface
- **Editor Area**: Main text editing region
- **Status Bar**: Line/column info, encoding, EOL mode
- **Side Panels**: Document Map, Function List, etc.

## Linux-Specific Features

### XDG Base Directory Support

Notepad++ follows the [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html):

| Data Type | Location |
|-----------|----------|
| Configuration | `~/.config/notepad++/` |
| User Data | `~/.local/share/notepad++/` |
| Cache | `~/.cache/notepad++/` |

### Native File Operations

- **Trash Integration**: Files deleted in Notepad++ are moved to the XDG Trash
- **File Watching**: Uses `inotify` for efficient file change monitoring
- **Symbolic Links**: Full support for creating and editing symbolic links

### Desktop Integration

Notepad++ integrates with the Linux desktop environment:

- **File Manager**: Open files from context menu (when configured)
- **Recent Files**: Appears in recent documents
- **MIME Types**: Associates with text files

### Terminal Integration

Launch Notepad++ from terminal:

```bash
# Open new instance
notepad-plus-plus

# Open specific file
notepad-plus-plus file.txt

# Open multiple files
notepad-plus-plus file1.txt file2.cpp file3.h

# Open at specific line
notepad-plus-plus -n100 file.txt    # Go to line 100

# Open with specific encoding
notepad-plus-plus -encoding=utf8 file.txt
```

## KDE Plasma Integration

Notepad++ provides enhanced integration with KDE Plasma desktop environment.

### Native Styling

When running on KDE Plasma:

- **Window Decorations**: Uses native KDE window borders
- **Color Scheme**: Follows KDE color scheme automatically
- **Icon Theme**: Uses current KDE icon theme

### KDE-Specific Features

#### KIO Slaves Support

Open files from KDE virtual file systems:

- `sftp://` - SSH/SFTP remote files
- `fish://` - FISH protocol
- `smb://` - Windows shares
- `webdav://` - WebDAV shares

#### KRunner Integration

Search and open files with KRunner:

1. Press `Alt+Space` to open KRunner
2. Type file name
3. Select "Open with Notepad++"

#### Plasma Activities

Notepad++ respects Plasma Activities:

- Remembers which activity a file was opened in
- Session restoration per activity

### Configuration for KDE

#### Recommended Settings

```ini
# ~/.config/notepad++/config.xml additions for KDE
<GUIConfig name="KDE">
    <Item id="NativeFileDialog" value="1"/>  ; Use KDE file dialogs
    <Item id="SingleClickOpen" value="1"/>   ; Respect KDE single-click setting
    <Item id="UseSystemIcons" value="1"/>    ; Use KDE icon theme
</GUIConfig>
```

#### Qt Platform Plugins

Force specific platform integration:

```bash
# Use native KDE file dialogs
QT_QPA_PLATFORM=xcb notepad-plus-plus

# Use Qt's built-in dialogs
QT_QPA_PLATFORM=offscreen notepad-plus-plus
```

## Keyboard Shortcuts

### Default Shortcuts

Notepad++ uses standard shortcuts with Linux conventions:

#### File Operations

| Action | Shortcut |
|--------|----------|
| New | Ctrl+N |
| Open | Ctrl+O |
| Save | Ctrl+S |
| Save As | Ctrl+Shift+S |
| Save All | Ctrl+Alt+S |
| Close | Ctrl+W |
| Close All | Ctrl+Shift+W |
| Print | Ctrl+P |
| Exit | Ctrl+Q |

Note: `Ctrl+Q` for Exit is the Linux convention (vs Alt+F4 on Windows).

#### Edit Operations

| Action | Shortcut |
|--------|----------|
| Undo | Ctrl+Z |
| Redo | Ctrl+Shift+Z |
| Cut | Ctrl+X |
| Copy | Ctrl+C |
| Paste | Ctrl+V |
| Delete | Delete |
| Select All | Ctrl+A |
| Find | Ctrl+F |
| Replace | Ctrl+H |
| Go To | Ctrl+G |

#### View Operations

| Action | Shortcut |
|--------|----------|
| Full Screen | F11 |
| Post-It | F12 |
| Word Wrap | Alt+Z |
| Show All Characters | Ctrl+Alt+S |
| Zoom In | Ctrl++ |
| Zoom Out | Ctrl+- |
| Restore Default Zoom | Ctrl+/ |

#### Tab Navigation

| Action | Shortcut |
|--------|----------|
| Next Tab | Ctrl+Tab |
| Previous Tab | Ctrl+Shift+Tab |
| Close Tab | Ctrl+W |
| Move Tab Forward | Ctrl+Shift+PageUp |
| Move Tab Backward | Ctrl+Shift+PageDown |

### Customizing Shortcuts

1. Go to **Settings > Shortcut Mapper**
2. Select category (Main menu, Macros, Run commands, Plugin commands, Scintilla commands)
3. Double-click the shortcut you want to change
4. Press the new key combination
5. Click OK

### Linux-Specific Shortcuts

Additional shortcuts for Linux users:

| Action | Shortcut | Description |
|--------|----------|-------------|
| Terminal | Ctrl+Alt+T | Open terminal in current file's directory |
| Copy Path | Ctrl+Shift+C | Copy full file path to clipboard |
| Reveal in File Manager | Ctrl+Alt+R | Open containing folder |

## Configuration Locations

### Main Configuration Files

| File | Purpose |
|------|---------|
| `~/.config/notepad++/config.xml` | Main settings |
| `~/.config/notepad++/shortcuts.xml` | Keyboard shortcuts |
| `~/.config/notepad++/langs.xml` | Language definitions |
| `~/.config/notepad++/stylers.xml` | Color schemes |
| `~/.config/notepad++/contextMenu.xml` | Context menu |
| `~/.config/notepad++/toolbarIcons.xml` | Toolbar customization |

### Session and Backup

| File/Directory | Purpose |
|----------------|---------|
| `~/.config/notepad++/session.xml` | Current session |
| `~/.local/share/notepad++/backup/` | Backup files |
| `~/.cache/notepad++/` | Temporary files |

### User-Defined Files

| Directory | Contents |
|-----------|----------|
| `~/.config/notepad++/userDefineLangs/` | Custom language definitions |
| `~/.config/notepad++/themes/` | Custom themes |
| `~/.local/share/notepad++/plugins/` | Plugin files |

### Importing Windows Configuration

If you have a Windows configuration you want to use:

```bash
# Create Linux config directory
mkdir -p ~/.config/notepad++

# Copy Windows config (adjust path as needed)
cp /path/to/windows/config.xml ~/.config/notepad++/
cp -r /path/to/windows/userDefineLangs ~/.config/notepad++/
cp -r /path/to/windows/themes ~/.config/notepad++/

# Fix line endings
dos2unix ~/.config/notepad++/*.xml
```

## Command Line Options

### Basic Usage

```bash
notepad-plus-plus [options] [files]
```

### Available Options

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message |
| `-v`, `--version` | Show version information |
| `-multiInst` | Launch new instance |
| `-nosession` | Don't load previous session |
| `-notabbar` | Hide tab bar |
| `-ro` | Read-only mode |
| `-ln<lang>` | Set language (e.g., `-lncpp`) |
| `-n<line>` | Go to line number |
| `-c<col>` | Go to column number |
| `-p<pos>` | Go to position |
| `-x<left>` | Set window X position |
| `-y<top>` | Set window Y position |
| `-lang=<lang>` | Set UI language |
| `-settingsDir=<dir>` | Use alternative config directory |

### Examples

```bash
# Open file at specific line and column
notepad-plus-plus -n100 -c20 file.txt

# Open with specific language
notepad-plus-plus -lnpython script.py

# New instance without session
notepad-plus-plus -multiInst -nosession file.txt

# Read-only mode
notepad-plus-plus -ro important.conf

# Use custom settings directory
notepad-plus-plus -settingsDir=/path/to/config

# Open multiple files
notepad-plus-plus file1.txt file2.cpp file3.h

# Open from stdin
cat file.txt | notepad-plus-plus -
```

### Environment Variables

| Variable | Description |
|----------|-------------|
| `NPP_CONFIG_DIR` | Override config directory |
| `NPP_PLUGIN_DIR` | Override plugin directory |
| `QT_SCALE_FACTOR` | UI scaling factor |
| `QT_AUTO_SCREEN_SCALE_FACTOR` | Enable auto HiDPI |
| `QT_QPA_PLATFORM` | Force platform plugin (xcb, wayland) |

## Tips and Tricks

### Working with Different Line Endings

Linux and Windows use different line endings:

- **Linux/Unix**: LF (`\n`)
- **Windows**: CRLF (`\r\n`)
- **Classic Mac**: CR (`\r`)

Notepad++ handles all three automatically.

To convert line endings:

1. Go to **Edit > EOL Conversion**
2. Select desired format:
   - Windows (CR LF)
   - Unix (LF)
   - Macintosh (CR)

Or use status bar:

1. Click on the EOL indicator in the status bar
2. Select conversion from popup menu

### File Permissions

View and edit file permissions:

1. Right-click on file tab
2. Select "Properties"
3. View/change permissions in the dialog

Or use the command palette:

1. Press `Ctrl+Shift+P`
2. Type "Properties"
3. Press Enter

### Working with Root-Owned Files

To edit system files:

```bash
# Method 1: Using pkexec
pkexec notepad-plus-plus /etc/fstab

# Method 2: Save with elevated permissions
# Edit normally, then use File > Save with elevated privileges
```

### Customizing the Interface

#### Themes

1. Go to **Settings > Style Configurator**
2. Select theme from dropdown
3. Click "Save & Close"

Available themes:
- Default (stylus)
- Black board
- Choco
- Deep Black
- Hello Kitty
- Hot Fudge Sundae
- Khaki
- Mono Industrial
- Monokai
- Obsidian
- Plastic Code Wrap
- Ruby Blue
- Solarized
- Solarized Light
- Twilight
- Vibrant Ink
- Zenburn

#### Toolbar Customization

1. Right-click on toolbar
2. Select "Customize"
3. Drag icons to rearrange
4. Click "Close" when done

### Macro Recording

Automate repetitive tasks:

1. Press `Ctrl+Shift+R` to start recording
2. Perform the actions
3. Press `Ctrl+Shift+R` to stop
4. Press `Ctrl+Shift+P` to play back

Save macros:

1. Go to **Macro > Save Current Recorded Macro**
2. Name the macro
3. Assign shortcut (optional)
4. Click OK

### Column Mode Editing

Edit multiple lines simultaneously:

1. Hold `Alt` while selecting with mouse
2. Or hold `Alt+Shift` and use arrow keys
3. Type to edit all selected lines

Useful for:
- Adding/removing comments on multiple lines
- Aligning code
- Bulk editing

### Regular Expressions

Powerful search and replace with regex:

1. Open Find/Replace (`Ctrl+H`)
2. Enable "Regular expression" mode
3. Use regex patterns

Common patterns:

| Pattern | Matches |
|---------|---------|
| `^` | Start of line |
| `$` | End of line |
| `\d` | Digit |
| `\w` | Word character |
| `\s` | Whitespace |
| `.` | Any character |
| `*` | 0 or more |
| `+` | 1 or more |
| `?` | 0 or 1 |
| `()` | Capture group |

Example: Add line numbers

Find: `^(.*)$`
Replace: `\1: $1`

### Function List

Navigate code structure:

1. Go to **View > Function List**
2. Click on function to jump to it
3. Supports many programming languages

### Document Map

Overview of entire document:

1. Go to **View > Document Map**
2. Click on area to navigate
3. Drag to scroll quickly

### Multi-View Editing

View two files side-by-side:

1. Right-click on file tab
2. Select "Move to Other View" or "Clone to Other View"
3. Drag splitter to adjust sizes

### Search in Files

Search across multiple files:

1. Press `Ctrl+Shift+F`
2. Enter search term
3. Select directory
4. Set file filters (e.g., `*.cpp;*.h`)
5. Click "Find All"

### Bookmarks

Mark important lines:

- Toggle bookmark: `Ctrl+F2`
- Next bookmark: `F2`
- Previous bookmark: `Shift+F2`
- Clear all bookmarks: `Ctrl+Shift+F2`

### Code Folding

Collapse/expand code blocks:

- Fold all: `Alt+0`
- Unfold all: `Alt+Shift+0`
- Fold current level: `Alt+(level number)`
- Click `[-]` or `[+]` in margin

### Auto-Completion

Enable code completion:

1. Go to **Settings > Preferences > Auto-Completion**
2. Enable "Auto-Completion on each input"
3. Set minimum characters
4. Choose what to complete (words, functions, etc.)

### Multi-Instance

Run multiple Notepad++ instances:

```bash
# From command line
notepad-plus-plus -multiInst

# Or set in preferences
# Settings > Preferences > Multi-Instance
```

### Backup and Session

Configure automatic backup:

1. Go to **Settings > Preferences > Backup**
2. Enable "Enable session snapshot and periodic backup"
3. Set backup interval
4. Choose backup path

### Performance Tips

For large files:

1. Disable word wrap (View > Word Wrap)
2. Disable document map for very large files
3. Increase "Large file restriction" threshold
4. Use "Find in Files" instead of opening all files

### Troubleshooting

#### Slow Startup

- Disable unnecessary plugins
- Clear recent files history
- Check file system watcher settings

#### Font Rendering Issues

```bash
# Force font antialiasing
export QT_QPA_PLATFORM=xcb
notepad-plus-plus
```

#### High Memory Usage

- Disable session snapshot
- Close unused documents
- Reduce undo levels (Settings > Preferences > Misc.)

#### Plugin Issues

Not all Windows plugins work on Linux. Check compatibility:

| Plugin | Status | Notes |
|--------|--------|-------|
| NppExport | Working | Basic functionality |
| Converter | Working | Native |
| mimeTools | Working | Native |
| Compare | Testing | In development |
| XML Tools | Testing | In development |

### Getting Help

- **Built-in Help**: Press `F1` or go to **? > Help**
- **Online Documentation**: https://npp-user-manual.org/
- **Community Forum**: https://community.notepad-plus-plus.org/
- **GitHub Issues**: https://github.com/notepad-plus-plus/notepad-plus-plus/issues

### Reporting Bugs

When reporting bugs, include:

1. Notepad++ version (Help > About)
2. Linux distribution and version
3. Desktop environment (KDE, GNOME, etc.)
4. Qt version
5. Steps to reproduce
6. Expected vs actual behavior
7. Relevant configuration files (sanitized)

To get debug info:

1. Go to **? > Debug Info...**
2. Click "Copy debug info into clipboard"
3. Paste in bug report
