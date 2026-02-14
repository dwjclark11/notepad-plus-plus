// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "IOTest.h"
#include "../Common/TestUtils.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace Tests {

IOTest::IOTest() {}

IOTest::~IOTest() {}

void IOTest::initTestCase()
{
	QVERIFY(TestEnvironment::getInstance().init());
}

void IOTest::cleanupTestCase()
{
	TestEnvironment::getInstance().cleanup();
}

void IOTest::init() {}

void IOTest::cleanup() {}

QString IOTest::createTestFile(const QString& fileName, const QString& content)
{
	return TestEnvironment::getInstance().createTempFile(fileName, content);
}

QString IOTest::getTestPath(const QString& relativePath)
{
	return QDir(TestEnvironment::getInstance().getTempDir()).filePath(relativePath);
}

// ============================================================================
// Real File I/O Tests Using QFile
// ============================================================================

void IOTest::testCreateAndVerifyFile()
{
	QString filePath = createTestFile("test_open.txt", "Test content");
	QVERIFY2(QFile::exists(filePath),
		qPrintable(QString("Created file should exist: %1").arg(filePath)));

	QFileInfo info(filePath);
	QVERIFY(info.size() > 0);
	QCOMPARE(info.isFile(), true);
}

void IOTest::testNonExistentFileDetection()
{
	QString filePath = getTestPath("non_existent_file.txt");
	QVERIFY2(!QFile::exists(filePath),
		"Non-existent file should not be reported as existing");
}

void IOTest::testCreateMultipleFiles()
{
	QString file1 = createTestFile("file1.txt", "Content 1");
	QString file2 = createTestFile("file2.txt", "Content 2");
	QString file3 = createTestFile("file3.txt", "Content 3");

	QVERIFY(QFile::exists(file1));
	QVERIFY(QFile::exists(file2));
	QVERIFY(QFile::exists(file3));

	// Verify they are distinct files
	QVERIFY(file1 != file2);
	QVERIFY(file2 != file3);
}

void IOTest::testWriteAndReadBackContent()
{
	QString expected = "Hello, this is test content with special chars: \xC3\xB1\xC3\xBC";
	QString filePath = createTestFile("readback.txt", expected);
	QVERIFY(QFile::exists(filePath));

	QFile file(filePath);
	QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
	QTextStream stream(&file);
	QString actual = stream.readAll();
	file.close();

	QCOMPARE(actual, expected);
}

void IOTest::testLargeFileCreation()
{
	QString largeContent;
	largeContent.reserve(1024 * 1024);
	for (int i = 0; i < 10000; ++i)
	{
		largeContent += QString("Line %1: This is a test line with some content to make it reasonably long.\n").arg(i);
	}

	QString filePath = createTestFile("large_file.txt", largeContent);
	QFileInfo info(filePath);
	QVERIFY2(info.size() > 100000,
		qPrintable(QString("Large file should be >100KB, got %1 bytes").arg(info.size())));
}

void IOTest::testBinaryFileCreation()
{
	QString filePath = getTestPath("binary_file.bin");
	QFile file(filePath);
	QVERIFY(file.open(QIODevice::WriteOnly));

	QByteArray binaryData;
	for (int i = 0; i < 256; ++i)
	{
		binaryData.append(static_cast<char>(i));
	}
	qint64 written = file.write(binaryData);
	file.close();

	QCOMPARE(written, static_cast<qint64>(256));
	QVERIFY(QFile::exists(filePath));

	// Read back and verify
	QFile readFile(filePath);
	QVERIFY(readFile.open(QIODevice::ReadOnly));
	QByteArray readBack = readFile.readAll();
	readFile.close();
	QCOMPARE(readBack, binaryData);
}

void IOTest::testSymlinkCreation()
{
	QString targetFile = createTestFile("symlink_target.txt", "Target content");
	QString linkPath = getTestPath("symlink_link.txt");

	QFile::link(targetFile, linkPath);

	if (!QFile::exists(linkPath))
	{
		QSKIP("Symlinks not supported on this filesystem");
	}

	QFileInfo linkInfo(linkPath);
	QVERIFY(linkInfo.isSymLink());
	QCOMPARE(linkInfo.symLinkTarget(), targetFile);
}

// ============================================================================
// Notepad++ Core Tests - Require Full Application
// ============================================================================

void IOTest::testOpenWithEncoding()
{
	QSKIP("Requires full Notepad_plus core for encoding-aware file open");
}

void IOTest::testSaveNewFile()
{
	QSKIP("Requires full Notepad_plus core for buffer save operations");
}

void IOTest::testSaveExistingFile()
{
	QSKIP("Requires full Notepad_plus core for buffer save operations");
}

void IOTest::testSaveAs()
{
	QSKIP("Requires full Notepad_plus core for Save As operations");
}

void IOTest::testSaveAll()
{
	QSKIP("Requires full Notepad_plus core for Save All operations");
}

void IOTest::testCloseFile()
{
	QSKIP("Requires full Notepad_plus core for file close operations");
}

void IOTest::testCloseAllFiles()
{
	QSKIP("Requires full Notepad_plus core for Close All operations");
}

void IOTest::testCloseWithUnsavedChanges()
{
	QSKIP("Requires full Notepad_plus core for unsaved changes dialog");
}

void IOTest::testAddToRecentFiles()
{
	QSKIP("Requires Settings subsystem for recent file tracking");
}

void IOTest::testGetRecentFiles()
{
	QSKIP("Requires Settings subsystem for recent file tracking");
}

void IOTest::testClearRecentFiles()
{
	QSKIP("Requires Settings subsystem for recent file tracking");
}

void IOTest::testSaveSession()
{
	QSKIP("Requires full Notepad_plus core for session management");
}

void IOTest::testLoadSession()
{
	QSKIP("Requires full Notepad_plus core for session management");
}

void IOTest::testRestoreSession()
{
	QSKIP("Requires full Notepad_plus core for session management");
}

void IOTest::testSaveLargeFile()
{
	QSKIP("Requires full Notepad_plus core for buffer save operations");
}

void IOTest::testOpenNetworkFile()
{
	QSKIP("Requires network setup for network file access testing");
}

} // namespace Tests

#include "IOTest.moc"
