// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DockingManagerTest.h"
#include "DockingManager.h"
#include "../Common/TestUtils.h"
#include <QWidget>
#include <QLabel>

using namespace QtControls::DockingManager;

namespace Tests {

DockingManagerTest::DockingManagerTest() {}

DockingManagerTest::~DockingManagerTest() {}

void DockingManagerTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void DockingManagerTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void DockingManagerTest::init() {
    _mainWindow = std::make_unique<QMainWindow>();
    _mainWindow->resize(1024, 768);
    _dockingManager = std::make_unique<Manager>();
}

void DockingManagerTest::cleanup() {
    _dockingManager.reset();
    _mainWindow.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void DockingManagerTest::testInit() {
    _dockingManager->init(_mainWindow.get());
    QVERIFY(_dockingManager->getWidget() != nullptr);
}

void DockingManagerTest::testDestroy() {
    _dockingManager->init(_mainWindow.get());
    _dockingManager->destroy();
    QVERIFY(_dockingManager->getWidget() == nullptr);
}

// ============================================================================
// Panel Management Tests
// ============================================================================
void DockingManagerTest::testAddPanel() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right, "Test Panel");

    QVERIFY(_dockingManager->hasPanel("testPanel"));
    QCOMPARE(_dockingManager->getPanelCount(), 1);
}

void DockingManagerTest::testRemovePanel() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);
    QVERIFY(_dockingManager->hasPanel("testPanel"));

    _dockingManager->removePanel("testPanel");
    QVERIFY(!_dockingManager->hasPanel("testPanel"));
    QCOMPARE(_dockingManager->getPanelCount(), 0);
}

void DockingManagerTest::testHasPanel() {
    _dockingManager->init(_mainWindow.get());

    QVERIFY(!_dockingManager->hasPanel("nonExistent"));

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);
    QVERIFY(_dockingManager->hasPanel("testPanel"));
}

// ============================================================================
// Visibility Tests
// ============================================================================
void DockingManagerTest::testShowPanel() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);
    _dockingManager->hidePanel("testPanel");
    QVERIFY(!_dockingManager->isPanelVisible("testPanel"));

    _dockingManager->showPanel("testPanel");
    QVERIFY(_dockingManager->isPanelVisible("testPanel"));
}

void DockingManagerTest::testHidePanel() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);
    QVERIFY(_dockingManager->isPanelVisible("testPanel"));

    _dockingManager->hidePanel("testPanel");
    QVERIFY(!_dockingManager->isPanelVisible("testPanel"));
}

void DockingManagerTest::testTogglePanel() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);
    QVERIFY(_dockingManager->isPanelVisible("testPanel"));

    _dockingManager->togglePanel("testPanel");
    QVERIFY(!_dockingManager->isPanelVisible("testPanel"));

    _dockingManager->togglePanel("testPanel");
    QVERIFY(_dockingManager->isPanelVisible("testPanel"));
}

void DockingManagerTest::testIsPanelVisible() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);

    QVERIFY(_dockingManager->isPanelVisible("testPanel"));

    _dockingManager->hidePanel("testPanel");
    QVERIFY(!_dockingManager->isPanelVisible("testPanel"));
}

// ============================================================================
// Panel Properties Tests
// ============================================================================
void DockingManagerTest::testSetPanelArea() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);
    QCOMPARE(_dockingManager->getPanelArea("testPanel"), DockArea::Right);

    _dockingManager->setPanelArea("testPanel", DockArea::Left);
    QCOMPARE(_dockingManager->getPanelArea("testPanel"), DockArea::Left);
}

void DockingManagerTest::testGetPanelArea() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Bottom);
    QCOMPARE(_dockingManager->getPanelArea("testPanel"), DockArea::Bottom);
}

void DockingManagerTest::testSetPanelTitle() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right, "Original Title");
    QCOMPARE(_dockingManager->getPanelTitle("testPanel"), QString("Original Title"));

    _dockingManager->setPanelTitle("testPanel", "New Title");
    QCOMPARE(_dockingManager->getPanelTitle("testPanel"), QString("New Title"));
}

void DockingManagerTest::testGetPanelTitle() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right, "My Panel");
    QCOMPARE(_dockingManager->getPanelTitle("testPanel"), QString("My Panel"));
}

void DockingManagerTest::testGetPanelWidget() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);

    QWidget* retrieved = _dockingManager->getPanelWidget("testPanel");
    QCOMPARE(retrieved, panel);
}

// ============================================================================
// Layout Tests
// ============================================================================
void DockingManagerTest::testSaveLayout() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel1 = new QLabel("Panel 1");
    QWidget* panel2 = new QLabel("Panel 2");
    _dockingManager->addPanel("panel1", panel1, DockArea::Right);
    _dockingManager->addPanel("panel2", panel2, DockArea::Left);

    QByteArray layout = _dockingManager->saveLayout();
    QVERIFY(!layout.isEmpty());
}

