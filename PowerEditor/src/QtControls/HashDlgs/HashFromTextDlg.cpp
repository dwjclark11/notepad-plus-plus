// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "HashFromTextDlg.h"
#include "MISC/md5/md5Dlgs.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtCore/QCryptographicHash>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>

namespace QtControls {

HashFromTextDlg::HashFromTextDlg(QWidget* parent)
    : StaticDialog(parent)
{
}

HashFromTextDlg::~HashFromTextDlg() = default;

void HashFromTextDlg::doDialog(bool isRTL)
{
    Q_UNUSED(isRTL);

    if (!isCreated()) {
        create(getHashAlgorithmName() + tr(" digest"), false);
        setupUI();
        connectSignals();
    }

    // Update window title based on hash type
    QDialog* dialog = getDialog();
    if (dialog) {
        switch (_ht) {
            case HashType::Md5:
                dialog->setWindowTitle(tr("Generate MD5 digest"));
                break;
            case HashType::Sha1:
                dialog->setWindowTitle(tr("Generate SHA-1 digest"));
                break;
            case HashType::Sha256:
                dialog->setWindowTitle(tr("Generate SHA-256 digest"));
                break;
            case HashType::Sha512:
                dialog->setWindowTitle(tr("Generate SHA-512 digest"));
                break;
            default:
                break;
        }
    }

    display(true, true);
    goToCenter();

    // Focus the text input
    if (_textEdit) {
        _textEdit->setFocus();
    }
}

void HashFromTextDlg::destroy()
{
    StaticDialog::destroy();
}

void HashFromTextDlg::setHashType(HashType hashType2set)
{
    _ht = hashType2set;
}

void HashFromTextDlg::setHashType(int hashType2set)
{
    switch (hashType2set) {
        case 16: // hash_md5
            _ht = HashType::Md5;
            break;
        case 20: // hash_sha1
            _ht = HashType::Sha1;
            break;
        case 32: // hash_sha256
            _ht = HashType::Sha256;
            break;
        case 64: // hash_sha512
            _ht = HashType::Sha512;
            break;
        default:
            _ht = HashType::Md5;
            break;
    }
}

void HashFromTextDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->resize(500, 450);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Text input section
    _textLabel = new QLabel(tr("Text to hash:"), dialog);
    mainLayout->addWidget(_textLabel);

    _textEdit = new QTextEdit(dialog);
    _textEdit->setFont(QFont("Courier New", 9));
    _textEdit->setPlaceholderText(tr("Enter text here..."));
    mainLayout->addWidget(_textEdit, 2);

    // Each line checkbox
    _eachLineCheck = new QCheckBox(tr("Treat each line as a separate string"), dialog);
    mainLayout->addWidget(_eachLineCheck);

    // Separator
    mainLayout->addSpacing(10);

    // Result section
    _resultLabel = new QLabel(tr("Hash result:"), dialog);
    mainLayout->addWidget(_resultLabel);

    _resultEdit = new QTextEdit(dialog);
    _resultEdit->setReadOnly(true);
    _resultEdit->setFont(QFont("Courier New", 9));
    _resultEdit->setPlaceholderText(tr("Hash result will appear here"));
    mainLayout->addWidget(_resultEdit, 1);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _copyButton = new QPushButton(tr("&Copy to clipboard"), dialog);
    _copyButton->setEnabled(false);
    buttonLayout->addWidget(_copyButton);

    buttonLayout->addSpacing(20);

    _closeButton = new QPushButton(tr("Close"), dialog);
    buttonLayout->addWidget(_closeButton);

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void HashFromTextDlg::connectSignals()
{
    connect(_textEdit, &QTextEdit::textChanged, this, &HashFromTextDlg::onTextChanged);
    connect(_eachLineCheck, &QCheckBox::toggled, this, &HashFromTextDlg::onEachLineCheckChanged);
    connect(_copyButton, &QPushButton::clicked, this, &HashFromTextDlg::onCopyToClipboardClicked);
    connect(_closeButton, &QPushButton::clicked, this, &HashFromTextDlg::onCloseClicked);
}

void HashFromTextDlg::generateHash()
{
    QString text = _textEdit->toPlainText();

    if (text.isEmpty()) {
        _resultEdit->clear();
        _copyButton->setEnabled(false);
        return;
    }

    QString hashValue = calculateTextHash(text);
    _resultEdit->setPlainText(hashValue);
    _copyButton->setEnabled(!hashValue.isEmpty());
}

void HashFromTextDlg::generateHashPerLine()
{
    QString text = _textEdit->toPlainText();

    if (text.isEmpty()) {
        _resultEdit->clear();
        _copyButton->setEnabled(false);
        return;
    }

    QStringList lines = text.split('\n');
    QStringList results;

    for (QString line : lines) {
        // Remove trailing carriage return if present (Windows EOL)
        if (line.endsWith('\r')) {
            line.chop(1);
        }

        if (line.isEmpty()) {
            results.append(QString());
        }
        else {
            QString hashValue = calculateTextHash(line);
            results.append(hashValue);
        }
    }

    _resultEdit->setPlainText(results.join("\n"));
    _copyButton->setEnabled(true);
}

void HashFromTextDlg::onTextChanged()
{
    if (_eachLineCheck->isChecked()) {
        generateHashPerLine();
    }
    else {
        generateHash();
    }
}

void HashFromTextDlg::onEachLineCheckChanged()
{
    if (_eachLineCheck->isChecked()) {
        generateHashPerLine();
    }
    else {
        generateHash();
    }
}

void HashFromTextDlg::onCopyToClipboardClicked()
{
    QString resultText = _resultEdit->toPlainText();
    if (!resultText.isEmpty()) {
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(resultText);
    }
}

void HashFromTextDlg::onCloseClicked()
{
    display(false);
}

QString HashFromTextDlg::getHashAlgorithmName() const
{
    switch (_ht) {
        case HashType::Md5:
            return tr("MD5");
        case HashType::Sha1:
            return tr("SHA-1");
        case HashType::Sha256:
            return tr("SHA-256");
        case HashType::Sha512:
            return tr("SHA-512");
        default:
            return tr("Hash");
    }
}

QString HashFromTextDlg::calculateTextHash(const QString& text) const
{
    // Convert text to UTF-8 (matching Windows behavior for Unicode support)
    QByteArray utf8Data = text.toUtf8();
    QByteArray hashData = this->hashData(utf8Data);

    if (hashData.isEmpty()) {
        return QString();
    }

    return bytesToHex(hashData);
}

QByteArray HashFromTextDlg::hashData(const QByteArray& data) const
{
    QCryptographicHash::Algorithm algorithm;

    switch (_ht) {
        case HashType::Md5:
            algorithm = QCryptographicHash::Md5;
            break;
        case HashType::Sha1:
            algorithm = QCryptographicHash::Sha1;
            break;
        case HashType::Sha256:
            algorithm = QCryptographicHash::Sha256;
            break;
        case HashType::Sha512:
            algorithm = QCryptographicHash::Sha512;
            break;
        default:
            return QByteArray();
    }

    QCryptographicHash hash(algorithm);
    hash.addData(data);
    return hash.result();
}

QString HashFromTextDlg::bytesToHex(const QByteArray& data) const
{
    return QString(data.toHex());
}

bool HashFromTextDlg::run_dlgProc(QEvent* /*event*/)
{
    return false;
}

} // namespace QtControls
