# Notepad++ Linux Test Framework

This directory contains the Qt Test-based testing framework for the Notepad++ Linux port.

## Directory Structure

```
Tests/
├── CMakeLists.txt           # Main CMake configuration for tests
├── README.md                # This file
├── run_tests.sh.in          # Test runner script template
├── Common/                  # Common test utilities
│   ├── TestUtils.h          # Test utility classes and functions
│   ├── TestUtils.cpp        # Test utility implementations
│   └── TestMain.cpp         # Common main() for test executables
├── Platform/                # Platform abstraction tests
│   ├── FileSystemTest.h/.cpp
│   ├── SettingsTest.h/.cpp
│   ├── ProcessTest.h/.cpp
│   ├── FileWatcherTest.h/.cpp
│   ├── ClipboardTest.h/.cpp
│   └── DialogsTest.h/.cpp
├── QtControls/              # QtControls tests
│   ├── WindowTest.h/.cpp
│   ├── StaticDialogTest.h/.cpp
│   ├── TreeViewTest.h/.cpp
│   ├── ListViewTest.h/.cpp
│   ├── DockingManagerTest.h/.cpp
│   ├── FindReplaceDlgTest.h/.cpp
│   ├── GoToLineDlgTest.h/.cpp
│   ├── RunDlgTest.h/.cpp
│   ├── AboutDlgTest.h/.cpp
│   ├── PreferenceDlgTest.h/.cpp
│   └── ShortcutMapperTest.h/.cpp
└── Integration/             # Integration tests
    ├── MainWindowTest.h/.cpp
    ├── BufferTest.h/.cpp
    ├── IOTest.h/.cpp
    └── CommandTest.h/.cpp
```

## Building Tests

From the project root:

```bash
mkdir -p build && cd build
cmake ../PowerEditor/src/Tests
make
```

Or integrate with the main build:

```bash
mkdir -p build && cd build
cmake ../PowerEditor/src
cmake --build . --target PlatformTests QtControlsTests IntegrationTests
```

## Running Tests

### Run all tests:
```bash
./run_tests.sh
```

### Run specific test categories:
```bash
./run_tests.sh --platform-only
./run_tests.sh --qtcontrols-only
./run_tests.sh --integration-only
```

### Run individual test executables:
```bash
./bin/PlatformTests
./bin/QtControlsTests
./bin/IntegrationTests
```

### Run with verbose output:
```bash
./run_tests.sh --verbose
```

### Run specific test functions:
```bash
./bin/PlatformTests testFileExists
./bin/QtControlsTests testAddItem
```

## Test Categories

### Platform Tests
Tests for the platform abstraction layer:
- **FileSystemTest**: File operations, paths, permissions
- **SettingsTest**: Read/write settings, persistence
- **ProcessTest**: Process launching, output capture
- **FileWatcherTest**: File change detection
- **ClipboardTest**: Clipboard operations
- **DialogsTest**: Dialog operations (mocked)

### QtControls Tests
Tests for Qt-based UI controls:
- **WindowTest**: Window creation, sizing
- **StaticDialogTest**: Dialog lifecycle
- **TreeViewTest**: Tree operations
- **ListViewTest**: List operations
- **DockingManagerTest**: Dock operations
- **FindReplaceDlgTest**: Find/Replace dialog
- **GoToLineDlgTest**: Go To Line dialog
- **RunDlgTest**: Run dialog
- **AboutDlgTest**: About dialog
- **PreferenceDlgTest**: Preferences dialog
- **ShortcutMapperTest**: Shortcut mapper dialog

### Integration Tests
End-to-end integration tests:
- **MainWindowTest**: Full application startup
- **BufferTest**: Document lifecycle
- **IOTest**: File open/save operations
- **CommandTest**: Command execution

## Writing New Tests

### Basic Test Structure

```cpp
#include <QtTest/QtTest>
#include "../Common/TestUtils.h"

namespace Tests {

class MyTest : public QObject {
    Q_OBJECT

public:
    MyTest();
    ~MyTest();

private slots:
    void initTestCase();    // Called once before all tests
    void cleanupTestCase(); // Called once after all tests
    void init();            // Called before each test
    void cleanup();         // Called after each test

    // Your test functions
    void testFunction1();
    void testFunction2();
};

MyTest::MyTest() {}
MyTest::~MyTest() {}

void MyTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void MyTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void MyTest::init() {
    // Setup before each test
}

void MyTest::cleanup() {
    // Cleanup after each test
}

void MyTest::testFunction1() {
    QVERIFY(true);  // Assertion that must pass
    QCOMPARE(1, 1); // Equality assertion
}

void MyTest::testFunction2() {
    // Your test code
}

} // namespace Tests

QTEST_MAIN(Tests::MyTest)
#include "MyTest.moc"
```

### Test Macros

- `QVERIFY(condition)` - Assert that condition is true
- `QCOMPARE(actual, expected)` - Assert that actual equals expected
- `QFAIL(message)` - Unconditional failure
- `QSKIP(message)` - Skip this test
- `QEXPECT_FAIL(dataIndex, comment, mode)` - Expected failure

### Test Utilities

The `TestUtils` namespace provides helper functions:

```cpp
// File operations
Tests::FileUtils::createFile(path, content);
Tests::FileUtils::readFile(path);
Tests::FileUtils::compareFiles(path1, path2);

// Widget operations
Tests::WidgetTestUtils::waitForWidgetVisible(widget);
Tests::WidgetTestUtils::simulateKeyPress(widget, Qt::Key_Enter);
Tests::WidgetTestUtils::simulateMouseClick(widget);

// Test data
Tests::TestData::randomString(length);
Tests::TestData::sampleText();
Tests::TestData::sampleCode();
```

## Continuous Integration

The tests are designed to run in CI environments. Use the `-platform offscreen` option for headless testing:

```bash
QT_QPA_PLATFORM=offscreen ./bin/PlatformTests
```

## Code Coverage

To generate coverage reports:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make
ctest
make coverage
```

## License

These tests are part of the Notepad++ project and are licensed under the GNU General Public License v3.0 or later.
