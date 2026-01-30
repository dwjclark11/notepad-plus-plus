// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DebugInfoDlg.h"
#include "../../resource.h"
#include "../../Parameters.h"
#include "json.hpp"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QDialog>
#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QFont>
#include <QtGui/QScreen>
#include <QtCore/QDate>
#include <QtCore/QSysInfo>
#include <QtCore/QFileInfo>

// For library version info
#if defined __has_include
#if __has_include("NppLibsVersion.h")
#include "NppLibsVersion.h"
#endif
#endif

// For library version info
#ifdef NPP_LINUX
#include <sys/utsname.h>
#endif

// Define fallback values if NppLibsVersion.h is not available
#ifndef NPP_SCINTILLA_VERSION
#define NPP_SCINTILLA_VERSION "N/A"
#endif
#ifndef NPP_LEXILLA_VERSION
#define NPP_LEXILLA_VERSION "N/A"
#endif
#ifndef NPP_BOOST_REGEX_VERSION
#define NPP_BOOST_REGEX_VERSION "N/A"
#endif

namespace QtControls {

DebugInfoDlg::DebugInfoDlg(QWidget* parent) : StaticDialog(parent)
{
}

void DebugInfoDlg::init(QWidget* parent, bool isAdmin, const QString& loadedPlugins)
{
    _isAdmin = isAdmin;
    _loadedPlugins = loadedPlugins;
    StaticDialog::init(parent);
}

void DebugInfoDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    // Set dialog properties
    dialog->setMinimumSize(550, 450);
    dialog->resize(550, 450);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Title label
    _titleLabel = new QLabel(tr("Debug Information"), dialog);
    QFont titleFont = _titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);
    _titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(_titleLabel);

    // Separator line
    auto* line = new QFrame(dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // Debug info text edit (read-only, but selectable)
    _debugInfoEdit = new QTextEdit(dialog);
    _debugInfoEdit->setReadOnly(true);
    _debugInfoEdit->setLineWrapMode(QTextEdit::NoWrap);
    _debugInfoEdit->setFont(QFont("Monospace", 9));
    _debugInfoEdit->setPlaceholderText(tr("Debug information will appear here..."));
    mainLayout->addWidget(_debugInfoEdit);

    // Separator line
    auto* line2 = new QFrame(dialog);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line2);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _copyButton = new QPushButton(tr("Copy to Clipboard"), dialog);
    buttonLayout->addWidget(_copyButton);

    buttonLayout->addSpacing(10);

    _refreshButton = new QPushButton(tr("Refresh"), dialog);
    buttonLayout->addWidget(_refreshButton);

    buttonLayout->addSpacing(10);

    _closeButton = new QPushButton(tr("Close"), dialog);
    _closeButton->setDefault(true);
    buttonLayout->addWidget(_closeButton);

    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void DebugInfoDlg::connectSignals()
{
    QObject::connect(_copyButton, &QPushButton::clicked, [this]() { onCopyToClipboardClicked(); });
    QObject::connect(_refreshButton, &QPushButton::clicked, [this]() { onRefreshClicked(); });
    QObject::connect(_closeButton, &QPushButton::clicked, [this]() { onCloseClicked(); });
}

void DebugInfoDlg::doDialog()
{
    if (!isCreated()) {
        create(tr("Debug Info"), false);
        setupUI();
        connectSignals();
    }

    // Refresh the debug information
    // For example, command line parameters may have changed since last open
    refreshDebugInfo();

    // Center the dialog on parent
    goToCenter();
    display(true, true);
}

