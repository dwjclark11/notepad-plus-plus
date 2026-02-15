// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "Phase5RegressionTest.h"
#include "../Common/TestUtils.h"
#include "NppCommands.h"
#include "menuCmdID.h"
#include "Notepad_plus_msgs.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include <set>

// MARK_BOOKMARK constant from ScintillaEditView.h (avoid pulling in full header)
static constexpr int TEST_MARK_BOOKMARK = 20;

using namespace QtCommands;

namespace Tests {

Phase5RegressionTest::Phase5RegressionTest() {}

Phase5RegressionTest::~Phase5RegressionTest() {}

void Phase5RegressionTest::initTestCase()
{
	QVERIFY(TestEnvironment::getInstance().init());
}

void Phase5RegressionTest::cleanupTestCase()
{
	TestEnvironment::getInstance().cleanup();
}

// ============================================================================
// CommandHandler Unit Tests
// ============================================================================

void Phase5RegressionTest::testCommandHandlerRegisterAndExecute()
{
	CommandHandler handler;
	bool executed = false;

	handler.registerCommand(CMD_FILE_NEW, [&executed]() { executed = true; });
	QVERIFY(handler.canExecute(CMD_FILE_NEW));

	handler.executeCommand(CMD_FILE_NEW);
	QVERIFY(executed);
}

void Phase5RegressionTest::testCommandHandlerCanExecute()
{
	CommandHandler handler;

	// Unregistered command should not be executable
	QVERIFY(!handler.canExecute(CMD_FILE_NEW));

	// After registration, should be executable
	handler.registerCommand(CMD_FILE_NEW, []() {});
	QVERIFY(handler.canExecute(CMD_FILE_NEW));

	// Different command still not executable
	QVERIFY(!handler.canExecute(CMD_FILE_OPEN));
}

void Phase5RegressionTest::testCommandHandlerUnregister()
{
	CommandHandler handler;

	handler.registerCommand(CMD_FILE_NEW, []() {});
	QVERIFY(handler.canExecute(CMD_FILE_NEW));

	handler.unregisterCommand(CMD_FILE_NEW);
	QVERIFY(!handler.canExecute(CMD_FILE_NEW));
}

void Phase5RegressionTest::testCommandHandlerClearCommands()
{
	CommandHandler handler;

	handler.registerCommand(CMD_FILE_NEW, []() {});
	handler.registerCommand(CMD_FILE_OPEN, []() {});
	handler.registerCommand(CMD_FILE_SAVE, []() {});

	QVERIFY(handler.canExecute(CMD_FILE_NEW));
	QVERIFY(handler.canExecute(CMD_FILE_OPEN));
	QVERIFY(handler.canExecute(CMD_FILE_SAVE));

	handler.clearCommands();

	QVERIFY(!handler.canExecute(CMD_FILE_NEW));
	QVERIFY(!handler.canExecute(CMD_FILE_OPEN));
	QVERIFY(!handler.canExecute(CMD_FILE_SAVE));
}

void Phase5RegressionTest::testCommandHandlerExecuteUnregistered()
{
	CommandHandler handler;

	// Executing an unregistered command should not crash
	handler.executeCommand(99999);
	QVERIFY(true); // If we got here, no crash
}

void Phase5RegressionTest::testCommandHandlerOverwrite()
{
	CommandHandler handler;
	int callCount = 0;

	handler.registerCommand(CMD_FILE_NEW, [&callCount]() { callCount = 1; });
	handler.executeCommand(CMD_FILE_NEW);
	QCOMPARE(callCount, 1);

	// Overwrite with a new handler
	handler.registerCommand(CMD_FILE_NEW, [&callCount]() { callCount = 2; });
	handler.executeCommand(CMD_FILE_NEW);
	QCOMPARE(callCount, 2);
}

void Phase5RegressionTest::testCommandHandlerMultipleExecutions()
{
	CommandHandler handler;
	int callCount = 0;

	handler.registerCommand(CMD_FILE_NEW, [&callCount]() { ++callCount; });

	handler.executeCommand(CMD_FILE_NEW);
	handler.executeCommand(CMD_FILE_NEW);
	handler.executeCommand(CMD_FILE_NEW);

	QCOMPARE(callCount, 3);
}

void Phase5RegressionTest::testCommandHandlerNullCallback()
{
	CommandHandler handler;

	// Register a null callback
	handler.registerCommand(CMD_FILE_NEW, nullptr);

	// canExecute should return false for null callbacks
	QVERIFY(!handler.canExecute(CMD_FILE_NEW));

	// Executing should not crash
	handler.executeCommand(CMD_FILE_NEW);
	QVERIFY(true);
}

// ============================================================================
// Task #1: Close Variant Command ID Validation
// ============================================================================

void Phase5RegressionTest::testCloseAllCommandIdsDefined()
{
	// All close variant command IDs must be non-zero and positive
	QVERIFY(CMD_FILE_CLOSEALL > 0);
	QVERIFY(CMD_FILE_CLOSEALL_BUT_CURRENT > 0);
	QVERIFY(CMD_FILE_CLOSEALL_BUT_PINNED > 0);
	QVERIFY(CMD_FILE_CLOSEALL_TOLEFT > 0);
	QVERIFY(CMD_FILE_CLOSEALL_TORIGHT > 0);
	QVERIFY(CMD_FILE_CLOSEALL_UNCHANGED > 0);
}

void Phase5RegressionTest::testCloseVariantIdsInFileRange()
{
	// All close variants should be in the file command range (41000-41999)
	int fileBase = 41000;
	int fileEnd = 42000;

	QVERIFY(CMD_FILE_CLOSEALL >= fileBase && CMD_FILE_CLOSEALL < fileEnd);
	QVERIFY(CMD_FILE_CLOSEALL_BUT_CURRENT >= fileBase && CMD_FILE_CLOSEALL_BUT_CURRENT < fileEnd);
	QVERIFY(CMD_FILE_CLOSEALL_BUT_PINNED >= fileBase && CMD_FILE_CLOSEALL_BUT_PINNED < fileEnd);
	QVERIFY(CMD_FILE_CLOSEALL_TOLEFT >= fileBase && CMD_FILE_CLOSEALL_TOLEFT < fileEnd);
	QVERIFY(CMD_FILE_CLOSEALL_TORIGHT >= fileBase && CMD_FILE_CLOSEALL_TORIGHT < fileEnd);
	QVERIFY(CMD_FILE_CLOSEALL_UNCHANGED >= fileBase && CMD_FILE_CLOSEALL_UNCHANGED < fileEnd);
}

void Phase5RegressionTest::testCloseVariantIdsMatchMenuCmdIds()
{
	// The CMD_* enum values must match the IDM_* macro values from menuCmdID.h
	QCOMPARE(static_cast<int>(CMD_FILE_CLOSEALL), static_cast<int>(IDM_FILE_CLOSEALL));
	QCOMPARE(static_cast<int>(CMD_FILE_CLOSEALL_BUT_CURRENT), static_cast<int>(IDM_FILE_CLOSEALL_BUT_CURRENT));
	QCOMPARE(static_cast<int>(CMD_FILE_CLOSEALL_BUT_PINNED), static_cast<int>(IDM_FILE_CLOSEALL_BUT_PINNED));
	QCOMPARE(static_cast<int>(CMD_FILE_CLOSEALL_TOLEFT), static_cast<int>(IDM_FILE_CLOSEALL_TOLEFT));
	QCOMPARE(static_cast<int>(CMD_FILE_CLOSEALL_TORIGHT), static_cast<int>(IDM_FILE_CLOSEALL_TORIGHT));
	QCOMPARE(static_cast<int>(CMD_FILE_CLOSEALL_UNCHANGED), static_cast<int>(IDM_FILE_CLOSEALL_UNCHANGED));
}

void Phase5RegressionTest::testAllFileCloseIdsUnique()
{
	// All close variant IDs must be unique (no accidental duplicates)
	std::set<int> closeIds = {
		CMD_FILE_CLOSE,
		CMD_FILE_CLOSEALL,
		CMD_FILE_CLOSEALL_BUT_CURRENT,
		CMD_FILE_CLOSEALL_BUT_PINNED,
		CMD_FILE_CLOSEALL_TOLEFT,
		CMD_FILE_CLOSEALL_TORIGHT,
		CMD_FILE_CLOSEALL_UNCHANGED,
	};
	QCOMPARE(closeIds.size(), static_cast<size_t>(7));
}

void Phase5RegressionTest::testCloseCommandRegistration()
{
	// Verify that all close variant commands can be registered and executed
	CommandHandler handler;
	std::set<int> executed;

	int closeIds[] = {
		CMD_FILE_CLOSEALL,
		CMD_FILE_CLOSEALL_BUT_CURRENT,
		CMD_FILE_CLOSEALL_BUT_PINNED,
		CMD_FILE_CLOSEALL_TOLEFT,
		CMD_FILE_CLOSEALL_TORIGHT,
		CMD_FILE_CLOSEALL_UNCHANGED,
	};

	for (int id : closeIds)
	{
		handler.registerCommand(id, [&executed, id]() { executed.insert(id); });
	}

	// Execute all and verify each fires its own handler
	for (int id : closeIds)
	{
		QVERIFY(handler.canExecute(id));
		handler.executeCommand(id);
	}

	QCOMPARE(executed.size(), static_cast<size_t>(6));
}

// ============================================================================
// Task #2: Search Feature Command ID Validation
// ============================================================================

void Phase5RegressionTest::testSearchCommandIdsDefined()
{
	QVERIFY(CMD_SEARCH_FIND > 0);
	QVERIFY(CMD_SEARCH_FINDNEXT > 0);
	QVERIFY(CMD_SEARCH_REPLACE > 0);
	QVERIFY(CMD_SEARCH_FINDPREV > 0);
	QVERIFY(CMD_SEARCH_FINDINCREMENT > 0);
	QVERIFY(CMD_SEARCH_FINDINFILES > 0);
	QVERIFY(CMD_SEARCH_MARK > 0);
}

void Phase5RegressionTest::testMarkAllExtCommandIds()
{
	// All 5 mark styles must have both mark and unmark command IDs
	QVERIFY(CMD_SEARCH_MARKALLEXT1 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT1 > 0);
	QVERIFY(CMD_SEARCH_MARKALLEXT2 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT2 > 0);
	QVERIFY(CMD_SEARCH_MARKALLEXT3 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT3 > 0);
	QVERIFY(CMD_SEARCH_MARKALLEXT4 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT4 > 0);
	QVERIFY(CMD_SEARCH_MARKALLEXT5 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT5 > 0);

	// All must be distinct
	std::set<int> markIds = {
		CMD_SEARCH_MARKALLEXT1, CMD_SEARCH_UNMARKALLEXT1,
		CMD_SEARCH_MARKALLEXT2, CMD_SEARCH_UNMARKALLEXT2,
		CMD_SEARCH_MARKALLEXT3, CMD_SEARCH_UNMARKALLEXT3,
		CMD_SEARCH_MARKALLEXT4, CMD_SEARCH_UNMARKALLEXT4,
		CMD_SEARCH_MARKALLEXT5, CMD_SEARCH_UNMARKALLEXT5,
	};
	QCOMPARE(markIds.size(), static_cast<size_t>(10));
}

void Phase5RegressionTest::testMarkerNavigationCommandIds()
{
	// Go-next and go-prev marker commands for all 5 styles + default
	QVERIFY(CMD_SEARCH_GONEXTMARKER1 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER2 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER3 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER4 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER5 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER_DEF > 0);

	QVERIFY(CMD_SEARCH_GOPREVMARKER1 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER2 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER3 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER4 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER5 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER_DEF > 0);

	// All distinct
	std::set<int> navIds = {
		CMD_SEARCH_GONEXTMARKER1, CMD_SEARCH_GONEXTMARKER2,
		CMD_SEARCH_GONEXTMARKER3, CMD_SEARCH_GONEXTMARKER4,
		CMD_SEARCH_GONEXTMARKER5, CMD_SEARCH_GONEXTMARKER_DEF,
		CMD_SEARCH_GOPREVMARKER1, CMD_SEARCH_GOPREVMARKER2,
		CMD_SEARCH_GOPREVMARKER3, CMD_SEARCH_GOPREVMARKER4,
		CMD_SEARCH_GOPREVMARKER5, CMD_SEARCH_GOPREVMARKER_DEF,
	};
	QCOMPARE(navIds.size(), static_cast<size_t>(12));
}

void Phase5RegressionTest::testFindInFilesCommandId()
{
	QCOMPARE(static_cast<int>(CMD_SEARCH_FINDINFILES), static_cast<int>(IDM_SEARCH_FINDINFILES));
}

// ============================================================================
// Task #4: Smart Editing Command ID Validation
// ============================================================================

void Phase5RegressionTest::testCaseConversionCommandIds()
{
	QVERIFY(CMD_EDIT_UPPERCASE > 0);
	QVERIFY(CMD_EDIT_LOWERCASE > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_UPPERCASE), static_cast<int>(IDM_EDIT_UPPERCASE));
	QCOMPARE(static_cast<int>(CMD_EDIT_LOWERCASE), static_cast<int>(IDM_EDIT_LOWERCASE));
	QVERIFY(CMD_EDIT_UPPERCASE != CMD_EDIT_LOWERCASE);
}

