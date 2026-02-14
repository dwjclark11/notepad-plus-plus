// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QObject>
#include <QtTest/QtTest>
#include <QString>
#include <QStringList>
#include <memory>
#include "Platform/Dialogs.h"

namespace Tests {

// Mock dialog provider for testing
class MockDialogsProvider;

class DialogsTest : public QObject {
    Q_OBJECT

public:
    DialogsTest();
    ~DialogsTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

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
    PlatformLayer::IDialogs* _dialogs = nullptr;
    std::unique_ptr<MockDialogsProvider> _mockProvider;

    // Helper to create a test dialog handle
    void* createTestDialog();
    void destroyTestDialog(void* handle);
};

// Mock implementation for testing - implements IDialogs so it can be injected
class MockDialogsProvider : public PlatformLayer::IDialogs {
public:
    // Setup methods
    void setNextResult(PlatformLayer::DialogResult result);
    void setNextFileName(const QString& fileName);
    void setNextFileNames(const QStringList& fileNames);
    void setNextBoolResult(bool result);
    void setNextIntResult(int result);
    void setNextInputValue(const std::wstring& value);

    PlatformLayer::DialogResult getNextResult();
    QString getNextFileName();
    QStringList getNextFileNames();
    bool getNextBoolResult();
    int getNextIntResult();

    void reset();

    // Track calls
    int callCount = 0;
    std::wstring lastMessage;
    std::wstring lastTitle;

    // IDialogs interface implementation
    PlatformLayer::DialogResult messageBox(const std::wstring& message,
                                           const std::wstring& title,
                                           PlatformLayer::MessageBoxType type,
                                           PlatformLayer::MessageBoxIcon icon,
                                           PlatformLayer::DialogResult defaultButton) override;

    void showInfo(const std::wstring& message, const std::wstring& title) override;
    void showWarning(const std::wstring& message, const std::wstring& title) override;
    void showError(const std::wstring& message, const std::wstring& title) override;
    bool askYesNo(const std::wstring& message, const std::wstring& title) override;
    PlatformLayer::DialogResult askYesNoCancel(const std::wstring& message,
                                               const std::wstring& title) override;
    bool askRetryCancel(const std::wstring& message, const std::wstring& title) override;

    std::wstring showOpenFileDialog(const std::wstring& title,
                                   const std::vector<PlatformLayer::FileFilter>& filters,
                                   const PlatformLayer::FileDialogOptions& options) override;
    std::vector<std::wstring> showOpenFilesDialog(const std::wstring& title,
                                                  const std::vector<PlatformLayer::FileFilter>& filters,
                                                  const PlatformLayer::FileDialogOptions& options) override;
    std::wstring showSaveFileDialog(const std::wstring& title,
                                   const std::vector<PlatformLayer::FileFilter>& filters,
                                   const std::wstring& defaultFileName,
                                   const PlatformLayer::FileDialogOptions& options) override;
    std::wstring showFolderDialog(const std::wstring& title,
                                 const PlatformLayer::FolderDialogOptions& options) override;

    bool showInputDialog(const std::wstring& title,
                        const std::wstring& prompt,
                        std::wstring& value,
                        bool isPassword) override;
    bool showInputDialogEx(const PlatformLayer::InputDialogOptions& options,
                          std::wstring& value) override;
    bool showMultiLineInputDialog(const std::wstring& title,
                                 const std::wstring& prompt,
                                 std::wstring& value) override;
    int showListDialog(const std::wstring& title,
                      const std::wstring& prompt,
                      const std::vector<std::wstring>& items,
                      int defaultIndex) override;

    PlatformLayer::DialogResult showCustomDialog(void* dialogData) override;
    void centerDialog(void* dialogHandle) override;
    void setDialogPosition(void* dialogHandle, int x, int y) override;
    void getDialogPosition(void* dialogHandle, int& x, int& y) override;
    void setDialogSize(void* dialogHandle, int width, int height) override;
    void getDialogSize(void* dialogHandle, int& width, int& height) override;
    void setDialogTitle(void* dialogHandle, const std::wstring& title) override;
    void enableDialog(void* dialogHandle, bool enable) override;
    bool isDialogEnabled(void* dialogHandle) override;
    void bringToFront(void* dialogHandle) override;
    void setModal(void* dialogHandle, bool modal) override;

private:
    std::vector<PlatformLayer::DialogResult> _results;
    std::vector<QString> _fileNames;
    std::vector<QStringList> _fileNamesList;
    std::vector<bool> _boolResults;
    std::vector<int> _intResults;
    std::vector<std::wstring> _inputValues;
    size_t _resultIndex = 0;
    size_t _fileNameIndex = 0;
    size_t _fileNamesListIndex = 0;
    size_t _boolIndex = 0;
    size_t _intIndex = 0;
    size_t _inputValueIndex = 0;
};

} // namespace Tests
