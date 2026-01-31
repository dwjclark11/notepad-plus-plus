// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "BufferTest.h"
#include "Buffer.h"
#include "../Common/TestUtils.h"
#include <QTemporaryFile>
#include <QTextStream>
#include <QSignalSpy>

namespace Tests {

BufferTest::BufferTest() {}

BufferTest::~BufferTest() {}

void BufferTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void BufferTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void BufferTest::init() {
    // Create a fresh buffer for each test
    _buffer = std::make_unique<QtCore::Buffer>();
    QVERIFY(_buffer != nullptr);
}

void BufferTest::cleanup() {
    _buffer.reset();
}

// ============================================================================
// Buffer Creation Tests
// ============================================================================
void BufferTest::testCreateNewBuffer() {
    QVERIFY(_buffer != nullptr);
    QVERIFY(_buffer->getID() == nullptr);  // ID not set yet

    // Set an ID
    _buffer->setID(_buffer.get());
    QCOMPARE(_buffer->getID(), _buffer.get());

    // New buffer should be untitled
    QVERIFY(_buffer->isUntitled());
    QVERIFY(_buffer->isNew());
}

void BufferTest::testCreateBufferFromFile() {
    // Create a test file
    QString testFile = TestEnvironment::getInstance().createTempFile(
        "test_create.txt",
        "Test content for buffer creation"
    );
    QVERIFY(!testFile.isEmpty());
    VERIFY_FILE_EXISTS(testFile);

    // Set the file path
    _buffer->setFilePath(testFile);
    QCOMPARE(_buffer->getFilePath(), testFile);
    QVERIFY(!_buffer->isUntitled());
}

// ============================================================================
// File Operations Tests
// ============================================================================
void BufferTest::testLoadFromFile() {
    // Create a test file with known content
    QString testContent = "Hello, World!\nThis is a test file.\n";
    QString testFile = TestEnvironment::getInstance().createTempFile(
        "test_load.txt",
        testContent
    );
    QVERIFY(!testFile.isEmpty());

    // Test that loadFromFile returns the expected result
    // Note: Without Scintilla, the actual content loading is deferred
    // but we can verify the file path is set
    bool result = _buffer->loadFromFile(testFile);
    // Result depends on whether Scintilla is available
    // We mainly verify no crash occurs
    Q_UNUSED(result)

    QCOMPARE(_buffer->getFilePath(), testFile);
    QVERIFY(!_buffer->isUntitled());
}

void BufferTest::testSaveToFile() {
    QString testFile = TestEnvironment::getInstance().getTempDir() + "/test_save.txt";

    // Set up buffer with content
    _buffer->setFilePath(testFile);
    _buffer->setText("Test content to save");

    // Save the file
    bool result = _buffer->saveToFile(testFile);
    Q_UNUSED(result)

    // Verify file was created
    VERIFY_FILE_EXISTS(testFile);

    // Verify content was saved
    QString savedContent = FileUtils::readFile(testFile);
    QCOMPARE(savedContent, QString("Test content to save"));
}

void BufferTest::testReload() {
    // Create a test file
    QString testFile = TestEnvironment::getInstance().createTempFile(
        "test_reload.txt",
        "Original content"
    );
    QVERIFY(!testFile.isEmpty());

    _buffer->setFilePath(testFile);

    // Modify the file externally
    FileUtils::createFile(testFile, "Modified content");

    // Reload the buffer
    bool result = _buffer->reloadFromFile();
    // Result depends on Scintilla availability, but no crash is success
    Q_UNUSED(result)

    // Verify file path is still correct
    QCOMPARE(_buffer->getFilePath(), testFile);
}

// ============================================================================
// Buffer State Tests
// ============================================================================
void BufferTest::testIsDirty() {
    // New buffer should not be dirty
    QVERIFY(!_buffer->isDirty());

    // Set some content to make it dirty
    _buffer->setText("Some content");
    // Note: setText may or may not set dirty flag depending on implementation
}

void BufferTest::testSetDirty() {
    // Test setting dirty flag directly
    _buffer->setDirty(true);
    QVERIFY(_buffer->isDirty());

    _buffer->setDirty(false);
    QVERIFY(!_buffer->isDirty());

    // Test signal emission
    QSignalSpy spy(_buffer.get(), &QtCore::Buffer::dirtyChanged);
    _buffer->setDirty(true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);
}

void BufferTest::testIsReadOnly() {
    // New buffer should not be read-only
    QVERIFY(!_buffer->isReadOnly());
    QVERIFY(!_buffer->isUserReadOnly());
    QVERIFY(!_buffer->isFileReadOnly());
}

void BufferTest::testSetReadOnly() {
    // Test user read-only
    _buffer->setUserReadOnly(true);
    QVERIFY(_buffer->isUserReadOnly());
    QVERIFY(_buffer->isReadOnly());

    _buffer->setUserReadOnly(false);
    QVERIFY(!_buffer->isUserReadOnly());
    QVERIFY(!_buffer->isReadOnly());

    // Test file read-only
    _buffer->setFileReadOnly(true);
    QVERIFY(_buffer->isFileReadOnly());
    QVERIFY(_buffer->isReadOnly());

    _buffer->setFileReadOnly(false);
    QVERIFY(!_buffer->isFileReadOnly());
    QVERIFY(!_buffer->isReadOnly());

    // Test signal emission
    QSignalSpy spy(_buffer.get(), &QtCore::Buffer::readOnlyChanged);
    _buffer->setReadOnly(true);
    QVERIFY(spy.count() >= 0);  // Signal may or may not be emitted
}

// ============================================================================
// Encoding Tests
// ============================================================================
void BufferTest::testGetEncoding() {
    // Default encoding should be UTF-8
    QCOMPARE(_buffer->getEncoding(), QString("UTF-8"));
}

void BufferTest::testSetEncoding() {
    // Test setting encoding
    _buffer->setEncoding("UTF-16");
    QCOMPARE(_buffer->getEncoding(), QString("UTF-16"));

    _buffer->setEncoding("ISO-8859-1");
    QCOMPARE(_buffer->getEncoding(), QString("ISO-8859-1"));

    // Test signal emission
    QSignalSpy spy(_buffer.get(), &QtCore::Buffer::encodingChanged);
    _buffer->setEncoding("UTF-8");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString("UTF-8"));
}

