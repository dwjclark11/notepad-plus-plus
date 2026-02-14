// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DialogsTest.h"
#include "Dialogs.h"
#include "../Common/TestUtils.h"
#include <QWidget>

using namespace PlatformLayer;

namespace Tests {

// ============================================================================
// MockDialogsProvider Implementation
// ============================================================================
void MockDialogsProvider::setNextResult(DialogResult result) {
    _results.push_back(result);
}

void MockDialogsProvider::setNextFileName(const QString& fileName) {
    _fileNames.push_back(fileName);
}

void MockDialogsProvider::setNextFileNames(const QStringList& fileNames) {
    _fileNamesList.push_back(fileNames);
}

void MockDialogsProvider::setNextBoolResult(bool result) {
    _boolResults.push_back(result);
}

void MockDialogsProvider::setNextIntResult(int result) {
    _intResults.push_back(result);
}

void MockDialogsProvider::setNextInputValue(const std::wstring& value) {
    _inputValues.push_back(value);
}

DialogResult MockDialogsProvider::getNextResult() {
    if (_resultIndex < _results.size()) {
        return _results[_resultIndex++];
    }
    return DialogResult::OK;
}

QString MockDialogsProvider::getNextFileName() {
    if (_fileNameIndex < _fileNames.size()) {
        return _fileNames[_fileNameIndex++];
    }
    return QString();
}

QStringList MockDialogsProvider::getNextFileNames() {
    if (_fileNamesListIndex < _fileNamesList.size()) {
        return _fileNamesList[_fileNamesListIndex++];
    }
    return QStringList();
}

bool MockDialogsProvider::getNextBoolResult() {
    if (_boolIndex < _boolResults.size()) {
        return _boolResults[_boolIndex++];
    }
    return false;
}

int MockDialogsProvider::getNextIntResult() {
    if (_intIndex < _intResults.size()) {
        return _intResults[_intIndex++];
    }
    return -1;
}

void MockDialogsProvider::reset() {
    _results.clear();
    _fileNames.clear();
    _fileNamesList.clear();
    _boolResults.clear();
    _intResults.clear();
    _inputValues.clear();
    _resultIndex = 0;
    _fileNameIndex = 0;
    _fileNamesListIndex = 0;
    _boolIndex = 0;
    _intIndex = 0;
    _inputValueIndex = 0;
    callCount = 0;
    lastMessage.clear();
    lastTitle.clear();
}

// IDialogs interface implementations
DialogResult MockDialogsProvider::messageBox(const std::wstring& message,
                                             const std::wstring& title,
                                             MessageBoxType /*type*/,
                                             MessageBoxIcon /*icon*/,
                                             DialogResult /*defaultButton*/) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
    return getNextResult();
}

void MockDialogsProvider::showInfo(const std::wstring& message, const std::wstring& title) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
}

void MockDialogsProvider::showWarning(const std::wstring& message, const std::wstring& title) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
}

void MockDialogsProvider::showError(const std::wstring& message, const std::wstring& title) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
}

bool MockDialogsProvider::askYesNo(const std::wstring& message, const std::wstring& title) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
    return getNextBoolResult();
}

DialogResult MockDialogsProvider::askYesNoCancel(const std::wstring& message,
                                                  const std::wstring& title) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
    return getNextResult();
}

bool MockDialogsProvider::askRetryCancel(const std::wstring& message, const std::wstring& title) {
    ++callCount;
    lastMessage = message;
    lastTitle = title;
    return getNextBoolResult();
}

std::wstring MockDialogsProvider::showOpenFileDialog(const std::wstring& title,
                                                     const std::vector<FileFilter>& /*filters*/,
                                                     const FileDialogOptions& /*options*/) {
    ++callCount;
    lastTitle = title;
    return getNextFileName().toStdWString();
}

std::vector<std::wstring> MockDialogsProvider::showOpenFilesDialog(const std::wstring& title,
                                                                    const std::vector<FileFilter>& /*filters*/,
                                                                    const FileDialogOptions& /*options*/) {
    ++callCount;
    lastTitle = title;
    std::vector<std::wstring> result;
    for (const auto& f : getNextFileNames()) {
        result.push_back(f.toStdWString());
    }
    return result;
}

