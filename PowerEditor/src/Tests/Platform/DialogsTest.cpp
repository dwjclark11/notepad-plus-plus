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
    _resultIndex = 0;
    _fileNameIndex = 0;
    _fileNamesListIndex = 0;
    _boolIndex = 0;
    _intIndex = 0;
}

// ============================================================================
// DialogsTest Implementation
// ============================================================================
DialogsTest::DialogsTest() {}

DialogsTest::~DialogsTest() {}

void DialogsTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _dialogs = &IDialogs::getInstance();
    QVERIFY(_dialogs != nullptr);
    _mockProvider = std::make_unique<MockDialogsProvider>();
}

void DialogsTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
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
    // messageBox() shows a modal dialog requiring user interaction.
    // Cannot be tested without a mock injection mechanism for IDialogs.
    QSKIP("messageBox() requires interactive display; no mock injection available");
}

void DialogsTest::testShowInfo() {
    QSKIP("showInfo() requires interactive display; no mock injection available");
}

void DialogsTest::testShowWarning() {
    QSKIP("showWarning() requires interactive display; no mock injection available");
}

void DialogsTest::testShowError() {
    QSKIP("showError() requires interactive display; no mock injection available");
}

void DialogsTest::testAskYesNo() {
    QSKIP("askYesNo() requires interactive display; no mock injection available");
}

void DialogsTest::testAskYesNoCancel() {
    QSKIP("askYesNoCancel() requires interactive display; no mock injection available");
}

void DialogsTest::testAskRetryCancel() {
    QSKIP("askRetryCancel() requires interactive display; no mock injection available");
}

// ============================================================================
// File Dialog Tests
// ============================================================================
void DialogsTest::testShowOpenFileDialog() {
    QSKIP("showOpenFileDialog() requires interactive display; no mock injection available");
}

void DialogsTest::testShowOpenFilesDialog() {
    QSKIP("showOpenFilesDialog() requires interactive display; no mock injection available");
}

void DialogsTest::testShowSaveFileDialog() {
    QSKIP("showSaveFileDialog() requires interactive display; no mock injection available");
}

// ============================================================================
// Folder Dialog Tests
// ============================================================================
void DialogsTest::testShowFolderDialog() {
    QSKIP("showFolderDialog() requires interactive display; no mock injection available");
}

// ============================================================================
// Input Dialog Tests
// ============================================================================
void DialogsTest::testShowInputDialog() {
    QSKIP("showInputDialog() requires interactive display; no mock injection available");
}

void DialogsTest::testShowInputDialogEx() {
    QSKIP("showInputDialogEx() requires interactive display; no mock injection available");
}

void DialogsTest::testShowMultiLineInputDialog() {
    QSKIP("showMultiLineInputDialog() requires interactive display; no mock injection available");
}

void DialogsTest::testShowListDialog() {
    QSKIP("showListDialog() requires interactive display; no mock injection available");
}

// ============================================================================
// Dialog Utilities Tests
// ============================================================================
void DialogsTest::testCenterDialog() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->centerDialog(handle);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogPosition() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setDialogPosition(handle, 100, 100);

    destroyTestDialog(handle);
}

void DialogsTest::testGetDialogPosition() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    int x = 0, y = 0;
    _dialogs->getDialogPosition(handle, x, y);

    // Values should be set (may be 0,0 for new widget)
    QVERIFY(x >= 0 && y >= 0);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogSize() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setDialogSize(handle, 400, 300);

    destroyTestDialog(handle);
}

void DialogsTest::testGetDialogSize() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    int width = 0, height = 0;
    _dialogs->getDialogSize(handle, width, height);

    // Values should be set (may be default size)
    QVERIFY(width >= 0 && height >= 0);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogTitle() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setDialogTitle(handle, L"New Title");

    destroyTestDialog(handle);
}

void DialogsTest::testEnableDialog() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    _dialogs->enableDialog(handle, false);
    QVERIFY(!_dialogs->isDialogEnabled(handle));

    _dialogs->enableDialog(handle, true);
    QVERIFY(_dialogs->isDialogEnabled(handle));

    destroyTestDialog(handle);
}

void DialogsTest::testIsDialogEnabled() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Default should be enabled
    QVERIFY(_dialogs->isDialogEnabled(handle));

    destroyTestDialog(handle);
}

void DialogsTest::testBringToFront() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->bringToFront(handle);

    destroyTestDialog(handle);
}

void DialogsTest::testSetModal() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
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
