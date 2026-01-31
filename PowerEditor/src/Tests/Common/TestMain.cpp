// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include <QtTest/QtTest>
#include <QApplication>
#include "TestUtils.h"

// Include Platform test headers
#include "../Platform/FileSystemTest.h"
#include "../Platform/SettingsTest.h"
#include "../Platform/ProcessTest.h"
#include "../Platform/FileWatcherTest.h"
#include "../Platform/ClipboardTest.h"
#include "../Platform/DialogsTest.h"

// This file provides a main() function for PlatformTests executable
// that runs all platform layer tests

#ifndef QTEST_CUSTOM_MAIN

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize test environment
    if (!Tests::TestEnvironment::getInstance().init()) {
        qWarning() << "Failed to initialize test environment";
        return 1;
    }

    int result = 0;

    // Run all platform tests
    result |= QTest::qExec(new Tests::FileSystemTest(), argc, argv);
    result |= QTest::qExec(new Tests::SettingsTest(), argc, argv);
    result |= QTest::qExec(new Tests::ProcessTest(), argc, argv);
    result |= QTest::qExec(new Tests::FileWatcherTest(), argc, argv);
    result |= QTest::qExec(new Tests::ClipboardTest(), argc, argv);
    result |= QTest::qExec(new Tests::DialogsTest(), argc, argv);

    Tests::TestEnvironment::getInstance().cleanup();

    return result;
}

#endif // QTEST_CUSTOM_MAIN
