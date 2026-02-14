// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FileWatcherTest.h"
#include "FileWatcher.h"
#include "../Common/TestUtils.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDir>

using namespace PlatformLayer;

namespace Tests {

FileWatcherTest::FileWatcherTest() {}

FileWatcherTest::~FileWatcherTest() {}

void FileWatcherTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _watcher = &IFileWatcher::getInstance();
    QVERIFY(_watcher != nullptr);
}

void FileWatcherTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void FileWatcherTest::init() {
    _tempDir = std::make_unique<QTemporaryDir>();
    QVERIFY(_tempDir->isValid());
    _tempPath = _tempDir->path();

    // Unwatch all from previous tests
    _watcher->unwatchAll();
}

void FileWatcherTest::cleanup() {
    _watcher->unwatchAll();
    _tempDir.reset();
}

QString FileWatcherTest::createTestFile(const QString& fileName, const QString& content) {
    QString fullPath = QDir(_tempPath).filePath(fileName);
    QDir().mkpath(QFileInfo(fullPath).path());

    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
    }
    return fullPath;
}

QString FileWatcherTest::getTestPath(const QString& relativePath) {
    return QDir(_tempPath).filePath(relativePath);
}

void FileWatcherTest::waitForEvents(int timeoutMs) {
    QThread::msleep(timeoutMs);
    _watcher->processEvents(0);
}

// ============================================================================
// Directory Watching Tests
// ============================================================================
void FileWatcherTest::testWatchDirectory() {
    QString dirPath = _tempPath;

    bool callbackReceived = false;
    FileChangeCallback callback = [&callbackReceived](const FileChangeEvent& event) {
        Q_UNUSED(event)
        callbackReceived = true;
    };

    FileWatchOptions options;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);

    QVERIFY(handle != INVALID_WATCH_HANDLE);
    QVERIFY(_watcher->isWatchValid(handle));
}

void FileWatcherTest::testUnwatchDirectory() {
    QString dirPath = _tempPath;

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    QVERIFY(_watcher->unwatchDirectory(handle));
    QVERIFY(!_watcher->isWatchValid(handle));
}

void FileWatcherTest::testWatchDirectoryWithOptions() {
    QString dirPath = _tempPath;

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    options.watchSubtree = true;
    options.watchFileSize = true;
    options.watchLastWrite = true;
    options.watchAttributes = false;

    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);
}

// ============================================================================
// File Watching Tests
// ============================================================================
void FileWatcherTest::testWatchFile() {
    QString filePath = createTestFile("watch_test.txt", "initial content");

    bool callbackReceived = false;
    FileChangeCallback callback = [&callbackReceived](const FileChangeEvent& event) {
        Q_UNUSED(event)
        callbackReceived = true;
    };

    FileWatchHandle handle = _watcher->watchFile(filePath.toStdWString(), callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);
    QVERIFY(_watcher->isWatchValid(handle));
}

