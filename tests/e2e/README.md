# Notepad++ End-to-End Tests

Visual E2E tests that launch the full Notepad++ Qt6 application inside an
offscreen virtual display and interact with it via mouse/keyboard, verifying
behavior through screenshots and window state.

## Prerequisites

### 1. Build Notepad++

```bash
cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### 2. Build the offscreen MCP server

```bash
cd tools/computer-use-mcp
npm install
npm run build
```

### 3. System packages (Arch Linux)

```bash
sudo pacman -S xorg-server-xvfb xdotool ffmpeg xorg-xdpyinfo openbox wmctrl
```

On other distros, install the equivalent packages for Xvfb, xdotool, ffmpeg,
xdpyinfo, openbox, and wmctrl.

## Running tests

```bash
# Run all E2E tests
./tests/e2e/run-all.sh

# Run only tests matching a keyword
./tests/e2e/run-all.sh zoom

# Run a single test directly
node tests/e2e/test-zoom-toolbar.mjs

# Override the binary path
NPP_BINARY=/path/to/notepad-plus-plus ./tests/e2e/run-all.sh
```

## Writing new tests

1. Create a file named `test-<feature>.mjs` in this directory.
2. Import the shared helpers:

```js
import {
    setup, teardown, launchNpp, screenshot, click, key, type,
    sleep, findWindows, assert, report,
} from "./helpers.mjs";
```

3. Follow this structure:

```js
try {
    const session = await setup({ width: 1280, height: 720 });
    const { pid } = await launchNpp();

    // ... interact with the app ...
    await click([x, y]);
    const img = await screenshot();
    assert(img !== null, "screenshot captured");

    report();
} catch (err) {
    console.error("TEST ERROR:", err);
    process.exitCode = 1;
} finally {
    await teardown();
}
```

### Helper API

| Function | Description |
|----------|-------------|
| `setup({ width, height })` | Create MCP client + virtual display session |
| `teardown()` | Destroy session and disconnect |
| `launchNpp({ args, waitMs })` | Launch Notepad++ with `-m`, resize to fill display |
| `screenshot()` | Take screenshot, return base64 PNG |
| `click([x, y])` | Left-click at coordinates |
| `key(combo)` | Press key combo, e.g. `"ctrl+s"`, `"Return"` |
| `type(text)` | Type a string (keep short to avoid xdotool timeouts) |
| `moveMouse([x, y])` | Move cursor without clicking |
| `findWindows(title?)` | List windows, optionally filtered by title regex |
| `sleep(ms)` | Wait for the given duration |
| `assert(cond, msg)` | Record pass/fail |
| `report()` | Print summary, set exit code on failure |

### Tips

- The virtual display is **1280x720** by default. Toolbar button coordinates
  are measured from this resolution with the window maximized via `wmctrl`.
- Keep `type()` calls short (under ~20 chars) to avoid xdotool timeouts.
- Use `sleep()` after clicks/key presses to let the UI settle before
  taking screenshots.
- Notepad++ must be launched with `-m` (multi-instance) to avoid conflicts
  with any running instance on the host.

## Test inventory

| Test | What it verifies |
|------|-----------------|
| `test-zoom-toolbar.mjs` | Toolbar Zoom In/Out buttons change zoom level and status bar updates |
