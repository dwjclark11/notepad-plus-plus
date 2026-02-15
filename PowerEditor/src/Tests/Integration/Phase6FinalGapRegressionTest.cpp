// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "Phase6FinalGapRegressionTest.h"
#include "../Common/TestUtils.h"
#include "NppCommands.h"
#include "menuCmdID.h"
#include "Notepad_plus_msgs.h"
#include <set>

using namespace QtCommands;

namespace Tests {

Phase6FinalGapRegressionTest::Phase6FinalGapRegressionTest() {}

Phase6FinalGapRegressionTest::~Phase6FinalGapRegressionTest() {}

void Phase6FinalGapRegressionTest::initTestCase()
{
	QVERIFY(TestEnvironment::getInstance().init());
}

void Phase6FinalGapRegressionTest::cleanupTestCase()
{
	TestEnvironment::getInstance().cleanup();
}

// ============================================================================
// Task #1: Clipboard - Paste HTML/RTF and Copy/Cut/Paste Binary
// ============================================================================

void Phase6FinalGapRegressionTest::testPasteAsHtmlCommandId()
{
	QVERIFY(CMD_EDIT_PASTE_AS_HTML > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_PASTE_AS_HTML), static_cast<int>(IDM_EDIT_PASTE_AS_HTML));
}

void Phase6FinalGapRegressionTest::testPasteAsRtfCommandId()
{
	QVERIFY(CMD_EDIT_PASTE_AS_RTF > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_PASTE_AS_RTF), static_cast<int>(IDM_EDIT_PASTE_AS_RTF));
}

void Phase6FinalGapRegressionTest::testCopyBinaryCommandId()
{
	QVERIFY(CMD_EDIT_COPY_BINARY > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_COPY_BINARY), static_cast<int>(IDM_EDIT_COPY_BINARY));
}

void Phase6FinalGapRegressionTest::testCutBinaryCommandId()
{
	QVERIFY(CMD_EDIT_CUT_BINARY > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_CUT_BINARY), static_cast<int>(IDM_EDIT_CUT_BINARY));
}

void Phase6FinalGapRegressionTest::testPasteBinaryCommandId()
{
	QVERIFY(CMD_EDIT_PASTE_BINARY > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_PASTE_BINARY), static_cast<int>(IDM_EDIT_PASTE_BINARY));
}

void Phase6FinalGapRegressionTest::testClipboardCommandIdsUnique()
{
	std::set<int> clipboardIds = {
		CMD_EDIT_PASTE_AS_HTML,
		CMD_EDIT_PASTE_AS_RTF,
		CMD_EDIT_COPY_BINARY,
		CMD_EDIT_CUT_BINARY,
		CMD_EDIT_PASTE_BINARY,
	};
	QCOMPARE(clipboardIds.size(), static_cast<size_t>(5));
}

void Phase6FinalGapRegressionTest::testClipboardCommandRegistration()
{
	CommandHandler handler;
	std::set<int> executed;

	int clipboardIds[] = {
		CMD_EDIT_PASTE_AS_HTML,
		CMD_EDIT_PASTE_AS_RTF,
		CMD_EDIT_COPY_BINARY,
		CMD_EDIT_CUT_BINARY,
		CMD_EDIT_PASTE_BINARY,
	};

	for (int id : clipboardIds)
	{
		handler.registerCommand(id, [&executed, id]() { executed.insert(id); });
	}

	for (int id : clipboardIds)
	{
		QVERIFY(handler.canExecute(id));
		handler.executeCommand(id);
	}

	QCOMPARE(executed.size(), static_cast<size_t>(5));
}

// ============================================================================
// Task #2: Browser - Search on Internet and View in Browser
// ============================================================================

void Phase6FinalGapRegressionTest::testSearchOnInternetCommandId()
{
	QVERIFY(CMD_EDIT_SEARCHONINTERNET > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_SEARCHONINTERNET), static_cast<int>(IDM_EDIT_SEARCHONINTERNET));
}