void FileWatcherTest::testUnwatchFile() {
    QString filePath = createTestFile("unwatch_test.txt", "content");

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchHandle handle = _watcher->watchFile(filePath.toStdWString(), callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    QVERIFY(_watcher->unwatchFile(handle));
    QVERIFY(!_watcher->isWatchValid(handle));
}

// ============================================================================
// Event Processing Tests
// ============================================================================
void FileWatcherTest::testProcessEvents() {
    // Watch a file (not directory) so processEvents() can detect timestamp changes
    QString filePath = createTestFile("process_events_test.txt", "initial content");

    int eventCount = 0;
    FileChangeCallback callback = [&eventCount](const FileChangeEvent& event) {
        Q_UNUSED(event)
        eventCount++;
    };

    FileWatchHandle handle = _watcher->watchFile(filePath.toStdWString(), callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    // Modify the file to trigger a change
    QThread::msleep(100);
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << "modified content for event processing";
    file.close();

    // Allow filesystem timestamps to update
    QThread::msleep(200);
    size_t processed = _watcher->processEvents(1000);

    QVERIFY(processed > 0);
    QVERIFY(eventCount > 0);
}

void FileWatcherTest::testHasPendingEvents() {
    QString dirPath = _tempPath;

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    // The Linux implementation (QFileSystemWatcher-based) always returns false
    // because Qt uses signals rather than a pollable event queue.
    // Verify the method returns a consistent boolean value without crashing.
    bool hasPending = _watcher->hasPendingEvents();
    QCOMPARE(hasPending, false);
}

// ============================================================================
// Watch Utilities Tests
// ============================================================================
void FileWatcherTest::testSetWatchEnabled() {
    QString dirPath = _tempPath;

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    // Disable the watch
    QVERIFY(_watcher->setWatchEnabled(handle, false));

    // Re-enable the watch
    QVERIFY(_watcher->setWatchEnabled(handle, true));
}

void FileWatcherTest::testUpdateWatchOptions() {
    QString dirPath = _tempPath;

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    options.watchSubtree = false;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    // Update options
    FileWatchOptions newOptions;
    newOptions.watchSubtree = true;
    QVERIFY(_watcher->updateWatchOptions(handle, newOptions));
}

void FileWatcherTest::testIsWatchValid() {
    // Invalid handle should return false
    QVERIFY(!_watcher->isWatchValid(INVALID_WATCH_HANDLE));
    QVERIFY(!_watcher->isWatchValid(999999));

    // Valid handle should return true
    QString dirPath = _tempPath;
    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);
    QVERIFY(_watcher->isWatchValid(handle));
}

void FileWatcherTest::testGetWatchPath() {
    QString dirPath = _tempPath;

    FileChangeCallback callback = [](const FileChangeEvent& event) {
        Q_UNUSED(event)
    };

    FileWatchOptions options;
    FileWatchHandle handle = _watcher->watchDirectory(dirPath.toStdWString(), options, callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    std::wstring watchPath = _watcher->getWatchPath(handle);
    QVERIFY(!watchPath.empty());
}

// ============================================================================
// Utility Functions Tests
// ============================================================================
void FileWatcherTest::testChangeTypeToString() {
    const wchar_t* str = FileWatcherUtils::changeTypeToString(FileChangeType::Created);
    QVERIFY(str != nullptr);

    str = FileWatcherUtils::changeTypeToString(FileChangeType::Modified);
    QVERIFY(str != nullptr);

    str = FileWatcherUtils::changeTypeToString(FileChangeType::Deleted);
    QVERIFY(str != nullptr);

    str = FileWatcherUtils::changeTypeToString(FileChangeType::Unknown);
    QVERIFY(str != nullptr);
}

void FileWatcherTest::testIsContentModified() {
    QVERIFY(FileWatcherUtils::isContentModified(FileChangeType::Modified));
    QVERIFY(FileWatcherUtils::isContentModified(FileChangeType::Created));
    QVERIFY(FileWatcherUtils::isContentModified(FileChangeType::Deleted));
    QVERIFY(!FileWatcherUtils::isContentModified(FileChangeType::Attributes));
    QVERIFY(!FileWatcherUtils::isContentModified(FileChangeType::Security));
}

// ============================================================================
// DirectoryWatcher Class Tests
// ============================================================================
void FileWatcherTest::testDirectoryWatcher() {
    DirectoryWatcher dirWatcher;
    dirWatcher.init();

    QString dirPath = _tempPath;
    dirWatcher.addDirectory(dirPath.toStdWString(), false, 0xFF);

    // Qt-based implementation returns nullptr for wait handle (uses signals instead)
    void* waitHandle = dirWatcher.getWaitHandle();
    QCOMPARE(waitHandle, nullptr);

    // Initially there should be no events in the queue
    QVERIFY(!dirWatcher.hasEvents());

    // Verify pop returns false when no events are queued
    FileChangeType type;
    std::wstring filename;
    QVERIFY(!dirWatcher.pop(type, filename));

    dirWatcher.terminate();
}

// ============================================================================
// FileWatcher Class Tests
// ============================================================================
void FileWatcherTest::testFileWatcherClass() {
    QString filePath = createTestFile("file_watcher_test.txt", "initial");

    FileWatcher fileWatcher;
    fileWatcher.addFile(filePath.toStdWString());

    // Initially should not detect changes
    QVERIFY(!fileWatcher.detectChanges());

    // Ensure at least 1 second passes so the timestamp differs
    // (filesystem timestamp resolution may be 1 second on some systems)
    QThread::msleep(1100);

    // Modify the file
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << "modified content that is different";
    file.close();

    // Give filesystem time to flush metadata
    QThread::msleep(200);

    // Now should detect the change based on timestamp comparison
    QVERIFY(fileWatcher.detectChanges());

    // After detecting, a second call without further changes should return false
    QVERIFY(!fileWatcher.detectChanges());

    fileWatcher.terminate();
}

// ============================================================================
// Integration Test
// ============================================================================
void FileWatcherTest::testFileChangeDetection() {
    // Use file watching (not directory) since processEvents() polls file timestamps
    QString filePath = createTestFile("integration_test.txt", "initial content");

    FileChangeEvent capturedEvent;
    bool eventReceived = false;

    FileChangeCallback callback = [&capturedEvent, &eventReceived](const FileChangeEvent& event) {
        capturedEvent = event;
        eventReceived = true;
    };

    FileWatchHandle handle = _watcher->watchFile(filePath.toStdWString(), callback);
    QVERIFY(handle != INVALID_WATCH_HANDLE);

    // Ensure timestamp will differ (filesystem resolution may be 1 second)
    QThread::msleep(1100);

    // Modify the file
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << "modified content for integration test";
    file.close();

    // Allow filesystem metadata to update
    QThread::msleep(200);
    _watcher->processEvents(1000);

    // Verify the callback was actually invoked with correct event details
    QVERIFY(eventReceived);
    QCOMPARE(capturedEvent.type, FileChangeType::Modified);
    QVERIFY(!capturedEvent.path.empty());
    QVERIFY(!capturedEvent.isDirectory);
}

} // namespace Tests

#include "FileWatcherTest.moc"