std::wstring MockDialogsProvider::showSaveFileDialog(const std::wstring& title,
                                                     const std::vector<FileFilter>& /*filters*/,
                                                     const std::wstring& /*defaultFileName*/,
                                                     const FileDialogOptions& /*options*/) {
    ++callCount;
    lastTitle = title;
    return getNextFileName().toStdWString();
}

std::wstring MockDialogsProvider::showFolderDialog(const std::wstring& title,
                                                   const FolderDialogOptions& /*options*/) {
    ++callCount;
    lastTitle = title;
    return getNextFileName().toStdWString();
}

bool MockDialogsProvider::showInputDialog(const std::wstring& title,
                                          const std::wstring& prompt,
                                          std::wstring& value,
                                          bool /*isPassword*/) {
    ++callCount;
    lastTitle = title;
    lastMessage = prompt;
    bool accepted = getNextBoolResult();
    if (accepted && _inputValueIndex < _inputValues.size()) {
        value = _inputValues[_inputValueIndex++];
    }
    return accepted;
}

bool MockDialogsProvider::showInputDialogEx(const InputDialogOptions& options,
                                            std::wstring& value) {
    ++callCount;
    lastTitle = options.title;
    lastMessage = options.prompt;
    bool accepted = getNextBoolResult();
    if (accepted && _inputValueIndex < _inputValues.size()) {
        value = _inputValues[_inputValueIndex++];
    }
    return accepted;
}

bool MockDialogsProvider::showMultiLineInputDialog(const std::wstring& title,
                                                   const std::wstring& prompt,
                                                   std::wstring& value) {
    ++callCount;
    lastTitle = title;
    lastMessage = prompt;
    bool accepted = getNextBoolResult();
    if (accepted && _inputValueIndex < _inputValues.size()) {
        value = _inputValues[_inputValueIndex++];
    }
    return accepted;
}

int MockDialogsProvider::showListDialog(const std::wstring& title,
                                        const std::wstring& prompt,
                                        const std::vector<std::wstring>& /*items*/,
                                        int /*defaultIndex*/) {
    ++callCount;
    lastTitle = title;
    lastMessage = prompt;
    return getNextIntResult();
}

DialogResult MockDialogsProvider::showCustomDialog(void* /*dialogData*/) {
    ++callCount;
    return getNextResult();
}

void MockDialogsProvider::centerDialog(void* /*dialogHandle*/) { ++callCount; }
void MockDialogsProvider::setDialogPosition(void* /*dialogHandle*/, int /*x*/, int /*y*/) { ++callCount; }
void MockDialogsProvider::getDialogPosition(void* /*dialogHandle*/, int& x, int& y) { ++callCount; x = 0; y = 0; }
void MockDialogsProvider::setDialogSize(void* /*dialogHandle*/, int /*width*/, int /*height*/) { ++callCount; }
void MockDialogsProvider::getDialogSize(void* /*dialogHandle*/, int& width, int& height) { ++callCount; width = 0; height = 0; }
void MockDialogsProvider::setDialogTitle(void* /*dialogHandle*/, const std::wstring& /*title*/) { ++callCount; }
void MockDialogsProvider::enableDialog(void* /*dialogHandle*/, bool /*enable*/) { ++callCount; }
bool MockDialogsProvider::isDialogEnabled(void* /*dialogHandle*/) { ++callCount; return true; }
void MockDialogsProvider::bringToFront(void* /*dialogHandle*/) { ++callCount; }
void MockDialogsProvider::setModal(void* /*dialogHandle*/, bool /*modal*/) { ++callCount; }

// ============================================================================
// DialogsTest Implementation
// ============================================================================
DialogsTest::DialogsTest() {}

DialogsTest::~DialogsTest() {}

void DialogsTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _mockProvider = std::make_unique<MockDialogsProvider>();
    _dialogs = &IDialogs::getInstance();
    QVERIFY(_dialogs != nullptr);
}

void DialogsTest::cleanupTestCase() {
    IDialogs::resetTestInstance();
    TestEnvironment::getInstance().cleanup();
}

