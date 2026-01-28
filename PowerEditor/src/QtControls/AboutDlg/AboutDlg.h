// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include <QString>

// Forward declarations
class QLabel;
class QPushButton;

namespace QtControls {

// License text constant (matching Windows version)
#define LICENCE_TXT \
"This program is free software; you can redistribute it and/or \
modify it under the terms of the GNU General Public License \
as published by the Free Software Foundation; either \
version 3 of the License, or at your option any later version.\n\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \
GNU General Public License for more details.\n\n\
You should have received a copy of the GNU General Public License \
along with this program. If not, see <https://www.gnu.org/licenses/>."

// ============================================================================
// AboutDlg - Qt implementation
// ============================================================================
class AboutDlg : public StaticDialog {
    Q_OBJECT

public:
    AboutDlg(QWidget* parent = nullptr);
    ~AboutDlg() override = default;

    void doDialog();

protected:
    void setupUI();
    void connectSignals();

private slots:
    void onOkClicked();
    void onCreditsClicked();
    void onWebsiteLinkClicked();
    void onGitHubLinkClicked();

private:
    // UI Components
    QLabel* _logoLabel = nullptr;
    QLabel* _versionLabel = nullptr;
    QLabel* _buildTimeLabel = nullptr;
    QLabel* _copyrightLabel = nullptr;
    QLabel* _licenseLabel = nullptr;
    QLabel* _websiteLabel = nullptr;
    QLabel* _gitHubLabel = nullptr;
    QPushButton* _creditsButton = nullptr;
    QPushButton* _okButton = nullptr;

    // Helper methods
    QString getVersionString() const;
    QString getBuildTimeString() const;
};

} // namespace QtControls
