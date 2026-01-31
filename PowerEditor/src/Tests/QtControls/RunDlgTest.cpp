// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "RunDlgTest.h"
#include "RunDlg.h"
#include "../Common/TestUtils.h"

using namespace QtControls::RunDlg;

namespace Tests {

RunDlgTest::RunDlgTest() {}

RunDlgTest::~RunDlgTest() {}

void RunDlgTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void RunDlgTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void RunDlgTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _dialog = std::make_unique<RunDlg>(_parentWidget.get());
}

void RunDlgTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void RunDlgTest::testInit() {
    QVERIFY(_dialog != nullptr);
}

void RunDlgTest::testDoDialog() {
    _dialog->doDialog();
    QVERIFY(_dialog->getWidget()->isVisible());
}

// ============================================================================
// Command Management Tests
// ============================================================================
void RunDlgTest::testGetCommand() {
    _dialog->setCommand("echo Hello");
    QCOMPARE(_dialog->getCommand(), QString("echo Hello"));
}

void RunDlgTest::testSetCommand() {
    _dialog->setCommand("ls -la");
    QCOMPARE(_dialog->getCommand(), QString("ls -la"));

    _dialog->setCommand("cat file.txt");
    QCOMPARE(_dialog->getCommand(), QString("cat file.txt"));
}

void RunDlgTest::testGetHistory() {
    std::vector<QString> history = {"cmd1", "cmd2", "cmd3"};
    _dialog->setHistory(history);

    std::vector<QString> retrieved = _dialog->getHistory();
    QCOMPARE(static_cast<int>(retrieved.size()), 3);
    QCOMPARE(retrieved[0], QString("cmd1"));
    QCOMPARE(retrieved[1], QString("cmd2"));
    QCOMPARE(retrieved[2], QString("cmd3"));
}

void RunDlgTest::testSetHistory() {
    std::vector<QString> history = {"echo 1", "echo 2"};
    _dialog->setHistory(history);

    std::vector<QString> retrieved = _dialog->getHistory();
    QCOMPARE(static_cast<int>(retrieved.size()), 2);
}

} // namespace Tests
