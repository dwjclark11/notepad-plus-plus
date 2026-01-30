// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "ScintillaEditView.h"
#include "Parameters.h"

// Forward declarations
class QLineEdit;
class QRadioButton;
class QPushButton;
class QGroupBox;
class QLabel;
class QComboBox;

namespace QtControls {

// ============================================================================
// ColumnEditorDlg - Qt implementation of Column Editor dialog
// ============================================================================
class ColumnEditorDlg : public StaticDialog {
    Q_OBJECT

public:
    ColumnEditorDlg(QWidget* parent = nullptr);
    ~ColumnEditorDlg() override;

    // Initialize the dialog (matches Windows interface)
    void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView);

    // Show the dialog (matches Windows interface)
    void doDialog(bool isRTL = false);

    // Display the dialog (Qt-specific interface)
    void display(bool toShow = true, bool enhancedPositioningCheckWhenShowing = false);

    // Switch between text and number modes
    void switchTo(bool toText);

    // Get the current number format
    UCHAR getFormat();

    // Get the leading option
    ColumnEditorParam::leadingChoice getLeading();

    // Get the hex case (BASE_16 or BASE_16_UPPERCASE)
    UCHAR getHexCase();

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onModeChanged();
    void onFormatChanged();
    void onTextChanged(const QString& text);
    void onLeadingChanged(int index);
    void onHexCaseChanged(int index);
    void onNumericFieldChanged();

private:
    // UI Components - Text mode
    QRadioButton* _textRadio = nullptr;
    QGroupBox* _textGroup = nullptr;
    QLineEdit* _textEdit = nullptr;

    // UI Components - Number mode
    QRadioButton* _numRadio = nullptr;
    QGroupBox* _numGroup = nullptr;
    QLabel* _initNumLabel = nullptr;
    QLineEdit* _initNumEdit = nullptr;
    QLabel* _incrNumLabel = nullptr;
    QLineEdit* _incrNumEdit = nullptr;
    QLabel* _repeatNumLabel = nullptr;
    QLineEdit* _repeatNumEdit = nullptr;

    // Format selection
    QGroupBox* _formatGroup = nullptr;
    QRadioButton* _decRadio = nullptr;
    QRadioButton* _hexRadio = nullptr;
    QRadioButton* _octRadio = nullptr;
    QRadioButton* _binRadio = nullptr;

    // Leading option
    QLabel* _leadingLabel = nullptr;
    QComboBox* _leadingCombo = nullptr;

    // Hex case option
    QComboBox* _hexCaseCombo = nullptr;

    // Buttons
    QPushButton* _okButton = nullptr;
    QPushButton* _cancelButton = nullptr;

    // Store the edit view pointer (matches Windows interface)
    ScintillaEditView **_ppEditView = nullptr;

    // Helper methods
    void setNumericFields(const ColumnEditorParam& colEditParam);
    int getNumericFieldValueFromText(int formatChoice, const QString& str);
    void showValidationError(int whichField, int formatChoice, const QString& str);
    void updateEnabledStates();

    // Timer for flashing error background
    int _flashFieldId = 0;
    QTimer* _flashTimer = nullptr;
    QTimer* _tooltipTimer = nullptr;

    // Field IDs for error flashing
    enum FieldId {
        FIELD_NONE = 0,
        FIELD_INIT_NUM = 1,
        FIELD_INCREASE_NUM = 2,
        FIELD_REPEAT_NUM = 3
    };
};

} // namespace QtControls
