// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QStringList>

namespace Tests {

// Simplified CmdLineParams for testing (avoids pulling in Parameters.h)
struct TestCmdLineParams {
	long long _line2go = -1;
	long long _column2go = -1;
	long long _pos2go = -1;
	bool _isReadOnly = false;
	bool _isNoSession = false;
	bool _isSessionFile = false;
	bool _monitorFiles = false;
};

class IPCParseTest : public QObject {
	Q_OBJECT

public:
	IPCParseTest();
	~IPCParseTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	// Bug 6: IPC parameter parsing with static variable
	void testParseIncomingDataOnce();
	void testParseIncomingDataTwiceLocalReset();
	void testParseIncomingDataTwiceStaticBug();

private:
	// Fixed version (local paramIndex)
	static void parseIncomingData(const QByteArray& data, QStringList& files, TestCmdLineParams& params);
	// Buggy version (static paramIndex) to prove the bug
	static void parseIncomingDataBuggy(const QByteArray& data, QStringList& files, TestCmdLineParams& params);
};

} // namespace Tests
