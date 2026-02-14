// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace NppFindReplace {
	class FindReplaceDlg;
}

namespace Tests {

class FindReplaceDlgInitTest : public QObject {
	Q_OBJECT

public:
	FindReplaceDlgInitTest();
	~FindReplaceDlgInitTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// Bug 2: FindReplaceDlg null guard before init
	void testIsCreatedFalseBeforeInit();
	void testShowDialogSafeBeforeInit();

	// Bug 5: Init sets isCreated
	void testInitSetsIsCreated();

	// Bug 5: getCurrentOptions reads from UI checkboxes
	void testGetCurrentOptionsReadsFromFindTab();
};

} // namespace Tests
