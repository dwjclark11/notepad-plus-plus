// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ShortcutMapperTest.h"
#include "ShortcutMapper.h"
#include "../Common/TestUtils.h"

using namespace QtControls;

namespace Tests {

ShortcutMapperTest::ShortcutMapperTest() {}

ShortcutMapperTest::~ShortcutMapperTest() {}

void ShortcutMapperTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void ShortcutMapperTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void ShortcutMapperTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _dialog = std::make_unique<ShortcutMapper>(_parentWidget.get());
}

void ShortcutMapperTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void ShortcutMapperTest::testInit() {
    QVERIFY(_dialog != nullptr);
}

void ShortcutMapperTest::testShowDialog() {
    _dialog->showDialog();
    QVERIFY(_dialog->getWidget()->isVisible());
}

} // namespace Tests

#include "ShortcutMapperTest.moc"
