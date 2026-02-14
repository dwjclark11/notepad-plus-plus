// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace PlatformLayer {

// ============================================================================
// Dialog Result Codes
// ============================================================================
enum class DialogResult {
    None = 0,
    OK = 1,
    Cancel = 2,
    Abort = 3,
    Retry = 4,
    Ignore = 5,
    Yes = 6,
    No = 7,
    TryAgain = 10,
    Continue = 11
};

// ============================================================================
// Message Box Types
// ============================================================================
enum class MessageBoxType {
    OK = 0,
    OKCancel = 1,
    AbortRetryIgnore = 2,
    YesNoCancel = 3,
    YesNo = 4,
    RetryCancel = 5,
    CancelTryAgainContinue = 6
};

// ============================================================================
// Message Box Icon
// ============================================================================
enum class MessageBoxIcon {
    None = 0,
    Error = 1,
    Question = 2,
    Warning = 3,
    Information = 4
};

// ============================================================================
// File Dialog Options
// ============================================================================
struct FileDialogOptions {
    bool allowMultiSelect = false;
    bool showHidden = false;
    bool overwritePrompt = true;      // For save dialogs
    bool fileMustExist = true;        // For open dialogs
    bool pathMustExist = true;
    bool createPrompt = false;        // For save dialogs
    bool noNetworkButton = false;
    bool noPlacesBar = false;
    std::wstring defaultExtension;
    std::wstring initialDirectory;
    std::wstring title;

    FileDialogOptions() = default;
};

// ============================================================================
// File Filter
// ============================================================================
struct FileFilter {
    std::wstring description;
    std::wstring pattern;  // e.g., L"*.txt;*.log"

    FileFilter() = default;
    FileFilter(const std::wstring& desc, const std::wstring& pat)
        : description(desc), pattern(pat) {}
};

// ============================================================================
// Folder Dialog Options
// ============================================================================
struct FolderDialogOptions {
    std::wstring title;
    std::wstring initialFolder;
    bool showNewFolderButton = true;
    bool showFiles = false;

    FolderDialogOptions() = default;
};

// ============================================================================
// Input Dialog Options
// ============================================================================
struct InputDialogOptions {
    std::wstring title;
    std::wstring prompt;
    std::wstring defaultValue;
    bool isPassword = false;
    bool multiline = false;
    size_t maxLength = 0;  // 0 = unlimited

    InputDialogOptions() = default;
};

// ============================================================================
// Progress Dialog
// ============================================================================
class IProgressDialog {
public:
    virtual ~IProgressDialog() = default;

    virtual void show(const std::wstring& title, const std::wstring& message) = 0;
    virtual void hide() = 0;

    virtual void setProgress(int percent) = 0;
    virtual void setMessage(const std::wstring& message) = 0;
    virtual void setStatus(const std::wstring& status) = 0;

    virtual bool isCancelled() const = 0;
    virtual void setCancelable(bool cancelable) = 0;

    virtual void step(int increment = 1) = 0;
    virtual void setRange(int min, int max) = 0;

    static std::unique_ptr<IProgressDialog> create();
};

// ============================================================================
// IDialogs Interface
// ============================================================================
class IDialogs {
public:
    virtual ~IDialogs() = default;

    // Singleton accessor
    static IDialogs& getInstance();

    // Test injection support
    static void setTestInstance(IDialogs* instance) { _testInstance = instance; }
    static void resetTestInstance() { _testInstance = nullptr; }

    // ------------------------------------------------------------------------
    // Message Boxes
    // ------------------------------------------------------------------------

    /// Show a message box
    virtual DialogResult messageBox(const std::wstring& message,
                                   const std::wstring& title,
                                   MessageBoxType type = MessageBoxType::OK,
                                   MessageBoxIcon icon = MessageBoxIcon::None,
                                   DialogResult defaultButton = DialogResult::OK) = 0;

    /// Show an information message
    virtual void showInfo(const std::wstring& message, const std::wstring& title = L"Information") = 0;

    /// Show a warning message
    virtual void showWarning(const std::wstring& message, const std::wstring& title = L"Warning") = 0;

    /// Show an error message
    virtual void showError(const std::wstring& message, const std::wstring& title = L"Error") = 0;

    /// Ask a yes/no question
    virtual bool askYesNo(const std::wstring& message, const std::wstring& title = L"Confirm") = 0;

    /// Ask a yes/no/cancel question
    virtual DialogResult askYesNoCancel(const std::wstring& message,
                                        const std::wstring& title = L"Confirm") = 0;

    /// Ask to retry/cancel
    virtual bool askRetryCancel(const std::wstring& message,
                               const std::wstring& title = L"Retry") = 0;

    // ------------------------------------------------------------------------
    // File Dialogs
    // ------------------------------------------------------------------------

    /// Show open file dialog
    virtual std::wstring showOpenFileDialog(const std::wstring& title,
                                           const std::vector<FileFilter>& filters,
                                           const FileDialogOptions& options = {}) = 0;

    /// Show open file dialog with multiple selection
    virtual std::vector<std::wstring> showOpenFilesDialog(const std::wstring& title,
                                                         const std::vector<FileFilter>& filters,
                                                         const FileDialogOptions& options = {}) = 0;