void Phase6FinalGapRegressionTest::testOpenDefaultViewerCommandId()
{
	// CMD_FILE_OPEN_DEFAULT_VIEWER already exists in the enum (41023)
	QVERIFY(CMD_FILE_OPEN_DEFAULT_VIEWER > 0);
	QCOMPARE(static_cast<int>(CMD_FILE_OPEN_DEFAULT_VIEWER), static_cast<int>(IDM_FILE_OPEN_DEFAULT_VIEWER));
}

void Phase6FinalGapRegressionTest::testBrowserCommandRegistration()
{
	CommandHandler handler;
	bool searchExecuted = false;
	bool viewerExecuted = false;

	handler.registerCommand(CMD_EDIT_SEARCHONINTERNET, [&searchExecuted]() { searchExecuted = true; });
	handler.registerCommand(CMD_FILE_OPEN_DEFAULT_VIEWER, [&viewerExecuted]() { viewerExecuted = true; });

	QVERIFY(handler.canExecute(CMD_EDIT_SEARCHONINTERNET));
	QVERIFY(handler.canExecute(CMD_FILE_OPEN_DEFAULT_VIEWER));

	handler.executeCommand(CMD_EDIT_SEARCHONINTERNET);
	handler.executeCommand(CMD_FILE_OPEN_DEFAULT_VIEWER);

	QVERIFY(searchExecuted);
	QVERIFY(viewerExecuted);
}

// ============================================================================
// Task #3: View - Always on Top, Tab Coloring, RTL/LTR
// ============================================================================

void Phase6FinalGapRegressionTest::testAlwaysOnTopCommandId()
{
	QVERIFY(CMD_VIEW_ALWAYSONTOP > 0);
	QCOMPARE(static_cast<int>(CMD_VIEW_ALWAYSONTOP), static_cast<int>(IDM_VIEW_ALWAYSONTOP));
}

void Phase6FinalGapRegressionTest::testTabColourCommandIds()
{
	QVERIFY(CMD_VIEW_TAB_COLOUR_NONE > 0);
	QVERIFY(CMD_VIEW_TAB_COLOUR_1 > 0);
	QVERIFY(CMD_VIEW_TAB_COLOUR_2 > 0);
	QVERIFY(CMD_VIEW_TAB_COLOUR_3 > 0);
	QVERIFY(CMD_VIEW_TAB_COLOUR_4 > 0);
	QVERIFY(CMD_VIEW_TAB_COLOUR_5 > 0);

	QCOMPARE(static_cast<int>(CMD_VIEW_TAB_COLOUR_NONE), static_cast<int>(IDM_VIEW_TAB_COLOUR_NONE));
	QCOMPARE(static_cast<int>(CMD_VIEW_TAB_COLOUR_1), static_cast<int>(IDM_VIEW_TAB_COLOUR_1));
	QCOMPARE(static_cast<int>(CMD_VIEW_TAB_COLOUR_2), static_cast<int>(IDM_VIEW_TAB_COLOUR_2));
	QCOMPARE(static_cast<int>(CMD_VIEW_TAB_COLOUR_3), static_cast<int>(IDM_VIEW_TAB_COLOUR_3));
	QCOMPARE(static_cast<int>(CMD_VIEW_TAB_COLOUR_4), static_cast<int>(IDM_VIEW_TAB_COLOUR_4));
	QCOMPARE(static_cast<int>(CMD_VIEW_TAB_COLOUR_5), static_cast<int>(IDM_VIEW_TAB_COLOUR_5));
}

void Phase6FinalGapRegressionTest::testTabColourIdsUnique()
{
	std::set<int> tabColourIds = {
		CMD_VIEW_TAB_COLOUR_NONE,
		CMD_VIEW_TAB_COLOUR_1,
		CMD_VIEW_TAB_COLOUR_2,
		CMD_VIEW_TAB_COLOUR_3,
		CMD_VIEW_TAB_COLOUR_4,
		CMD_VIEW_TAB_COLOUR_5,
	};
	QCOMPARE(tabColourIds.size(), static_cast<size_t>(6));
}