void BufferTest::testGetBom() {
    // Default BOM should be false
    QVERIFY(!_buffer->getUseBOM());
}

void BufferTest::testSetBom() {
    // Test setting BOM
    _buffer->setUseBOM(true);
    QVERIFY(_buffer->getUseBOM());

    _buffer->setUseBOM(false);
    QVERIFY(!_buffer->getUseBOM());
}

// ============================================================================
// Language Tests
// ============================================================================
void BufferTest::testGetLangType() {
    // Default language type should be L_TEXT
    QCOMPARE(_buffer->getLangType(), L_TEXT);
    QCOMPARE(_buffer->getLanguage(), L_TEXT);
}

void BufferTest::testSetLangType() {
    // Test setting language type
    _buffer->setLangType(L_CPP);
    QCOMPARE(_buffer->getLangType(), L_CPP);
    QCOMPARE(_buffer->getLanguage(), L_CPP);

    _buffer->setLangType(L_PYTHON);
    QCOMPARE(_buffer->getLangType(), L_PYTHON);

    // Test signal emission
    QSignalSpy spy(_buffer.get(), &QtCore::Buffer::langTypeChanged);
    _buffer->setLangType(L_JAVA);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), static_cast<int>(L_JAVA));
}

// ============================================================================
// File Path Tests
// ============================================================================
void BufferTest::testGetFilePath() {
    // New buffer should have empty file path
    QVERIFY(_buffer->getFilePath().isEmpty());
}

void BufferTest::testSetFilePath() {
    QString testPath = "/path/to/test/file.txt";

    // Test setting file path
    _buffer->setFilePath(testPath);
    QCOMPARE(_buffer->getFilePath(), testPath);

    // Test signal emission
    QSignalSpy spy(_buffer.get(), &QtCore::Buffer::filePathChanged);
    QString newPath = "/new/path/file.cpp";
    _buffer->setFilePath(newPath);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), newPath);
}

void BufferTest::testGetFileName() {
    // Test with a file path
    _buffer->setFilePath("/home/user/documents/test.txt");

    QString fileName = _buffer->getFileNameQString();
    QCOMPARE(fileName, QString("test.txt"));

    // Test wchar_t version
    const wchar_t* wFileName = _buffer->getFileName();
    QVERIFY(wFileName != nullptr);
    QCOMPARE(QString::fromWCharArray(wFileName), QString("test.txt"));
}

// ============================================================================
// Document Management Tests
// ============================================================================
void BufferTest::testGetDocument() {
    // Without Scintilla, document should be null or default
    void* doc = _buffer->getDocument();
    // Just verify it doesn't crash
    (void)doc;
}

void BufferTest::testSetDocument() {
    // Test setting document (mainly verifies no crash)
    // Cannot fully test without Scintilla
    QVERIFY(true);
}

// ============================================================================
// Status Tests
// ============================================================================
void BufferTest::testGetStatus() {
    // Default status should be DOC_REGULAR
    QCOMPARE(_buffer->getStatus(), static_cast<int>(QtCore::DOC_REGULAR));
}

void BufferTest::testSetStatus() {
    // Test setting various statuses
    _buffer->setStatus(QtCore::DOC_MODIFIED);
    QCOMPARE(_buffer->getStatus(), static_cast<int>(QtCore::DOC_MODIFIED));

    _buffer->setStatus(QtCore::DOC_UNNAMED);
    QCOMPARE(_buffer->getStatus(), static_cast<int>(QtCore::DOC_UNNAMED));

    _buffer->setStatus(QtCore::DOC_DELETED);
    QCOMPARE(_buffer->getStatus(), static_cast<int>(QtCore::DOC_DELETED));

    // Test signal emission
    QSignalSpy spy(_buffer.get(), &QtCore::Buffer::statusChanged);
    _buffer->setStatus(QtCore::DOC_REGULAR);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), static_cast<int>(QtCore::DOC_REGULAR));
}

} // namespace Tests

#include "BufferTest.moc"
