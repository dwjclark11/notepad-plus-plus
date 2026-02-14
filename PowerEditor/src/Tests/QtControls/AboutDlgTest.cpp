// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "AboutDlgTest.h"
#include "AboutDlg.h"
#include "../Common/TestUtils.h"
#include <QLabel>
#include <QTextEdit>

using namespace QtControls;

namespace Tests {

AboutDlgTest::AboutDlgTest() {}

AboutDlgTest::~AboutDlgTest() {}

void AboutDlgTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void AboutDlgTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void AboutDlgTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _dialog = std::make_unique<AboutDlg>(_parentWidget.get());
}

void AboutDlgTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void AboutDlgTest::testInit() {
    QVERIFY(_dialog != nullptr);
}

void AboutDlgTest::testDoDialog() {
    _dialog->doDialog();
    QVERIFY(_dialog->getWidget()->isVisible());
}

// ============================================================================
// Content Tests
// ============================================================================
void AboutDlgTest::testVersionString() {
    _dialog->doDialog();

    // Find the label containing the version string by searching all QLabel children
    QWidget* widget = _dialog->getWidget();
    QVERIFY(widget != nullptr);

    QList<QLabel*> labels = widget->findChildren<QLabel*>();
    bool foundVersion = false;
    for (QLabel* label : labels)
    {
        if (label->text().contains("Notepad++") && label->text().contains("v"))
        {
            foundVersion = true;
            QVERIFY(!label->text().isEmpty());
            break;
        }
    }
    QVERIFY(foundVersion);
}

void AboutDlgTest::testBuildTimeString() {
    _dialog->doDialog();

    QWidget* widget = _dialog->getWidget();
    QVERIFY(widget != nullptr);

    QList<QLabel*> labels = widget->findChildren<QLabel*>();
    bool foundBuildTime = false;
    for (QLabel* label : labels)
    {
        if (label->text().contains("Build time"))
        {
            foundBuildTime = true;
            QVERIFY(!label->text().isEmpty());
            // Build time should contain a date (month name from __DATE__)
            QVERIFY(label->text().length() > QString("Build time: ").length());
            break;
        }
    }
    QVERIFY(foundBuildTime);
}

void AboutDlgTest::testLicenseText() {
    _dialog->doDialog();

    QWidget* widget = _dialog->getWidget();
    QVERIFY(widget != nullptr);

    // The license text is in a QTextEdit widget
    QList<QTextEdit*> textEdits = widget->findChildren<QTextEdit*>();
    QVERIFY(!textEdits.isEmpty());

    bool foundLicense = false;
    for (QTextEdit* edit : textEdits)
    {
        QString text = edit->toPlainText();
        if (text.contains("GNU General Public License") || text.contains("GPL"))
        {
            foundLicense = true;
            QVERIFY(text.contains("free software"));
            break;
        }
    }
    QVERIFY(foundLicense);
}

} // namespace Tests