QString DebugInfoDlg::generateDebugInfo() const
{
    QString info;
    NppParameters& nppParam = NppParameters::getInstance();
    NppGUI& nppGui = nppParam.getNppGUI();

    // Notepad++ version
    info += QString::fromWCharArray(NOTEPAD_PLUS_VERSION);
    info += "   (64-bit)";
    info += "\n";

    // Build time
    info += "Build time: ";
    info += QString(__DATE__);
    info += " - ";
    info += QString(__TIME__);
    info += "\n";

    // Compiler info
    info += getCompilerInfo();
    info += "\n";

    // Scintilla/Lexilla version
    info += "Scintilla/Lexilla included: ";
    info += NPP_SCINTILLA_VERSION;
    info += "/";
    info += NPP_LEXILLA_VERSION;
    info += "\n";

    // Boost Regex version
    info += "Boost Regex included: ";
    info += NPP_BOOST_REGEX_VERSION;
    info += "\n";

    // pugixml version
#if defined(PUGIXML_VERSION)
    info += "pugixml included: ";
    info += QString::number(PUGIXML_VERSION / 1000);
    info += ".";
    info += QString::number((PUGIXML_VERSION % 1000) / 10);
    info += "\n";
#endif

    // JSON version
    info += "nlohmann JSON included: ";
    info += QString::number(NLOHMANN_JSON_VERSION_MAJOR);
    info += ".";
    info += QString::number(NLOHMANN_JSON_VERSION_MINOR);
    info += ".";
    info += QString::number(NLOHMANN_JSON_VERSION_PATCH);
    info += "\n";

    // Qt version (Linux-specific addition)
    info += getQtVersionInfo();
    info += "\n";

    // Binary path
    info += "Path: ";
    info += QApplication::applicationFilePath();
    info += "\n";

    // Command line placeholder (will be replaced in refreshDebugInfo)
    info += "Command Line: ";
    info += _cmdLinePlaceHolder;
    info += "\n";

    // Administrator mode
    info += "Admin mode: ";
    info += _isAdmin ? "ON" : "OFF";
    info += "\n";

    // Local conf
    info += "Local Conf mode: ";
    info += nppParam.isLocal() ? "ON" : "OFF";
    info += "\n";

    // Cloud config directory
    info += "Cloud Config: ";
    const std::wstring& cloudPath = nppGui._cloudPath;
    info += cloudPath.empty() ? "OFF" : QString::fromStdWString(cloudPath);
    info += "\n";

    // Periodic Backup
    info += "Periodic Backup: ";
    info += nppGui.isSnapshotMode() ? "ON" : "OFF";
    info += "\n";

    // Placeholders
    info += "Placeholders: ";
    info += nppGui._keepSessionAbsentFileEntries ? "ON" : "OFF";
    info += "\n";

    // SC_TECHNOLOGY
    info += "Scintilla Rendering Mode: ";
    switch (nppGui._writeTechnologyEngine)
    {
        case defaultTechnology:
            info += "SC_TECHNOLOGY_DEFAULT (0)";
            break;
        case directWriteTechnology:
            info += "SC_TECHNOLOGY_DIRECTWRITE (1)";
            break;
        case directWriteRetainTechnology:
            info += "SC_TECHNOLOGY_DIRECTWRITERETAIN (2)";
            break;
        case directWriteDcTechnology:
            info += "SC_TECHNOLOGY_DIRECTWRITEDC (3)";
            break;
        case directWriteDX11Technology:
            info += "SC_TECHNOLOGY_DIRECT_WRITE_1 (4)";
            break;
        case directWriteTechnologyUnavailable:
            info += "DirectWrite Technology Unavailable (5, same as SC_TECHNOLOGY_DEFAULT)";
            break;
        default:
            info += "unknown (";
            info += QString::number(nppGui._writeTechnologyEngine);
            info += ")";
    }
    info += "\n";

    // Multi-instance
    info += "Multi-instance Mode: ";
    switch (nppGui._multiInstSetting)
    {
        case monoInst:
            info += "monoInst";
            break;
        case multiInstOnSession:
            info += "multiInstOnSession";
            break;
        case multiInst:
            info += "multiInst";
            break;
        default:
            info += "unknown(";
            info += QString::number(nppGui._multiInstSetting);
            info += ")";
    }
    info += "\n";

    // asNotepad
    info += "asNotepad: ";
    info += nppParam.isAsNotepadStyle() ? "ON" : "OFF";
    info += "\n";

    // File Status Auto-Detection
    info += "File Status Auto-Detection: ";
    if (nppGui._fileAutoDetection == cdDisabled)
    {
        info += "cdDisabled";
    }
    else
    {
        if (nppGui._fileAutoDetection & cdEnabledOld)
            info += "cdEnabledOld (for all opened files/tabs)";
        else if (nppGui._fileAutoDetection & cdEnabledNew)
            info += "cdEnabledNew (for current file/tab only)";
        else
            info += "cdUnknown (?!)";

        if (nppGui._fileAutoDetection & cdAutoUpdate)
            info += " + cdAutoUpdate";
        if (nppGui._fileAutoDetection & cdGo2end)
            info += " + cdGo2end";
    }
    info += "\n";

    // Dark Mode
    info += "Dark Mode: ";
    info += nppGui._darkmode._isEnabled ? "ON" : "OFF";
    info += "\n";

    // Display Info
    info += "Display Info:\n";
    {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect geometry = screen->geometry();
            info += "    primary monitor: ";
            info += QString::number(geometry.width());
            info += "x";
            info += QString::number(geometry.height());
            info += ", scaling ";
            info += QString::number(qRound(screen->devicePixelRatio() * 100));
            info += "%";
        }
        info += "\n";
        info += "    visible monitors count: ";
        info += QString::number(QApplication::screens().size());
        info += "\n";
    }

    // OS information
    info += getOsInfo();
    info += "\n";

    // Plugins
    info += "Plugins: ";
    info += _loadedPlugins.isEmpty() ? "none" : _loadedPlugins;
    info += "\n";

    return info;
}