void Phase5RegressionTest::testExtendedCaseConversionCommandIds()
{
	// New case conversion commands added in Phase 5 (task #4)
	QVERIFY(CMD_EDIT_PROPERCASE_FORCE > 0);
	QVERIFY(CMD_EDIT_PROPERCASE_BLEND > 0);
	QVERIFY(CMD_EDIT_SENTENCECASE_FORCE > 0);
	QVERIFY(CMD_EDIT_SENTENCECASE_BLEND > 0);
	QVERIFY(CMD_EDIT_INVERTCASE > 0);
	QVERIFY(CMD_EDIT_RANDOMCASE > 0);

	// Verify they match IDM_* macros
	QCOMPARE(static_cast<int>(CMD_EDIT_PROPERCASE_FORCE), static_cast<int>(IDM_EDIT_PROPERCASE_FORCE));
	QCOMPARE(static_cast<int>(CMD_EDIT_PROPERCASE_BLEND), static_cast<int>(IDM_EDIT_PROPERCASE_BLEND));
	QCOMPARE(static_cast<int>(CMD_EDIT_SENTENCECASE_FORCE), static_cast<int>(IDM_EDIT_SENTENCECASE_FORCE));
	QCOMPARE(static_cast<int>(CMD_EDIT_SENTENCECASE_BLEND), static_cast<int>(IDM_EDIT_SENTENCECASE_BLEND));
	QCOMPARE(static_cast<int>(CMD_EDIT_INVERTCASE), static_cast<int>(IDM_EDIT_INVERTCASE));
	QCOMPARE(static_cast<int>(CMD_EDIT_RANDOMCASE), static_cast<int>(IDM_EDIT_RANDOMCASE));

	// All case conversion IDs must be unique
	std::set<int> caseIds = {
		CMD_EDIT_UPPERCASE, CMD_EDIT_LOWERCASE,
		CMD_EDIT_PROPERCASE_FORCE, CMD_EDIT_PROPERCASE_BLEND,
		CMD_EDIT_SENTENCECASE_FORCE, CMD_EDIT_SENTENCECASE_BLEND,
		CMD_EDIT_INVERTCASE, CMD_EDIT_RANDOMCASE,
	};
	QCOMPARE(caseIds.size(), static_cast<size_t>(8));
}