    /// Show save file dialog
    virtual std::wstring showSaveFileDialog(const std::wstring& title,
                                           const std::vector<FileFilter>& filters,
                                           const std::wstring& defaultFileName = L"",
                                           const FileDialogOptions& options = {}) = 0;

    // ------------------------------------------------------------------------
    // Folder Dialogs
    // ------------------------------------------------------------------------

    /// Show folder browser dialog
    virtual std::wstring showFolderDialog(const std::wstring& title,
                                         const FolderDialogOptions& options = {}) = 0;

    // ------------------------------------------------------------------------
    // Input Dialogs
    // ------------------------------------------------------------------------

    /// Show input dialog for single line text
    virtual bool showInputDialog(const std::wstring& title,
                                const std::wstring& prompt,
                                std::wstring& value,
                                bool isPassword = false) = 0;

    /// Show input dialog with options
    virtual bool showInputDialogEx(const InputDialogOptions& options,
                                  std::wstring& value) = 0;

    /// Show multi-line input dialog
    virtual bool showMultiLineInputDialog(const std::wstring& title,
                                         const std::wstring& prompt,
                                         std::wstring& value) = 0;

    /// Show a list selection dialog
    virtual int showListDialog(const std::wstring& title,
                              const std::wstring& prompt,
                              const std::vector<std::wstring>& items,
                              int defaultIndex = 0) = 0;

    // ------------------------------------------------------------------------
    // Custom Dialogs
    // ------------------------------------------------------------------------

    /// Show a custom dialog (platform-specific handle)
    virtual DialogResult showCustomDialog(void* dialogData) = 0;

    // ------------------------------------------------------------------------
    // Dialog Utilities
    // ------------------------------------------------------------------------

    /// Center a dialog on its parent or screen
    virtual void centerDialog(void* dialogHandle) = 0;

    /// Set dialog position
    virtual void setDialogPosition(void* dialogHandle, int x, int y) = 0;

    /// Get dialog position
    virtual void getDialogPosition(void* dialogHandle, int& x, int& y) = 0;

    /// Set dialog size
    virtual void setDialogSize(void* dialogHandle, int width, int height) = 0;

    /// Get dialog size
    virtual void getDialogSize(void* dialogHandle, int& width, int& height) = 0;

    /// Set dialog title
    virtual void setDialogTitle(void* dialogHandle, const std::wstring& title) = 0;

    /// Enable/disable dialog
    virtual void enableDialog(void* dialogHandle, bool enable) = 0;

    /// Check if dialog is enabled
    virtual bool isDialogEnabled(void* dialogHandle) = 0;

    /// Bring dialog to front
    virtual void bringToFront(void* dialogHandle) = 0;

    /// Set dialog as modal
    virtual void setModal(void* dialogHandle, bool modal) = 0;

private:
    static IDialogs* _testInstance;
};

// ============================================================================
// File Filter Helpers
// ============================================================================
namespace DialogFilters {

// Common filter sets
std::vector<FileFilter> allFiles();
std::vector<FileFilter> textFiles();
std::vector<FileFilter> sourceCodeFiles();
std::vector<FileFilter> xmlFiles();
std::vector<FileFilter> htmlFiles();
std::vector<FileFilter> imageFiles();

// Create a single filter
FileFilter filter(const std::wstring& description, const std::wstring& pattern);

// Combine filters
std::vector<FileFilter> combine(const std::vector<FileFilter>& a,
                                const std::vector<FileFilter>& b);

} // namespace DialogFilters

// ============================================================================
// Convenience Functions
// ============================================================================
namespace Dialogs {

// Quick message boxes
inline void info(const std::wstring& message, const std::wstring& title = L"Information") {
    IDialogs::getInstance().showInfo(message, title);
}

inline void warning(const std::wstring& message, const std::wstring& title = L"Warning") {
    IDialogs::getInstance().showWarning(message, title);
}

inline void error(const std::wstring& message, const std::wstring& title = L"Error") {
    IDialogs::getInstance().showError(message, title);
}

inline bool confirm(const std::wstring& message, const std::wstring& title = L"Confirm") {
    return IDialogs::getInstance().askYesNo(message, title);
}

// Quick file dialogs
inline std::wstring openFile(const std::wstring& title = L"Open File",
                            const std::vector<FileFilter>& filters = DialogFilters::allFiles()) {
    return IDialogs::getInstance().showOpenFileDialog(title, filters);
}

inline std::wstring saveFile(const std::wstring& title = L"Save File",
                            const std::vector<FileFilter>& filters = DialogFilters::allFiles(),
                            const std::wstring& defaultName = L"") {
    return IDialogs::getInstance().showSaveFileDialog(title, filters, defaultName);
}

inline std::wstring selectFolder(const std::wstring& title = L"Select Folder") {
    return IDialogs::getInstance().showFolderDialog(title);
}

// Quick input
inline bool input(const std::wstring& title,
                 const std::wstring& prompt,
                 std::wstring& value) {
    return IDialogs::getInstance().showInputDialog(title, prompt, value);
}

} // namespace Dialogs

} // namespace PlatformLayer
