// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "AboutDlg.h"
#include "../../resource.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QDialog>
#include <QtGui/QDesktopServices>
#include <QtGui/QPixmap>
#include <QtCore/QUrl>
#include <QtCore/QDate>

namespace QtControls {

AboutDlg::AboutDlg(QWidget* parent)
{
    init(parent);
}

void AboutDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    // Set fixed size for the about dialog
    dialog->setFixedSize(450, 500);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Logo section
    auto* logoLayout = new QHBoxLayout();
    logoLayout->addStretch();

    _logoLabel = new QLabel(dialog);
    _logoLabel->setFixedSize(80, 80);
    _logoLabel->setAlignment(Qt::AlignCenter);
    // Use the chameleon icon (IDI_CHAMELEON = 101)
    // For now, use a placeholder text - in production would load from resources
    _logoLabel->setText(tr("N++"));
    _logoLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6B8E23;");
    logoLayout->addWidget(_logoLabel);
    logoLayout->addStretch();

    mainLayout->addLayout(logoLayout);

    // Application name
    auto* nameLabel = new QLabel(tr("Notepad++"), dialog);
    nameLabel->setAlignment(Qt::AlignCenter);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(16);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    mainLayout->addWidget(nameLabel);

    // Version info
    _versionLabel = new QLabel(getVersionString(), dialog);
    _versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(_versionLabel);

    // Build time
    _buildTimeLabel = new QLabel(getBuildTimeString(), dialog);
    _buildTimeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(_buildTimeLabel);

    // Separator line
    auto* line = new QFrame(dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // Copyright
    _copyrightLabel = new QLabel(
        tr("Copyright \u00A9 2021-2025 Don HO <don.h@free.fr>\n")
        + tr("Copyright \u00A9 2024-2025 Notepad++ contributors"),
        dialog);
    _copyrightLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(_copyrightLabel);

    // License text (in a read-only text edit for scrollability)
    auto* licenseEdit = new QTextEdit(dialog);
    licenseEdit->setReadOnly(true);
    licenseEdit->setPlainText(LICENCE_TXT);
    licenseEdit->setMaximumHeight(100);
    mainLayout->addWidget(licenseEdit);

    // Website link
    _websiteLabel = new QLabel(
        tr("Website: ") + "<a href=\"https://notepad-plus-plus.org/\">https://notepad-plus-plus.org/</a>",
        dialog);
    _websiteLabel->setAlignment(Qt::AlignCenter);
    _websiteLabel->setOpenExternalLinks(true);
    mainLayout->addWidget(_websiteLabel);

    // GitHub link
    _gitHubLabel = new QLabel(
        tr("GitHub: ") + "<a href=\"https://github.com/notepad-plus-plus/notepad-plus-plus/\">https://github.com/notepad-plus-plus/notepad-plus-plus/</a>",
        dialog);
    _gitHubLabel->setAlignment(Qt::AlignCenter);
    _gitHubLabel->setOpenExternalLinks(true);
    mainLayout->addWidget(_gitHubLabel);

    // Separator line
    auto* line2 = new QFrame(dialog);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line2);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _creditsButton = new QPushButton(tr("Credits"), dialog);
    buttonLayout->addWidget(_creditsButton);

    buttonLayout->addSpacing(10);

    _okButton = new QPushButton(tr("OK"), dialog);
    _okButton->setDefault(true);
    buttonLayout->addWidget(_okButton);

    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void AboutDlg::connectSignals()
{
    // Use lambda functions to avoid needing QObject as receiver context
    QObject::connect(_okButton, &QPushButton::clicked, [this]() { onOkClicked(); });
    QObject::connect(_creditsButton, &QPushButton::clicked, [this]() { onCreditsClicked(); });

    // Connect link activation signals
    QObject::connect(_websiteLabel, &QLabel::linkActivated, [this]() { onWebsiteLinkClicked(); });
    QObject::connect(_gitHubLabel, &QLabel::linkActivated, [this]() { onGitHubLinkClicked(); });
}

void AboutDlg::doDialog()
{
    if (!isCreated()) {
        create(tr("About Notepad++"), false);
        setupUI();
        connectSignals();
    }

    // Center the dialog on parent
    goToCenter();
    display(true, true);
}

QString AboutDlg::getVersionString() const
{
    // Convert wide string to QString
    return QString::fromWCharArray(NOTEPAD_PLUS_VERSION);
}

QString AboutDlg::getBuildTimeString() const
{
    QString buildTime = tr("Build time: ");
    buildTime += QString(__DATE__);
    buildTime += " - ";
    buildTime += QString(__TIME__);
    return buildTime;
}

void AboutDlg::onOkClicked()
{
    display(false);
}

void AboutDlg::onCreditsClicked()
{
    // TODO: Implement credits dialog
    // For now, just open the GitHub contributors page
    QDesktopServices::openUrl(QUrl("https://github.com/notepad-plus-plus/notepad-plus-plus/graphs/contributors"));
}

void AboutDlg::onWebsiteLinkClicked()
{
    QDesktopServices::openUrl(QUrl("https://notepad-plus-plus.org/"));
}

void AboutDlg::onGitHubLinkClicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/notepad-plus-plus/notepad-plus-plus/"));
}

} // namespace QtControls
