# Notepad++ Tests CMakeLists.txt
# Qt Test-based testing framework for Linux port

cmake_minimum_required(VERSION 3.16)

project(NotepadPlusPlusTests VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt Test
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Test)

# Include directories
# Note: CMAKE_SOURCE_DIR is PowerEditor/src (the directory passed to cmake)
# IMPORTANT: QtControls paths must come BEFORE any inherited WinControls paths
set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Platform
    ${CMAKE_SOURCE_DIR}/QtControls
    ${CMAKE_SOURCE_DIR}/QtControls/StaticDialog
    ${CMAKE_SOURCE_DIR}/QtControls/ToolBar
    ${CMAKE_SOURCE_DIR}/QtControls/StatusBar
    ${CMAKE_SOURCE_DIR}/QtControls/TabBar
    ${CMAKE_SOURCE_DIR}/QtControls/Splitter
    ${CMAKE_SOURCE_DIR}/QtControls/GoToLine
    ${CMAKE_SOURCE_DIR}/QtControls/RunDlg
    ${CMAKE_SOURCE_DIR}/QtControls/AboutDlg
    ${CMAKE_SOURCE_DIR}/QtControls/TreeView
    ${CMAKE_SOURCE_DIR}/QtControls/ListView
    ${CMAKE_SOURCE_DIR}/QtControls/DockingManager
    ${CMAKE_SOURCE_DIR}/QtControls/FindReplace
    ${CMAKE_SOURCE_DIR}/QtControls/Preference
    ${CMAKE_SOURCE_DIR}/QtControls/ShortcutMapper
    ${CMAKE_SOURCE_DIR}/QtControls/WordStyleDlg
    ${CMAKE_SOURCE_DIR}/QtControls/UserDefineDialog
    ${CMAKE_SOURCE_DIR}/QtControls/ClipboardHistory
    ${CMAKE_SOURCE_DIR}/QtControls/DocumentMap
    ${CMAKE_SOURCE_DIR}/QtControls/FileBrowser
    ${CMAKE_SOURCE_DIR}/QtControls/FunctionList
    ${CMAKE_SOURCE_DIR}/QtControls/ProjectPanel
    ${CMAKE_SOURCE_DIR}/QtControls/MainWindow
    ${CMAKE_SOURCE_DIR}/QtControls/Shortcut
    ${CMAKE_SOURCE_DIR}/QtControls/ShortcutManager
    ${CMAKE_SOURCE_DIR}/QtControls/FindCharsInRange
    ${CMAKE_SOURCE_DIR}/QtControls/PluginsAdmin
    ${CMAKE_SOURCE_DIR}/QtControls/RunMacroDlg
    ${CMAKE_SOURCE_DIR}/QtControls/SplitterContainer
    ${CMAKE_SOURCE_DIR}/QtControls/DocTabView
    ${CMAKE_SOURCE_DIR}/QtControls/ColumnEditor
    ${CMAKE_SOURCE_DIR}/QtControls/HashDlgs
    ${CMAKE_SOURCE_DIR}/QtControls/AnsiCharPanel
    ${CMAKE_SOURCE_DIR}/QtControls/VerticalFileSwitcher
    ${CMAKE_SOURCE_DIR}/QtControls/SmartHighlighter
    ${CMAKE_SOURCE_DIR}/QtControls/KDEStyle
    ${CMAKE_SOURCE_DIR}/QtCore
    ${CMAKE_SOURCE_DIR}/WinControls/DockingWnd
    ${CMAKE_SOURCE_DIR}/WinControls/Window
    ${CMAKE_SOURCE_DIR}/WinControls/FindCharsInRange
    ${CMAKE_SOURCE_DIR}/WinControls/WindowsDlg
    ${CMAKE_SOURCE_DIR}/WinControls/ContextMenu
    ${CMAKE_SOURCE_DIR}/WinControls/TaskList
    ${CMAKE_SOURCE_DIR}/WinControls/Grid
    ${CMAKE_SOURCE_DIR}/WinControls/ToolTip
    ${CMAKE_SOURCE_DIR}/WinControls/ImageListSet
    ${CMAKE_SOURCE_DIR}/WinControls/ColourPicker
    ${CMAKE_SOURCE_DIR}/WinControls/AnsiCharPanel
    ${CMAKE_SOURCE_DIR}/WinControls/VerticalFileSwitcher
    ${CMAKE_SOURCE_DIR}/WinControls/DocumentMap
    ${CMAKE_SOURCE_DIR}/WinControls/OpenSaveFileDialog
    ${CMAKE_SOURCE_DIR}/WinControls/ReadDirectoryChanges
    ${CMAKE_SOURCE_DIR}/WinControls/DoubleBuffer
    ${CMAKE_SOURCE_DIR}/WinControls/TabBar
    ${CMAKE_SOURCE_DIR}/WinControls/StaticDialog
    ${CMAKE_SOURCE_DIR}/WinControls/AboutDlg
    ${CMAKE_SOURCE_DIR}/ScintillaComponent
    ${CMAKE_SOURCE_DIR}/MISC/Process
    ${CMAKE_SOURCE_DIR}/MISC/SysMsg
    ${CMAKE_SOURCE_DIR}/MISC/RegExt
    ${CMAKE_SOURCE_DIR}/MISC/Exception
    ${CMAKE_SOURCE_DIR}/MISC/md5
    ${CMAKE_SOURCE_DIR}/MISC/sha1
    ${CMAKE_SOURCE_DIR}/MISC/sha2
    ${CMAKE_SOURCE_DIR}/MISC/sha512
    ${CMAKE_SOURCE_DIR}/uchardet
    ${CMAKE_SOURCE_DIR}/pugixml
    ${CMAKE_SOURCE_DIR}/json
    ${CMAKE_SOURCE_DIR}/QtCore
    ${CMAKE_SOURCE_DIR}/../../scintilla/include
    ${CMAKE_SOURCE_DIR}/../../lexilla/include
    ${CMAKE_SOURCE_DIR}/../../lexilla/lexlib
    ${CMAKE_SOURCE_DIR}/TinyXml
    ${CMAKE_SOURCE_DIR}/MISC/Common
    ${CMAKE_SOURCE_DIR}/MISC/PluginsManager
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Test_INCLUDE_DIRS}
)