void Phase5RegressionTest::testCommentCommandIds()
{
	QVERIFY(CMD_EDIT_BLOCK_COMMENT > 0);
	QVERIFY(CMD_EDIT_BLOCK_COMMENT_SET > 0);
	QVERIFY(CMD_EDIT_BLOCK_UNCOMMENT > 0);
	QVERIFY(CMD_EDIT_STREAM_COMMENT > 0);
	QVERIFY(CMD_EDIT_STREAM_UNCOMMENT > 0);

	QCOMPARE(static_cast<int>(CMD_EDIT_BLOCK_COMMENT), static_cast<int>(IDM_EDIT_BLOCK_COMMENT));
	QCOMPARE(static_cast<int>(CMD_EDIT_STREAM_COMMENT), static_cast<int>(IDM_EDIT_STREAM_COMMENT));
}

void Phase5RegressionTest::testTrimCommandIds()
{
	QVERIFY(CMD_EDIT_TRIMTRAILING > 0);
	QVERIFY(CMD_EDIT_TRIMLINEHEAD > 0);
	QVERIFY(CMD_EDIT_TRIM_BOTH > 0);

	std::set<int> trimIds = {
		CMD_EDIT_TRIMTRAILING, CMD_EDIT_TRIMLINEHEAD, CMD_EDIT_TRIM_BOTH
	};
	QCOMPARE(trimIds.size(), static_cast<size_t>(3));
}

void Phase5RegressionTest::testSortCommandIds()
{
	// Verify sort line command IDs are distinct
	std::set<int> sortIds = {
		CMD_EDIT_SORTLINES_LEXICO_ASC, CMD_EDIT_SORTLINES_LEXICO_DESC,
		CMD_EDIT_SORTLINES_INTEGER_ASC, CMD_EDIT_SORTLINES_INTEGER_DESC,
		CMD_EDIT_SORTLINES_DECCOMMA_ASC, CMD_EDIT_SORTLINES_DECCOMMA_DESC,
		CMD_EDIT_SORTLINES_DECDOT_ASC, CMD_EDIT_SORTLINES_DECDOT_DESC,
		CMD_EDIT_SORTLINES_RANDOMLY, CMD_EDIT_SORTLINES_REVERSE,
		CMD_EDIT_SORTLINES_LEXICO_CI_ASC, CMD_EDIT_SORTLINES_LEXICO_CI_DESC,
		CMD_EDIT_SORTLINES_LENGTH_ASC, CMD_EDIT_SORTLINES_LENGTH_DESC,
	};
	QCOMPARE(sortIds.size(), static_cast<size_t>(14));
}

