// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ToolBarTest.h"
#include "ToolBar.h"
#include "../../menuCmdID.h"
#include "../Common/TestUtils.h"
#include <QWidget>
#include <QToolBar>
#include <QAction>
#include <QSignalSpy>

using namespace QtControls;

namespace Tests {

ToolBarTest::ToolBarTest() {}

ToolBarTest::~ToolBarTest() {}

void ToolBarTest::initTestCase() {
	QVERIFY(TestEnvironment::getInstance().init());
}

void ToolBarTest::cleanupTestCase() {
	TestEnvironment::getInstance().cleanup();
}

void ToolBarTest::init() {
	_parentWidget = std::make_unique<QWidget>();
	_parentWidget->resize(800, 600);
	_toolBar = std::make_unique<QtControls::ToolBar>();
}

void ToolBarTest::cleanup() {
	_toolBar.reset();
	_parentWidget.reset();
}

// ============================================================================
// Bug 1: Toolbar button clicks must emit commandTriggered signal
// ============================================================================
void ToolBarTest::testActionTriggeredEmitsCommandSignal() {
	ToolBarButtonUnit buttons[] = {
		{IDM_FILE_NEW, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // separator
	};

	QVERIFY(_toolBar->init(_parentWidget.get(), TB_SMALL, buttons, 2));

	QSignalSpy spy(_toolBar.get(), &QtControls::ToolBar::commandTriggered);
	QVERIFY(spy.isValid());

	QToolBar* qtToolBar = _toolBar->getToolBar();
	QVERIFY(qtToolBar != nullptr);

	// Find the action with our command ID
	QAction* targetAction = nullptr;
	for (QAction* action : qtToolBar->actions()) {
		if (action->data().toInt() == IDM_FILE_NEW) {
			targetAction = action;
			break;
		}
	}
	QVERIFY(targetAction != nullptr);

	targetAction->trigger();

	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.first().first().toInt(), static_cast<int>(IDM_FILE_NEW));
}

void ToolBarTest::testActionTriggeredIgnoresZeroCmdID() {
	ToolBarButtonUnit buttons[] = {
		{IDM_FILE_NEW, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // separator (cmdID == 0)
	};

	QVERIFY(_toolBar->init(_parentWidget.get(), TB_SMALL, buttons, 2));

	QSignalSpy spy(_toolBar.get(), &QtControls::ToolBar::commandTriggered);
	QVERIFY(spy.isValid());

	QToolBar* qtToolBar = _toolBar->getToolBar();
	QVERIFY(qtToolBar != nullptr);

	// Find a separator action (cmdID == 0)
	for (QAction* action : qtToolBar->actions()) {
		if (action->data().toInt() == 0) {
			action->trigger();
			break;
		}
	}

	QCOMPARE(spy.count(), 0);
}

} // namespace Tests
