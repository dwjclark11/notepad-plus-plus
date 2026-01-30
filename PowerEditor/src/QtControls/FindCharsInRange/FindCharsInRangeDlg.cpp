// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FindCharsInRangeDlg.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QMessageBox>

namespace QtControls {

FindCharsInRangeDlg::FindCharsInRangeDlg(QWidget* parent)
	: StaticDialog(parent)
{
}

FindCharsInRangeDlg::~FindCharsInRangeDlg() = default;

void FindCharsInRangeDlg::init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView)
{
	Q_UNUSED(hInst);
	Q_UNUSED(hPere);

	// Store the edit view pointer for later use
	if (!ppEditView)
		throw std::runtime_error("FindCharsInRangeDlg::init : ppEditView is null.");
	_ppEditView = ppEditView;
}

void FindCharsInRangeDlg::setupUI()
{
	QDialog* dialog = getDialog();
	if (!dialog) return;

	dialog->setWindowTitle(tr("Find Characters in Range"));
	dialog->resize(350, 280);

	auto* mainLayout = new QVBoxLayout(dialog);
	mainLayout->setSpacing(10);
	mainLayout->setContentsMargins(12, 12, 12, 12);

	// Range selection group
	auto* rangeGroup = new QGroupBox(tr("Range"), dialog);
	auto* rangeLayout = new QVBoxLayout(rangeGroup);

	// Non-ASCII radio (128-255)
	_nonAsciiRadio = new QRadioButton(tr("Non-ASCII Characters (128-255)"), dialog);
	_nonAsciiRadio->setChecked(true);
	rangeLayout->addWidget(_nonAsciiRadio);

	// ASCII radio (0-127)
	_asciiRadio = new QRadioButton(tr("ASCII Characters (0-127)"), dialog);
	rangeLayout->addWidget(_asciiRadio);

	// My Range radio with input fields
	_myRangeRadio = new QRadioButton(tr("My Range:"), dialog);
	rangeLayout->addWidget(_myRangeRadio);

	// Range input fields
	auto* rangeInputLayout = new QHBoxLayout();
	rangeInputLayout->addSpacing(20);

	_rangeStartLabel = new QLabel(tr("From:"), dialog);
	rangeInputLayout->addWidget(_rangeStartLabel);

	_rangeStartEdit = new QSpinBox(dialog);
	_rangeStartEdit->setMinimum(0);
	_rangeStartEdit->setMaximum(255);
	_rangeStartEdit->setValue(0);
	_rangeStartEdit->setMaximumWidth(60);
	_rangeStartEdit->setEnabled(false);
	rangeInputLayout->addWidget(_rangeStartEdit);

	_rangeEndLabel = new QLabel(tr("To:"), dialog);
	rangeInputLayout->addWidget(_rangeEndLabel);

	_rangeEndEdit = new QSpinBox(dialog);
	_rangeEndEdit->setMinimum(0);
	_rangeEndEdit->setMaximum(255);
	_rangeEndEdit->setValue(255);
	_rangeEndEdit->setMaximumWidth(60);
	_rangeEndEdit->setEnabled(false);
	rangeInputLayout->addWidget(_rangeEndEdit);

	rangeInputLayout->addStretch();
	rangeLayout->addLayout(rangeInputLayout);

	mainLayout->addWidget(rangeGroup);

	// Direction group
	auto* dirGroup = new QGroupBox(tr("Direction"), dialog);
	auto* dirLayout = new QHBoxLayout(dirGroup);

	_dirUpRadio = new QRadioButton(tr("&Up"), dialog);
	dirLayout->addWidget(_dirUpRadio);

	_dirDownRadio = new QRadioButton(tr("&Down"), dialog);
	_dirDownRadio->setChecked(true);
	dirLayout->addWidget(_dirDownRadio);

	dirLayout->addStretch();
	mainLayout->addWidget(dirGroup);

	// Wrap checkbox
	_wrapCheck = new QCheckBox(tr("&Wrap around"), dialog);
	_wrapCheck->setChecked(true);
	mainLayout->addWidget(_wrapCheck);

	// Add stretch to push buttons to bottom
	mainLayout->addStretch();

	// Button layout
	auto* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();

	_findButton = new QPushButton(tr("&Find"), dialog);
	_findButton->setDefault(true);
	buttonLayout->addWidget(_findButton);

	_closeButton = new QPushButton(tr("Close"), dialog);
	buttonLayout->addWidget(_closeButton);

	mainLayout->addLayout(buttonLayout);

	// Store initial rect
	_rc = dialog->geometry();
}

void FindCharsInRangeDlg::connectSignals()
{
	// Button clicks
	connect(_findButton, &QPushButton::clicked, this, &FindCharsInRangeDlg::onFindClicked);
	connect(_closeButton, &QPushButton::clicked, this, &FindCharsInRangeDlg::onCloseClicked);

	// Range mode changes
	connect(_nonAsciiRadio, &QRadioButton::toggled, this, &FindCharsInRangeDlg::onRangeModeChanged);
	connect(_asciiRadio, &QRadioButton::toggled, this, &FindCharsInRangeDlg::onRangeModeChanged);
	connect(_myRangeRadio, &QRadioButton::toggled, this, &FindCharsInRangeDlg::onRangeModeChanged);
}

