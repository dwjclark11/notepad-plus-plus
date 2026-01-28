# Porting Guide for Notepad++ Linux

This guide is for developers who want to port Windows-specific Notepad++ features to Linux or contribute to the Linux port.

## Table of Contents

- [Overview](#overview)
- [Porting Workflow](#porting-workflow)
- [Platform Abstraction](#platform-abstraction)
- [UI Control Porting](#ui-control-porting)
- [Coding Standards](#coding-standards)
- [Testing Requirements](#testing-requirements)
- [Platform-Specific Considerations](#platform-specific-considerations)
- [Common Porting Patterns](#common-porting-patterns)
- [Troubleshooting](#troubleshooting)

## Overview

Porting a feature from Windows to Linux involves:

1. Understanding the Windows implementation
2. Identifying platform-specific dependencies
3. Implementing the Platform Abstraction Layer (PAL) interface
4. Creating the QtControls UI implementation (if needed)
5. Testing on both platforms

## Porting Workflow

### Step 1: Analyze the Feature

Identify what the feature does and its dependencies:

```cpp
// Example: Analyzing a Windows feature
class SomeWindowsFeature {
    // Uses Win32 API
    HWND _hwnd;
    HANDLE _hFile;

    // Uses Windows messages
    LRESULT onMessage(UINT msg, WPARAM w, LPARAM l);

    // Uses Windows registry
    void loadSettings();
};
```

Checklist:
- [ ] Identify Win32 API calls
- [ ] Identify Windows messages used
- [ ] Identify registry usage
- [ ] Identify file system operations
- [ ] Identify UI controls used
- [ ] Check for COM/OLE usage
- [ ] Check for shell extensions

### Step 2: Check Existing Abstractions

Before implementing, check if an abstraction already exists:

```cpp
// Check Platform/ headers
#include "Platform/FileSystem.h"      // File operations
#include "Platform/Settings.h"        // Configuration
#include "Platform/Process.h"         // Process management
#include "Platform/Clipboard.h"       // Clipboard operations
#include "Platform/Threading.h"       // Thread utilities
#include "Platform/Dialogs.h"         // Common dialogs
```

### Step 3: Extend Platform Abstraction (if needed)

If the feature requires new platform capabilities:

1. Add method to interface header
2. Implement for Windows
3. Implement for Linux
4. Document the interface

### Step 4: Implement QtControls (if needed)

For UI features, create Qt-based controls in `QtControls/`.

### Step 5: Test

- Test on Linux
- Verify Windows build still works
- Run unit tests
- Test edge cases

## Platform Abstraction

### Adding New Platform Interface Methods

Example: Adding symbolic link support

```cpp
// Platform/FileSystem.h
class IFileSystem {
public:
    // ... existing methods ...

    // New method
    virtual bool createSymbolicLink(const std::wstring& target,
                                     const std::wstring& linkPath,
                                     bool isDirectory = false) = 0;
    virtual bool isSymbolicLink(const std::wstring& path) = 0;
    virtual std::wstring getSymbolicLinkTarget(const std::wstring& path) = 0;
};
```

Windows implementation:

```cpp
// Platform/Windows/FileSystem.cpp
class FileSystemWin32 : public IFileSystem {
    bool createSymbolicLink(const std::wstring& target,
                           const std::wstring& linkPath,
                           bool isDirectory) override {
        DWORD flags = isDirectory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
        flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
        return CreateSymbolicLinkW(linkPath.c_str(), target.c_str(), flags) != FALSE;
    }

    bool isSymbolicLink(const std::wstring& path) override {
        DWORD attrs = GetFileAttributesW(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES) &&
               (attrs & FILE_ATTRIBUTE_REPARSE_POINT);
    }

    std::wstring getSymbolicLinkTarget(const std::wstring& path) override {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                   nullptr, OPEN_EXISTING,
                                   FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return L"";

        // Read reparse point data...
        CloseHandle(hFile);
        return target;
    }
};
```

Linux implementation:

```cpp
// Platform/Linux/FileSystem.cpp
class FileSystemLinux : public IFileSystem {
    bool createSymbolicLink(const std::wstring& target,
                           const std::wstring& linkPath,
                           bool isDirectory) override {
        (void)isDirectory; // Ignored on Linux
        std::string t = wstringToUtf8(target);
        std::string l = wstringToUtf8(linkPath);
        return symlink(t.c_str(), l.c_str()) == 0;
    }

    bool isSymbolicLink(const std::wstring& path) override {
        struct stat st;
        return lstat(wstringToUtf8(path).c_str(), &st) == 0 &&
               S_ISLNK(st.st_mode);
    }

    std::wstring getSymbolicLinkTarget(const std::wstring& path) override {
        std::string p = wstringToUtf8(path);
        char buffer[PATH_MAX];
        ssize_t len = readlink(p.c_str(), buffer, sizeof(buffer) - 1);
        if (len == -1) return L"";
        buffer[len] = '\0';
        return utf8ToWstring(buffer);
    }
};
```

## UI Control Porting

### Creating a New QtControl

Let's port a hypothetical `CustomDialog` from Windows to Linux.

#### 1. Analyze the Windows Implementation

```cpp
// WinControls/CustomDialog/CustomDialog.h (Windows)
class CustomDialog : public StaticDialog {
public:
    void init(HINSTANCE hInst, HWND parent);
    void doDialog();

protected:
    INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND _hEditControl;
    HWND _hButton;
};
```

#### 2. Create the Qt Implementation

```cpp
// QtControls/CustomDialog/CustomDialog.h (Linux)
#pragma once

#include "StaticDialog/StaticDialog.h"
#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class CustomDialog; }
QT_END_NAMESPACE

namespace QtControls {

class CustomDialog : public StaticDialog {
    Q_OBJECT

public:
    CustomDialog() = default;
    ~CustomDialog() override;

    void init(HINSTANCE hInst, HWND parent) override;
    void doDialog() override;

protected:
    // Map Windows messages to Qt slots
    void onInitDialog();
    void onCommand(WPARAM wParam, LPARAM lParam);

private slots:
    void onButtonClicked();
    void onTextChanged(const QString& text);

private:
    std::unique_ptr<Ui::CustomDialog> _ui;

    // Data members matching Windows version
    std::wstring _editText;
    bool _isConfirmed = false;
};

} // namespace QtControls
```

```cpp
// QtControls/CustomDialog/CustomDialog.cpp (Linux)
#include "CustomDialog.h"
#include "ui_CustomDialog.h"
#include <QPushButton>
#include <QLineEdit>

namespace QtControls {

CustomDialog::~CustomDialog() = default;

void CustomDialog::init(HINSTANCE hInst, HWND parent) {
    StaticDialog::init(hInst, parent);

    _ui = std::make_unique<Ui::CustomDialog>();
    _ui->setupUi(static_cast<QDialog*>(_widget));

    // Connect signals
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &CustomDialog::onButtonClicked);
    connect(_ui->lineEdit, &QLineEdit::textChanged,
            this, &CustomDialog::onTextChanged);
}

void CustomDialog::doDialog() {
    if (!_widget) return;

    // Equivalent to Windows ::DialogBox
    static_cast<QDialog*>(_widget)->exec();
}

void CustomDialog::onButtonClicked() {
    _isConfirmed = true;
    _editText = _ui->lineEdit->text().toStdWString();
    close();
}

void CustomDialog::onTextChanged(const QString& text) {
    // Enable/disable button based on input
    bool hasText = !text.isEmpty();
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasText);
}

} // namespace QtControls
```

#### 3. Create UI File (Optional)

```xml
<!-- QtControls/CustomDialog/CustomDialog.ui -->
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>CustomDialog</class>
 <widget class="QDialog" name="CustomDialog">
  <layout class="QVBoxLayout">
   <item>
    <widget class="QLineEdit" name="lineEdit"/>
   </item>
   <item>
    <widget class="QDialogButtonBox" name="buttonBox">
     <standardbuttons>QDialogButtonBox::Ok|QDialogButtonBox::Cancel</standardbuttons>
    </widget>
   </item>
  </layout>
 </widget>
</ui>
```

#### 4. Update CMakeLists.txt

```cmake
# Add to QtControls/CMakeLists.txt
set(QTCONTROLS_SOURCES
    # ... existing sources ...
    CustomDialog/CustomDialog.cpp
)

set(QTCONTROLS_HEADERS
    # ... existing headers ...
    CustomDialog/CustomDialog.h
)
```

### Mapping Windows Messages to Qt Signals

| Windows Message | Qt Equivalent |
|-----------------|---------------|
| `WM_COMMAND` | `QAction::triggered`, button `clicked()` |
| `WM_NOTIFY` | Various notification signals |
| `WM_PAINT` | `paintEvent()` override |
| `WM_SIZE` | `resizeEvent()` override |
| `WM_KEYDOWN` | `keyPressEvent()` override |
| `WM_MOUSEMOVE` | `mouseMoveEvent()` override |
| `WM_INITDIALOG` | Constructor or `showEvent()` |
| `WM_DESTROY` | Destructor or `hideEvent()` |

## Coding Standards

### General Guidelines

1. **Follow existing code style**: Match the surrounding code
2. **Use tabs for indentation**: As per project convention
3. **Brace style**: Allman style (brace on new line)
4. **Member variables**: Prefix with underscore (`_variable`)

### Platform-Specific Code

```cpp
// Good: Isolate platform code
#ifdef NPP_LINUX
    // Linux-specific implementation
    Platform::IFileSystem::getInstance().moveToTrash(path);
#else
    // Windows-specific implementation
    SHFileOperation(&fileOp);
#endif

// Bad: Scattered platform checks
void function() {
    doSomething();
#ifdef NPP_LINUX
    linuxSpecific();
#endif
    doMore();
#ifdef NPP_LINUX
    moreLinux();
#else
    windowsSpecific();
#endif
}
```

### String Handling

```cpp
// Use std::wstring for internal strings
std::wstring filePath = getFilePath();

// Convert at platform boundaries
#ifdef NPP_LINUX
    std::string utf8Path = wstringToUtf8(filePath);
    std::ifstream file(utf8Path);
#else
    std::ifstream file(filePath); // Windows uses wchar_t directly
#endif
```

### Error Handling

```cpp
// Use exceptions for exceptional cases
try {
    Platform::IFileSystem::getInstance().copyFile(src, dest);
} catch (const std::exception& e) {
    // Log error
    return false;
}

// Use return values for expected failures
if (!Platform::IFileSystem::getInstance().fileExists(path)) {
    // Handle gracefully
    return false;
}
```

## Testing Requirements

### Unit Tests

Create tests for platform abstraction implementations:

```cpp
// Test/Platform/FileSystemTest.cpp
#include <gtest/gtest.h>
#include "Platform/FileSystem.h"

using namespace Platform;

TEST(FileSystemTest, CreateAndDeleteFile) {
    auto& fs = IFileSystem::getInstance();
    std::wstring testPath = L"/tmp/test_file.txt";

    // Create file
    File file(testPath, FileMode::Write);
    EXPECT_TRUE(file.isOpen());
    file.writeString("test content");
    file.close();

    // Verify exists
    EXPECT_TRUE(fs.fileExists(testPath));

    // Delete
    EXPECT_TRUE(fs.deleteFile(testPath));
    EXPECT_FALSE(fs.fileExists(testPath));
}

TEST(FileSystemTest, SymbolicLinks) {
    auto& fs = IFileSystem::getInstance();
    std::wstring target = L"/tmp/target.txt";
    std::wstring link = L"/tmp/link.txt";

    // Create target
    File file(target, FileMode::Write);
    file.writeString("target");
    file.close();

    // Create symlink
    EXPECT_TRUE(fs.createSymbolicLink(target, link));
    EXPECT_TRUE(fs.isSymbolicLink(link));
    EXPECT_EQ(fs.getSymbolicLinkTarget(link), target);

    // Cleanup
    fs.deleteFile(link);
    fs.deleteFile(target);
}
```

### Integration Tests

Test UI controls:

```cpp
// Test/QtControls/CustomDialogTest.cpp
#include <QTest>
#include "QtControls/CustomDialog/CustomDialog.h"

class CustomDialogTest : public QObject {
    Q_OBJECT

private slots:
    void testDialogAcceptsInput() {
        QtControls::CustomDialog dialog;
        dialog.init(nullptr, nullptr);

        // Simulate user input
        // Verify behavior
    }
};

QTEST_MAIN(CustomDialogTest)
#include "CustomDialogTest.moc"
```

### Manual Testing Checklist

- [ ] Feature works on Linux
- [ ] Feature still works on Windows
- [ ] Error handling works correctly
- [ ] Edge cases handled
- [ ] Performance is acceptable
- [ ] Memory leaks checked (valgrind)
- [ ] Thread safety verified (if applicable)

## Platform-Specific Considerations

### File System Differences

| Aspect | Windows | Linux |
|--------|---------|-------|
| Path separator | `\` | `/` |
| Case sensitivity | Case-preserving | Case-sensitive |
| Hidden files | Attribute | Leading `.` |
| Executable | Extension | Permission bit |
| Line endings | CRLF | LF |

### Path Handling

```cpp
// Always use platform abstraction for path operations
std::wstring fullPath = IFileSystem::pathAppend(directory, filename);

// Don't assume path separator
// Bad:
std::wstring path = dir + L"\\" + file;  // Windows only

// Good:
std::wstring path = IFileSystem::pathAppend(dir, file);  // Cross-platform
```

### Configuration Storage

| Windows | Linux |
|---------|-------|
| Registry / `%APPDATA%` | `~/.config/notepad++/` |

Use `ISettings` interface for configuration.

### Process Management

```cpp
// Windows: CreateProcess
// Linux: fork() + exec()
// Both: Use IProcess interface

class IProcess {
public:
    virtual bool launch(const std::wstring& executable,
                       const std::vector<std::wstring>& args,
                       bool wait = false) = 0;
    virtual bool launchElevated(const std::wstring& executable,
                               const std::vector<std::wstring>& args) = 0;
};
```

### Threading

```cpp
// Prefer std::thread for portable code
std::thread worker([this]() {
    doWork();
});

// For platform-specific thread features, use IThreading
Platform::IThreading::getInstance().setThreadName("WorkerThread");
```

## Common Porting Patterns

### Pattern 1: Platform-Specific Implementation Files

```cpp
// MyFeature.h - Platform-neutral interface
class IMyFeature {
public:
    virtual void doSomething() = 0;
    static std::unique_ptr<IMyFeature> create();
};

// MyFeature_Win32.cpp - Windows implementation
class MyFeatureWin32 : public IMyFeature {
    void doSomething() override {
        // Win32 implementation
    }
};

std::unique_ptr<IMyFeature> IMyFeature::create() {
    return std::make_unique<MyFeatureWin32>();
}

// MyFeature_Linux.cpp - Linux implementation
class MyFeatureLinux : public IMyFeature {
    void doSomething() override {
        // Linux implementation
    }
};

std::unique_ptr<IMyFeature> IMyFeature::create() {
    return std::make_unique<MyFeatureLinux>();
}
```

### Pattern 2: Conditional Compilation

```cpp
class MyClass {
public:
    void method() {
        commonCode();

#ifdef NPP_LINUX
        linuxSpecific();
#else
        windowsSpecific();
#endif

        moreCommonCode();
    }
};
```

### Pattern 3: Strategy Pattern

```cpp
class FileWatcher {
public:
    FileWatcher() {
#ifdef NPP_LINUX
        _impl = std::make_unique<LinuxWatcher>();
#else
        _impl = std::make_unique<WindowsWatcher>();
#endif
    }

    void startWatching(const std::wstring& path) {
        _impl->startWatching(path);
    }

private:
    std::unique_ptr<FileWatcherImpl> _impl;
};
```

## Troubleshooting

### Common Issues

#### 1. Encoding Problems

**Symptom:** Garbled text when reading files

**Solution:**
```cpp
// Ensure proper UTF-8 conversion on Linux
std::string utf8Content = FileSystemUtils::wcharToChar(wideContent);
// Linux uses UTF-8 natively

// Windows uses UTF-16
std::wstring wideContent = FileSystemUtils::charToWchar(utf8Content);
```

#### 2. Path Case Sensitivity

**Symptom:** File not found on Linux

**Solution:**
```cpp
// Always preserve original case
std::wstring originalPath = getPathFromUser();
// Don't convert case
```

#### 3. Line Ending Issues

**Symptom:** Files show `^M` characters or all text on one line

**Solution:**
```cpp
// Handle both CRLF and LF
std::string normalizeLineEndings(const std::string& content) {
    std::string result;
    result.reserve(content.size());

    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\r') {
            if (i + 1 < content.size() && content[i + 1] == '\n') {
                // CRLF - skip CR, keep LF
                continue;
            }
            // Lone CR - convert to LF
            result += '\n';
        } else {
            result += content[i];
        }
    }
    return result;
}
```

#### 4. Qt Signal/Slot Connection Failures

**Symptom:** UI elements don't respond

**Solution:**
```cpp
// Check connections
bool connected = connect(button, &QPushButton::clicked,
                        this, &MyClass::onClick);
Q_ASSERT(connected);  // Fails in debug if connection fails

// Use new-style connections (compile-time checked)
connect(sender, &Sender::signal, receiver, &Receiver::slot);
```

### Debugging Tips

1. **Enable Qt debugging:**
   ```bash
   QT_DEBUG_PLUGINS=1 ./notepad-plus-plus
   ```

2. **Check for missing symbols:**
   ```bash
   ldd ./notepad-plus-plus | grep "not found"
   ```

3. **Use AddressSanitizer:**
   ```bash
   cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
   ```

4. **Valgrind for memory issues:**
   ```bash
   valgrind --leak-check=full ./notepad-plus-plus
   ```

### Getting Help

- Check [ARCHITECTURE.md](ARCHITECTURE.md) for design details
- Review existing implementations in `QtControls/`
- Ask on the [Notepad++ Community Forum](https://community.notepad-plus-plus.org/)
- Create a GitHub issue with:
  - What you're trying to port
  - Current approach
  - Specific problem or question
