// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "ScintillaEditView.h"

// Forward declarations
class QSpinBox;
class QRadioButton;
class QPushButton;
class QCheckBox;
class QLabel;

namespace QtControls {

// ============================================================================
// FindCharsInRangeDlg - Qt implementation of Find Characters in Range dialog
// ============================================================================
class FindCharsInRangeDlg : public StaticDialog {
	Q_OBJECT

public:
	FindCharsInRangeDlg(QWidget* parent = nullptr);
	~FindCharsInRangeDlg() override;

	// Initialize the dialog (matches Windows interface)
	void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView);

	// Show the dialog (matches Windows interface)
	void doDialog(bool isRTL = false);

	// Display the dialog (Qt-specific interface)
	void display(bool toShow = true, bool enhancedPositioningCheckWhenShowing = false);

protected:
	void setupUI() override;
	void connectSignals() override;
	bool run_dlgProc(QEvent* event) override;

private slots:
	void onFindClicked();
	void onCloseClicked();
	void onRangeModeChanged();
	void updateRangeControls();

private:
	// UI Components
	QRadioButton* _nonAsciiRadio = nullptr;
	QRadioButton* _asciiRadio = nullptr;
	QRadioButton* _myRangeRadio = nullptr;
	QSpinBox* _rangeStartEdit = nullptr;
	QSpinBox* _rangeEndEdit = nullptr;
	QLabel* _rangeStartLabel = nullptr;
	QLabel* _rangeEndLabel = nullptr;
	QRadioButton* _dirUpRadio = nullptr;
	QRadioButton* _dirDownRadio = nullptr;
	QCheckBox* _wrapCheck = nullptr;
	QPushButton* _findButton = nullptr;
	QPushButton* _closeButton = nullptr;

	// Store the edit view pointer (matches Windows interface)
	ScintillaEditView **_ppEditView = nullptr;

	// Direction constant
	static constexpr bool dirDown = true;
	static constexpr bool dirUp = false;

	// Helper methods (matching Windows implementation)
	bool findCharInRange(unsigned char beginRange, unsigned char endRange, intptr_t startPos, bool direction, bool wrap);
	bool getRangeFromUI(unsigned char& startRange, unsigned char& endRange);
	void getDirectionFromUI(bool& whichDirection, bool& isWrap);
};

} // namespace QtControls