void DialogsTest::init() {
    _mockProvider->reset();
    IDialogs::setTestInstance(_mockProvider.get());
}

void DialogsTest::cleanup() {
    IDialogs::resetTestInstance();
}

void* DialogsTest::createTestDialog() {
    // Create a simple widget to use as dialog handle for testing
    return new QWidget();
}

void DialogsTest::destroyTestDialog(void* handle) {
    if (handle) {
        delete static_cast<QWidget*>(handle);
    }
}

// ============================================================================
// Message Box Tests
// ============================================================================
void DialogsTest::testMessageBox() {
    _mockProvider->setNextResult(DialogResult::Yes);

    auto& dlg = IDialogs::getInstance();
    DialogResult result = dlg.messageBox(L"Save changes?", L"Confirm",
                                          MessageBoxType::YesNo,
                                          MessageBoxIcon::Question,
                                          DialogResult::Yes);

    QCOMPARE(result, DialogResult::Yes);
    QCOMPARE(_mockProvider->callCount, 1);
    QCOMPARE(_mockProvider->lastMessage, std::wstring(L"Save changes?"));
    QCOMPARE(_mockProvider->lastTitle, std::wstring(L"Confirm"));
}

void DialogsTest::testShowInfo() {
    auto& dlg = IDialogs::getInstance();
    dlg.showInfo(L"Operation completed", L"Info");

    QCOMPARE(_mockProvider->callCount, 1);
    QCOMPARE(_mockProvider->lastMessage, std::wstring(L"Operation completed"));
    QCOMPARE(_mockProvider->lastTitle, std::wstring(L"Info"));
}

void DialogsTest::testShowWarning() {
    auto& dlg = IDialogs::getInstance();
    dlg.showWarning(L"Disk is almost full", L"Warning");

    QCOMPARE(_mockProvider->callCount, 1);
    QCOMPARE(_mockProvider->lastMessage, std::wstring(L"Disk is almost full"));
    QCOMPARE(_mockProvider->lastTitle, std::wstring(L"Warning"));
}

void DialogsTest::testShowError() {
    auto& dlg = IDialogs::getInstance();
    dlg.showError(L"File not found", L"Error");

    QCOMPARE(_mockProvider->callCount, 1);
    QCOMPARE(_mockProvider->lastMessage, std::wstring(L"File not found"));
    QCOMPARE(_mockProvider->lastTitle, std::wstring(L"Error"));
}

void DialogsTest::testAskYesNo() {
    _mockProvider->setNextBoolResult(true);

    auto& dlg = IDialogs::getInstance();
    bool result = dlg.askYesNo(L"Continue?", L"Confirm");

    QVERIFY(result);
    QCOMPARE(_mockProvider->callCount, 1);
    QCOMPARE(_mockProvider->lastMessage, std::wstring(L"Continue?"));
}

void DialogsTest::testAskYesNoCancel() {
    _mockProvider->setNextResult(DialogResult::Cancel);

    auto& dlg = IDialogs::getInstance();
    DialogResult result = dlg.askYesNoCancel(L"Save before closing?", L"Confirm");

    QCOMPARE(result, DialogResult::Cancel);
    QCOMPARE(_mockProvider->callCount, 1);
}

void DialogsTest::testAskRetryCancel() {
    _mockProvider->setNextBoolResult(true);

    auto& dlg = IDialogs::getInstance();
    bool result = dlg.askRetryCancel(L"Connection failed. Retry?", L"Retry");

    QVERIFY(result);
    QCOMPARE(_mockProvider->callCount, 1);
}

// ============================================================================
// File Dialog Tests
// ============================================================================
void DialogsTest::testShowOpenFileDialog() {
    _mockProvider->setNextFileName(QString("/home/user/test.txt"));

    auto& dlg = IDialogs::getInstance();
    std::vector<FileFilter> filters = { FileFilter(L"Text Files", L"*.txt") };
    std::wstring result = dlg.showOpenFileDialog(L"Open File", filters);

    QCOMPARE(result, std::wstring(L"/home/user/test.txt"));
    QCOMPARE(_mockProvider->callCount, 1);
}