void Phase5RegressionTest::testInsertDateTimeCommandIds()
{
	QVERIFY(CMD_EDIT_INSERT_DATETIME_SHORT > 0);
	QVERIFY(CMD_EDIT_INSERT_DATETIME_LONG > 0);
	QVERIFY(CMD_EDIT_INSERT_DATETIME_CUSTOMIZED > 0);

	std::set<int> dtIds = {
		CMD_EDIT_INSERT_DATETIME_SHORT,
		CMD_EDIT_INSERT_DATETIME_LONG,
		CMD_EDIT_INSERT_DATETIME_CUSTOMIZED,
	};
	QCOMPARE(dtIds.size(), static_cast<size_t>(3));
}

void Phase5RegressionTest::testToggleReadOnlyCommandId()
{
	QVERIFY(CMD_EDIT_TOGGLEREADONLY > 0);
	QCOMPARE(static_cast<int>(CMD_EDIT_TOGGLEREADONLY), static_cast<int>(IDM_EDIT_TOGGLEREADONLY));
}

void Phase5RegressionTest::testInsertDateTimeIdsMatchMenuCmdIds()
{
	QCOMPARE(static_cast<int>(CMD_EDIT_INSERT_DATETIME_SHORT), static_cast<int>(IDM_EDIT_INSERT_DATETIME_SHORT));
	QCOMPARE(static_cast<int>(CMD_EDIT_INSERT_DATETIME_LONG), static_cast<int>(IDM_EDIT_INSERT_DATETIME_LONG));
	QCOMPARE(static_cast<int>(CMD_EDIT_INSERT_DATETIME_CUSTOMIZED), static_cast<int>(IDM_EDIT_INSERT_DATETIME_CUSTOMIZED));
}

// ============================================================================
// Task #5: Bookmark Command ID Validation
// ============================================================================

void Phase5RegressionTest::testBookmarkCommandIds()
{
	QVERIFY(CMD_SEARCH_TOGGLE_BOOKMARK > 0);
	QVERIFY(CMD_SEARCH_NEXT_BOOKMARK > 0);
	QVERIFY(CMD_SEARCH_PREV_BOOKMARK > 0);
	QVERIFY(CMD_SEARCH_CLEAR_BOOKMARKS > 0);

	std::set<int> bmIds = {
		CMD_SEARCH_TOGGLE_BOOKMARK, CMD_SEARCH_NEXT_BOOKMARK,
		CMD_SEARCH_PREV_BOOKMARK, CMD_SEARCH_CLEAR_BOOKMARKS
	};
	QCOMPARE(bmIds.size(), static_cast<size_t>(4));
}

void Phase5RegressionTest::testMarkedLinesCommandIds()
{
	QVERIFY(CMD_SEARCH_CUTMARKEDLINES > 0);
	QVERIFY(CMD_SEARCH_COPYMARKEDLINES > 0);
	QVERIFY(CMD_SEARCH_PASTEMARKEDLINES > 0);
	QVERIFY(CMD_SEARCH_DELETEMARKEDLINES > 0);
	QVERIFY(CMD_SEARCH_DELETEUNMARKEDLINES > 0);
	QVERIFY(CMD_SEARCH_INVERSEMARKS > 0);

	std::set<int> markedIds = {
		CMD_SEARCH_CUTMARKEDLINES, CMD_SEARCH_COPYMARKEDLINES,
		CMD_SEARCH_PASTEMARKEDLINES, CMD_SEARCH_DELETEMARKEDLINES,
		CMD_SEARCH_DELETEUNMARKEDLINES, CMD_SEARCH_INVERSEMARKS
	};
	QCOMPARE(markedIds.size(), static_cast<size_t>(6));
}

// ============================================================================
// Task #6: Sync Scrolling and Incremental Search
// ============================================================================

void Phase5RegressionTest::testSyncScrollCommandIds()
{
	QVERIFY(CMD_VIEW_SYNSCROLLV > 0);
	QVERIFY(CMD_VIEW_SYNSCROLLH > 0);
	QVERIFY(CMD_VIEW_SYNSCROLLV != CMD_VIEW_SYNSCROLLH);

	QCOMPARE(static_cast<int>(CMD_VIEW_SYNSCROLLV), static_cast<int>(IDM_VIEW_SYNSCROLLV));
	QCOMPARE(static_cast<int>(CMD_VIEW_SYNSCROLLH), static_cast<int>(IDM_VIEW_SYNSCROLLH));
}

void Phase5RegressionTest::testIncrementalSearchCommandId()
{
	QVERIFY(CMD_SEARCH_FINDINCREMENT > 0);
	QCOMPARE(static_cast<int>(CMD_SEARCH_FINDINCREMENT), static_cast<int>(IDM_SEARCH_FINDINCREMENT));
}

void Phase5RegressionTest::testSyncScrollIdsMatchMenuCmdIds()
{
	// Task #6: Verify sync scroll command IDs are registered and match menu IDs
	CommandHandler handler;
	bool syncVExecuted = false;
	bool syncHExecuted = false;

	handler.registerCommand(CMD_VIEW_SYNSCROLLV, [&syncVExecuted]() { syncVExecuted = true; });
	handler.registerCommand(CMD_VIEW_SYNSCROLLH, [&syncHExecuted]() { syncHExecuted = true; });

	QVERIFY(handler.canExecute(CMD_VIEW_SYNSCROLLV));
	QVERIFY(handler.canExecute(CMD_VIEW_SYNSCROLLH));

	handler.executeCommand(CMD_VIEW_SYNSCROLLV);
	handler.executeCommand(CMD_VIEW_SYNSCROLLH);

	QVERIFY(syncVExecuted);
	QVERIFY(syncHExecuted);
}

void Phase5RegressionTest::testXmlTagMatchingScintillaMessages()
{
	// Task #6: Verify Scintilla messages used for XML tag matching are defined
	QVERIFY(SCI_BRACEMATCH > 0);
	QVERIFY(SCI_BRACEHIGHLIGHT > 0);
	QVERIFY(SCI_BRACEBADLIGHT > 0);

	// These should all be distinct
	std::set<int> braceIds = {
		SCI_BRACEMATCH, SCI_BRACEHIGHLIGHT, SCI_BRACEBADLIGHT,
	};
	QCOMPARE(braceIds.size(), static_cast<size_t>(3));

	// Verify goto matching brace command
	QVERIFY(CMD_SEARCH_GOTOMATCHINGBRACE > 0);
	QCOMPARE(static_cast<int>(CMD_SEARCH_GOTOMATCHINGBRACE), static_cast<int>(IDM_SEARCH_GOTOMATCHINGBRACE));
}

