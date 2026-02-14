// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>

namespace Tests {

class IOTest : public QObject {
	Q_OBJECT

public:
	IOTest();
	~IOTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	// File creation and existence (real tests using QFile)
	void testCreateAndVerifyFile();
	void testNonExistentFileDetection();
	void testCreateMultipleFiles();
	void testWriteAndReadBackContent();
	void testLargeFileCreation();
	void testBinaryFileCreation();
	void testSymlinkCreation();

	// Notepad++ file open operations (require core)
	void testOpenWithEncoding();

	// Notepad++ file save operations (require core)
	void testSaveNewFile();
	void testSaveExistingFile();
	void testSaveAs();
	void testSaveAll();

	// Notepad++ file close operations (require core)
	void testCloseFile();
	void testCloseAllFiles();
	void testCloseWithUnsavedChanges();

	// Recent files (require Settings)
	void testAddToRecentFiles();
	void testGetRecentFiles();
	void testClearRecentFiles();

	// Session management (require core)
	void testSaveSession();
	void testLoadSession();
	void testRestoreSession();

	// Large file handling via core (require core)
	void testSaveLargeFile();

	// Network files (require network setup)
	void testOpenNetworkFile();

private:
	QString createTestFile(const QString& fileName, const QString& content);
	QString getTestPath(const QString& relativePath);
};

} // namespace Tests
