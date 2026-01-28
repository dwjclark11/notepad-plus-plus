# Contributing to Notepad++ Linux Port

Thank you for your interest in contributing to the Notepad++ Linux port! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Code Style Guidelines](#code-style-guidelines)
- [Pull Request Process](#pull-request-process)
- [Linux Port Specific Guidelines](#linux-port-specific-guidelines)
- [Testing Requirements](#testing-requirements)
- [Documentation](#documentation)
- [Getting Help](#getting-help)

## Code of Conduct

This project follows the Notepad++ community standards. Be respectful, constructive, and professional in all interactions.

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Development Environment**:
   - Linux distribution (Ubuntu 22.04+, Fedora 39+, or equivalent)
   - GCC 11+ or Clang 14+
   - CMake 3.16+
   - Qt6 development packages
   - Git

2. **Required Tools**:
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential cmake git qt6-base-dev

   # Fedora
   sudo dnf install gcc-c++ cmake git qt6-qtbase-devel

   # Arch Linux
   sudo pacman -S base-devel cmake qt6-base
   ```

3. **Optional Tools**:
   - `clang-format` for code formatting
   - `cppcheck` for static analysis
   - `valgrind` for memory checking
   - `gdb` or `lldb` for debugging

### Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/notepad-plus-plus.git
   cd notepad-plus-plus
   ```
3. Add upstream remote:
   ```bash
   git remote add upstream https://github.com/notepad-plus-plus/notepad-plus-plus.git
   ```

### Build the Project

```bash
mkdir build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

See [BUILD.md](BUILD.md) for detailed build instructions.

## Development Workflow

### Branch Naming

Use descriptive branch names with the following prefixes:

- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation updates
- `refactor/` - Code refactoring
- `test/` - Test additions/improvements
- `linux/` - Linux-specific changes

Examples:
```
feature/add-native-file-dialogs
fix/file-watcher-memory-leak
docs/update-build-instructions
linux/improve-kde-integration
```

### Commit Messages

Follow conventional commit format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat` - New feature
- `fix` - Bug fix
- `docs` - Documentation only
- `style` - Code style changes (formatting, semicolons, etc.)
- `refactor` - Code refactoring
- `perf` - Performance improvements
- `test` - Adding or updating tests
- `chore` - Build process or auxiliary tool changes

Scopes (for Linux port):
- `platform` - Platform abstraction layer
- `qtcontrols` - Qt UI controls
- `build` - Build system
- `docs` - Documentation
- `test` - Tests

Examples:
```
feat(platform): add symbolic link support to IFileSystem

Implement createSymbolicLink, isSymbolicLink, and getSymbolicLinkTarget
methods in both Windows and Linux implementations.

fix(qtcontrols): resolve crash in FindReplaceDlg on Wayland

Check for null pointer before accessing widget in close event handler.

docs(build): update Fedora build instructions

Add missing qt6-qtnetwork-devel package requirement.
```

### Keeping Your Fork Updated

```bash
git fetch upstream
git checkout master
git merge upstream/master
git push origin master
```

## Code Style Guidelines

Notepad++ follows specific coding conventions. The Linux port maintains these conventions.

### General Style

1. **Braces**: Allman style (opening brace on new line)
   ```cpp
   void function()
   {
       if (condition)
       {
           // code
       }
   }
   ```

2. **Indentation**: Use tabs (configure editor to show as 4 spaces)

3. **Line Length**: Maximum 120 characters

4. **Naming Conventions**:
   - Classes: `PascalCase`
   - Methods: `camelCase`
   - Member variables: `_camelCase` with leading underscore
   - Constants: `UPPER_SNAKE_CASE`
   - Macros: `UPPER_SNAKE_CASE`

### C++ Standards

- Use C++20 features where appropriate
- Prefer `constexpr` over `const` for compile-time constants
- Use `auto` when it improves readability
- Use range-based for loops
- Use smart pointers (`unique_ptr`, `shared_ptr`)

Example:
```cpp
class FileManager
{
public:
    explicit FileManager(const std::wstring& path);

    bool openFile(FileMode mode);
    void closeFile();

    size_t read(std::vector<uint8_t>& buffer);
    size_t write(const std::vector<uint8_t>& buffer);

private:
    std::wstring _filePath;
    std::unique_ptr<FileHandle> _handle;
    bool _isOpen = false;
};
```

### Platform-Specific Code

When writing platform-specific code:

1. **Keep it isolated**: Use Platform Abstraction Layer
2. **Document differences**: Comment platform-specific behavior
3. **Maintain symmetry**: Implement for both platforms

```cpp
// Good: Platform-specific implementation in PAL
// Platform/Linux/FileSystem.cpp
bool FileSystemLinux::moveToTrash(const std::wstring& path)
{
    // XDG Trash specification implementation
    // See: https://specifications.freedesktop.org/trash-spec/
    std::filesystem::path trashBase = getXdgTrashDir();
    // ... implementation
}

// Bad: Platform-specific code scattered in application logic
void SomeFunction()
{
    // ... code ...
#ifdef NPP_LINUX
    system("gio trash \"" + path + "\"");
#else
    SHFileOperation(&fileOp);
#endif
    // ... more code ...
}
```

### Qt-Specific Guidelines

For QtControls development:

1. **Use Qt's memory management**: Parent-child hierarchy
2. **Prefer Qt containers** when interfacing with Qt APIs
3. **Use signals/slots** for communication
4. **Follow Qt naming** for consistency

```cpp
// Good
class MyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MyDialog(QWidget* parent = nullptr);

signals:
    void fileSelected(const QString& path);

private slots:
    void onBrowseClicked();

private:
    QLineEdit* _pathEdit;
    QPushButton* _browseButton;
};

// Implementation
MyDialog::MyDialog(QWidget* parent)
    : QDialog(parent)
    , _pathEdit(new QLineEdit(this))  // Parent handles deletion
    , _browseButton(new QPushButton(tr("Browse..."), this))
{
    connect(_browseButton, &QPushButton::clicked,
            this, &MyDialog::onBrowseClicked);
}
```

## Pull Request Process

### Before Submitting

1. **Test your changes**:
   ```bash
   make test
   ```

2. **Check code style**:
   ```bash
   clang-format -i YourModifiedFile.cpp
   ```

3. **Run static analysis**:
   ```bash
   cppcheck --enable=all --suppress=missingIncludeSystem PowerEditor/src/Platform/Linux/
   ```

4. **Update documentation** if needed

5. **Add tests** for new functionality

### Creating the Pull Request

1. Push your branch to your fork
2. Create PR against `notepad-plus-plus:master`
3. Fill out the PR template completely
4. Link related issues

### PR Title Format

```
[Linux] <type>: <description>
```

Examples:
```
[Linux] feat: add native file dialog support
[Linux] fix: resolve crash on file save with invalid permissions
[Linux] docs: update build instructions for openSUSE
```

### PR Description Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
Describe testing performed:
- [ ] Built on Ubuntu 22.04
- [ ] Built on Fedora 39
- [ ] Unit tests pass
- [ ] Manual testing performed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings generated

## Related Issues
Fixes #123
Related to #456
```

### Review Process

1. Automated checks must pass
2. At least one maintainer review required
3. Address review feedback promptly
4. Keep PR focused on single feature/fix

## Linux Port Specific Guidelines

### Platform Abstraction Layer

When adding new PAL interfaces:

1. Define interface in `Platform/InterfaceName.h`
2. Implement for Windows in `Platform/Windows/InterfaceName.cpp`
3. Implement for Linux in `Platform/Linux/InterfaceName.cpp`
4. Document interface methods
5. Add unit tests

### QtControls Development

When creating new UI controls:

1. Mirror the Windows control hierarchy
2. Use Qt Designer .ui files for complex layouts
3. Maintain API compatibility with Windows version
4. Test on multiple desktop environments

### Build System

When modifying CMake files:

1. Test on both Windows and Linux
2. Maintain backward compatibility
3. Document new options
4. Update BUILD.md if needed

## Testing Requirements

### Unit Tests

Add tests for new functionality:

```cpp
// Test/Platform/FileSystemTest.cpp
TEST(FileSystemTest, CreateDirectoryRecursive)
{
    auto& fs = Platform::IFileSystem::getInstance();
    std::wstring testPath = L"/tmp/npp_test/nested/dir";

    EXPECT_TRUE(fs.createDirectoryRecursive(testPath));
    EXPECT_TRUE(fs.directoryExists(testPath));

    // Cleanup
    fs.removeDirectoryRecursive(L"/tmp/npp_test");
}
```

### Integration Tests

Test UI components:

```cpp
// Test/QtControls/DialogTest.cpp
TEST_F(DialogTest, AcceptsValidInput)
{
    QtControls::MyDialog dialog;
    dialog.init(nullptr, nullptr);

    // Simulate user input
    dialog.setInputValue(L"test");

    // Verify behavior
    EXPECT_TRUE(dialog.validate());
}
```

### Manual Testing Checklist

Before submitting PR:

- [ ] Feature works as expected
- [ ] No regressions in existing functionality
- [ ] Works on different desktop environments
- [ ] Error handling works correctly
- [ ] Memory leaks checked with valgrind
- [ ] No compiler warnings

### Test Environments

Test on at least one of:

- Ubuntu 22.04+ (GNOME)
- Fedora 39+ (GNOME/KDE)
- KDE Plasma (any distribution)

## Documentation

### Code Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Moves a file to the trash/recycle bin
 * @param path The file path to move
 * @return true if successful, false otherwise
 * @note On Linux, follows XDG Trash specification
 * @note On Windows, uses SHFileOperation
 */
virtual bool moveToTrash(const std::wstring& path) = 0;
```

### User Documentation

Update relevant documentation files:

- `docs/USER_MANUAL.md` - User-facing features
- `docs/BUILD.md` - Build instructions
- `docs/INSTALL.md` - Installation methods
- `docs/PORTING.md` - Developer porting guide

### Architecture Documentation

Update `docs/ARCHITECTURE.md` when:

- Adding new PAL interfaces
- Changing architecture decisions
- Adding major new components

## Getting Help

### Resources

- [ARCHITECTURE.md](ARCHITECTURE.md) - Architecture overview
- [PORTING.md](PORTING.md) - Porting guide
- [BUILD.md](BUILD.md) - Build instructions
- [FAQ.md](FAQ.md) - Common questions

### Communication Channels

- GitHub Issues: Bug reports and feature requests
- GitHub Discussions: General questions
- [Notepad++ Community Forum](https://community.notepad-plus-plus.org/)

### Asking Questions

When asking for help:

1. Search existing issues first
2. Provide context and background
3. Include relevant code snippets
4. Describe what you've tried
5. Be specific about the problem

### Reporting Bugs

Use the bug report template and include:

1. Linux distribution and version
2. Desktop environment
3. Qt version
4. Compiler version
5. Steps to reproduce
6. Expected vs actual behavior
7. Debug information from `? > Debug Info...`

## Recognition

Contributors will be recognized in:

- Git commit history
- Release notes
- CONTRIBUTORS file (for significant contributions)

Thank you for contributing to Notepad++!
