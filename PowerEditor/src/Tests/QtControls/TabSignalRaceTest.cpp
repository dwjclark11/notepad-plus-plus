// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "TabSignalRaceTest.h"
#include "../Common/TestUtils.h"
#include <QTabWidget>
#include <QWidget>
#include <QMap>
#include <QString>

namespace Tests {

TabSignalRaceTest::TabSignalRaceTest() {}

TabSignalRaceTest::~TabSignalRaceTest() {}

void TabSignalRaceTest::initTestCase() {
	QVERIFY(TestEnvironment::getInstance().init());
}

void TabSignalRaceTest::cleanupTestCase() {
	TestEnvironment::getInstance().cleanup();
}

void TabSignalRaceTest::init() {
	_tabWidget = std::make_unique<QTabWidget>();
}

void TabSignalRaceTest::cleanup() {
	_tabWidget.reset();
}

// ============================================================================
// Bug 3: Tab signal race condition
// ============================================================================

// Test that blockSignals prevents currentChanged from firing before mapping
// is stored. This is the FIX pattern: block signals, add tab, store mapping,
// then unblock signals.
void TabSignalRaceTest::testAddTabDoesNotEmitBeforeMappingStored() {
	QMap<int, QString> indexToBuffer;
	bool signalFiredWithMissingMapping = false;

	connect(_tabWidget.get(), &QTabWidget::currentChanged, this,
		[&indexToBuffer, &signalFiredWithMissingMapping](int index) {
			if (index >= 0 && !indexToBuffer.contains(index)) {
				signalFiredWithMissingMapping = true;
			}
		});

	// With blockSignals: signal cannot fire during addTab
	_tabWidget->blockSignals(true);
	QWidget* page = new QWidget(_tabWidget.get());
	int newIndex = _tabWidget->addTab(page, QStringLiteral("Tab 1"));
	indexToBuffer.insert(newIndex, QStringLiteral("buffer_1"));
	_tabWidget->blockSignals(false);

	// The signal should not have fired with a missing mapping
	QVERIFY(!signalFiredWithMissingMapping);
	QVERIFY(indexToBuffer.contains(newIndex));
	QCOMPARE(indexToBuffer.value(newIndex), QStringLiteral("buffer_1"));
}

// Test that WITHOUT blockSignals, the currentChanged signal fires during
// addTab BEFORE the caller can store the mapping. This demonstrates the
// bug pattern that causes crashes in the real code.
void TabSignalRaceTest::testAddTabRaceWithoutBlockSignals() {
	QMap<int, QString> indexToBuffer;
	bool signalFiredBeforeMapping = false;

	connect(_tabWidget.get(), &QTabWidget::currentChanged, this,
		[&indexToBuffer, &signalFiredBeforeMapping](int index) {
			if (index >= 0 && !indexToBuffer.contains(index)) {
				signalFiredBeforeMapping = true;
			}
		});

	// Without blockSignals: signal fires during addTab, before we store mapping
	QWidget* page = new QWidget(_tabWidget.get());
	int newIndex = _tabWidget->addTab(page, QStringLiteral("Tab 1"));
	// This line runs AFTER addTab returns, but the signal already fired inside addTab
	indexToBuffer.insert(newIndex, QStringLiteral("buffer_1"));

	// The signal should have fired before the mapping was stored,
	// demonstrating the bug condition
	QVERIFY(signalFiredBeforeMapping);
}

} // namespace Tests