# Common test utilities (without TestMain.cpp - each executable has its own)
set(COMMON_TEST_SOURCES
    Common/TestUtils.cpp
    Common/TestUtils.h
)

# Platform Tests
set(PLATFORM_TEST_SOURCES
    Platform/FileSystemTest.cpp
    Platform/FileSystemTest.h
    Platform/SettingsTest.cpp
    Platform/SettingsTest.h
    Platform/ProcessTest.cpp
    Platform/ProcessTest.h
    Platform/FileWatcherTest.cpp
    Platform/FileWatcherTest.h
    Platform/ClipboardTest.cpp
    Platform/ClipboardTest.h
    Platform/DialogsTest.cpp
    Platform/DialogsTest.h
)

# QtControls Tests
# Note: FindReplaceDlgTest, GoToLineDlgTest, PreferenceDlgTest, and ShortcutMapperTest
# require NppParameters which is a complex dependency. Excluded for now.
set(QTCONTROLS_TEST_SOURCES
    QtControls/WindowTest.cpp
    QtControls/WindowTest.h
    QtControls/StaticDialogTest.cpp
    QtControls/StaticDialogTest.h
    QtControls/TreeViewTest.cpp
    QtControls/TreeViewTest.h
    QtControls/ListViewTest.cpp
    QtControls/ListViewTest.h
    QtControls/DockingManagerTest.cpp
    QtControls/DockingManagerTest.h
    QtControls/RunDlgTest.cpp
    QtControls/RunDlgTest.h
    QtControls/AboutDlgTest.cpp
    QtControls/AboutDlgTest.h
    QtControls/ToolBarTest.cpp
    QtControls/ToolBarTest.h
    QtControls/TabSignalRaceTest.cpp
    QtControls/TabSignalRaceTest.h
)

# Integration Tests
set(INTEGRATION_TEST_SOURCES
    Integration/MainWindowTest.cpp
    Integration/MainWindowTest.h
    Integration/BufferTest.cpp
    Integration/BufferTest.h
    Integration/IOTest.cpp
    Integration/IOTest.h
    Integration/CommandTest.cpp
    Integration/CommandTest.h
)

# Create individual test executables for each test category

# Platform Tests Executable
add_executable(PlatformTests
    ${COMMON_TEST_SOURCES}
    Common/TestMain.cpp
    ${PLATFORM_TEST_SOURCES}
)

target_include_directories(PlatformTests PRIVATE ${TEST_INCLUDE_DIRS})