void DialogsTest::testShowOpenFilesDialog() {
    _mockProvider->setNextFileNames(QStringList{"/home/user/a.txt", "/home/user/b.txt"});

    auto& dlg = IDialogs::getInstance();
    std::vector<FileFilter> filters = { FileFilter(L"Text Files", L"*.txt") };
    std::vector<std::wstring> result = dlg.showOpenFilesDialog(L"Open Files", filters);

    QCOMPARE(static_cast<int>(result.size()), 2);
    QCOMPARE(result[0], std::wstring(L"/home/user/a.txt"));
    QCOMPARE(result[1], std::wstring(L"/home/user/b.txt"));
    QCOMPARE(_mockProvider->callCount, 1);
}

void DialogsTest::testShowSaveFileDialog() {
    _mockProvider->setNextFileName(QString("/home/user/output.txt"));

    auto& dlg = IDialogs::getInstance();
    std::vector<FileFilter> filters = { FileFilter(L"Text Files", L"*.txt") };
    std::wstring result = dlg.showSaveFileDialog(L"Save File", filters, L"output.txt");

    QCOMPARE(result, std::wstring(L"/home/user/output.txt"));
    QCOMPARE(_mockProvider->callCount, 1);
}

// ============================================================================
// Folder Dialog Tests
// ============================================================================
void DialogsTest::testShowFolderDialog() {
    _mockProvider->setNextFileName(QString("/home/user/project"));

    auto& dlg = IDialogs::getInstance();
    std::wstring result = dlg.showFolderDialog(L"Select Folder");

    QCOMPARE(result, std::wstring(L"/home/user/project"));
    QCOMPARE(_mockProvider->callCount, 1);
}

// ============================================================================
// Input Dialog Tests
// ============================================================================
void DialogsTest::testShowInputDialog() {
    _mockProvider->setNextBoolResult(true);
    _mockProvider->setNextInputValue(L"user input text");

    auto& dlg = IDialogs::getInstance();
    std::wstring value;
    bool accepted = dlg.showInputDialog(L"Input", L"Enter name:", value);

    QVERIFY(accepted);
    QCOMPARE(value, std::wstring(L"user input text"));
    QCOMPARE(_mockProvider->callCount, 1);
}

void DialogsTest::testShowInputDialogEx() {
    _mockProvider->setNextBoolResult(true);
    _mockProvider->setNextInputValue(L"advanced input");

    auto& dlg = IDialogs::getInstance();
    InputDialogOptions opts;
    opts.title = L"Advanced Input";
    opts.prompt = L"Enter value:";
    opts.defaultValue = L"default";
    std::wstring value;
    bool accepted = dlg.showInputDialogEx(opts, value);

    QVERIFY(accepted);
    QCOMPARE(value, std::wstring(L"advanced input"));
    QCOMPARE(_mockProvider->callCount, 1);
}

void DialogsTest::testShowMultiLineInputDialog() {
    _mockProvider->setNextBoolResult(true);
    _mockProvider->setNextInputValue(L"line1\nline2\nline3");

    auto& dlg = IDialogs::getInstance();
    std::wstring value;
    bool accepted = dlg.showMultiLineInputDialog(L"Multi-Line", L"Enter text:", value);

    QVERIFY(accepted);
    QCOMPARE(value, std::wstring(L"line1\nline2\nline3"));
    QCOMPARE(_mockProvider->callCount, 1);
}

void DialogsTest::testShowListDialog() {
    _mockProvider->setNextIntResult(2);

    auto& dlg = IDialogs::getInstance();
    std::vector<std::wstring> items = {L"Apple", L"Banana", L"Cherry"};
    int selected = dlg.showListDialog(L"Select Item", L"Choose a fruit:", items);

    QCOMPARE(selected, 2);
    QCOMPARE(_mockProvider->callCount, 1);
}

// ============================================================================
// Dialog Utilities Tests
// ============================================================================
void DialogsTest::testCenterDialog() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Use real dialogs instance for widget-based tests
    IDialogs::resetTestInstance();
    _dialogs->centerDialog(handle);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogPosition() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    _dialogs->setDialogPosition(handle, 100, 100);

    destroyTestDialog(handle);
}

