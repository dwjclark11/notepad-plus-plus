// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include <QString>
#include <QWidget>

// Forward declarations
class QTextEdit;
class QPushButton;
class QLabel;

namespace QtControls {

// ============================================================================
// DebugInfoDlg - Qt implementation
// ============================================================================
class DebugInfoDlg : public StaticDialog {
    Q_OBJECT

public:
    DebugInfoDlg(QWidget* parent = nullptr);
    ~DebugInfoDlg() override = default;

    // Initialize the dialog with admin status and loaded plugins
    // Adapted from Windows: void init(HINSTANCE hInst, HWND parent, bool isAdmin, const std::wstring& loadedPlugins)
    void init(QWidget* parent, bool isAdmin, const QString& loadedPlugins);

    // Show the dialog
    void doDialog();

    // Regenerate debug info (e.g., when command line parameters change)
    void refreshDebugInfo();

    // Cleanup override
    void destroy() override {}

protected:
    void setupUI();
    void connectSignals();

private slots:
    void onCopyToClipboardClicked();
    void onRefreshClicked();
    void onCloseClicked();

private:
    // UI Components
    QTextEdit* _debugInfoEdit = nullptr;
    QPushButton* _copyButton = nullptr;
    QPushButton* _refreshButton = nullptr;
    QPushButton* _closeButton = nullptr;
    QLabel* _titleLabel = nullptr;

    // Member variables (adapted from Windows version)
    bool _isAdmin = false;
    QString _loadedPlugins;
    QString _debugInfoStr;      // Raw debug info with placeholders
    QString _debugInfoDisplay;  // Debug info with placeholders replaced

    // Placeholder for command line (will be replaced in refreshDebugInfo)
    const QString _cmdLinePlaceHolder{ "$COMMAND_LINE_PLACEHOLDER$" };

    // Helper methods
    QString generateDebugInfo() const;
    QString getCompilerInfo() const;
    QString getBuildTimeString() const;
    QString getOsInfo() const;
    QString getQtVersionInfo() const;
};

} // namespace QtControls
