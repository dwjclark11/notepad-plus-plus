// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "CmdLineArgsDlg.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialog>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

namespace QtControls {

// Command line help text (matching Windows version)
const char COMMAND_ARG_HELP[] =
"Usage:\n\n"
"notepad++ [--help] [-multiInst] [-noPlugin] [-lLanguage] [-udl=\"My UDL Name\"]\n"
"[-LlangCode] [-nLineNumber] [-cColumnNumber] [-pPosition] [-xLeftPos] [-yTopPos]\n"
"[-monitor] [-nosession] [-notabbar] [-systemtray] [-loadingTime] [-alwaysOnTop]\n"
"[-ro] [-fullReadOnly] [-fullReadOnlySavingForbidden] [-openSession] [-r]\n"
"[-qn=\"Easter egg name\" | -qt=\"a text to display.\" | -qf=\"D:\\my quote.txt\"]\n"
"[-qSpeed1|2|3] [-quickPrint] [-settingsDir=\"d:\\your settings dir\\\"]\n"
"[-openFoldersAsWorkspace]  [-titleAdd=\"additional title bar text\"]\n"
"[filePath]\n\n"
"--help: This help message\n"
"-multiInst: Launch another Notepad++ instance\n"
"-noPlugin: Launch Notepad++ without loading any plugin\n"
"-l: Open file or Ghost type with syntax highlighting of choice\n"
"-udl=\"My UDL Name\": Open file by applying User Defined Language\n"
"-L: Apply indicated localization, langCode is browser language code\n"
"-n: Scroll to indicated line on filePath\n"
"-c: Scroll to indicated column on filePath\n"
"-p: Scroll to indicated position on filePath\n"
"-x: Move Notepad++ to indicated left side position on the screen\n"
"-y: Move Notepad++ to indicated top position on the screen\n"
"-monitor: Open file with file monitoring enabled\n"
"-nosession: Launch Notepad++ without previous session\n"
"-notabbar: Launch Notepad++ without tab bar\n"
"-ro: Make the filePath read-only\n"
"-fullReadOnly: Open all files read-only by default, toggling the R/O off and saving is allowed\n"
"-fullReadOnlySavingForbidden: Open all files read-only by default, toggling the R/O off and saving is disabled\n"
"-systemtray: Launch Notepad++ directly in system tray\n"
"-loadingTime: Display Notepad++ loading time\n"
"-alwaysOnTop: Make Notepad++ always on top\n"
"-openSession: Open a session. filePath must be a session file\n"
"-r: Open files recursively. This argument will be ignored if filePath contains no wildcard character\n"
"-qn=\"Easter egg name\": Ghost type easter egg via its name\n"
"-qt=\"text to display.\": Ghost type the given text\n"
"-qf=\"D:\\my quote.txt\": Ghost type a file content via the file path\n"
"-qSpeed: Ghost typing speed. Value from 1 to 3 for slow, fast and fastest\n"
"-quickPrint: Print the file given as argument then quit Notepad++\n"
"-settingsDir=\"d:\\your settings dir\\\": Override the default settings dir\n"
"-openFoldersAsWorkspace: open filePath of folder(s) as workspace\n"
"-titleAdd=\"string\": add string to Notepad++ title bar\n"
"filePath: file or folder name to open (absolute or relative path name)";

CmdLineArgsDlg::CmdLineArgsDlg(QWidget* parent)
	: StaticDialog(parent)
{
}

CmdLineArgsDlg::~CmdLineArgsDlg()
{
	delete _monospaceFont;
}

void CmdLineArgsDlg::setupUI()
{
	QDialog* dialog = getDialog();
	if (!dialog) return;

	// Set dialog size
	dialog->resize(550, 450);

	auto* mainLayout = new QVBoxLayout(dialog);
	mainLayout->setSpacing(10);
	mainLayout->setContentsMargins(12, 12, 12, 12);

	// Create read-only text edit for command line arguments
	_textEdit = new QTextEdit(dialog);
	_textEdit->setReadOnly(true);
	_textEdit->setPlainText(QString::fromUtf8(COMMAND_ARG_HELP));

	// Set monospace font for better readability
	_monospaceFont = new QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	_monospaceFont->setPointSize(9);
	_textEdit->setFont(*_monospaceFont);

	mainLayout->addWidget(_textEdit);

	// Button layout
	auto* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();

	_okButton = new QPushButton(tr("OK"), dialog);
	_okButton->setDefault(true);
	buttonLayout->addWidget(_okButton);

	buttonLayout->addStretch();

	mainLayout->addLayout(buttonLayout);

	// Store initial rect
	_rc = dialog->geometry();
}

void CmdLineArgsDlg::connectSignals()
{
	QObject::connect(_okButton, &QPushButton::clicked, [this]() { onOkClicked(); });
}

void CmdLineArgsDlg::doDialog()
{
	if (!isCreated()) {
		create(tr("Command Line Arguments"), false);
		setupUI();
		connectSignals();
	}

	// Center the dialog on parent
	goToCenter();
	display(true, true);
}

void CmdLineArgsDlg::destroy()
{
	// Cleanup is handled in destructor
	StaticDialog::destroy();
}

void CmdLineArgsDlg::onOkClicked()
{
	display(false);
}

} // namespace QtControls
