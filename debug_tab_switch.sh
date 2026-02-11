#!/bin/bash

# Script to run Notepad++ with debug logging and filter for tab switching issues

BUILD_DIR="${1:-build}"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory '$BUILD_DIR' not found"
    echo "Usage: $0 [build_directory]"
    exit 1
fi

if [ ! -f "$BUILD_DIR/notepad-plus-plus" ]; then
    echo "Error: notepad-plus-plus binary not found in '$BUILD_DIR'"
    echo "Please build first with: cd $BUILD_DIR && make -j\$(nproc)"
    exit 1
fi

# Create logs directory
mkdir -p logs

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="logs/debug_${TIMESTAMP}.log"

echo "=================================="
echo "Notepad++ Tab Switching Debug Run"
echo "=================================="
echo "Log file: $LOG_FILE"
echo ""
echo "Running Notepad++..."
echo "Reproduce the issue:"
echo "  1. Create new file (Ctrl+N), type some text"
echo "  2. Create another file (Ctrl+N), type different text"
echo "  3. Switch between tabs and observe content loss"
echo "  4. Close the application"
echo ""
echo "Press Ctrl+C to cancel, or wait 3 seconds to continue..."
sleep 3

# Run with output captured
"$BUILD_DIR/notepad-plus-plus" 2>&1 | tee "$LOG_FILE"

echo ""
echo "=================================="
echo "Debug Summary"
echo "=================================="
echo ""

# Check for critical issues in the log
if [ -f "$LOG_FILE" ]; then
    echo "Analyzing log file..."
    echo ""

    # Count document pointer NULL warnings
    NULL_DOC_COUNT=$(grep -c "document pointer is NULL" "$LOG_FILE" 2>/dev/null || echo 0)
    if [ "$NULL_DOC_COUNT" -gt 0 ]; then
        echo "WARNING: Found $NULL_DOC_COUNT occurrences of NULL document pointer!"
        echo "This is the likely cause of the content loss issue."
        echo ""
        grep -n "document pointer is NULL" "$LOG_FILE" | head -5
        echo ""
    fi

    # Count buffer mapping errors
    MAPPING_ERRORS=$(grep -c "No buffer found for index" "$LOG_FILE" 2>/dev/null || echo 0)
    if [ "$MAPPING_ERRORS" -gt 0 ]; then
        echo "WARNING: Found $MAPPING_ERRORS buffer mapping errors!"
        echo ""
    fi

    # Show document pointer values
    echo "Document pointer values from log:"
    grep "NEW BUFFER document pointer" "$LOG_FILE" | head -10
    echo ""

    # Show buffer creation
    echo "Buffer creation summary:"
    grep "Created buffer" "$LOG_FILE" | head -10
    echo ""

    # Show activateBuffer flow
    echo "Tab switch operations:"
    grep -c "ScintillaEditView::activateBuffer.*ENTER" "$LOG_FILE" 2>/dev/null || echo 0
    ACTIVATE_COUNT=$(grep -c "ScintillaEditView::activateBuffer.*ENTER" "$LOG_FILE" 2>/dev/null || echo 0)
    echo "Total buffer activations: $ACTIVATE_COUNT"
    echo ""

    echo "Full log saved to: $LOG_FILE"
else
    echo "Error: Log file was not created"
fi
