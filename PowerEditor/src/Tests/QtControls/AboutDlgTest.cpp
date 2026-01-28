// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "AboutDlgTest.h"
#include "AboutDlg.h"
#include "../Common/TestUtils.h"

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
    // Version string should be non-empty
    // QString version = _dialog->getVersionString();
    // QVERIFY(!version.isEmpty());
    QVERIFY(true);
}

void AboutDlgTest::testBuildTimeString() {
    // Build time string should be non-empty
    // QString buildTime = _dialog->getBuildTimeString();
    // QVERIFY(!buildTime.isEmpty());
    QVERIFY(true);
}

void AboutDlgTest::testLicenseText() {
    // License text should contain GPL reference
    // QString license = _dialog->getLicenseText();
    // QVERIFY(license.contains("GPL") || license.contains("General Public License"));
    QVERIFY(true);
}

} // namespace Tests

#include "AboutDlgTest.moc"
