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
class QTextEdit;
class QPushButton;
class QLabel;
class QCheckBox;

namespace QtControls {

// ============================================================================
// HashFromTextDlg - Qt implementation of hash from text dialog
// ============================================================================
class HashFromTextDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit HashFromTextDlg(QWidget* parent = nullptr);
    ~HashFromTextDlg() override;

    // Show the dialog
    void doDialog(bool isRTL = false);

    // Destroy the dialog
    void destroy() override;

    // Generate hash for entire text
    void generateHash();

    // Generate hash for each line
    void generateHashPerLine();

    // Set the hash type
    void setHashType(HashType hashType2set);

    // Set the hash type (compatibility overload - defined in cpp)
    void setHashType(int hashType2set);

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;

private slots:
    void onTextChanged();
    void onEachLineCheckChanged();
    void onCopyToClipboardClicked();
    void onCloseClicked();

private:
    // UI Components
    QLabel* _textLabel = nullptr;
    QTextEdit* _textEdit = nullptr;
    QCheckBox* _eachLineCheck = nullptr;
    QLabel* _resultLabel = nullptr;
    QTextEdit* _resultEdit = nullptr;
    QPushButton* _copyButton = nullptr;
    QPushButton* _closeButton = nullptr;

    // State
    HashType _ht = HashType::Md5;

    // Helper methods
    QString getHashAlgorithmName() const;
    QString calculateTextHash(const QString& text) const;
    QByteArray hashData(const QByteArray& data) const;
    QString bytesToHex(const QByteArray& data) const;
};

} // namespace QtControls
