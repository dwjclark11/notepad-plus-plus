// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"

// Forward declarations
class QSpinBox;
class QLabel;
class QRadioButton;
class QPushButton;

namespace QtControls {

// ============================================================================
// GoToLineDlg - Qt implementation of Go To Line dialog
// ============================================================================
class GoToLineDlg : public StaticDialog {
    Q_OBJECT

public:
    GoToLineDlg(QWidget* parent = nullptr);
    ~GoToLineDlg() override;

    // Initialize the dialog with current position info
    void init(int currentLine, int totalLines, int currentPos);

    // Show the dialog
    void doDialog();

    // Get the target line number (1-based)
    int getLine() const;

    // Check if in line mode (true) or offset mode (false)
    bool isLineMode() const;

protected:
    void setupUI() override;
    void connectSignals() override;

private slots:
    void onGoClicked();
    void onCancelClicked();
    void onModeChanged();
    void updateRangeLabel();

private:
    QSpinBox* _lineSpinBox = nullptr;
    QLabel* _rangeLabel = nullptr;
    QRadioButton* _lineModeRadio = nullptr;
    QRadioButton* _offsetModeRadio = nullptr;
    QPushButton* _goButton = nullptr;
    QPushButton* _cancelButton = nullptr;

    int _currentLine = 0;
    int _totalLines = 0;
    int _currentPos = 0;

    enum class Mode { GoToLine, GoToOffset };
    Mode _mode = Mode::GoToLine;
};

} // namespace QtControls