target_link_libraries(PlatformTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)

# Link with Platform implementation
if(UNIX AND NOT APPLE)
    target_sources(PlatformTests PRIVATE
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileSystem.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Settings.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Process.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileWatcher.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Clipboard.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Dialogs.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Threading.cpp
    )
endif()

# QtControls Tests Executable
add_executable(QtControlsTests
    ${COMMON_TEST_SOURCES}
    Common/TestMainQtControls.cpp
    ${QTCONTROLS_TEST_SOURCES}
)

target_include_directories(QtControlsTests PRIVATE ${TEST_INCLUDE_DIRS})

# Define NPP_LINUX for Linux builds and link with QtControls implementation
if(UNIX AND NOT APPLE)
    target_compile_definitions(QtControlsTests PRIVATE NPP_LINUX)

    # Add QtControls implementation sources
    # Note: FindReplaceDlg, GoToLineDlg, PreferenceDlg, ShortcutMapper, ShortcutManager
    # excluded due to NppParameters dependency
    target_sources(QtControlsTests PRIVATE
        ${CMAKE_SOURCE_DIR}/QtControls/Window.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/StaticDialog/StaticDialog.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/TreeView/TreeView.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/ListView/ListView.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/DockingManager/DockingManager.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/RunDlg/RunDlg.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/AboutDlg/AboutDlg.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/ToolBar/ToolBar.cpp
    )

    # Stubs for pre-existing linker errors (RunDlg deps on NppParameters/Buffer/ScintillaEditView)
    target_sources(QtControlsTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/QtControls/Stubs/QtControlsStubs.cpp
        ${CMAKE_SOURCE_DIR}/MISC/Common/CommonLinux.cpp
    )

    # Add Platform implementation sources (needed by QtControls)
    target_sources(QtControlsTests PRIVATE
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileSystem.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Settings.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Process.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileWatcher.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Clipboard.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Dialogs.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Threading.cpp
    )
endif()

target_link_libraries(QtControlsTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)

# Integration Tests Executable
# Note: MainWindowTest and BufferTest are excluded due to heavy dependencies
# on Notepad_plus, NppParameters, and QtCore::Buffer that would require
# pulling in most of the application.
add_executable(IntegrationTests
    ${COMMON_TEST_SOURCES}
    Common/TestMainIntegration.cpp
    Integration/IOTest.cpp
    Integration/IOTest.h
    Integration/CommandTest.cpp
    Integration/CommandTest.h
    Integration/IPCParseTest.cpp
    Integration/IPCParseTest.h
    Integration/FindReplaceDlgInitTest.cpp
    Integration/FindReplaceDlgInitTest.h
    Integration/Stubs/NppParametersStub.cpp
)

target_include_directories(IntegrationTests PRIVATE ${TEST_INCLUDE_DIRS})

# Define NPP_LINUX for Linux builds and link with Platform implementation
if(UNIX AND NOT APPLE)
    target_compile_definitions(IntegrationTests PRIVATE NPP_LINUX)

    # Add Platform implementation sources (needed by tests)
    target_sources(IntegrationTests PRIVATE
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileSystem.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Settings.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Process.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileWatcher.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Clipboard.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Dialogs.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Threading.cpp
    )

    # Add QtControls implementation sources (needed by FindReplaceDlgInitTest)
    target_sources(IntegrationTests PRIVATE
        ${CMAKE_SOURCE_DIR}/QtControls/StaticDialog/StaticDialog.cpp
        ${CMAKE_SOURCE_DIR}/QtControls/FindReplace/FindReplaceDlg.cpp
    )
endif()

target_link_libraries(IntegrationTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)

# Enable testing
enable_testing()

# Add tests
add_test(NAME PlatformTests COMMAND PlatformTests)
add_test(NAME QtControlsTests COMMAND QtControlsTests)
add_test(NAME IntegrationTests COMMAND IntegrationTests)

# Test runner script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh.in
    ${CMAKE_BINARY_DIR}/run_tests.sh
    @ONLY
)

# Make test runner executable
file(CHMOD ${CMAKE_BINARY_DIR}/run_tests.sh
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

# Output directories
set_target_properties(PlatformTests QtControlsTests IntegrationTests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PlatformTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(QtControlsTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(IntegrationTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS PlatformTests QtControlsTests IntegrationTests
    RUNTIME DESTINATION bin/tests
)
