# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Notepad++ is a free source code editor and Notepad replacement. This repository contains:
- **Windows**: Native Win32 API implementation (original)
- **Linux**: Native Qt6 port using a Platform Abstraction Layer (in progress)

## Build Commands

### Linux (Qt6 Port)

```bash
# Basic build
mkdir build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Debug build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run the application
./notepad-plus-plus
./notepad-plus-plus /path/to/file.txt
```

### Windows

Visual Studio 2022 (v17.5+):
```bash
# Open solution
PowerEditor/visual.net/notepadPlus.sln

# Build configurations: Debug/Release x64/Win32/ARM64
```

MinGW-w64/MSYS2:
```bash
cd PowerEditor/gcc
mingw32-make
```

## Testing

### Linux Tests

```bash
# Build with tests
cmake ../PowerEditor/src -DBUILD_TESTING=ON
make -j$(nproc)

# Run all tests
./run_tests.sh

# Run specific test categories
./run_tests.sh --platform-only      # Platform abstraction tests
./run_tests.sh --qtcontrols-only    # UI control tests
./run_tests.sh --integration-only   # Integration tests

# Or use ctest
ctest --output-on-failure
```

### Windows Tests

Function list parser tests are in `PowerEditor/Test/`. Run with Python.

## Architecture

### Platform Abstraction Layer (PAL)

Located in `PowerEditor/src/Platform/`, the PAL enables cross-platform support:

| Interface | Purpose | Windows | Linux |
|-----------|---------|---------|-------|
| `IFileSystem` | File operations | Win32 API | std::filesystem + POSIX |
| `ISettings` | Configuration | Registry + INI | XDG directories + INI |
| `IProcess` | Process management | CreateProcess | fork() + exec() |
| `IFileWatcher` | File monitoring | ReadDirectoryChangesW | inotify |
| `IClipboard` | Clipboard | Win32 API | Qt Clipboard |
| `IThreading` | Thread utilities | Win32 Threads | std::thread |
| `IDialogs` | Common dialogs | Win32 Dialogs | Qt Dialogs |

Usage pattern:
```cpp
auto& fs = Platform::IFileSystem::getInstance();
fs.fileExists(path);
```

### UI Control Layers

**Windows**: `PowerEditor/src/WinControls/` - Native Win32 controls
**Linux**: `PowerEditor/src/QtControls/` - Qt6-based implementations

The QtControls framework mirrors Windows control hierarchy to minimize application layer changes. Key mappings:
- `HWND` → `QWidget*`
- `StaticDialog` → `QtControls::StaticDialog`
- `ToolBar` → `QtControls::ToolBar`

### Key Source Files

| File | Purpose |
|------|---------|
| `Notepad_plus.cpp` | Main application class (~369K) |
| `Parameters.cpp` | Configuration management (~364K) |
| `NppCommands.cpp` | Command handling (~134K) |
| `NppBigSwitch.cpp` | Message handling (~130K) |
| `NppDarkMode.cpp` | Theming (~115K) |
| `NppIO.cpp` | File I/O operations (~90K) |
| `FindReplaceDlg.cpp` | Search/replace (~228K) |
| `ScintillaEditView.cpp` | Editor component wrapper (~179K) |
| `Buffer.cpp` | Document buffer management |

### Linux Port Status

The Linux port is functional but incomplete. See `REMAINING_WORK.md` for current status. Key missing implementations:
- `QtCore::FileManager::loadFile()` - File loading
- `QtCore::Buffer::getFullPathName()` - Path retrieval
- `QtCore::Buffer::setUnsync()` - Sync tracking

### Testing with Computer Use MCP

When using Claude Code's computer-use MCP with the Notepad++ Qt6 port:

**Screenshot Configuration (Wayland/KDE)**
The default computer-use-mcp uses nut-tree-fork/nut-js which has X11 compatibility issues on Wayland. To fix this:

