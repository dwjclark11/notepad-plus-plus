# Tab Switching Debug - Quick Reference

## Running the Debug Build

```bash
# Quick start
mkdir -p build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./notepad-plus-plus 2>&1 | grep -E "(document pointer|activateBuffer|WARNING|ERROR)"
```

Or use the provided script:
```bash
./debug_tab_switch.sh build
```

## Critical Log Patterns

### GOOD: Document Pointer is Valid
```
[Buffer::getDocument] buffer=0x55a3b8e2c8c0 _document=0x7f8b4c0b5000  <-- NON-ZERO
[ScintillaEditView::activateBuffer] NEW BUFFER document pointer=0x7f8b4c0b5000
[ScintillaEditView::activateBuffer] Scintilla text length after activation=123  <-- HAS CONTENT
```

### BAD: Document Pointer is NULL (Expected Bug)
```
[Buffer::getDocument] buffer=0x55a3b8e2c8c0 _document=0  <-- ZERO = BUG
[ScintillaEditView::activateBuffer] WARNING: document pointer is NULL! Content will not display correctly.
[ScintillaEditView::activateBuffer] Scintilla text length after activation=0  <-- EMPTY
```

### BAD: Buffer Not Found in Mapping
```
[DocTabView::getBufferByIndex] ERROR: No buffer found for index=1
```

## Filtering Logs

```bash
# Show only document pointer related logs
grep "document pointer" debug.log

# Show only errors and warnings
grep -E "(ERROR|WARNING)" debug.log

# Show full tab switch sequence for a specific buffer
grep "buffer=0x55a3b8e2c8c0" debug.log

# Show activateBuffer flow
grep "activateBuffer" debug.log

# Show buffer creation
grep -E "(newEmptyDocument|loadFile|Created buffer)" debug.log
```

## Log Flow During Tab Switch

When you click a tab, the log flow should be:

```
[MainWindow::onMainTabChanged] ENTER - index=0           <-- User clicked tab 0
[DocTabView::getBufferByIndex] ENTER - index=0           <-- Looking up buffer for tab
[DocTabView::getBufferByIndex] FOUND - index=0 buffer=0xAAA  <-- Found buffer 0xAAA
[MainWindow::onMainTabChanged] Got bufferId=0xAAA        <-- Will switch to buffer 0xAAA
[Notepad_plus::switchToFile] ENTER - id=0xAAA            <-- Starting file switch
[DocTabView::activateBuffer] ENTER - buffer=0xAAA        <-- Tab widget activation
[DocTabView::activateBuffer] EXIT - success              <-- Tab switched visually
[ScintillaEditView::activateBuffer] ENTER                <-- Editor content activation
  [Buffer::getDocument] buffer=0xAAA _document=0xDOC     <-- Getting document pointer
  [ScintillaEditView::activateBuffer] NEW BUFFER document pointer=0xDOC  <-- Setting doc
  [ScintillaEditView::activateBuffer] Scintilla text length after activation=123  <-- Content!
[ScintillaEditView::activateBuffer] EXIT                 <-- Done
```

## Most Important Lines

Look for these specific lines:

| Line | Meaning | Expected Value |
|------|---------|----------------|
| `Buffer::getDocument] _document=0x...` | Scintilla document pointer | NON-ZERO |
| `WARNING: document pointer is NULL!` | Document not created | SHOULD NOT APPEAR |
| `Scintilla text length after activation=0` | Content in editor | NON-ZERO for non-empty files |
| `ERROR: No buffer found for index` | Buffer mapping broken | SHOULD NOT APPEAR |

## Quick Diagnosis

### Scenario 1: Content disappears after tab switch
**Check**: `document pointer is NULL` warning present?
- **YES** → Root cause: `Buffer._document` is never set. Need to implement `SCI_CREATEDOCUMENT`.
- **NO** → Check if text length is 0. May be pending content issue.

### Scenario 2: Wrong content shown in tab
**Check**: Buffer mapping errors present?
- **YES** → Root cause: `_bufferToIndex` map is corrupted.
- **NO** → May be shared document issue (all buffers using same default document).

### Scenario 3: Tab switch does nothing
**Check**: `onMainTabChanged` called in logs?
- **NO** → Signal connection issue between QTabWidget and handler.
- **YES** → Check `switchToFile` and `activateBuffer` flow.

## Known Issues in Qt Port

1. **Document Creation**: Qt port does NOT call `SCI_CREATEDOCUMENT` when creating buffers
   - Windows: `FileManager::newEmptyDocument()` creates doc via Scintilla
   - Qt: Buffer is created with `_document = nullptr`

2. **Document Association**: `Buffer::setDocument()` method doesn't exist in Qt port

3. **Pending Content**: Content is loaded via `takePendingContent()` but stored in Scintilla's default document, not a per-buffer document.

## Fix Strategy

Once logs confirm NULL document pointers:

1. Add `SCI_CREATEDOCUMENT` call in `FileManager::newEmptyDocument()`
2. Add `Buffer::setDocument(void* doc)` method
3. Store document pointer when creating buffers
4. All buffers should then have unique non-NULL `_document` values

Reference Windows implementation:
```cpp
// ScintillaComponent/Buffer.cpp (Windows)
Document doc = static_cast<Document>(_pscratchTilla->execute(SCI_CREATEDOCUMENT, ...));
Buffer* newBuf = new Buffer(this, _nextBufferID, doc, ...);
```
