// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"

// Forward declarations
class QTextEdit;
class QPushButton;
class QFont;

namespace QtControls {

// ============================================================================
// CmdLineArgsDlg - Qt implementation of Command Line Arguments dialog
// ============================================================================
class CmdLineArgsDlg : public StaticDialog {
	Q_OBJECT

public:
	CmdLineArgsDlg(QWidget* parent = nullptr);
	~CmdLineArgsDlg() override;

	// Show the dialog
	void doDialog();

	// Cleanup override
	void destroy() override;

protected:
	void setupUI();
	void connectSignals();

private slots:
	void onOkClicked();

private:
	// UI Components
	QTextEdit* _textEdit = nullptr;
	QPushButton* _okButton = nullptr;

	// Monospace font for displaying command line arguments
	QFont* _monospaceFont = nullptr;
};

} // namespace QtControls
