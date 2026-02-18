#!/usr/bin/env bash
# Run all Notepad++ E2E tests.
#
# Prerequisites:
#   - Build notepad-plus-plus:  cd build && cmake ../PowerEditor/src && make -j$(nproc)
#   - Build MCP server:         cd tools/computer-use-mcp && npm install && npm run build
#   - System packages:          xorg-server-xvfb xdotool ffmpeg xorg-xdpyinfo openbox wmctrl
#
# Usage:
#   ./tests/e2e/run-all.sh              # run all tests
#   ./tests/e2e/run-all.sh zoom         # run only tests matching "zoom"
#
# Environment variables:
#   NPP_BINARY   Path to notepad-plus-plus binary (default: build/notepad-plus-plus)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

export NPP_BINARY="${NPP_BINARY:-$PROJECT_ROOT/build/notepad-plus-plus}"

if [[ ! -x "$NPP_BINARY" ]]; then
    echo "ERROR: Notepad++ binary not found at $NPP_BINARY"
    echo "       Build it first:  cd build && cmake ../PowerEditor/src && make -j\$(nproc)"
    exit 1
fi

MCP_DIST="$PROJECT_ROOT/tools/computer-use-mcp/dist/main.js"
if [[ ! -f "$MCP_DIST" ]]; then
    echo "ERROR: MCP server not built.  Run:  cd tools/computer-use-mcp && npm install && npm run build"
    exit 1
fi

# Auto-install test dependencies if needed
if [[ ! -d "$SCRIPT_DIR/node_modules" ]]; then
    echo "Installing E2E test dependencies..."
    (cd "$SCRIPT_DIR" && npm install)
fi

FILTER="${1:-}"
TOTAL=0
PASSED=0
FAILED=0

for test_file in "$SCRIPT_DIR"/test-*.mjs; do
    test_name="$(basename "$test_file")"

    # Apply filter if given
    if [[ -n "$FILTER" ]] && [[ "$test_name" != *"$FILTER"* ]]; then
        continue
    fi

    echo ""
    echo "================================================================"
    echo "Running: $test_name"
    echo "================================================================"
    TOTAL=$((TOTAL + 1))

    if node "$test_file"; then
        PASSED=$((PASSED + 1))
    else
        FAILED=$((FAILED + 1))
        echo "  ^^^ FAILED ^^^"
    fi
done

echo ""
echo "================================================================"
echo "E2E Summary: $PASSED/$TOTAL passed, $FAILED failed"
echo "================================================================"

[[ $FAILED -eq 0 ]]
