// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "RunMacroDlg.h"
#include "Parameters.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QMessageBox>

namespace QtControls {

RunMacroDlg::RunMacroDlg(QWidget* parent)
    : StaticDialog(parent)
{
}

void RunMacroDlg::doDialog(bool /*isRTL*/)
{
    if (!isCreated()) {
        create(tr("Run a Macro Multiple Times"), false);
        setupUI();
        connectSignals();
    } else {
        // Shortcut might have been updated for current session
        // So reload the macro list (issue #4526)
        initMacroList();
    }

    goToCenter();
    display(true, true);
}

void RunMacroDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Run a Macro Multiple Times"));
    dialog->resize(400, 250);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Macro selection group
    auto* macroGroup = new QGroupBox(tr("Macro to Run"), dialog);
    auto* macroLayout = new QHBoxLayout(macroGroup);

    _macroLabel = new QLabel(tr("Macro:"), macroGroup);
    macroLayout->addWidget(_macroLabel);

    _macroCombo = new QComboBox(macroGroup);
    _macroCombo->setMinimumWidth(250);
    macroLayout->addWidget(_macroCombo, 1);

    mainLayout->addWidget(macroGroup);

    // Run options group
    auto* optionsGroup = new QGroupBox(tr("Run Options"), dialog);
    auto* optionsLayout = new QVBoxLayout(optionsGroup);

    // Run multiple times option
    auto* multiLayout = new QHBoxLayout();
    _runMultiRadio = new QRadioButton(tr("Run"), optionsGroup);
    _runMultiRadio->setChecked(true);
    multiLayout->addWidget(_runMultiRadio);

    _timesEdit = new QLineEdit(optionsGroup);
    _timesEdit->setText(QString::number(_times));
    _timesEdit->setMaximumWidth(60);
    _timesEdit->setValidator(new QIntValidator(1, 9999, this));
    multiLayout->addWidget(_timesEdit);

    _timesLabel = new QLabel(tr("time(s)"), optionsGroup);
    multiLayout->addWidget(_timesLabel);
    multiLayout->addStretch();

    optionsLayout->addLayout(multiLayout);

    // Run until EOF option
    _runEOFRadio = new QRadioButton(tr("Run until the end of file"), optionsGroup);
    optionsLayout->addWidget(_runEOFRadio);

    mainLayout->addWidget(optionsGroup);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _okButton = new QPushButton(tr("Run"), dialog);
    _okButton->setDefault(true);
    buttonLayout->addWidget(_okButton);

    _cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_cancelButton);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    // Store initial rect
    _rc = dialog->geometry();

    // Initialize macro list
    initMacroList();
}

void RunMacroDlg::connectSignals()
{
    connect(_okButton, &QPushButton::clicked, this, &RunMacroDlg::onOKClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &RunMacroDlg::onCancelClicked);
    connect(_runMultiRadio, &QRadioButton::toggled, this, &RunMacroDlg::onRunMultiToggled);
    connect(_runEOFRadio, &QRadioButton::toggled, this, &RunMacroDlg::onRunEOFToggled);
    connect(_timesEdit, &QLineEdit::textChanged, this, &RunMacroDlg::onTimesChanged);
    connect(_macroCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RunMacroDlg::onMacroSelectionChanged);
}

void RunMacroDlg::initMacroList()
{
    if (!_macroCombo) return;

    QString currentSelection = _macroCombo->currentText();
    _macroCombo->clear();

    _hasCurrentMacro = _hasRecordedMacro;
    if (_hasCurrentMacro)
    {
        _macroCombo->addItem(tr("Current recorded macro"));
    }

    // Add saved macros from NppParameters
    NppParameters& nppParams = NppParameters::getInstance();
    std::vector<MacroShortcut>& macros = nppParams.getMacroList();
    for (size_t i = 0; i < macros.size(); ++i)
    {
        QString name = QString::fromUtf8(macros[i].getName());
        _macroCombo->addItem(name);
    }

    // Restore selection if possible
    int index = _macroCombo->findText(currentSelection);
    if (index >= 0) {
        _macroCombo->setCurrentIndex(index);
    } else {
        _macroCombo->setCurrentIndex(0);
    }

    _macroIndex = _macroCombo->currentIndex();
}

int RunMacroDlg::isMulti() const
{
    return _runMultiRadio && _runMultiRadio->isChecked() ? 1 : 0;
}

int RunMacroDlg::getMacro2Exec() const
{
    // Index 0 is "Current recorded macro" when present, which maps to -1
    if (_hasCurrentMacro)
        return _macroIndex - 1;
    return _macroIndex;
}

bool RunMacroDlg::run_dlgProc(QEvent* /*event*/)
{
    return false;
}

void RunMacroDlg::onOKClicked()
{
    // Validate times value
    bool ok;
    int times = _timesEdit->text().toInt(&ok);
    if (!ok || times < 1) {
        times = 1;
    }
    _times = times;

    // Signal the parent Notepad_plus to run the macro
    emit runMacroRequested();

    display(false);
}

void RunMacroDlg::onCancelClicked()
{
    display(false);
}

void RunMacroDlg::onRunMultiToggled(bool checked)
{
    if (checked) {
        _runUntilEOF = false;
        _timesEdit->setEnabled(true);
    }
}

void RunMacroDlg::onRunEOFToggled(bool checked)
{
    if (checked) {
        _runUntilEOF = true;
        _timesEdit->setEnabled(false);
    }
}

void RunMacroDlg::onTimesChanged(const QString& text)
{
    bool ok;
    int times = text.toInt(&ok);
    if (ok && times >= 1) {
        _times = times;
    }
}

void RunMacroDlg::onMacroSelectionChanged(int index)
{
    _macroIndex = index;
}

} // namespace QtControls
