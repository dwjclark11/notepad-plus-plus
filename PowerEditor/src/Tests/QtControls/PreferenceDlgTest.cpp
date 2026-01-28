// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "PreferenceDlgTest.h"
#include "preferenceDlg.h"
#include "../Common/TestUtils.h"

using namespace QtControls;

namespace Tests {

PreferenceDlgTest::PreferenceDlgTest() {}

PreferenceDlgTest::~PreferenceDlgTest() {}

void PreferenceDlgTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void PreferenceDlgTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void PreferenceDlgTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _dialog = std::make_unique<PreferenceDlg>(_parentWidget.get());
}

void PreferenceDlgTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void PreferenceDlgTest::testInit() {
    QVERIFY(_dialog != nullptr);
}

void PreferenceDlgTest::testDoDialog() {
    _dialog->doDialog();
    QVERIFY(_dialog->getWidget()->isVisible());
}

// ============================================================================
// Navigation Tests
// ============================================================================
void PreferenceDlgTest::testShowPage() {
    _dialog->doDialog();

    // Show general page
    _dialog->showPage("General");
    QVERIFY(true);

    // Show editing page
    _dialog->showPage("Editing");
    QVERIFY(true);
}

void PreferenceDlgTest::testGetCurrentPageIndex() {
    _dialog->doDialog();

    int index = _dialog->getCurrentPageIndex();
    QVERIFY(index >= 0);
}

} // namespace Tests

#include "PreferenceDlgTest.moc"
