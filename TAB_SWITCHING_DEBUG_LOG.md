# Tab Switching Content Loss - Debug Logging

This document describes the comprehensive logging added to debug the tab switching content loss issue.

## Problem Summary

When switching between file tabs, content disappears. This is likely due to the `_document` pointer in `Buffer` objects being `nullptr` - the Qt port doesn't implement `SCI_CREATEDOCUMENT` to create Scintilla documents like the Windows version does.

## Logging Added

### 1. ScintillaEditViewQt.cpp::activateBuffer() (lines 1608-1740)

**Key logging points:**
- Entry with buffer pointer and force flag
- Validation of buffer and newBuf pointers
- Current buffer state BEFORE switch (pointer values)
- Current buffer document pointer value
- Current buffer hasPendingContent status
- Switch assignment confirmation
- **CRITICAL**: New buffer document pointer value (will show `nullptr` if not set)
- Warning if document pointer is NULL
- SCI_SETDOCPOINTER call confirmation
- Pending content loading details
- Scintilla text length after activation (verifies content)
- Exit with final text length

**What to look for:**
```
[ScintillaEditView::activateBuffer] NEW BUFFER document pointer=0  <-- NULL = BUG
[ScintillaEditView::activateBuffer] WARNING: document pointer is NULL! Content will not display correctly.
```

### 2. DocTabView.cpp

#### addBuffer() (lines 109-153)
- Entry with buffer pointer
- Validation checks
- Tab title being added
- Index where tab was added
- Buffer-to-index mapping storage

#### getBufferByIndex() (lines 253-283)
- Entry with index and map size
- Tab widget validation
- Range validation
- **CRITICAL**: Search through mapping with results
- If not found: dumps all current mappings for debugging

#### getIndexByBuffer() (lines 224-252)
- Entry with buffer ID
- Map lookup result
- Fallback tab search if not in map
- Return value

#### activateBuffer() (lines 179-200)
- Entry with buffer ID
- getIndexByBuffer result
- activateAt call confirmation

### 3. Buffer.cpp

#### getDocument() (lines 1872-1879)
- **CRITICAL**: Logs buffer pointer and `_document` value on every call

#### hasPendingContent() (lines 1941-1947)
- Buffer pointer and `_hasPendingContent` status

#### takePendingContent() (lines 1949-1957)
- Buffer pointer and content size being retrieved

#### FileManager::newEmptyDocument() (lines 2001-2020)
- Entry log
- **WARNING**: Logs that document pointer will be NULL
- Created buffer pointer

#### FileManager::loadFile() (lines 2135-2180)
- Entry with filename and doc parameter
- Validation checks
- Created buffer pointer
- Load success/failure
- **NOTE**: Does NOT create Scintilla document - buffer's `_document` remains null

### 4. MainWindow/Notepad_plus_Window.cpp

#### onMainTabChanged() (lines 2676-2712)
- Entry with tab index
- Validation of required objects
- Buffer ID retrieved from tab
- switchToFile call confirmation
- Exit

#### onSubTabChanged() (lines 2714-2748)
- Same logging as onMainTabChanged for sub view

### 5. Notepad_plus.cpp::switchToFile() (lines 1572-1610)

- Entry with buffer ID
- Validation of all required objects
- DocTab activateBuffer call and result
- EditView activateBuffer call confirmation
- Exit with final result

## How to Build and Run

```bash
# Clean build
mkdir -p build && cd build
rm -rf *

# Configure with Debug build for more verbose output
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)

# Run and capture output
./notepad-plus-plus 2>&1 | tee debug_output.log
```

## Reproduction Steps

1. Start the application
2. Create a new file (Ctrl+N) - type some text
3. Create another new file (Ctrl+N) - type different text
4. Switch back to first tab
5. **Observe**: First tab content is gone or shows second tab's content
6. Switch to second tab
7. **Observe**: Second tab content is also gone

## Expected Log Output During Reproduction

### Normal Flow (when working correctly):
```
[MainWindow::onMainTabChanged] ENTER - index=0
[DocTabView::getBufferByIndex] ENTER - index=0 _bufferToIndex.size()=2
[DocTabView::getBufferByIndex] FOUND - index=0 buffer=0x55a3b8e2c8c0
[MainWindow::onMainTabChanged] Got bufferId=0x55a3b8e2c8c0 for index=0
[MainWindow::onMainTabChanged] Calling switchToFile...
[Notepad_plus::switchToFile] ENTER - id=0x55a3b8e2c8c0
[Notepad_plus::switchToFile] Calling _pDocTab->activateBuffer...
[DocTabView::activateBuffer] ENTER - buffer=0x55a3b8e2c8c0
[DocTabView::getIndexByBuffer] ENTER - id=0x55a3b8e2c8c0
[DocTabView::getIndexByBuffer] FOUND in map - id=0x55a3b8e2c8c0 index=0
[DocTabView::activateBuffer] Calling activateAt(0)
[DocTabView::activateBuffer] EXIT - success
[Notepad_plus::switchToFile] _pDocTab->activateBuffer returned 1
[Notepad_plus::switchToFile] Calling _pEditView->activateBuffer...
[ScintillaEditView::activateBuffer] ENTER - buffer=0x55a3b8e2c8c0 force=0 _currentBuffer=0x55a3b8e2d200
[ScintillaEditView::activateBuffer] NEW BUFFER document pointer=0x7f8b4c0b5000  <-- NON-NULL = GOOD
[ScintillaEditView::activateBuffer] Scintilla text length after activation=123
[ScintillaEditView::activateBuffer] EXIT - final text length=123 for buffer=0x55a3b8e2c8c0
```

### Bug Flow (document pointer is null):
```
[ScintillaEditView::activateBuffer] NEW BUFFER document pointer=0  <-- NULL = BUG
[ScintillaEditView::activateBuffer] WARNING: document pointer is NULL! Content will not display correctly.
[ScintillaEditView::activateBuffer] Scintilla text length after activation=0  <-- NO CONTENT
```

## Key Debug Patterns

### Pattern 1: Document Pointer is NULL
If you see:
```
[Buffer::getDocument] buffer=0xXXX _document=0
[ScintillaEditView::activateBuffer] WARNING: document pointer is NULL!
```
**Root Cause**: `Buffer._document` is never set. The Qt port needs to call `SCI_CREATEDOCUMENT` and store the result.

### Pattern 2: Wrong Buffer Mapping
If you see:
```
[DocTabView::getBufferByIndex] ERROR: No buffer found for index=1
[DocTabView::getBufferByIndex] Current mappings: [buffer=0xAAA -> index=0] [buffer=0xBBB -> index=0]
```
**Root Cause**: Buffer-to-index map is corrupted (both mapped to same index).

### Pattern 3: Tab Switch Not Triggering Buffer Switch
If `onMainTabChanged` is not called when clicking tabs:
**Root Cause**: Signal connection issue between QTabWidget and DocTabView.

## Next Steps After Logging

Once you confirm the document pointer is NULL (which is the expected root cause):

1. **Implement SCI_CREATEDOCUMENT** in `Buffer.cpp` when creating new buffers
2. **Associate document with buffer** via a new `Buffer::setDocument()` method
3. **Verify** that `getDocument()` returns non-NULL for all buffers

The Windows implementation does this in `FileManager::newEmptyDocument()`:
```cpp
Document doc = static_cast<Document>(_pscratchTilla->execute(SCI_CREATEDOCUMENT, ...));
Buffer* newBuf = new Buffer(this, _nextBufferID, doc, ...);
```

The Qt port needs similar logic.