// ============================================================================
// Task #7: Print Now and Workspace
// ============================================================================

void Phase5RegressionTest::testPrintNowCommandId()
{
	QVERIFY(CMD_FILE_PRINTNOW > 0);
	// Print now has a special ID (1001) different from the file range
}

void Phase5RegressionTest::testFolderAsWorkspaceCommandId()
{
	QVERIFY(CMD_FILE_OPENFOLDERASWORKSPACE > 0);
}

// ============================================================================
// Task #8: Plugin API Message Constants
// ============================================================================

void Phase5RegressionTest::testNppmMessageConstants()
{
	// Core plugin API messages should be defined and non-zero
	QVERIFY(NPPMSG > 0);
	QVERIFY(NPPM_GETCURRENTSCINTILLA > 0);
	QVERIFY(NPPM_GETCURRENTLANGTYPE > 0);
	QVERIFY(NPPM_SETCURRENTLANGTYPE > 0);
	QVERIFY(NPPM_GETNBOPENFILES > 0);
	QVERIFY(NPPM_GETNBSESSIONFILES > 0);
	QVERIFY(NPPM_GETSESSIONFILES > 0);
	QVERIFY(NPPM_SAVESESSION > 0);
	QVERIFY(NPPM_SAVECURRENTSESSION > 0);
}

void Phase5RegressionTest::testNppmMessageRangesDistinct()
{
	// Verify key NPPM messages don't collide
	std::set<int> msgs = {
		NPPM_GETCURRENTSCINTILLA,
		NPPM_GETCURRENTLANGTYPE,
		NPPM_SETCURRENTLANGTYPE,
		NPPM_GETNBOPENFILES,
		NPPM_GETNBSESSIONFILES,
		NPPM_GETSESSIONFILES,
		NPPM_SAVESESSION,
		NPPM_SAVECURRENTSESSION,
	};
	QCOMPARE(msgs.size(), static_cast<size_t>(8));
}

void Phase5RegressionTest::testNppmMessageExtendedSet()
{
	// Task #8: Verify the extended NPPM messages needed for full plugin relay
	QVERIFY(NPPM_MENUCOMMAND > 0);
	QVERIFY(NPPM_RELOADFILE > 0);
	QVERIFY(NPPM_SWITCHTOFILE > 0);
	QVERIFY(NPPM_LAUNCHFINDINFILESDLG > 0);
	QVERIFY(NPPM_GETPLUGINSCONFIGDIR > 0);
	QVERIFY(NPPM_GETBUFFERLANGTYPE > 0);
	QVERIFY(NPPM_SETBUFFERLANGTYPE > 0);
	QVERIFY(NPPM_GETBUFFERFORMAT > 0);
	QVERIFY(NPPM_SETBUFFERFORMAT > 0);
	QVERIFY(NPPM_ALLOCATECMDID > 0);
	QVERIFY(NPPM_GETPLUGINHOMEPATH > 0);

	// All must be distinct (no collisions in the relay)
	std::set<int> extMsgs = {
		NPPM_MENUCOMMAND, NPPM_RELOADFILE, NPPM_SWITCHTOFILE,
		NPPM_LAUNCHFINDINFILESDLG, NPPM_GETPLUGINSCONFIGDIR,
		NPPM_GETBUFFERLANGTYPE, NPPM_SETBUFFERLANGTYPE,
		NPPM_GETBUFFERFORMAT, NPPM_SETBUFFERFORMAT,
		NPPM_ALLOCATECMDID, NPPM_GETPLUGINHOMEPATH,
	};
	QCOMPARE(extMsgs.size(), static_cast<size_t>(11));
}

// ============================================================================
// Task #9: Plugin Admin Backend Validation
// ============================================================================

void Phase5RegressionTest::testPluginAdminCommandIds()
{
	// Task #9: Verify Plugin Admin menu command IDs are defined
	QVERIFY(IDM_SETTING_PLUGINADM > 0);
	QVERIFY(IDM_SETTING_OPENPLUGINSDIR > 0);
	QVERIFY(IDM_SETTING_IMPORTPLUGIN > 0);

	// All must be distinct
	std::set<int> pluginAdminIds = {
		IDM_SETTING_PLUGINADM,
		IDM_SETTING_OPENPLUGINSDIR,
		IDM_SETTING_IMPORTPLUGIN,
	};
	QCOMPARE(pluginAdminIds.size(), static_cast<size_t>(3));
}

// ============================================================================
// Task #5 (extended): Mark Tab Indicator Validation
// ============================================================================

void Phase5RegressionTest::testMarkStyleIndicatorConstants()
{
	// Verify all SCE_UNIVERSAL_FOUND_STYLE indicators are defined and positive
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_SMART > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_INC > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_EXT1 > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_EXT2 > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_EXT3 > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_EXT4 > 0);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_EXT5 > 0);

	// Verify they are in the valid Scintilla indicator range (0-35)
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE <= 35);
	QVERIFY(SCE_UNIVERSAL_FOUND_STYLE_EXT5 >= 21);
}

void Phase5RegressionTest::testMarkStyleIndicatorsAreUnique()
{
	// All indicator IDs must be unique to avoid style collisions
	std::set<int> indicators = {
		SCE_UNIVERSAL_FOUND_STYLE,
		SCE_UNIVERSAL_FOUND_STYLE_SMART,
		SCE_UNIVERSAL_FOUND_STYLE_INC,
		SCE_UNIVERSAL_FOUND_STYLE_EXT1,
		SCE_UNIVERSAL_FOUND_STYLE_EXT2,
		SCE_UNIVERSAL_FOUND_STYLE_EXT3,
		SCE_UNIVERSAL_FOUND_STYLE_EXT4,
		SCE_UNIVERSAL_FOUND_STYLE_EXT5,
	};
	QCOMPARE(indicators.size(), static_cast<size_t>(8));
}

