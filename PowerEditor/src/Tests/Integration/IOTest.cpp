// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "IOTest.h"
#include "../Common/TestUtils.h"
#include "../../Platform/Settings.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringConverter>
#include <QUuid>
#include <algorithm>

using PlatformLayer::ISettings;
using PlatformLayer::SessionInfo;

// ============================================================================
// MockSettings - minimal ISettings implementation for testing recent files
// ============================================================================
class MockSettings : public ISettings {
public:
	bool init() override { return true; }
	std::wstring getConfigPath() const override { return L"/tmp/test"; }
	std::wstring getSettingsDir() const override { return L"/tmp/test"; }
	std::wstring getUserPluginsDir() const override { return L"/tmp/test/plugins"; }

	bool writeInt(const std::wstring&, const std::wstring&, int) override { return true; }
	bool writeString(const std::wstring&, const std::wstring&, const std::wstring&) override { return true; }
	bool writeBool(const std::wstring&, const std::wstring&, bool) override { return true; }
	bool writeBinary(const std::wstring&, const std::wstring&, const uint8_t*, size_t) override { return true; }

	int readInt(const std::wstring&, const std::wstring&, int defaultValue) override { return defaultValue; }
	std::wstring readString(const std::wstring&, const std::wstring&, const std::wstring& defaultValue) override { return defaultValue; }
	bool readBool(const std::wstring&, const std::wstring&, bool defaultValue) override { return defaultValue; }
	std::vector<uint8_t> readBinary(const std::wstring&, const std::wstring&) override { return {}; }

	bool saveConfig() override { return true; }
	bool loadConfig() override { return true; }

	bool setXmlValue(const std::wstring&, const std::wstring&) override { return true; }
	bool setXmlValueInt(const std::wstring&, int) override { return true; }
	bool setXmlValueBool(const std::wstring&, bool) override { return true; }

	std::wstring getXmlValue(const std::wstring&, const std::wstring& defaultValue) override { return defaultValue; }
	int getXmlValueInt(const std::wstring&, int defaultValue) override { return defaultValue; }
	bool getXmlValueBool(const std::wstring&, bool defaultValue) override { return defaultValue; }

	bool saveSession(const SessionInfo&) override { return true; }
	bool loadSession(SessionInfo&) override { return true; }

	void addToRecentFiles(const std::wstring& filePath) override
	{
		auto it = std::find(_recentFiles.begin(), _recentFiles.end(), filePath);
		if (it != _recentFiles.end())
		{
			_recentFiles.erase(it);
		}
		_recentFiles.insert(_recentFiles.begin(), filePath);
	}

	std::vector<std::wstring> getRecentFiles() override
	{
		return _recentFiles;
	}

	void clearRecentFiles() override
	{
		_recentFiles.clear();
	}

	bool registerFileAssociation(const std::wstring&, const std::wstring&) override { return true; }
	bool unregisterFileAssociation(const std::wstring&) override { return true; }
	bool isFileAssociated(const std::wstring&) override { return false; }

	bool writePluginSetting(const std::wstring&, const std::wstring&, const std::wstring&) override { return true; }
	std::wstring readPluginSetting(const std::wstring&, const std::wstring&, const std::wstring& defaultValue) override { return defaultValue; }

private:
	std::vector<std::wstring> _recentFiles;
};

static MockSettings s_mockSettings;

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

void IOTest::init()
{
	ISettings::setTestInstance(&s_mockSettings);
}

void IOTest::cleanup()
{
	ISettings::resetTestInstance();
}

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
// Encoding and Save Tests Using QFile/QTextStream
// ============================================================================

void IOTest::testOpenWithEncoding()
{
	// Test UTF-8 with BOM
	{
		QString filePath = getTestPath("utf8bom_test.txt");
		QFile file(filePath);
		QVERIFY(file.open(QIODevice::WriteOnly));
		// Write UTF-8 BOM followed by content
		QByteArray bom("\xEF\xBB\xBF");
		QByteArray content = QString("Hello UTF-8 BOM \xC3\xA9\xC3\xA0\xC3\xBC").toUtf8();
		file.write(bom);
		file.write(content);
		file.close();

		// Read back with QTextStream (auto-detects BOM)
		QFile readFile(filePath);
		QVERIFY(readFile.open(QIODevice::ReadOnly));
		QTextStream stream(&readFile);
		QString actual = stream.readAll();
		readFile.close();

		QCOMPARE(actual, QString("Hello UTF-8 BOM \xC3\xA9\xC3\xA0\xC3\xBC"));
	}

	// Test UTF-16LE
	{
		QString filePath = getTestPath("utf16le_test.txt");
		QString expected = QString::fromUtf8("Hello UTF-16LE \xC3\xA9\xC3\xA0\xC3\xBC");

		// Write as UTF-16LE
		QFile writeFile(filePath);
		QVERIFY(writeFile.open(QIODevice::WriteOnly));
		QTextStream writeStream(&writeFile);
		writeStream.setEncoding(QStringConverter::Utf16LE);
		writeStream.setGenerateByteOrderMark(true);
		writeStream << expected;
		writeStream.flush();
		writeFile.close();

		// Read back with matching encoding
		QFile readFile(filePath);
		QVERIFY(readFile.open(QIODevice::ReadOnly));
		QTextStream readStream(&readFile);
		readStream.setEncoding(QStringConverter::Utf16LE);
		QString actual = readStream.readAll();
		readFile.close();

		QCOMPARE(actual, expected);
	}
}

