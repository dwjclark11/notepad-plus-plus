// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include <QtTest/QtTest>
#include <QApplication>
#include "TestUtils.h"

// This file provides a main() function for test executables
// that don't define their own QTEST_MAIN

// If a test file defines QTEST_MAIN, this file should not be linked
// If a test file doesn't define QTEST_MAIN, this provides a fallback

#ifndef QTEST_CUSTOM_MAIN

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize test environment
    if (!Tests::TestEnvironment::getInstance().init()) {
        qWarning() << "Failed to initialize test environment";
        return 1;
    }

    int result = QTest::qExec(&app, argc, argv);

    Tests::TestEnvironment::getInstance().cleanup();

    return result;
}

#endif // QTEST_CUSTOM_MAIN
