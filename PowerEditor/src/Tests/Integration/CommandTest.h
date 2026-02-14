// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace Tests {

class CommandTest : public QObject {
	Q_OBJECT

public:
	CommandTest();
	~CommandTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// Real validation tests for command ID definitions
	void testCommandIdRangesAreDistinct();
	void testCommandIdsAreNonZero();
	void testFileMenuRange();
	void testEditMenuRange();
	void testSearchMenuRange();
	void testViewMenuRange();

	// Command dispatch tests (require full Notepad_plus core)
	void testNewFile();
	void testOpenFile();
	void testSaveFile();
	void testCloseFile();
	void testUndo();
	void testRedo();
	void testCut();
	void testCopy();
	void testPaste();
	void testDelete();
	void testSelectAll();
	void testFind();
	void testReplace();
	void testGoToLine();
	void testZoomIn();
	void testZoomOut();
	void testZoomReset();
	void testToggleWordWrap();
	void testToggleLineNumbers();
	void testStartRecording();
	void testStopRecording();
	void testPlayback();
	void testRunCommand();
	void testNewWindow();
	void testCloseWindow();
	void testSplitView();
};

} // namespace Tests
