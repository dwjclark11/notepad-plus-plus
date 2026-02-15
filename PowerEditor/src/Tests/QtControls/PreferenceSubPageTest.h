// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace Tests {

class PreferenceSubPageTest : public QObject {
	Q_OBJECT

public:
	PreferenceSubPageTest();
	~PreferenceSubPageTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// Task #3: Verify all 8 new Preference sub-pages instantiate correctly
	void testToolbarSubDlgInstantiation();
	void testTabbarSubDlgInstantiation();
	void testEditing2SubDlgInstantiation();
	void testDarkModeSubDlgInstantiation();
	void testMarginsBorderEdgeSubDlgInstantiation();
	void testFileAssocSubDlgInstantiation();
	void testIndentationSubDlgInstantiation();
	void testPerformanceSubDlgInstantiation();

	// Verify the setupUI/connectSignals/loadSettings/saveSettings pattern
	void testToolbarSubDlgSettingsRoundTrip();
	void testTabbarSubDlgSettingsRoundTrip();
	void testEditing2SubDlgSettingsRoundTrip();
	void testDarkModeSubDlgSettingsRoundTrip();
	void testMarginsBorderEdgeSubDlgSettingsRoundTrip();
	void testFileAssocSubDlgSettingsRoundTrip();
	void testIndentationSubDlgSettingsRoundTrip();
	void testPerformanceSubDlgSettingsRoundTrip();

	// Verify sub-pages are proper QWidgets
	void testAllSubPagesAreQWidgets();
};

} // namespace Tests
