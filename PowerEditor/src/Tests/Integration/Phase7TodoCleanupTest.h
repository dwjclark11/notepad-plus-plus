// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace Tests {

class Phase7TodoCleanupTest : public QObject {
	Q_OBJECT

public:
	Phase7TodoCleanupTest();
	~Phase7TodoCleanupTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// ============================================================================
	// Task #1: Backup file creation (Notepad_plus.cpp item 1)
	// ============================================================================
	void testBackupFileCreation();
	void testBackupFileWithTimestamp();

	// ============================================================================
	// Task #2-3: checkSyncState / checkDocState don't crash (items 2-3)
	// ============================================================================
	void testCheckSyncStateNoCrash();
	void testCheckDocStateNoCrash();

	// ============================================================================
	// Task #4: performPostReload doesn't crash (item 4)
	// ============================================================================
	void testPerformPostReloadNoCrash();

	// ============================================================================
	// Task #9-10: Clipboard history save/load round-trip (items 9-10)
	// ============================================================================
	void testClipboardHistorySaveLoad();
	void testClipboardHistorySaveLoadEmpty();
	void testClipboardHistorySaveLoadMultiple();

	// ============================================================================
	// Task #13: Shortcut category assignment (item 13)
	// ============================================================================
	void testShortcutCategoryMainMenu();
	void testShortcutCategoryMacros();
	void testShortcutCategoryRunCommands();
	void testShortcutCategoryPlugins();

	// ============================================================================
	// Task #15-16: Toolbar initTheme/initHideButtonsConf don't crash (items 15-16)
	// ============================================================================
	void testToolBarInitThemeNullNoCrash();
	void testToolBarInitHideButtonsConfNullNoCrash();

	// ============================================================================
	// Task #22-25: UserDefineDialog type-safe accessors (items 22-25)
	// ============================================================================
	void testUserLangContainerAccessors();
	void testUserLangContainerKeywordListAccessors();
	void testUserLangContainerPrefixAccessors();

	// ============================================================================
	// Task #26-28: PluginsAdmin TODO comments removed (items 26-28)
	// ============================================================================
	void testPluginsAdminNoTodoComments();
};

} // namespace Tests
