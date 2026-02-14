// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "IPCParseTest.h"
#include "../Common/TestUtils.h"

namespace Tests {

IPCParseTest::IPCParseTest() {}

IPCParseTest::~IPCParseTest() {}

void IPCParseTest::initTestCase() {
	QVERIFY(TestEnvironment::getInstance().init());
}

void IPCParseTest::cleanupTestCase() {
	TestEnvironment::getInstance().cleanup();
}

// ============================================================================
// Parse helpers
// ============================================================================

void IPCParseTest::parseIncomingData(const QByteArray& data, QStringList& files, TestCmdLineParams& params) {
	QStringList lines = QString(data).split('\n');
	bool inParams = false;
	int paramIndex = 0;  // LOCAL - the fix

	for (const QString& line : lines) {
		if (line == "CMDLINE_PARAMS") { inParams = true; paramIndex = 0; continue; }
		else if (line == "END_PARAMS") { inParams = false; continue; }
		else if (line == "END_FILES") { continue; }

		if (inParams) {
			switch (paramIndex++) {
				case 0: params._line2go = line.toLongLong(); break;
				case 1: params._column2go = line.toLongLong(); break;
				case 2: params._pos2go = line.toLongLong(); break;
				case 3: params._isReadOnly = (line == "1"); break;
				case 4: params._isNoSession = (line == "1"); break;
				case 5: params._isSessionFile = (line == "1"); break;
				case 6: params._monitorFiles = (line == "1"); break;
			}
		} else if (line.startsWith("FILE:")) {
			files.append(line.mid(5));
		}
	}
}

void IPCParseTest::parseIncomingDataBuggy(const QByteArray& data, QStringList& files, TestCmdLineParams& params) {
	QStringList lines = QString(data).split('\n');
	bool inParams = false;
	// BUG: static paramIndex persists across calls and is NOT reset inside
	// the CMDLINE_PARAMS handler. After the first call processes 7 params
	// (indices 0-6), paramIndex is 7. On the second call the switch cases
	// 0-6 never match, so all parameters are silently dropped.
	static int paramIndex = 0;

	for (const QString& line : lines) {
		if (line == "CMDLINE_PARAMS") { inParams = true; continue; }
		else if (line == "END_PARAMS") { inParams = false; continue; }
		else if (line == "END_FILES") { continue; }

		if (inParams) {
			switch (paramIndex++) {
				case 0: params._line2go = line.toLongLong(); break;
				case 1: params._column2go = line.toLongLong(); break;
				case 2: params._pos2go = line.toLongLong(); break;
				case 3: params._isReadOnly = (line == "1"); break;
				case 4: params._isNoSession = (line == "1"); break;
				case 5: params._isSessionFile = (line == "1"); break;
				case 6: params._monitorFiles = (line == "1"); break;
			}
		} else if (line.startsWith("FILE:")) {
			files.append(line.mid(5));
		}
	}
}

// ============================================================================
// Helper to build an IPC message
// ============================================================================

static QByteArray buildIPCMessage(long long line, long long column, long long pos,
                                  bool readOnly, bool noSession, bool sessionFile,
                                  bool monitor, const QStringList& filePaths) {
	QString msg;
	msg += "CMDLINE_PARAMS\n";
	msg += QString::number(line) + "\n";
	msg += QString::number(column) + "\n";
	msg += QString::number(pos) + "\n";
	msg += (readOnly ? "1" : "0") + QString("\n");
	msg += (noSession ? "1" : "0") + QString("\n");
	msg += (sessionFile ? "1" : "0") + QString("\n");
	msg += (monitor ? "1" : "0") + QString("\n");
	msg += "END_PARAMS\n";
	for (const QString& f : filePaths) {
		msg += "FILE:" + f + "\n";
	}
	msg += "END_FILES";
	return msg.toUtf8();
}

// ============================================================================
// Bug 6: IPC parameter parsing tests
// ============================================================================

void IPCParseTest::testParseIncomingDataOnce() {
	// Single parse should work correctly with either version
	QByteArray data = buildIPCMessage(10, 5, 100, true, false, false, true, {"/tmp/test.txt"});

	QStringList files;
	TestCmdLineParams params;
	parseIncomingData(data, files, params);

	QCOMPARE(params._line2go, 10LL);
	QCOMPARE(params._column2go, 5LL);
	QCOMPARE(params._pos2go, 100LL);
	QCOMPARE(params._isReadOnly, true);
	QCOMPARE(params._isNoSession, false);
	QCOMPARE(params._isSessionFile, false);
	QCOMPARE(params._monitorFiles, true);
	QCOMPARE(files.size(), 1);
	QCOMPARE(files.at(0), QString("/tmp/test.txt"));
}

void IPCParseTest::testParseIncomingDataTwiceLocalReset() {
	// With the fixed (local) version, the second parse should work correctly
	QByteArray data1 = buildIPCMessage(10, 5, 100, true, false, false, true, {"/tmp/test.txt"});
	QByteArray data2 = buildIPCMessage(20, 30, 200, false, true, true, false, {"/tmp/other.txt"});

	// First parse
	QStringList files1;
	TestCmdLineParams params1;
	parseIncomingData(data1, files1, params1);

	// Second parse
	QStringList files2;
	TestCmdLineParams params2;
	parseIncomingData(data2, files2, params2);

	// Second parse should have correct values
	QCOMPARE(params2._line2go, 20LL);
	QCOMPARE(params2._column2go, 30LL);
	QCOMPARE(params2._pos2go, 200LL);
	QCOMPARE(params2._isReadOnly, false);
	QCOMPARE(params2._isNoSession, true);
	QCOMPARE(params2._isSessionFile, true);
	QCOMPARE(params2._monitorFiles, false);
	QCOMPARE(files2.size(), 1);
	QCOMPARE(files2.at(0), QString("/tmp/other.txt"));
}

void IPCParseTest::testParseIncomingDataTwiceStaticBug() {
	// With the buggy (static) version, the second parse fails because
	// paramIndex persists across calls. After the first call processes
	// 7 parameters (indices 0-6), the static paramIndex is left at 7.
	// On the second call, paramIndex continues from 7, so none of the
	// switch cases (0-6) match and all parameters stay at defaults.

	QByteArray data1 = buildIPCMessage(10, 5, 100, true, false, false, true, {"/tmp/test.txt"});
	QByteArray data2 = buildIPCMessage(20, 30, 200, false, true, true, false, {"/tmp/other.txt"});

	// First parse with buggy version
	QStringList files1;
	TestCmdLineParams params1;
	parseIncomingDataBuggy(data1, files1, params1);

	// First call should still work (paramIndex starts at 0)
	QCOMPARE(params1._line2go, 10LL);
	QCOMPARE(params1._column2go, 5LL);
	QCOMPARE(params1._pos2go, 100LL);
	QCOMPARE(params1._isReadOnly, true);

	// Second parse with buggy version
	// The static paramIndex is now 7 from the first call, so the switch
	// will never match cases 0-6 again. All params stay at defaults.
	QStringList files2;
	TestCmdLineParams params2;
	parseIncomingDataBuggy(data2, files2, params2);

	// Verify second call gets WRONG results - paramIndex continues from 7
	// so cases 0-6 never match again, all params remain at defaults
	QCOMPARE(params2._line2go, -1LL);     // Still default, proving the bug
	QCOMPARE(params2._column2go, -1LL);   // Still default
	QCOMPARE(params2._pos2go, -1LL);      // Still default
	QCOMPARE(params2._isReadOnly, false); // Still default
	QCOMPARE(params2._isNoSession, false); // Still default

	// Files should still be parsed (file parsing doesn't use paramIndex)
	QCOMPARE(files2.size(), 1);
	QCOMPARE(files2.at(0), QString("/tmp/other.txt"));
}

} // namespace Tests
