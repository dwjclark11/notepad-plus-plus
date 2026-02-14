// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <memory>

class QTabWidget;

namespace Tests {

class TabSignalRaceTest : public QObject {
	Q_OBJECT

public:
	TabSignalRaceTest();
	~TabSignalRaceTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	// Bug 3: Tab signal race condition
	void testAddTabDoesNotEmitBeforeMappingStored();
	void testAddTabRaceWithoutBlockSignals();

private:
	std::unique_ptr<QTabWidget> _tabWidget;
};

} // namespace Tests
