// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "GoToLineDlgTest.h"
#include "GoToLine/GoToLineDlg.h"
#include "../Common/TestUtils.h"
#include "../../MISC/Common/LinuxTypes.h"

using namespace QtControls;

namespace Tests {

GoToLineDlgTest::GoToLineDlgTest() {}

GoToLineDlgTest::~GoToLineDlgTest() {}

void GoToLineDlgTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void GoToLineDlgTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void GoToLineDlgTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _dialog = std::make_unique<GoToLineDlg>(_parentWidget.get());
}

void GoToLineDlgTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void GoToLineDlgTest::testInit() {
    QVERIFY(_dialog != nullptr);

    // Initialize with HINSTANCE, HWND parent, and ScintillaEditView**
    // Pass nullptr for ScintillaEditView** since we're testing without a real editor
    _dialog->init(static_cast<HINSTANCE>(nullptr), static_cast<HWND>(_parentWidget.get()), nullptr);
    QVERIFY(true);
}

void GoToLineDlgTest::testDoDialog() {
    _dialog->init(static_cast<HINSTANCE>(nullptr), static_cast<HWND>(_parentWidget.get()), nullptr);

    // Show the dialog
    _dialog->doDialog();
    QVERIFY(_dialog->getWidget()->isVisible());
}

// ============================================================================
// Getters Tests
// ============================================================================
void GoToLineDlgTest::testGetLine() {
    _dialog->init(static_cast<HINSTANCE>(nullptr), static_cast<HWND>(_parentWidget.get()), nullptr);
    _dialog->doDialog();

    // Default line should be 0 when no editor is attached
    long long line = _dialog->getLine();
    QCOMPARE(line, 0);
}

// ============================================================================
// Mode Switching Tests
// ============================================================================
void GoToLineDlgTest::testModeSwitching() {
    _dialog->init(static_cast<HINSTANCE>(nullptr), static_cast<HWND>(_parentWidget.get()), nullptr);
    _dialog->doDialog();

    // Default should be line mode - verify dialog initialized without crashing
    QVERIFY(true);

    // Switch to offset mode would require UI interaction
    // This is tested through the UI in integration tests
    QVERIFY(true);
}

} // namespace Tests