void Phase6FinalGapRegressionTest::testRtlLtrCommandIds()
{
	QVERIFY(CMD_EDIT_RTL > 0);
	QVERIFY(CMD_EDIT_LTR > 0);

	QCOMPARE(static_cast<int>(CMD_EDIT_RTL), static_cast<int>(IDM_EDIT_RTL));
	QCOMPARE(static_cast<int>(CMD_EDIT_LTR), static_cast<int>(IDM_EDIT_LTR));
}

void Phase6FinalGapRegressionTest::testRtlLtrIdsDistinct()
{
	QVERIFY(CMD_EDIT_RTL != CMD_EDIT_LTR);
}

void Phase6FinalGapRegressionTest::testViewFeatureCommandRegistration()
{
	CommandHandler handler;
	std::set<int> executed;

	int viewIds[] = {
		CMD_VIEW_ALWAYSONTOP,
		CMD_VIEW_TAB_COLOUR_NONE,
		CMD_VIEW_TAB_COLOUR_1,
		CMD_VIEW_TAB_COLOUR_2,
		CMD_VIEW_TAB_COLOUR_3,
		CMD_VIEW_TAB_COLOUR_4,
		CMD_VIEW_TAB_COLOUR_5,
		CMD_EDIT_RTL,
		CMD_EDIT_LTR,
	};

	for (int id : viewIds)
	{
		handler.registerCommand(id, [&executed, id]() { executed.insert(id); });
	}

	for (int id : viewIds)
	{
		QVERIFY(handler.canExecute(id));
		handler.executeCommand(id);
	}

	QCOMPARE(executed.size(), static_cast<size_t>(9));
}

// ============================================================================
// Task #4: Plugin Notifications and Change History / Hide Lines
// ============================================================================

void Phase6FinalGapRegressionTest::testAllNppnConstantsDefined()
{
	// All 32 NPPN_* notification constants must be defined and positive
	QVERIFY(NPPN_READY > 0);
	QVERIFY(NPPN_TBMODIFICATION > 0);
	QVERIFY(NPPN_FILEBEFORECLOSE > 0);
	QVERIFY(NPPN_FILEOPENED > 0);
	QVERIFY(NPPN_FILECLOSED > 0);
	QVERIFY(NPPN_FILEBEFOREOPEN > 0);
	QVERIFY(NPPN_FILEBEFORESAVE > 0);
	QVERIFY(NPPN_FILESAVED > 0);
	QVERIFY(NPPN_SHUTDOWN > 0);
	QVERIFY(NPPN_BUFFERACTIVATED > 0);
	QVERIFY(NPPN_LANGCHANGED > 0);
	QVERIFY(NPPN_WORDSTYLESUPDATED > 0);
	QVERIFY(NPPN_SHORTCUTREMAPPED > 0);
	QVERIFY(NPPN_FILEBEFORELOAD > 0);
	QVERIFY(NPPN_FILELOADFAILED > 0);
	QVERIFY(NPPN_READONLYCHANGED > 0);
	QVERIFY(NPPN_DOCORDERCHANGED > 0);
	QVERIFY(NPPN_SNAPSHOTDIRTYFILELOADED > 0);
	QVERIFY(NPPN_BEFORESHUTDOWN > 0);
	QVERIFY(NPPN_CANCELSHUTDOWN > 0);
	QVERIFY(NPPN_FILEBEFORERENAME > 0);
	QVERIFY(NPPN_FILERENAMECANCEL > 0);
	QVERIFY(NPPN_FILERENAMED > 0);
	QVERIFY(NPPN_FILEBEFOREDELETE > 0);
	QVERIFY(NPPN_FILEDELETEFAILED > 0);
	QVERIFY(NPPN_FILEDELETED > 0);
	QVERIFY(NPPN_DARKMODECHANGED > 0);
	QVERIFY(NPPN_CMDLINEPLUGINMSG > 0);
	QVERIFY(NPPN_EXTERNALLEXERBUFFER > 0);
	QVERIFY(NPPN_GLOBALMODIFIED > 0);
	QVERIFY(NPPN_NATIVELANGCHANGED > 0);
	QVERIFY(NPPN_TOOLBARICONSETCHANGED > 0);
}