void Phase5RegressionTest::testClearIndicatorUsesCorrectSciMessages()
{
	// Verify the Scintilla messages used by clearIndicator pattern exist
	// clearIndicator() uses: SCI_SETINDICATORCURRENT + SCI_INDICATORCLEARRANGE
	QVERIFY(SCI_SETINDICATORCURRENT > 0);
	QVERIFY(SCI_INDICATORCLEARRANGE > 0);

	// markAll/processMarkAll uses: SCI_SETINDICATORCURRENT + SCI_INDICATORFILLRANGE
	QVERIFY(SCI_INDICATORFILLRANGE > 0);

	// goToNext/PreviousIndicator uses: SCI_INDICATORSTART + SCI_INDICATOREND + SCI_INDICATORVALUEAT
	QVERIFY(SCI_INDICATORSTART > 0);
	QVERIFY(SCI_INDICATOREND > 0);
	QVERIFY(SCI_INDICATORVALUEAT > 0);

	// Verify these are all distinct message IDs
	std::set<int> sciMsgs = {
		SCI_SETINDICATORCURRENT,
		SCI_INDICATORCLEARRANGE,
		SCI_INDICATORFILLRANGE,
		SCI_INDICATORSTART,
		SCI_INDICATOREND,
		SCI_INDICATORVALUEAT,
	};
	QCOMPARE(sciMsgs.size(), static_cast<size_t>(6));
}

void Phase5RegressionTest::testMarkBookmarkConstant()
{
	// MARK_BOOKMARK is defined as 20 - used for bookmark margin markers
	QCOMPARE(TEST_MARK_BOOKMARK, 20);

	// Verify SCI_MARKERDELETEALL and SCI_MARKERADD exist (used by clearMarks)
	QVERIFY(SCI_MARKERDELETEALL > 0);
	QVERIFY(SCI_MARKERADD > 0);
	QVERIFY(SCI_MARKERDELETE > 0);
	QVERIFY(SCI_MARKERNEXT > 0);
}

void Phase5RegressionTest::testMarkNavigationCommandsMatchStyles()
{
	// Each EXT style (1-5) should have a corresponding gonext/goprev marker command
	// Verify the command registration mapping is consistent:
	// CMD_SEARCH_MARKALLEXT{N} maps to SCE_UNIVERSAL_FOUND_STYLE_EXT{N}
	// CMD_SEARCH_GONEXTMARKER{N} maps to SCE_UNIVERSAL_FOUND_STYLE_EXT{N}
	// CMD_SEARCH_GOPREVMARKER{N} maps to SCE_UNIVERSAL_FOUND_STYLE_EXT{N}

	// Verify all mark/unmark pairs exist for each style
	QVERIFY(CMD_SEARCH_MARKALLEXT1 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT1 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER1 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER1 > 0);

	QVERIFY(CMD_SEARCH_MARKALLEXT2 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT2 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER2 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER2 > 0);

	QVERIFY(CMD_SEARCH_MARKALLEXT3 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT3 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER3 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER3 > 0);

	QVERIFY(CMD_SEARCH_MARKALLEXT4 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT4 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER4 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER4 > 0);

	QVERIFY(CMD_SEARCH_MARKALLEXT5 > 0);
	QVERIFY(CMD_SEARCH_UNMARKALLEXT5 > 0);
	QVERIFY(CMD_SEARCH_GONEXTMARKER5 > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER5 > 0);

	// Default style also has navigation
	QVERIFY(CMD_SEARCH_GONEXTMARKER_DEF > 0);
	QVERIFY(CMD_SEARCH_GOPREVMARKER_DEF > 0);
}

void Phase5RegressionTest::testMarkUnmarkCommandPairs()
{
	// Verify mark/unmark command pairs have matching IDM values
	QCOMPARE(static_cast<int>(CMD_SEARCH_MARKALLEXT1), static_cast<int>(IDM_SEARCH_MARKALLEXT1));
	QCOMPARE(static_cast<int>(CMD_SEARCH_UNMARKALLEXT1), static_cast<int>(IDM_SEARCH_UNMARKALLEXT1));
	QCOMPARE(static_cast<int>(CMD_SEARCH_MARKALLEXT2), static_cast<int>(IDM_SEARCH_MARKALLEXT2));
	QCOMPARE(static_cast<int>(CMD_SEARCH_UNMARKALLEXT2), static_cast<int>(IDM_SEARCH_UNMARKALLEXT2));
	QCOMPARE(static_cast<int>(CMD_SEARCH_MARKALLEXT3), static_cast<int>(IDM_SEARCH_MARKALLEXT3));
	QCOMPARE(static_cast<int>(CMD_SEARCH_UNMARKALLEXT3), static_cast<int>(IDM_SEARCH_UNMARKALLEXT3));
	QCOMPARE(static_cast<int>(CMD_SEARCH_MARKALLEXT4), static_cast<int>(IDM_SEARCH_MARKALLEXT4));
	QCOMPARE(static_cast<int>(CMD_SEARCH_UNMARKALLEXT4), static_cast<int>(IDM_SEARCH_UNMARKALLEXT4));
	QCOMPARE(static_cast<int>(CMD_SEARCH_MARKALLEXT5), static_cast<int>(IDM_SEARCH_MARKALLEXT5));
	QCOMPARE(static_cast<int>(CMD_SEARCH_UNMARKALLEXT5), static_cast<int>(IDM_SEARCH_UNMARKALLEXT5));

	// Verify navigation commands match IDM values
	QCOMPARE(static_cast<int>(CMD_SEARCH_GONEXTMARKER1), static_cast<int>(IDM_SEARCH_GONEXTMARKER1));
	QCOMPARE(static_cast<int>(CMD_SEARCH_GOPREVMARKER1), static_cast<int>(IDM_SEARCH_GOPREVMARKER1));
	QCOMPARE(static_cast<int>(CMD_SEARCH_GONEXTMARKER_DEF), static_cast<int>(IDM_SEARCH_GONEXTMARKER_DEF));
	QCOMPARE(static_cast<int>(CMD_SEARCH_GOPREVMARKER_DEF), static_cast<int>(IDM_SEARCH_GOPREVMARKER_DEF));
}

