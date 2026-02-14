// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FindReplaceDlgInitTest.h"
#include "../Common/TestUtils.h"
// Use explicit path to avoid collision with ScintillaComponent/FindReplaceDlg.h (Windows version)
#include "QtControls/FindReplace/FindReplaceDlg.h"

using namespace NppFindReplace;

namespace Tests {

FindReplaceDlgInitTest::FindReplaceDlgInitTest() {}
FindReplaceDlgInitTest::~FindReplaceDlgInitTest() {}

void FindReplaceDlgInitTest::initTestCase() {
	QVERIFY(TestEnvironment::getInstance().init());
}

void FindReplaceDlgInitTest::cleanupTestCase() {
	TestEnvironment::getInstance().cleanup();
}

void FindReplaceDlgInitTest::testIsCreatedFalseBeforeInit() {
	FindReplaceDlg dlg;
	QVERIFY(!dlg.isCreated());
}

void FindReplaceDlgInitTest::testShowDialogSafeBeforeInit() {
	FindReplaceDlg dlg;
	// Should not crash - null guard on _tabWidget
	dlg.showDialog(FindDialogType::Find);
	dlg.showDialog(FindDialogType::Replace);
	QVERIFY(!dlg.isCreated());  // Still not created
}

void FindReplaceDlgInitTest::testInitSetsIsCreated() {
	FindReplaceDlg dlg;
	QVERIFY(!dlg.isCreated());
	dlg.init(nullptr);  // Pass nullptr for ScintillaEditView** - should still set up UI
	QVERIFY(dlg.isCreated());
}

void FindReplaceDlgInitTest::testGetCurrentOptionsReadsFromFindTab() {
	FindReplaceDlg dlg;
	dlg.init(nullptr);

	// getCurrentOptions should return valid defaults without crash
	auto opts = dlg.getCurrentOptions();
	QVERIFY(!opts.isMatchCase);  // Default is false
	QVERIFY(!opts.isWholeWord);  // Default is false
	QVERIFY(opts.isWrapAround);  // Default is true
}

} // namespace Tests