QString DebugInfoDlg::getCompilerInfo() const
{
    QString info = "Built with: ";

#if defined(__clang__)
    info += "Clang ";
    info += __clang_version__;
#elif defined(__GNUC__)
    info += "GCC ";
    info += __VERSION__;
#elif defined(_MSC_VER)
    info += "MSVC ";
    info += QString::number(_MSC_VER);
#else
    info += "(unknown)";
#endif

    return info;
}

QString DebugInfoDlg::getBuildTimeString() const
{
    QString buildTime = tr("Build time: ");
    buildTime += QString(__DATE__);
    buildTime += " - ";
    buildTime += QString(__TIME__);
    return buildTime;
}

QString DebugInfoDlg::getOsInfo() const
{
    QString info;

    // OS Name
    info += "OS Name: ";
    info += QSysInfo::prettyProductName();
    info += " (";
    info += QSysInfo::currentCpuArchitecture();
    info += ")";
    info += "\n";

    // OS Version
    info += "OS Version: ";
    info += QSysInfo::productVersion();
    info += "\n";

    // Kernel version
    info += "OS Build: ";
    info += QSysInfo::kernelVersion();
    info += "\n";

    // Current ANSI codepage (Linux uses UTF-8 typically)
    info += "Current ANSI codepage: ";
    info += "65001 (UTF-8)";

    return info;
}

QString DebugInfoDlg::getQtVersionInfo() const
{
    QString info = "Qt included: ";
    info += QT_VERSION_STR;
    return info;
}

void DebugInfoDlg::refreshDebugInfo()
{
    // Generate the base debug info if not already generated
    if (_debugInfoStr.isEmpty()) {
        _debugInfoStr = generateDebugInfo();
    }

    // Create display version with placeholder replaced
    _debugInfoDisplay = _debugInfoStr;
    _debugInfoDisplay.replace(_cmdLinePlaceHolder, QString::fromStdWString(NppParameters::getInstance().getCmdLineString()));

    // Update the text edit
    if (_debugInfoEdit) {
        _debugInfoEdit->setPlainText(_debugInfoDisplay);

        // Select all text (matching Windows behavior)
        _debugInfoEdit->selectAll();

        // Set focus to the edit control
        _debugInfoEdit->setFocus();
    }
}

void DebugInfoDlg::onCopyToClipboardClicked()
{
    if (!_debugInfoEdit) return;

    // Select all text for visual effect
    _debugInfoEdit->selectAll();

    // Copy to clipboard
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard) {
        clipboard->setText(_debugInfoDisplay);
    }

    // Keep focus on edit control
    _debugInfoEdit->setFocus();
}

void DebugInfoDlg::onRefreshClicked()
{
    // Regenerate debug info from scratch
    _debugInfoStr = generateDebugInfo();
    refreshDebugInfo();
}

void DebugInfoDlg::onCloseClicked()
{
    display(false);
}

} // namespace QtControls