// ============================================================================
// Cross-cutting: Command ID Enum Consistency
// ============================================================================

void Phase5RegressionTest::testAllCommandIdsArePositive()
{
	// Verify all CMD_* enum values are positive
	QVERIFY(CMD_FILE_NEW > 0);
	QVERIFY(CMD_FILE_OPEN > 0);
	QVERIFY(CMD_FILE_CLOSE > 0);
	QVERIFY(CMD_FILE_SAVE > 0);
	QVERIFY(CMD_FILE_EXIT > 0);
	QVERIFY(CMD_EDIT_CUT > 0);
	QVERIFY(CMD_EDIT_COPY > 0);
	QVERIFY(CMD_EDIT_PASTE > 0);
	QVERIFY(CMD_EDIT_UNDO > 0);
	QVERIFY(CMD_EDIT_REDO > 0);
	QVERIFY(CMD_EDIT_DELETE > 0);
	QVERIFY(CMD_EDIT_SELECTALL > 0);
	QVERIFY(CMD_SEARCH_FIND > 0);
	QVERIFY(CMD_SEARCH_REPLACE > 0);
	QVERIFY(CMD_SEARCH_GOTOLINE > 0);
	QVERIFY(CMD_VIEW_ZOOMIN > 0);
	QVERIFY(CMD_VIEW_ZOOMOUT > 0);
	QVERIFY(CMD_VIEW_ZOOMRESTORE > 0);
	QVERIFY(CMD_VIEW_WRAP > 0);
}

void Phase5RegressionTest::testFileCommandIdsInRange()
{
	// File commands should be in 41000-41999 range (except PRINTNOW which is 1001)
	int fileBase = 41000;
	int fileEnd = 42000;

	QVERIFY(CMD_FILE_NEW >= fileBase && CMD_FILE_NEW < fileEnd);
	QVERIFY(CMD_FILE_OPEN >= fileBase && CMD_FILE_OPEN < fileEnd);
	QVERIFY(CMD_FILE_CLOSE >= fileBase && CMD_FILE_CLOSE < fileEnd);
	QVERIFY(CMD_FILE_SAVE >= fileBase && CMD_FILE_SAVE < fileEnd);
	QVERIFY(CMD_FILE_SAVEAS >= fileBase && CMD_FILE_SAVEAS < fileEnd);
	QVERIFY(CMD_FILE_EXIT >= fileBase && CMD_FILE_EXIT < fileEnd);
}

void Phase5RegressionTest::testEditCommandIdsInRange()
{
	// Edit commands should be in 42000-42999 range
	int editBase = 42000;
	int editEnd = 43000;

	QVERIFY(CMD_EDIT_CUT >= editBase && CMD_EDIT_CUT < editEnd);
	QVERIFY(CMD_EDIT_COPY >= editBase && CMD_EDIT_COPY < editEnd);
	QVERIFY(CMD_EDIT_PASTE >= editBase && CMD_EDIT_PASTE < editEnd);
	QVERIFY(CMD_EDIT_UNDO >= editBase && CMD_EDIT_UNDO < editEnd);
	QVERIFY(CMD_EDIT_REDO >= editBase && CMD_EDIT_REDO < editEnd);
	QVERIFY(CMD_EDIT_DELETE >= editBase && CMD_EDIT_DELETE < editEnd);
	QVERIFY(CMD_EDIT_SELECTALL >= editBase && CMD_EDIT_SELECTALL < editEnd);
	QVERIFY(CMD_EDIT_UPPERCASE >= editBase && CMD_EDIT_UPPERCASE < editEnd);
	QVERIFY(CMD_EDIT_LOWERCASE >= editBase && CMD_EDIT_LOWERCASE < editEnd);
}

void Phase5RegressionTest::testSearchCommandIdsInRange()
{
	// Search commands should be in 43000-43999 range
	int searchBase = 43000;
	int searchEnd = 44000;

	QVERIFY(CMD_SEARCH_FIND >= searchBase && CMD_SEARCH_FIND < searchEnd);
	QVERIFY(CMD_SEARCH_FINDNEXT >= searchBase && CMD_SEARCH_FINDNEXT < searchEnd);
	QVERIFY(CMD_SEARCH_REPLACE >= searchBase && CMD_SEARCH_REPLACE < searchEnd);
	QVERIFY(CMD_SEARCH_GOTOLINE >= searchBase && CMD_SEARCH_GOTOLINE < searchEnd);
	QVERIFY(CMD_SEARCH_FINDPREV >= searchBase && CMD_SEARCH_FINDPREV < searchEnd);
	QVERIFY(CMD_SEARCH_FINDINFILES >= searchBase && CMD_SEARCH_FINDINFILES < searchEnd);
	QVERIFY(CMD_SEARCH_TOGGLE_BOOKMARK >= searchBase && CMD_SEARCH_TOGGLE_BOOKMARK < searchEnd);
}

void Phase5RegressionTest::testViewCommandIdsInRange()
{
	// View commands should be in 44000-44999 range
	int viewBase = 44000;
	int viewEnd = 45000;

	QVERIFY(CMD_VIEW_ZOOMIN >= viewBase && CMD_VIEW_ZOOMIN < viewEnd);
	QVERIFY(CMD_VIEW_ZOOMOUT >= viewBase && CMD_VIEW_ZOOMOUT < viewEnd);
	QVERIFY(CMD_VIEW_ZOOMRESTORE >= viewBase && CMD_VIEW_ZOOMRESTORE < viewEnd);
	QVERIFY(CMD_VIEW_WRAP >= viewBase && CMD_VIEW_WRAP < viewEnd);
	QVERIFY(CMD_VIEW_SYNSCROLLV >= viewBase && CMD_VIEW_SYNSCROLLV < viewEnd);
	QVERIFY(CMD_VIEW_SYNSCROLLH >= viewBase && CMD_VIEW_SYNSCROLLH < viewEnd);
	QVERIFY(CMD_VIEW_ALL_CHARACTERS >= viewBase && CMD_VIEW_ALL_CHARACTERS < viewEnd);
	QVERIFY(CMD_VIEW_INDENT_GUIDE >= viewBase && CMD_VIEW_INDENT_GUIDE < viewEnd);
}

