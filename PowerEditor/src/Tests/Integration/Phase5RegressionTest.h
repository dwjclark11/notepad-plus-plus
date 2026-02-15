// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace Tests {

class Phase5RegressionTest : public QObject {
	Q_OBJECT

public:
	Phase5RegressionTest();
	~Phase5RegressionTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// ============================================================================
	// CommandHandler Unit Tests (no Notepad_plus dependency)
	// ============================================================================
	void testCommandHandlerRegisterAndExecute();
	void testCommandHandlerCanExecute();
	void testCommandHandlerUnregister();
	void testCommandHandlerClearCommands();
	void testCommandHandlerExecuteUnregistered();
	void testCommandHandlerOverwrite();
	void testCommandHandlerMultipleExecutions();
	void testCommandHandlerNullCallback();

	// ============================================================================
	// Task #1: Close Variant Command ID Validation
	// ============================================================================
	void testCloseAllCommandIdsDefined();
	void testCloseVariantIdsInFileRange();
	void testCloseVariantIdsMatchMenuCmdIds();
	void testAllFileCloseIdsUnique();
	void testCloseCommandRegistration();

	// ============================================================================
	// Task #2: Search Feature Command ID Validation
	// ============================================================================
	void testSearchCommandIdsDefined();
	void testMarkAllExtCommandIds();
	void testMarkerNavigationCommandIds();
	void testFindInFilesCommandId();

	// ============================================================================
	// Task #4: Smart Editing Command ID Validation
	// ============================================================================
	void testCaseConversionCommandIds();
	void testExtendedCaseConversionCommandIds();
	void testCommentCommandIds();
	void testTrimCommandIds();
	void testSortCommandIds();
	void testInsertDateTimeCommandIds();
	void testToggleReadOnlyCommandId();
	void testInsertDateTimeIdsMatchMenuCmdIds();

	// ============================================================================
	// Task #5: Bookmark Command ID Validation
	// ============================================================================
	void testBookmarkCommandIds();
	void testMarkedLinesCommandIds();

	// ============================================================================
	// Task #6: Sync Scrolling and Incremental Search Command IDs
	// ============================================================================
	void testSyncScrollCommandIds();
	void testIncrementalSearchCommandId();
	void testSyncScrollIdsMatchMenuCmdIds();
	void testXmlTagMatchingScintillaMessages();

	// ============================================================================
	// Task #7: Print Now and Workspace Command IDs
	// ============================================================================
	void testPrintNowCommandId();
	void testFolderAsWorkspaceCommandId();

	// ============================================================================
	// Task #8: Plugin API Message Constants
	// ============================================================================
	void testNppmMessageConstants();
	void testNppmMessageRangesDistinct();
	void testNppmMessageExtendedSet();

	// ============================================================================
	// Task #9: Plugin Admin Backend Validation
	// ============================================================================
	void testPluginAdminCommandIds();

	// ============================================================================
	// Task #5 (extended): Mark Tab Indicator Validation
	// ============================================================================
	void testMarkStyleIndicatorConstants();
	void testMarkStyleIndicatorsAreUnique();
	void testClearIndicatorUsesCorrectSciMessages();
	void testMarkBookmarkConstant();
	void testMarkNavigationCommandsMatchStyles();
	void testMarkUnmarkCommandPairs();

	// ============================================================================
	// Cross-cutting: Command ID Enum Consistency
	// ============================================================================
	void testAllCommandIdsArePositive();
	void testFileCommandIdsInRange();
	void testEditCommandIdsInRange();
	void testSearchCommandIdsInRange();
	void testViewCommandIdsInRange();
	void testEditCommandIdsMatchMenuCmdIds();
	void testSearchCommandIdsMatchMenuCmdIds();
	void testViewCommandIdsMatchMenuCmdIds();
};

} // namespace Tests