void Phase6FinalGapRegressionTest::testNppnConstantsAreUnique()
{
	std::set<int> nppnIds = {
		NPPN_READY,
		NPPN_TBMODIFICATION,
		NPPN_FILEBEFORECLOSE,
		NPPN_FILEOPENED,
		NPPN_FILECLOSED,
		NPPN_FILEBEFOREOPEN,
		NPPN_FILEBEFORESAVE,
		NPPN_FILESAVED,
		NPPN_SHUTDOWN,
		NPPN_BUFFERACTIVATED,
		NPPN_LANGCHANGED,
		NPPN_WORDSTYLESUPDATED,
		NPPN_SHORTCUTREMAPPED,
		NPPN_FILEBEFORELOAD,
		NPPN_FILELOADFAILED,
		NPPN_READONLYCHANGED,
		NPPN_DOCORDERCHANGED,
		NPPN_SNAPSHOTDIRTYFILELOADED,
		NPPN_BEFORESHUTDOWN,
		NPPN_CANCELSHUTDOWN,
		NPPN_FILEBEFORERENAME,
		NPPN_FILERENAMECANCEL,
		NPPN_FILERENAMED,
		NPPN_FILEBEFOREDELETE,
		NPPN_FILEDELETEFAILED,
		NPPN_FILEDELETED,
		NPPN_DARKMODECHANGED,
		NPPN_CMDLINEPLUGINMSG,
		NPPN_EXTERNALLEXERBUFFER,
		NPPN_GLOBALMODIFIED,
		NPPN_NATIVELANGCHANGED,
		NPPN_TOOLBARICONSETCHANGED,
	};
	QCOMPARE(nppnIds.size(), static_cast<size_t>(32));
}