void DialogsTest::testGetDialogPosition() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    int x = 0, y = 0;
    _dialogs->getDialogPosition(handle, x, y);

    // Values should be set (may be 0,0 for new widget)
    QVERIFY(x >= 0 && y >= 0);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogSize() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    _dialogs->setDialogSize(handle, 400, 300);

    destroyTestDialog(handle);
}

void DialogsTest::testGetDialogSize() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    int width = 0, height = 0;
    _dialogs->getDialogSize(handle, width, height);

    // Values should be set (may be default size)
    QVERIFY(width >= 0 && height >= 0);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogTitle() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    _dialogs->setDialogTitle(handle, L"New Title");

    destroyTestDialog(handle);
}

void DialogsTest::testEnableDialog() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    _dialogs->enableDialog(handle, false);
    QVERIFY(!_dialogs->isDialogEnabled(handle));

    _dialogs->enableDialog(handle, true);
    QVERIFY(_dialogs->isDialogEnabled(handle));

    destroyTestDialog(handle);
}

void DialogsTest::testIsDialogEnabled() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    // Default should be enabled
    QVERIFY(_dialogs->isDialogEnabled(handle));

    destroyTestDialog(handle);
}

void DialogsTest::testBringToFront() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    _dialogs->bringToFront(handle);

    destroyTestDialog(handle);
}

void DialogsTest::testSetModal() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    IDialogs::resetTestInstance();
    _dialogs->setModal(handle, true);
    _dialogs->setModal(handle, false);

    destroyTestDialog(handle);
}

// ============================================================================
// File Filter Helpers Tests
// ============================================================================
void DialogsTest::testAllFilesFilter() {
    std::vector<FileFilter> filters = DialogFilters::allFiles();
    QVERIFY(!filters.empty());

    // Should contain "All Files" or similar
    bool foundAllFiles = false;
    for (const auto& filter : filters) {
        if (filter.pattern == L"*.*" || filter.pattern == L"*") {
            foundAllFiles = true;
            break;
        }
    }
    QVERIFY(foundAllFiles);
}

void DialogsTest::testTextFilesFilter() {
    std::vector<FileFilter> filters = DialogFilters::textFiles();
    QVERIFY(!filters.empty());

    // Should contain text file patterns
    bool foundTextPattern = false;
    for (const auto& filter : filters) {
        if (filter.pattern.find(L"*.txt") != std::wstring::npos) {
            foundTextPattern = true;
            break;
        }
    }
    QVERIFY(foundTextPattern);
}

void DialogsTest::testSourceCodeFilesFilter() {
    std::vector<FileFilter> filters = DialogFilters::sourceCodeFiles();
    QVERIFY(!filters.empty());

    // Should contain source code patterns
    bool foundSourcePattern = false;
    for (const auto& filter : filters) {
        if (filter.pattern.find(L"*.cpp") != std::wstring::npos ||
            filter.pattern.find(L"*.c") != std::wstring::npos ||
            filter.pattern.find(L"*.h") != std::wstring::npos) {
            foundSourcePattern = true;
            break;
        }
    }
    QVERIFY(foundSourcePattern);
}

void DialogsTest::testCombineFilters() {
    std::vector<FileFilter> filters1 = DialogFilters::textFiles();
    std::vector<FileFilter> filters2 = DialogFilters::sourceCodeFiles();

    std::vector<FileFilter> combined = DialogFilters::combine(filters1, filters2);

    QCOMPARE(static_cast<int>(combined.size()),
             static_cast<int>(filters1.size() + filters2.size()));
}

// ============================================================================
// Progress Dialog Tests
// ============================================================================
void DialogsTest::testProgressDialog() {
    std::unique_ptr<IProgressDialog> progress(IProgressDialog::create());
    QVERIFY(progress != nullptr);

    progress->show(L"Test Progress", L"Processing...");
    progress->setProgress(50);
    progress->setMessage(L"Halfway done");
    progress->setStatus(L"Working...");
    progress->step(10);
    progress->setRange(0, 100);

    QVERIFY(!progress->isCancelled());

    progress->hide();
}

} // namespace Tests

#include "DialogsTest.moc"
