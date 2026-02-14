// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "CommandTest.h"
#include "../Common/TestUtils.h"
#include "menuCmdID.h"
#include <set>

namespace Tests {

CommandTest::CommandTest() {}

CommandTest::~CommandTest() {}

void CommandTest::initTestCase()
{
	QVERIFY(TestEnvironment::getInstance().init());
}

void CommandTest::cleanupTestCase()
{
	TestEnvironment::getInstance().cleanup();
}

// ============================================================================
// Real Validation Tests - Command ID Definitions
// ============================================================================

void CommandTest::testCommandIdRangesAreDistinct()
{
	// Verify that the base ranges for each menu category do not overlap.
	// Each category uses IDM + offset*1000, so bases must be distinct.
	int bases[] = {IDM_FILE, IDM_EDIT, IDM_SEARCH, IDM_VIEW, IDM_FORMAT, IDM_LANG, IDM_ABOUT, IDM_SETTING};
	std::set<int> baseSet(std::begin(bases), std::end(bases));
	QCOMPARE(baseSet.size(), static_cast<size_t>(sizeof(bases) / sizeof(bases[0])));
}

void CommandTest::testCommandIdsAreNonZero()
{
	// All command IDs must be positive non-zero values
	QVERIFY(IDM > 0);
	QVERIFY(IDM_FILE_NEW > 0);
	QVERIFY(IDM_FILE_OPEN > 0);
	QVERIFY(IDM_FILE_SAVE > 0);
	QVERIFY(IDM_FILE_CLOSE > 0);
	QVERIFY(IDM_EDIT_CUT > 0);
	QVERIFY(IDM_EDIT_COPY > 0);
	QVERIFY(IDM_EDIT_UNDO > 0);
	QVERIFY(IDM_EDIT_REDO > 0);
	QVERIFY(IDM_EDIT_PASTE > 0);
	QVERIFY(IDM_EDIT_DELETE > 0);
	QVERIFY(IDM_EDIT_SELECTALL > 0);
	QVERIFY(IDM_SEARCH_FIND > 0);
	QVERIFY(IDM_SEARCH_REPLACE > 0);
	QVERIFY(IDM_SEARCH_GOTOLINE > 0);
	QVERIFY(IDM_VIEW_ZOOMIN > 0);
	QVERIFY(IDM_VIEW_ZOOMOUT > 0);
	QVERIFY(IDM_VIEW_ZOOMRESTORE > 0);
	QVERIFY(IDM_VIEW_WRAP > 0);
}

void CommandTest::testFileMenuRange()
{
	// All file menu commands should fall within the IDM_FILE range
	QVERIFY(IDM_FILE_NEW > IDM_FILE);
	QVERIFY(IDM_FILE_OPEN > IDM_FILE);
	QVERIFY(IDM_FILE_CLOSE > IDM_FILE);
	QVERIFY(IDM_FILE_SAVE > IDM_FILE);
	QVERIFY(IDM_FILE_SAVEAS > IDM_FILE);
	QVERIFY(IDM_FILE_SAVEALL > IDM_FILE);
	QVERIFY(IDM_FILE_EXIT > IDM_FILE);

	// File commands should be below the Edit range
	QVERIFY(IDM_FILE_NEW < IDM_EDIT);
	QVERIFY(IDM_FILE_EXIT < IDM_EDIT);
	QVERIFY(IDM_FILEMENU_LASTONE < IDM_EDIT);
}

void CommandTest::testEditMenuRange()
{
	// All edit menu commands should fall within the IDM_EDIT range
	QVERIFY(IDM_EDIT_CUT > IDM_EDIT);
	QVERIFY(IDM_EDIT_COPY > IDM_EDIT);
	QVERIFY(IDM_EDIT_UNDO > IDM_EDIT);
	QVERIFY(IDM_EDIT_REDO > IDM_EDIT);
	QVERIFY(IDM_EDIT_PASTE > IDM_EDIT);
	QVERIFY(IDM_EDIT_DELETE > IDM_EDIT);
	QVERIFY(IDM_EDIT_SELECTALL > IDM_EDIT);

	// Edit commands should be below the Search range
	QVERIFY(IDM_EDIT_CUT < IDM_SEARCH);
	QVERIFY(IDM_EDIT_SELECTALL < IDM_SEARCH);
}

void CommandTest::testSearchMenuRange()
{
	// All search menu commands should fall within the IDM_SEARCH range
	QVERIFY(IDM_SEARCH_FIND > IDM_SEARCH);
	QVERIFY(IDM_SEARCH_FINDNEXT > IDM_SEARCH);
	QVERIFY(IDM_SEARCH_REPLACE > IDM_SEARCH);
	QVERIFY(IDM_SEARCH_GOTOLINE > IDM_SEARCH);

	// Search commands should be below the View range
	QVERIFY(IDM_SEARCH_FIND < IDM_VIEW);
	QVERIFY(IDM_SEARCH_GOTOLINE < IDM_VIEW);
}

void CommandTest::testViewMenuRange()
{
	// Core view commands should fall within the IDM_VIEW range
	QVERIFY(IDM_VIEW_ZOOMIN > IDM_VIEW);
	QVERIFY(IDM_VIEW_ZOOMOUT > IDM_VIEW);
	QVERIFY(IDM_VIEW_ZOOMRESTORE > IDM_VIEW);
	QVERIFY(IDM_VIEW_WRAP > IDM_VIEW);
	QVERIFY(IDM_VIEW_ALL_CHARACTERS > IDM_VIEW);
	QVERIFY(IDM_VIEW_INDENT_GUIDE > IDM_VIEW);
}

// ============================================================================
// Command Dispatch Tests - Require Full Notepad_plus Core
// ============================================================================

void CommandTest::testNewFile()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testOpenFile()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testSaveFile()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testCloseFile()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testUndo()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testRedo()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testCut()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testCopy()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testPaste()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testDelete()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testSelectAll()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testFind()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testReplace()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testGoToLine()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testZoomIn()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testZoomOut()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testZoomReset()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testToggleWordWrap()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testToggleLineNumbers()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testStartRecording()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testStopRecording()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testPlayback()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testRunCommand()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testNewWindow()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testCloseWindow()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

void CommandTest::testSplitView()
{
	QSKIP("Requires full Notepad_plus core for command dispatch");
}

} // namespace Tests

#include "CommandTest.moc"
