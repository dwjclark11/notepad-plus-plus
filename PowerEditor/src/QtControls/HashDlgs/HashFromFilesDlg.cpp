// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "HashFromFilesDlg.h"
#include "MISC/md5/md5Dlgs.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDialog>
#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>

namespace QtControls {

HashFromFilesDlg::HashFromFilesDlg(QWidget* parent)
    : StaticDialog(parent)
{
}

HashFromFilesDlg::~HashFromFilesDlg() = default;

void HashFromFilesDlg::doDialog(bool isRTL)
{
    Q_UNUSED(isRTL);

    if (!isCreated()) {
        create(getHashAlgorithmName() + tr(" digest from files"), false);
        setupUI();
        connectSignals();
    }

    // Update window title and button text based on hash type
    QDialog* dialog = getDialog();
    if (dialog) {
        switch (_ht) {
            case HashType::Md5:
                dialog->setWindowTitle(tr("Generate MD5 digest from files"));
                _browseButton->setText(tr("Choose files to &generate MD5..."));
                break;
            case HashType::Sha1:
                dialog->setWindowTitle(tr("Generate SHA-1 digest from files"));
                _browseButton->setText(tr("Choose files to &generate SHA-1..."));
                break;
            case HashType::Sha256:
                dialog->setWindowTitle(tr("Generate SHA-256 digest from files"));
                _browseButton->setText(tr("Choose files to &generate SHA-256..."));
                break;
            case HashType::Sha512:
                dialog->setWindowTitle(tr("Generate SHA-512 digest from files"));
                _browseButton->setText(tr("Choose files to &generate SHA-512..."));
                break;
            default:
                break;
        }
    }

    display(true, true);
    goToCenter();
}

void HashFromFilesDlg::destroy()
{
    StaticDialog::destroy();
}

void HashFromFilesDlg::setHashType(HashType hashType2set)
{
    _ht = hashType2set;
}

void HashFromFilesDlg::setHashType(int hashType2set)
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

void HashFromFilesDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->resize(550, 400);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // File path section
    _filePathLabel = new QLabel(tr("Selected files:"), dialog);
    mainLayout->addWidget(_filePathLabel);

    _filePathEdit = new QTextEdit(dialog);
    _filePathEdit->setReadOnly(true);
    _filePathEdit->setFont(QFont("Courier New", 9));
    _filePathEdit->setPlaceholderText(tr("No files selected"));
    mainLayout->addWidget(_filePathEdit, 1);

    // Browse button
    auto* browseLayout = new QHBoxLayout();
    browseLayout->addStretch();
    _browseButton = new QPushButton(tr("Choose files to &generate MD5..."), dialog);
    browseLayout->addWidget(_browseButton);
    mainLayout->addLayout(browseLayout);

    // Separator
    mainLayout->addSpacing(10);

    // Result section
    _resultLabel = new QLabel(tr("Hash results:"), dialog);
    mainLayout->addWidget(_resultLabel);

    _resultEdit = new QTextEdit(dialog);
    _resultEdit->setReadOnly(true);
    _resultEdit->setFont(QFont("Courier New", 9));
    _resultEdit->setPlaceholderText(tr("Hash results will appear here"));
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

void HashFromFilesDlg::connectSignals()
{
    connect(_browseButton, &QPushButton::clicked, this, &HashFromFilesDlg::onBrowseClicked);
    connect(_copyButton, &QPushButton::clicked, this, &HashFromFilesDlg::onCopyToClipboardClicked);
    connect(_closeButton, &QPushButton::clicked, this, &HashFromFilesDlg::onCloseClicked);
}

void HashFromFilesDlg::onBrowseClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        getDialog(),
        tr("Select Files"),
        QString(),
        tr("All Files (*)")
    );

    if (fileNames.isEmpty()) {
        return;
    }

    _selectedFiles = fileNames;

    // Build file list and hash results
    QString fileList;
    QString hashResults;

    for (const QString& filePath : fileNames) {
        QString hashValue = calculateFileHash(filePath);

        if (!hashValue.isEmpty()) {
            fileList += filePath + "\n";

            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            hashResults += hashValue + "  " + fileName + "\n";
        }
    }

    // Remove trailing newlines
    if (!fileList.isEmpty()) {
        fileList.chop(1);
    }
    if (!hashResults.isEmpty()) {
        hashResults.chop(1);
    }

    _filePathEdit->setPlainText(fileList);
    _resultEdit->setPlainText(hashResults);
    _copyButton->setEnabled(!hashResults.isEmpty());
}

void HashFromFilesDlg::onCopyToClipboardClicked()
{
    QString resultText = _resultEdit->toPlainText();
    if (!resultText.isEmpty()) {
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(resultText);
    }
}

void HashFromFilesDlg::onCloseClicked()
{
    display(false);
}

QString HashFromFilesDlg::getHashAlgorithmName() const
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

QString HashFromFilesDlg::calculateFileHash(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QByteArray fileData = file.readAll();
    file.close();

    QByteArray hashData = this->hashData(fileData);
    if (hashData.isEmpty()) {
        return QString();
    }

    return QString(hashData.toHex());
}

QByteArray HashFromFilesDlg::hashData(const QByteArray& data) const
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

bool HashFromFilesDlg::run_dlgProc(QEvent* /*event*/)
{
    return false;
}

} // namespace QtControls
