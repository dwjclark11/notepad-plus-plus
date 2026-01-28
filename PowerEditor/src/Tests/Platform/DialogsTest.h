// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QString>
#include <memory>

namespace Platform {
    class IDialogs;
}

namespace Tests {

// Mock dialog provider for testing
class MockDialogsProvider;

class DialogsTest : public QObject {
    Q_OBJECT

public:
    DialogsTest();
    ~DialogsTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Message boxes
    void testMessageBox();
    void testShowInfo();
    void testShowWarning();
    void testShowError();
    void testAskYesNo();
    void testAskYesNoCancel();
    void testAskRetryCancel();

    // File dialogs
    void testShowOpenFileDialog();
    void testShowOpenFilesDialog();
    void testShowSaveFileDialog();

    // Folder dialogs
    void testShowFolderDialog();

    // Input dialogs
    void testShowInputDialog();
    void testShowInputDialogEx();
    void testShowMultiLineInputDialog();
    void testShowListDialog();

    // Dialog utilities
    void testCenterDialog();
    void testSetDialogPosition();
    void testGetDialogPosition();
    void testSetDialogSize();
    void testGetDialogSize();
    void testSetDialogTitle();
    void testEnableDialog();
    void testIsDialogEnabled();
    void testBringToFront();
    void testSetModal();

    // File filter helpers
    void testAllFilesFilter();
    void testTextFilesFilter();
    void testSourceCodeFilesFilter();
    void testCombineFilters();

    // Progress dialog
    void testProgressDialog();

private:
    Platform::IDialogs* _dialogs = nullptr;
    std::unique_ptr<MockDialogsProvider> _mockProvider;

    // Helper to create a test dialog handle
    void* createTestDialog();
    void destroyTestDialog(void* handle);
};

// Mock implementation for testing
class MockDialogsProvider {
public:
    void setNextResult(Platform::DialogResult result);
    void setNextFileName(const QString& fileName);
    void setNextFileNames(const QStringList& fileNames);
    void setNextBoolResult(bool result);
    void setNextIntResult(int result);

    Platform::DialogResult getNextResult();
    QString getNextFileName();
    QStringList getNextFileNames();
    bool getNextBoolResult();
    int getNextIntResult();

    void reset();

private:
    QList<Platform::DialogResult> _results;
    QList<QString> _fileNames;
    QList<QStringList> _fileNamesList;
    QList<bool> _boolResults;
    QList<int> _intResults;
    int _resultIndex = 0;
    int _fileNameIndex = 0;
    int _fileNamesListIndex = 0;
    int _boolIndex = 0;
    int _intIndex = 0;
};

} // namespace Tests