void Phase6FinalGapRegressionTest::testNppnConstantsInRange()
{
	// All NPPN_* constants should be in the range NPPN_FIRST+1 to NPPN_FIRST+32
	int rangeStart = NPPN_FIRST + 1;
	int rangeEnd = NPPN_FIRST + 33; // exclusive

	QVERIFY(NPPN_READY >= rangeStart && NPPN_READY < rangeEnd);
	QVERIFY(NPPN_TBMODIFICATION >= rangeStart && NPPN_TBMODIFICATION < rangeEnd);
	QVERIFY(NPPN_FILEBEFORECLOSE >= rangeStart && NPPN_FILEBEFORECLOSE < rangeEnd);
	QVERIFY(NPPN_FILEOPENED >= rangeStart && NPPN_FILEOPENED < rangeEnd);
	QVERIFY(NPPN_FILECLOSED >= rangeStart && NPPN_FILECLOSED < rangeEnd);
	QVERIFY(NPPN_FILEBEFOREOPEN >= rangeStart && NPPN_FILEBEFOREOPEN < rangeEnd);
	QVERIFY(NPPN_FILEBEFORESAVE >= rangeStart && NPPN_FILEBEFORESAVE < rangeEnd);
	QVERIFY(NPPN_FILESAVED >= rangeStart && NPPN_FILESAVED < rangeEnd);
	QVERIFY(NPPN_SHUTDOWN >= rangeStart && NPPN_SHUTDOWN < rangeEnd);
	QVERIFY(NPPN_BUFFERACTIVATED >= rangeStart && NPPN_BUFFERACTIVATED < rangeEnd);
	QVERIFY(NPPN_LANGCHANGED >= rangeStart && NPPN_LANGCHANGED < rangeEnd);
	QVERIFY(NPPN_WORDSTYLESUPDATED >= rangeStart && NPPN_WORDSTYLESUPDATED < rangeEnd);
	QVERIFY(NPPN_SHORTCUTREMAPPED >= rangeStart && NPPN_SHORTCUTREMAPPED < rangeEnd);
	QVERIFY(NPPN_FILEBEFORELOAD >= rangeStart && NPPN_FILEBEFORELOAD < rangeEnd);
	QVERIFY(NPPN_FILELOADFAILED >= rangeStart && NPPN_FILELOADFAILED < rangeEnd);
	QVERIFY(NPPN_READONLYCHANGED >= rangeStart && NPPN_READONLYCHANGED < rangeEnd);
	QVERIFY(NPPN_DOCORDERCHANGED >= rangeStart && NPPN_DOCORDERCHANGED < rangeEnd);
	QVERIFY(NPPN_SNAPSHOTDIRTYFILELOADED >= rangeStart && NPPN_SNAPSHOTDIRTYFILELOADED < rangeEnd);
	QVERIFY(NPPN_BEFORESHUTDOWN >= rangeStart && NPPN_BEFORESHUTDOWN < rangeEnd);
	QVERIFY(NPPN_CANCELSHUTDOWN >= rangeStart && NPPN_CANCELSHUTDOWN < rangeEnd);
	QVERIFY(NPPN_FILEBEFORERENAME >= rangeStart && NPPN_FILEBEFORERENAME < rangeEnd);
	QVERIFY(NPPN_FILERENAMECANCEL >= rangeStart && NPPN_FILERENAMECANCEL < rangeEnd);
	QVERIFY(NPPN_FILERENAMED >= rangeStart && NPPN_FILERENAMED < rangeEnd);
	QVERIFY(NPPN_FILEBEFOREDELETE >= rangeStart && NPPN_FILEBEFOREDELETE < rangeEnd);
	QVERIFY(NPPN_FILEDELETEFAILED >= rangeStart && NPPN_FILEDELETEFAILED < rangeEnd);
	QVERIFY(NPPN_FILEDELETED >= rangeStart && NPPN_FILEDELETED < rangeEnd);
	QVERIFY(NPPN_DARKMODECHANGED >= rangeStart && NPPN_DARKMODECHANGED < rangeEnd);
	QVERIFY(NPPN_CMDLINEPLUGINMSG >= rangeStart && NPPN_CMDLINEPLUGINMSG < rangeEnd);
	QVERIFY(NPPN_EXTERNALLEXERBUFFER >= rangeStart && NPPN_EXTERNALLEXERBUFFER < rangeEnd);
	QVERIFY(NPPN_GLOBALMODIFIED >= rangeStart && NPPN_GLOBALMODIFIED < rangeEnd);
	QVERIFY(NPPN_NATIVELANGCHANGED >= rangeStart && NPPN_NATIVELANGCHANGED < rangeEnd);
	QVERIFY(NPPN_TOOLBARICONSETCHANGED >= rangeStart && NPPN_TOOLBARICONSETCHANGED < rangeEnd);
}

void Phase6FinalGapRegressionTest::testChangeHistoryCommandIds()
{
	QVERIFY(CMD_SEARCH_CHANGED_NEXT > 0);
	QVERIFY(CMD_SEARCH_CHANGED_PREV > 0);
	QVERIFY(CMD_SEARCH_CLEAR_CHANGE_HISTORY > 0);

	QCOMPARE(static_cast<int>(CMD_SEARCH_CHANGED_NEXT), static_cast<int>(IDM_SEARCH_CHANGED_NEXT));
	QCOMPARE(static_cast<int>(CMD_SEARCH_CHANGED_PREV), static_cast<int>(IDM_SEARCH_CHANGED_PREV));
	QCOMPARE(static_cast<int>(CMD_SEARCH_CLEAR_CHANGE_HISTORY), static_cast<int>(IDM_SEARCH_CLEAR_CHANGE_HISTORY));

	// All three must be distinct
	std::set<int> changeIds = {
		CMD_SEARCH_CHANGED_NEXT,
		CMD_SEARCH_CHANGED_PREV,
		CMD_SEARCH_CLEAR_CHANGE_HISTORY,
	};
	QCOMPARE(changeIds.size(), static_cast<size_t>(3));
}

