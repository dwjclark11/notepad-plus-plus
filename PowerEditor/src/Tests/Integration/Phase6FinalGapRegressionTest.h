// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace Tests {

class Phase6FinalGapRegressionTest : public QObject {
	Q_OBJECT

public:
	Phase6FinalGapRegressionTest();
	~Phase6FinalGapRegressionTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// ============================================================================
	// Task #1: Clipboard - Paste HTML/RTF and Copy/Cut/Paste Binary
	// ============================================================================
	void testPasteAsHtmlCommandId();
	void testPasteAsRtfCommandId();
	void testCopyBinaryCommandId();
	void testCutBinaryCommandId();
	void testPasteBinaryCommandId();
	void testClipboardCommandIdsUnique();
	void testClipboardCommandRegistration();

	// ============================================================================
	// Task #2: Browser - Search on Internet and View in Browser
	// ============================================================================
	void testSearchOnInternetCommandId();
	void testOpenDefaultViewerCommandId();
	void testBrowserCommandRegistration();

	// ============================================================================
	// Task #3: View - Always on Top, Tab Coloring, RTL/LTR
	// ============================================================================
	void testAlwaysOnTopCommandId();
	void testTabColourCommandIds();
	void testTabColourIdsUnique();
	void testRtlLtrCommandIds();
	void testRtlLtrIdsDistinct();
	void testViewFeatureCommandRegistration();

	// ============================================================================
	// Task #4: Plugin Notifications and Change History / Hide Lines
	// ============================================================================
	void testAllNppnConstantsDefined();
	void testNppnConstantsAreUnique();
	void testNppnConstantsInRange();
	void testChangeHistoryCommandIds();
	void testHideLinesCommandId();
	void testChangeHistoryCommandRegistration();

	// ============================================================================
	// Cross-cutting: New Command ID Range Validation
	// ============================================================================
	void testNewEditCommandIdsInEditRange();
	void testNewViewCommandIdsInViewRange();
};

} // namespace Tests
