// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "HashType.h"

// Forward declarations
class QLineEdit;
class QTextEdit;
class QPushButton;
class QComboBox;
class QLabel;

namespace QtControls {

// ============================================================================
// HashFromFilesDlg - Qt implementation of hash from files dialog
// ============================================================================
class HashFromFilesDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit HashFromFilesDlg(QWidget* parent = nullptr);
    ~HashFromFilesDlg() override;

    // Show the dialog
    void doDialog(bool isRTL = false);

    // Destroy the dialog
    void destroy() override;

    // Set the hash type
    void setHashType(HashType hashType2set);

    // Set the hash type (compatibility overload - defined in cpp)
    void setHashType(int hashType2set);

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;

private slots:
    void onBrowseClicked();
    void onCopyToClipboardClicked();
    void onCloseClicked();

private:
    // UI Components
    QLabel* _filePathLabel = nullptr;
    QTextEdit* _filePathEdit = nullptr;
    QPushButton* _browseButton = nullptr;
    QLabel* _resultLabel = nullptr;
    QTextEdit* _resultEdit = nullptr;
    QPushButton* _copyButton = nullptr;
    QPushButton* _closeButton = nullptr;

    // State
    HashType _ht = HashType::Md5;
    QStringList _selectedFiles;

    // Helper methods
    QString getHashAlgorithmName() const;
    QString calculateFileHash(const QString& filePath) const;
    QByteArray hashData(const QByteArray& data) const;
};

} // namespace QtControls