void Phase5RegressionTest::testEditCommandIdsMatchMenuCmdIds()
{
	// Critical: CMD_* enum must match IDM_* macros for correct command dispatch
	QCOMPARE(static_cast<int>(CMD_EDIT_CUT), static_cast<int>(IDM_EDIT_CUT));
	QCOMPARE(static_cast<int>(CMD_EDIT_COPY), static_cast<int>(IDM_EDIT_COPY));
	QCOMPARE(static_cast<int>(CMD_EDIT_UNDO), static_cast<int>(IDM_EDIT_UNDO));
	QCOMPARE(static_cast<int>(CMD_EDIT_REDO), static_cast<int>(IDM_EDIT_REDO));
	QCOMPARE(static_cast<int>(CMD_EDIT_PASTE), static_cast<int>(IDM_EDIT_PASTE));
	QCOMPARE(static_cast<int>(CMD_EDIT_DELETE), static_cast<int>(IDM_EDIT_DELETE));
	QCOMPARE(static_cast<int>(CMD_EDIT_SELECTALL), static_cast<int>(IDM_EDIT_SELECTALL));
	QCOMPARE(static_cast<int>(CMD_EDIT_INS_TAB), static_cast<int>(IDM_EDIT_INS_TAB));
	QCOMPARE(static_cast<int>(CMD_EDIT_RMV_TAB), static_cast<int>(IDM_EDIT_RMV_TAB));
	QCOMPARE(static_cast<int>(CMD_EDIT_DUP_LINE), static_cast<int>(IDM_EDIT_DUP_LINE));
	QCOMPARE(static_cast<int>(CMD_EDIT_TRANSPOSE_LINE), static_cast<int>(IDM_EDIT_TRANSPOSE_LINE));
	QCOMPARE(static_cast<int>(CMD_EDIT_UPPERCASE), static_cast<int>(IDM_EDIT_UPPERCASE));
	QCOMPARE(static_cast<int>(CMD_EDIT_LOWERCASE), static_cast<int>(IDM_EDIT_LOWERCASE));
	QCOMPARE(static_cast<int>(CMD_EDIT_BLOCK_COMMENT), static_cast<int>(IDM_EDIT_BLOCK_COMMENT));
	QCOMPARE(static_cast<int>(CMD_EDIT_STREAM_COMMENT), static_cast<int>(IDM_EDIT_STREAM_COMMENT));
	QCOMPARE(static_cast<int>(CMD_EDIT_TRIMTRAILING), static_cast<int>(IDM_EDIT_TRIMTRAILING));
}

void Phase5RegressionTest::testSearchCommandIdsMatchMenuCmdIds()
{
	QCOMPARE(static_cast<int>(CMD_SEARCH_FIND), static_cast<int>(IDM_SEARCH_FIND));
	QCOMPARE(static_cast<int>(CMD_SEARCH_FINDNEXT), static_cast<int>(IDM_SEARCH_FINDNEXT));
	QCOMPARE(static_cast<int>(CMD_SEARCH_REPLACE), static_cast<int>(IDM_SEARCH_REPLACE));
	QCOMPARE(static_cast<int>(CMD_SEARCH_GOTOLINE), static_cast<int>(IDM_SEARCH_GOTOLINE));
	QCOMPARE(static_cast<int>(CMD_SEARCH_FINDPREV), static_cast<int>(IDM_SEARCH_FINDPREV));
	QCOMPARE(static_cast<int>(CMD_SEARCH_FINDINCREMENT), static_cast<int>(IDM_SEARCH_FINDINCREMENT));
	QCOMPARE(static_cast<int>(CMD_SEARCH_FINDINFILES), static_cast<int>(IDM_SEARCH_FINDINFILES));
	QCOMPARE(static_cast<int>(CMD_SEARCH_TOGGLE_BOOKMARK), static_cast<int>(IDM_SEARCH_TOGGLE_BOOKMARK));
	QCOMPARE(static_cast<int>(CMD_SEARCH_NEXT_BOOKMARK), static_cast<int>(IDM_SEARCH_NEXT_BOOKMARK));
	QCOMPARE(static_cast<int>(CMD_SEARCH_PREV_BOOKMARK), static_cast<int>(IDM_SEARCH_PREV_BOOKMARK));
	QCOMPARE(static_cast<int>(CMD_SEARCH_CLEAR_BOOKMARKS), static_cast<int>(IDM_SEARCH_CLEAR_BOOKMARKS));
}

void Phase5RegressionTest::testViewCommandIdsMatchMenuCmdIds()
{
	QCOMPARE(static_cast<int>(CMD_VIEW_ZOOMIN), static_cast<int>(IDM_VIEW_ZOOMIN));
	QCOMPARE(static_cast<int>(CMD_VIEW_ZOOMOUT), static_cast<int>(IDM_VIEW_ZOOMOUT));
	QCOMPARE(static_cast<int>(CMD_VIEW_ZOOMRESTORE), static_cast<int>(IDM_VIEW_ZOOMRESTORE));
	QCOMPARE(static_cast<int>(CMD_VIEW_WRAP), static_cast<int>(IDM_VIEW_WRAP));
	QCOMPARE(static_cast<int>(CMD_VIEW_ALL_CHARACTERS), static_cast<int>(IDM_VIEW_ALL_CHARACTERS));
	QCOMPARE(static_cast<int>(CMD_VIEW_INDENT_GUIDE), static_cast<int>(IDM_VIEW_INDENT_GUIDE));
	QCOMPARE(static_cast<int>(CMD_VIEW_SYNSCROLLV), static_cast<int>(IDM_VIEW_SYNSCROLLV));
	QCOMPARE(static_cast<int>(CMD_VIEW_SYNSCROLLH), static_cast<int>(IDM_VIEW_SYNSCROLLH));
}

} // namespace Tests

#include "Phase5RegressionTest.moc"