void FindCharsInRangeDlg::doDialog(bool isRTL)
{
	Q_UNUSED(isRTL);

	if (!isCreated()) {
		create(tr("Find Characters in Range"), false);
		setupUI();
		connectSignals();
	}

	display(true);
	goToCenter();
}

void FindCharsInRangeDlg::display(bool toShow, bool enhancedPositioningCheckWhenShowing)
{
	StaticDialog::display(toShow, enhancedPositioningCheckWhenShowing);
}

void FindCharsInRangeDlg::onFindClicked()
{
	if (!_ppEditView || !*_ppEditView) return;

	intptr_t currentPos = (*_ppEditView)->execute(SCI_GETCURRENTPOS);
	unsigned char startRange = 0;
	unsigned char endRange = 255;
	bool direction = dirDown;
	bool isWrap = true;

	if (!getRangeFromUI(startRange, endRange))
	{
		QMessageBox::warning(getDialog(), tr("Range Value Problem"),
			tr("You should type between 0 and 255."));
		return;
	}

	getDirectionFromUI(direction, isWrap);
	findCharInRange(startRange, endRange, currentPos, direction, isWrap);
}

void FindCharsInRangeDlg::onCloseClicked()
{
	display(false);
}

void FindCharsInRangeDlg::onRangeModeChanged()
{
	updateRangeControls();
}

void FindCharsInRangeDlg::updateRangeControls()
{
	bool isMyRange = _myRangeRadio && _myRangeRadio->isChecked();
	if (_rangeStartEdit) _rangeStartEdit->setEnabled(isMyRange);
	if (_rangeEndEdit) _rangeEndEdit->setEnabled(isMyRange);

	// Update label appearance to show enabled/disabled state
	if (_rangeStartLabel) {
		_rangeStartLabel->setEnabled(isMyRange);
	}
	if (_rangeEndLabel) {
		_rangeEndLabel->setEnabled(isMyRange);
	}
}

bool FindCharsInRangeDlg::findCharInRange(unsigned char beginRange, unsigned char endRange, intptr_t startPos, bool direction, bool wrap)
{
	if (!_ppEditView || !*_ppEditView) return false;

	ScintillaEditView* view = *_ppEditView;
	size_t totalSize = view->getCurrentDocLen();

	if (startPos == -1)
		startPos = direction == dirDown ? 0 : static_cast<intptr_t>(totalSize) - 1;
	if (startPos > static_cast<intptr_t>(totalSize))
		return false;

	char *content = new char[totalSize + 1];
	view->getText(content, 0, totalSize);

	bool isFound = false;
	size_t found = 0;

	for (int64_t i = startPos - (direction == dirDown ? 0 : 1);
		(direction == dirDown) ? i < static_cast<int64_t>(totalSize) : i >= 0;
		(direction == dirDown) ? (++i) : (--i))
	{
		if (static_cast<unsigned char>(content[i]) >= beginRange && static_cast<unsigned char>(content[i]) <= endRange)
		{
			found = static_cast<size_t>(i);
			isFound = true;
			break;
		}
	}

	if (!isFound)
	{
		if (wrap)
		{
			for (int64_t i = (direction == dirDown ? 0 : static_cast<int64_t>(totalSize) - 1);
				(direction == dirDown) ? i < static_cast<int64_t>(totalSize) : i >= 0;
				(direction == dirDown) ? (++i) : (--i))
			{
				if (static_cast<unsigned char>(content[i]) >= beginRange && static_cast<unsigned char>(content[i]) <= endRange)
				{
					found = static_cast<size_t>(i);
					isFound = true;
					break;
				}
			}
		}
	}

	if (isFound)
	{
		auto sci_line = view->execute(SCI_LINEFROMPOSITION, found);
		view->execute(SCI_ENSUREVISIBLE, sci_line);
		view->execute(SCI_GOTOPOS, found);
		view->execute(SCI_SETSEL, (direction == dirDown) ? found : found + 1,
			(direction == dirDown) ? found + 1 : found);
	}

	delete[] content;
	return isFound;
}

void FindCharsInRangeDlg::getDirectionFromUI(bool& whichDirection, bool& isWrap)
{
	whichDirection = _dirUpRadio && _dirUpRadio->isChecked();
	isWrap = _wrapCheck && _wrapCheck->isChecked();
}

bool FindCharsInRangeDlg::getRangeFromUI(unsigned char& startRange, unsigned char& endRange)
{
	if (_nonAsciiRadio && _nonAsciiRadio->isChecked())
	{
		startRange = 128;
		endRange = 255;
		return true;
	}

	if (_asciiRadio && _asciiRadio->isChecked())
	{
		startRange = 0;
		endRange = 127;
		return true;
	}

	if (_myRangeRadio && _myRangeRadio->isChecked())
	{
		if (!_rangeStartEdit || !_rangeEndEdit)
			return false;

		int start = _rangeStartEdit->value();
		int end = _rangeEndEdit->value();

		if (start > 255 || end > 255)
			return false;
		if (start > end)
			return false;

		startRange = static_cast<unsigned char>(start);
		endRange = static_cast<unsigned char>(end);
		return true;
	}

	return false;
}

bool FindCharsInRangeDlg::run_dlgProc(QEvent* event)
{
	Q_UNUSED(event);
	return true;
}

} // namespace QtControls