void IOTest::testSaveNewFile()
{
	QString uniqueName = QString("new_file_%1.txt").arg(QUuid::createUuid().toString(QUuid::Id128).left(8));
	QString filePath = getTestPath(uniqueName);

	QVERIFY2(!QFile::exists(filePath), "File should not exist before save");

	QString content = "This is new file content.\nSecond line.\n";

	QFile file(filePath);
	QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
	QTextStream stream(&file);
	stream << content;
	stream.flush();
	file.close();

	QVERIFY2(QFile::exists(filePath), "File should exist after save");

	// Read back and verify
	QFile readFile(filePath);
	QVERIFY(readFile.open(QIODevice::ReadOnly | QIODevice::Text));
	QTextStream readStream(&readFile);
	QString actual = readStream.readAll();
	readFile.close();

	QCOMPARE(actual, content);
}

void IOTest::testSaveExistingFile()
{
	QString filePath = createTestFile("existing_file.txt", "Original content");
	QVERIFY(QFile::exists(filePath));

	// Verify original content
	{
		QFile file(filePath);
		QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
		QTextStream stream(&file);
		QCOMPARE(stream.readAll(), QString("Original content"));
		file.close();
	}

	// Overwrite with new content
	QString newContent = "Updated content with more data";
	{
		QFile file(filePath);
		QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
		QTextStream stream(&file);
		stream << newContent;
		stream.flush();
		file.close();
	}

	// Read back and verify new content
	{
		QFile file(filePath);
		QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
		QTextStream stream(&file);
		QString actual = stream.readAll();
		file.close();
		QCOMPARE(actual, newContent);
	}
}

void IOTest::testAddToRecentFiles()
{
	auto& settings = ISettings::getInstance();
	settings.clearRecentFiles();

	settings.addToRecentFiles(L"/tmp/test/file1.txt");

	auto files = settings.getRecentFiles();
	QCOMPARE(static_cast<int>(files.size()), 1);
	QCOMPARE(files[0], std::wstring(L"/tmp/test/file1.txt"));
}

void IOTest::testGetRecentFiles()
{
	auto& settings = ISettings::getInstance();
	settings.clearRecentFiles();

	settings.addToRecentFiles(L"/tmp/test/first.txt");
	settings.addToRecentFiles(L"/tmp/test/second.txt");
	settings.addToRecentFiles(L"/tmp/test/third.txt");

	auto files = settings.getRecentFiles();
	QCOMPARE(static_cast<int>(files.size()), 3);
	// Most recently added should be first
	QCOMPARE(files[0], std::wstring(L"/tmp/test/third.txt"));
	QCOMPARE(files[1], std::wstring(L"/tmp/test/second.txt"));
	QCOMPARE(files[2], std::wstring(L"/tmp/test/first.txt"));

	// Adding a duplicate should move it to the front
	settings.addToRecentFiles(L"/tmp/test/first.txt");
	files = settings.getRecentFiles();
	QCOMPARE(static_cast<int>(files.size()), 3);
	QCOMPARE(files[0], std::wstring(L"/tmp/test/first.txt"));
}

void IOTest::testClearRecentFiles()
{
	auto& settings = ISettings::getInstance();

	settings.addToRecentFiles(L"/tmp/test/file1.txt");
	settings.addToRecentFiles(L"/tmp/test/file2.txt");
	QVERIFY(!settings.getRecentFiles().empty());

	settings.clearRecentFiles();

	auto files = settings.getRecentFiles();
	QVERIFY2(files.empty(), "Recent files list should be empty after clear");
}

} // namespace Tests

#include "IOTest.moc"