void DockingManagerTest::testRestoreLayout() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel1 = new QLabel("Panel 1");
    _dockingManager->addPanel("panel1", panel1, DockArea::Right);

    QByteArray layout = _dockingManager->saveLayout();
    QVERIFY(!layout.isEmpty());

    // Modify layout
    _dockingManager->hidePanel("panel1");

    // Restore layout
    _dockingManager->restoreLayout(layout);
    QVERIFY(true); // Should not crash
}

void DockingManagerTest::testSetTabbedDocking() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel1 = new QLabel("Panel 1");
    QWidget* panel2 = new QLabel("Panel 2");
    _dockingManager->addPanel("panel1", panel1, DockArea::Right);
    _dockingManager->addPanel("panel2", panel2, DockArea::Right);

    // Should not crash
    _dockingManager->setTabbedDocking("panel1", "panel2");
    QVERIFY(true);
}

// ============================================================================
// Batch Operations Tests
// ============================================================================
void DockingManagerTest::testShowAllPanels() {
    _dockingManager->init(_mainWindow.get());

    _dockingManager->addPanel("panel1", new QLabel("Panel 1"), DockArea::Right);
    _dockingManager->addPanel("panel2", new QLabel("Panel 2"), DockArea::Left);
    _dockingManager->hideAllPanels();

    QVERIFY(!_dockingManager->isPanelVisible("panel1"));
    QVERIFY(!_dockingManager->isPanelVisible("panel2"));

    _dockingManager->showAllPanels();

    QVERIFY(_dockingManager->isPanelVisible("panel1"));
    QVERIFY(_dockingManager->isPanelVisible("panel2"));
}

void DockingManagerTest::testHideAllPanels() {
    _dockingManager->init(_mainWindow.get());

    _dockingManager->addPanel("panel1", new QLabel("Panel 1"), DockArea::Right);
    _dockingManager->addPanel("panel2", new QLabel("Panel 2"), DockArea::Left);

    QVERIFY(_dockingManager->isPanelVisible("panel1"));
    QVERIFY(_dockingManager->isPanelVisible("panel2"));

    _dockingManager->hideAllPanels();

    QVERIFY(!_dockingManager->isPanelVisible("panel1"));
    QVERIFY(!_dockingManager->isPanelVisible("panel2"));
}

void DockingManagerTest::testGetPanelNames() {
    _dockingManager->init(_mainWindow.get());

    _dockingManager->addPanel("panel1", new QLabel("Panel 1"), DockArea::Right);
    _dockingManager->addPanel("panel2", new QLabel("Panel 2"), DockArea::Left);

    QStringList names = _dockingManager->getPanelNames();
    QCOMPARE(names.size(), 2);
    QVERIFY(names.contains("panel1"));
    QVERIFY(names.contains("panel2"));
}

void DockingManagerTest::testGetVisiblePanels() {
    _dockingManager->init(_mainWindow.get());

    _dockingManager->addPanel("panel1", new QLabel("Panel 1"), DockArea::Right);
    _dockingManager->addPanel("panel2", new QLabel("Panel 2"), DockArea::Left);
    _dockingManager->hidePanel("panel2");

    QStringList visible = _dockingManager->getVisiblePanels();
    QCOMPARE(visible.size(), 1);
    QVERIFY(visible.contains("panel1"));
    QVERIFY(!visible.contains("panel2"));
}

// ============================================================================
// Features Tests
// ============================================================================
void DockingManagerTest::testSetPanelFeatures() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);

    // Should not crash
    _dockingManager->setPanelFeatures("testPanel", false, true, true);
    QVERIFY(true);
}

void DockingManagerTest::testRaisePanel() {
    _dockingManager->init(_mainWindow.get());

    QWidget* panel = new QLabel("Test Panel");
    _dockingManager->addPanel("testPanel", panel, DockArea::Right);

    // Should not crash
    _dockingManager->raisePanel("testPanel");
    QVERIFY(true);
}

void DockingManagerTest::testGetPanelCount() {
    _dockingManager->init(_mainWindow.get());

    QCOMPARE(_dockingManager->getPanelCount(), 0);

    _dockingManager->addPanel("panel1", new QLabel("Panel 1"), DockArea::Right);
    QCOMPARE(_dockingManager->getPanelCount(), 1);

    _dockingManager->addPanel("panel2", new QLabel("Panel 2"), DockArea::Left);
    QCOMPARE(_dockingManager->getPanelCount(), 2);

    _dockingManager->removePanel("panel1");
    QCOMPARE(_dockingManager->getPanelCount(), 1);
}

} // namespace Tests

#include "DockingManagerTest.moc"