void Phase6FinalGapRegressionTest::testHideLinesCommandId()
{
	QVERIFY(CMD_VIEW_HIDELINES > 0);
	QCOMPARE(static_cast<int>(CMD_VIEW_HIDELINES), static_cast<int>(IDM_VIEW_HIDELINES));
}

void Phase6FinalGapRegressionTest::testChangeHistoryCommandRegistration()
{
	CommandHandler handler;
	std::set<int> executed;

	int changeIds[] = {
		CMD_SEARCH_CHANGED_NEXT,
		CMD_SEARCH_CHANGED_PREV,
		CMD_SEARCH_CLEAR_CHANGE_HISTORY,
	};

	for (int id : changeIds)
	{
		handler.registerCommand(id, [&executed, id]() { executed.insert(id); });
	}

	for (int id : changeIds)
	{
		QVERIFY(handler.canExecute(id));
		handler.executeCommand(id);
	}

	QCOMPARE(executed.size(), static_cast<size_t>(3));
}

// ============================================================================
// Cross-cutting: New Command ID Range Validation
// ============================================================================

void Phase6FinalGapRegressionTest::testNewEditCommandIdsInEditRange()
{
	// Edit commands should be in 42000-42999 range
	int editBase = 42000;
	int editEnd = 43000;

	QVERIFY(CMD_EDIT_PASTE_AS_HTML >= editBase && CMD_EDIT_PASTE_AS_HTML < editEnd);
	QVERIFY(CMD_EDIT_PASTE_AS_RTF >= editBase && CMD_EDIT_PASTE_AS_RTF < editEnd);
	QVERIFY(CMD_EDIT_COPY_BINARY >= editBase && CMD_EDIT_COPY_BINARY < editEnd);
	QVERIFY(CMD_EDIT_CUT_BINARY >= editBase && CMD_EDIT_CUT_BINARY < editEnd);
	QVERIFY(CMD_EDIT_PASTE_BINARY >= editBase && CMD_EDIT_PASTE_BINARY < editEnd);
	QVERIFY(CMD_EDIT_SEARCHONINTERNET >= editBase && CMD_EDIT_SEARCHONINTERNET < editEnd);
	QVERIFY(CMD_EDIT_RTL >= editBase && CMD_EDIT_RTL < editEnd);
	QVERIFY(CMD_EDIT_LTR >= editBase && CMD_EDIT_LTR < editEnd);
}

void Phase6FinalGapRegressionTest::testNewViewCommandIdsInViewRange()
{
	// View commands should be in 44000-44999 range
	int viewBase = 44000;
	int viewEnd = 45000;

	QVERIFY(CMD_VIEW_TAB_COLOUR_NONE >= viewBase && CMD_VIEW_TAB_COLOUR_NONE < viewEnd);
	QVERIFY(CMD_VIEW_TAB_COLOUR_1 >= viewBase && CMD_VIEW_TAB_COLOUR_1 < viewEnd);
	QVERIFY(CMD_VIEW_TAB_COLOUR_2 >= viewBase && CMD_VIEW_TAB_COLOUR_2 < viewEnd);
	QVERIFY(CMD_VIEW_TAB_COLOUR_3 >= viewBase && CMD_VIEW_TAB_COLOUR_3 < viewEnd);
	QVERIFY(CMD_VIEW_TAB_COLOUR_4 >= viewBase && CMD_VIEW_TAB_COLOUR_4 < viewEnd);
	QVERIFY(CMD_VIEW_TAB_COLOUR_5 >= viewBase && CMD_VIEW_TAB_COLOUR_5 < viewEnd);
	QVERIFY(CMD_VIEW_ALWAYSONTOP >= viewBase && CMD_VIEW_ALWAYSONTOP < viewEnd);
	QVERIFY(CMD_VIEW_HIDELINES >= viewBase && CMD_VIEW_HIDELINES < viewEnd);
}

} // namespace Tests

#include "Phase6FinalGapRegressionTest.moc"
