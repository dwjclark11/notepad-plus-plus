// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "GoToLineDlg.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDialog>
#include <QtGui/QKeyEvent>

namespace QtControls {

GoToLineDlg::GoToLineDlg(QWidget* parent)
    : StaticDialog(parent)
{
}

GoToLineDlg::~GoToLineDlg() = default;

void GoToLineDlg::init(int currentLine, int totalLines, int currentPos)
{
    _currentLine = currentLine;
    _totalLines = totalLines;
    _currentPos = currentPos;
}

void GoToLineDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Go To"));
    dialog->resize(300, 180);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Mode selection group
    auto* modeGroup = new QGroupBox(tr("Destination"), dialog);
    auto* modeLayout = new QHBoxLayout(modeGroup);

    _lineModeRadio = new QRadioButton(tr("&Line"), dialog);
    _lineModeRadio->setChecked(true);
    modeLayout->addWidget(_lineModeRadio);

    _offsetModeRadio = new QRadioButton(tr("&Offset"), dialog);
    modeLayout->addWidget(_offsetModeRadio);

    modeLayout->addStretch();
    mainLayout->addWidget(modeGroup);

    // Input section
    auto* inputLayout = new QHBoxLayout();

    auto* goToLabel = new QLabel(tr("Go to:"), dialog);
    inputLayout->addWidget(goToLabel);

    _lineSpinBox = new QSpinBox(dialog);
    _lineSpinBox->setMinimum(1);
    _lineSpinBox->setMaximum(999999999);
    _lineSpinBox->setValue(_currentLine > 0 ? _currentLine : 1);
    _lineSpinBox->setMinimumWidth(100);
    inputLayout->addWidget(_lineSpinBox);

    inputLayout->addStretch();
    mainLayout->addLayout(inputLayout);

    // Range info label
    _rangeLabel = new QLabel(dialog);
    _rangeLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    _rangeLabel->setMinimumHeight(24);
    mainLayout->addWidget(_rangeLabel);

    // Add stretch to push buttons to bottom
    mainLayout->addStretch();

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _goButton = new QPushButton(tr("&Go"), dialog);
    _goButton->setDefault(true);
    buttonLayout->addWidget(_goButton);

    _cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void GoToLineDlg::connectSignals()
{
    connect(_goButton, &QPushButton::clicked, this, &GoToLineDlg::onGoClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &GoToLineDlg::onCancelClicked);
    connect(_lineModeRadio, &QRadioButton::toggled, this, &GoToLineDlg::onModeChanged);
    connect(_offsetModeRadio, &QRadioButton::toggled, this, &GoToLineDlg::onModeChanged);
}

void GoToLineDlg::doDialog()
{
    if (!isCreated()) {
        create(tr("Go To"), false);
        setupUI();
        connectSignals();
    }

    updateRangeLabel();
    display(true, true);
    goToCenter();

    // Set focus to the spin box and select all text
    if (_lineSpinBox) {
        _lineSpinBox->setFocus();
        _lineSpinBox->selectAll();
    }
}

int GoToLineDlg::getLine() const
{
    if (_lineSpinBox) {
        return _lineSpinBox->value();
    }
    return -1;
}

bool GoToLineDlg::isLineMode() const
{
    return _mode == Mode::GoToLine;
}

void GoToLineDlg::onGoClicked()
{
    display(false);
}

void GoToLineDlg::onCancelClicked()
{
    display(false);
}

void GoToLineDlg::onModeChanged()
{
    if (_lineModeRadio && _lineModeRadio->isChecked()) {
        _mode = Mode::GoToLine;
    } else if (_offsetModeRadio && _offsetModeRadio->isChecked()) {
        _mode = Mode::GoToOffset;
    }
    updateRangeLabel();
}

void GoToLineDlg::updateRangeLabel()
{
    if (!_rangeLabel) return;

    QString text;
    if (_mode == Mode::GoToLine) {
        text = tr("You are here: %1\nYou want to go to: %2")
                   .arg(_currentLine)
                   .arg(_totalLines > 0 ? _totalLines : 1);
        if (_lineSpinBox) {
            _lineSpinBox->setMaximum(_totalLines > 0 ? _totalLines : 1);
            if (_lineSpinBox->value() > _totalLines && _totalLines > 0) {
                _lineSpinBox->setValue(_totalLines);
            }
        }
    } else {
        // Offset mode - show position info
        int maxOffset = _totalLines > 0 ? _totalLines * 100 : 0; // Approximate
        text = tr("You are here: %1\nYou want to go to: %2")
                   .arg(_currentPos)
                   .arg(maxOffset > 0 ? maxOffset : 1);
        if (_lineSpinBox) {
            _lineSpinBox->setMaximum(maxOffset > 0 ? maxOffset : 999999999);
        }
    }
    _rangeLabel->setText(text);
}

} // namespace QtControls