1. Install spectacle (KDE's screenshot tool):
   ```bash
   sudo pacman -S spectacle
   ```

2. Patch the MCP server to use spectacle instead of nut-js for screenshots:
   - File: `~/.local/share/fnm/node-versions/v24.3.0/installation/lib/node_modules/computer-use-mcp/dist/tools/computer.js`
   - Replace `screen.grab()` with `execAsync('spectacle -b -o <tmpfile>')`
   - Use `readFileSync()` to read the screenshot file

3. Environment variables needed:
   ```bash
   export DISPLAY=:0
   export WAYLAND_DISPLAY=wayland-0
   export XDG_SESSION_TYPE=wayland
   export XAUTHORITY=/run/user/1000/xauth_<hash>
   ```

**Screenshot Timeout Fix (Critical)**
Claude Code has a hardcoded 10-second timeout for MCP tools. The default screenshot implementation (`spectacle`) takes ~11 seconds, causing timeouts. The fix involves:

1. **Replace `spectacle` with `ffmpeg`** in `computer.js`:
   ```javascript
   // Before (slow):
   await execAsync(`spectacle -b -o "${tmpFile}"`, { timeout: 30000 });

   // After (fast):
   await execAsync(`ffmpeg -f x11grab -i :0 -vframes 1 "${tmpFile}" -y`, { timeout: 5000 });
   ```

2. **Remove delays and optimize polling**:
   - Remove `await setTimeout(1000)` initial delay
   - Reduce polling: `while (!existsSync(tmpFile) && attempts < 20)` with `setTimeout(50)`

3. **Use sharp instead of Jimp** for image processing (faster resize)

4. **Skip crosshair drawing** (pixel-by-pixel JavaScript is slow for 4K displays)

5. **Reconnect MCP server** after changes:
   ```bash
   claude mcp remove computer-use
   claude mcp add --transport stdio computer-use -- /home/josh/.local/bin/computer-use-mcp-wrapper.sh
   ```

**Debugging Screenshot Issues**
If screenshots fail with `ETIMEDOUT` or `Connection closed`:

1. Test MCP server directly:
   ```bash
   echo '{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"computer","arguments":{"action":"get_screenshot"}}}' | /home/josh/.local/bin/computer-use-mcp-wrapper.sh
   ```
   - If this works but Claude fails, the MCP server needs reconnection

2. Check timing of individual components:
   ```bash
   time spectacle -b -o /tmp/test.png        # Should be < 10s
   time ffmpeg -f x11grab -i :0 -vframes 1 /tmp/test.png -y  # Should be < 1s
   ```

3. Verify MCP server status:
   ```bash
   claude mcp list
   ```

4. Common errors:
   - `spawnSync /bin/sh ETIMEDOUT` - Screenshot tool too slow, use ffmpeg
   - `Connection closed` - MCP server crashed or needs reconnection
   - `Screenshot file not created` - Display/permissions issue with X11

**Keyboard Automation**
When screenshots are unavailable, keyboard commands work reliably:
- `ctrl+n` - New file
- `ctrl+s` - Save
- `ctrl+shift+s` - Save As (use absolute paths like `/home/josh/filename.txt`)
- `ctrl+a` - Select all (useful for replacing filename in save dialog)

**Known Quirks**
- The Qt6 save dialog may interpret `++` in filenames as `==` due to keyboard event handling
- Use absolute paths (starting with `/`) to ensure files save to correct location
- Use `ctrl+shift+s` (Save As) rather than `ctrl+s` for first save to specify location

## Coding Standards

From `CONTRIBUTING.md`:

### Style
- **Bracing**: Allman style (not Java-style)
- **Indentation**: Tabs (4-space equivalent)
- **Language**: C++20

### Naming
- Classes: `PascalCase`
- Methods/parameters: `camelCase`
- Member variables: prefixed with underscore (`_memberName`)

### Key Rules
- Use `!`, `&&`, `||` (not `not`, `and`, `or`)
- Prefer C++ casts over C-style casts
- Use `empty()` for string emptiness checks
- Initialize with curly braces: `MyClass instance{10.4}`
- Prefer pre-increment (`++i`) over post-increment
- Avoid `using namespace` in headers
- Use `unique_ptr` over `shared_ptr` when possible

### Platform-Specific Code

Use conditional compilation:
```cpp
#ifdef NPP_LINUX
    // Linux-specific code
#else
    // Windows code
#endif
```

CMake handles source file selection based on platform.

## Contribution Guidelines

- **First rule**: You do not ask for permission to contribute
- **Second rule**: You DO NOT ask for permission to contribute
- Create unique branch names (add dates: `feature_20240129`)
- Single feature/fix per PR
- PRs need attachment to issues (except translations/docs)
- Issues need `Accepted` label from administrators
- Respect existing coding style
- Do not reformat source code in PRs
- Test changes before submitting

## Documentation

- `README.md` - Project overview
- `BUILD.md` - Windows build instructions
- `docs/BUILD.md` - Linux build instructions
- `docs/ARCHITECTURE.md` - Detailed architecture
- `docs/PORTING.md` - Porting guide
- `REMAINING_WORK.md` - Linux port pending tasks
- `CONTRIBUTING.md` - Full contribution guidelines
