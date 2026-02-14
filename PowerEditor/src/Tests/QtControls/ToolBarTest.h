// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <memory>

class QWidget;

namespace QtControls {
	class ToolBar;
}

namespace Tests {

class ToolBarTest : public QObject {
	Q_OBJECT

public:
	ToolBarTest();
	~ToolBarTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	// Bug 1: Verify toolbar button clicks emit commandTriggered signal
	void testActionTriggeredEmitsCommandSignal();
	void testActionTriggeredIgnoresZeroCmdID();

private:
	std::unique_ptr<QWidget> _parentWidget;
	std::unique_ptr<QtControls::ToolBar> _toolBar;
};

} // namespace Tests
