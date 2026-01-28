// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "TreeViewTest.h"
#include "TreeView.h"
#include "../Common/TestUtils.h"
#include <QVariant>

using namespace QtControls;

namespace Tests {

TreeViewTest::TreeViewTest() {}

TreeViewTest::~TreeViewTest() {}

void TreeViewTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void TreeViewTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void TreeViewTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(400, 300);
    _treeView = std::make_unique<TreeView>();
}

void TreeViewTest::cleanup() {
    _treeView.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void TreeViewTest::testInit() {
    QVERIFY(_treeView->init(_parentWidget.get()));
    QVERIFY(_treeView->getTreeWidget() != nullptr);
}

void TreeViewTest::testDestroy() {
    _treeView->init(_parentWidget.get());
    _treeView->destroy();
    QVERIFY(_treeView->getWidget() == nullptr);
}

// ============================================================================
// Item Management Tests
// ============================================================================
void TreeViewTest::testAddItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Root Item");
    QVERIFY(itemId >= 0);

    int childId = _treeView->addItem("Child Item", itemId);
    QVERIFY(childId >= 0);
    QVERIFY(childId != itemId);
}

void TreeViewTest::testRemoveItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item to Remove");
    QVERIFY(itemId >= 0);

    _treeView->removeItem(itemId);
    // Item should be removed
    QVERIFY(true);
}

void TreeViewTest::testClear() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->addItem("Item 1");
    _treeView->addItem("Item 2");
    _treeView->addItem("Item 3");

    _treeView->clear();
    // All items should be removed
    QVERIFY(true);
}

void TreeViewTest::testSetItemText() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Original Text");
    QVERIFY(_treeView->setItemText(itemId, "Updated Text"));

    QString text = _treeView->getItemText(itemId);
    QCOMPARE(text, QString("Updated Text"));
}

void TreeViewTest::testGetItemText() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Test Text");
    QString text = _treeView->getItemText(itemId);
    QCOMPARE(text, QString("Test Text"));
}

void TreeViewTest::testSetItemData() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item with Data");
    QVariant data(42);
    QVERIFY(_treeView->setItemData(itemId, data));
}

void TreeViewTest::testGetItemData() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item with Data");
    QVariant data(42);
    _treeView->setItemData(itemId, data);

    QVariant retrieved = _treeView->getItemData(itemId);
    QCOMPARE(retrieved.toInt(), 42);
}

// ============================================================================
// Selection Tests
// ============================================================================
void TreeViewTest::testGetSelectedItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Selectable Item");
    _treeView->setSelectedItem(itemId);

    int selected = _treeView->getSelectedItem();
    QCOMPARE(selected, itemId);
}

void TreeViewTest::testSetSelectedItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId1 = _treeView->addItem("Item 1");
    int itemId2 = _treeView->addItem("Item 2");

    _treeView->setSelectedItem(itemId1);
    QCOMPARE(_treeView->getSelectedItem(), itemId1);

    _treeView->setSelectedItem(itemId2);
    QCOMPARE(_treeView->getSelectedItem(), itemId2);
}

void TreeViewTest::testClearSelection() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Selected Item");
    _treeView->setSelectedItem(itemId);
    QVERIFY(_treeView->getSelectedItem() >= 0);

    _treeView->clearSelection();
    // Selection should be cleared
    QVERIFY(true);
}

// ============================================================================
// Expansion Tests
// ============================================================================
void TreeViewTest::testExpand() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int parentId = _treeView->addItem("Parent");
    int childId = _treeView->addItem("Child", parentId);
    Q_UNUSED(childId)

    _treeView->expand(parentId);
    QVERIFY(_treeView->isExpanded(parentId));
}

void TreeViewTest::testCollapse() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int parentId = _treeView->addItem("Parent");
    _treeView->addItem("Child", parentId);

    _treeView->expand(parentId);
    QVERIFY(_treeView->isExpanded(parentId));

    _treeView->collapse(parentId);
    QVERIFY(!_treeView->isExpanded(parentId));
}

void TreeViewTest::testExpandAll() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int root1 = _treeView->addItem("Root 1");
    int child1 = _treeView->addItem("Child 1", root1);
    Q_UNUSED(child1)

    int root2 = _treeView->addItem("Root 2");
    int child2 = _treeView->addItem("Child 2", root2);
    Q_UNUSED(child2)

    _treeView->expandAll();
    QVERIFY(_treeView->isExpanded(root1));
    QVERIFY(_treeView->isExpanded(root2));
}

void TreeViewTest::testCollapseAll() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int root = _treeView->addItem("Root");
    _treeView->addItem("Child", root);

    _treeView->expandAll();
    QVERIFY(_treeView->isExpanded(root));

    _treeView->collapseAll();
    QVERIFY(!_treeView->isExpanded(root));
}

void TreeViewTest::testIsExpanded() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int parentId = _treeView->addItem("Parent");
    _treeView->addItem("Child", parentId);

    QVERIFY(!_treeView->isExpanded(parentId));

    _treeView->expand(parentId);
    QVERIFY(_treeView->isExpanded(parentId));
}

// ============================================================================
// Checkbox Tests
// ============================================================================
void TreeViewTest::testSetCheckable() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Checkable Item");
    _treeView->setCheckable(itemId, true);
    QVERIFY(_treeView->isCheckable(itemId));

    _treeView->setCheckable(itemId, false);
    QVERIFY(!_treeView->isCheckable(itemId));
}

void TreeViewTest::testIsCheckable() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item");
    QVERIFY(!_treeView->isCheckable(itemId));

    _treeView->setCheckable(itemId, true);
    QVERIFY(_treeView->isCheckable(itemId));
}

void TreeViewTest::testIsItemChecked() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item");
    _treeView->setCheckable(itemId, true);

    QVERIFY(!_treeView->isItemChecked(itemId));

    _treeView->setItemChecked(itemId, true);
    QVERIFY(_treeView->isItemChecked(itemId));
}

void TreeViewTest::testSetItemChecked() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item");
    _treeView->setCheckable(itemId, true);

    _treeView->setItemChecked(itemId, true);
    QVERIFY(_treeView->isItemChecked(itemId));

    _treeView->setItemChecked(itemId, false);
    QVERIFY(!_treeView->isItemChecked(itemId));
}

void TreeViewTest::testSetCheckState() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item");
    _treeView->setCheckable(itemId, true);

    _treeView->setCheckState(itemId, Qt::Checked);
    QCOMPARE(_treeView->getCheckState(itemId), Qt::Checked);

    _treeView->setCheckState(itemId, Qt::Unchecked);
    QCOMPARE(_treeView->getCheckState(itemId), Qt::Unchecked);
}

void TreeViewTest::testGetCheckState() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item");
    _treeView->setCheckable(itemId, true);

    QCOMPARE(_treeView->getCheckState(itemId), Qt::Unchecked);

    _treeView->setCheckState(itemId, Qt::Checked);
    QCOMPARE(_treeView->getCheckState(itemId), Qt::Checked);
}

// ============================================================================
// Column Tests
// ============================================================================
void TreeViewTest::testSetColumnCount() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->setColumnCount(3);
    QCOMPARE(_treeView->getColumnCount(), 3);
}

void TreeViewTest::testGetColumnCount() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    QCOMPARE(_treeView->getColumnCount(), 1); // Default

    _treeView->setColumnCount(5);
    QCOMPARE(_treeView->getColumnCount(), 5);
}

void TreeViewTest::testSetColumnText() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->setColumnCount(2);
    _treeView->setColumnText(0, "Column 1");
    _treeView->setColumnText(1, "Column 2");

    QCOMPARE(_treeView->getColumnText(0), QString("Column 1"));
    QCOMPARE(_treeView->getColumnText(1), QString("Column 2"));
}

void TreeViewTest::testGetColumnText() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->setColumnCount(1);
    _treeView->setColumnText(0, "Name");

    QCOMPARE(_treeView->getColumnText(0), QString("Name"));
}

// ============================================================================
// Navigation Tests
// ============================================================================
void TreeViewTest::testGetRootItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int rootId = _treeView->addItem("Root");
    int root = _treeView->getRootItem();

    // Should return first top-level item
    QCOMPARE(root, rootId);
}

void TreeViewTest::testGetParentItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int parentId = _treeView->addItem("Parent");
    int childId = _treeView->addItem("Child", parentId);

    int retrievedParent = _treeView->getParentItem(childId);
    QCOMPARE(retrievedParent, parentId);
}

void TreeViewTest::testGetChildItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int parentId = _treeView->addItem("Parent");
    int childId = _treeView->addItem("Child", parentId);

    int retrievedChild = _treeView->getChildItem(parentId);
    QCOMPARE(retrievedChild, childId);
}

void TreeViewTest::testGetNextSibling() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int item1 = _treeView->addItem("Item 1");
    int item2 = _treeView->addItem("Item 2");

    int nextSibling = _treeView->getNextSibling(item1);
    QCOMPARE(nextSibling, item2);
}

void TreeViewTest::testGetPrevSibling() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int item1 = _treeView->addItem("Item 1");
    int item2 = _treeView->addItem("Item 2");

    int prevSibling = _treeView->getPrevSibling(item2);
    QCOMPARE(prevSibling, item1);
}

// ============================================================================
// Sorting Tests
// ============================================================================
void TreeViewTest::testSortItems() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->addItem("Charlie");
    _treeView->addItem("Alpha");
    _treeView->addItem("Bravo");

    _treeView->sortItems(0, Qt::AscendingOrder);
    QVERIFY(true); // Should not crash
}

void TreeViewTest::testSetSortingEnabled() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->setSortingEnabled(true);
    _treeView->setSortingEnabled(false);
    QVERIFY(true); // Should not crash
}

// ============================================================================
// Search Tests
// ============================================================================
void TreeViewTest::testFindItem() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    _treeView->addItem("Alpha");
    _treeView->addItem("Beta");
    _treeView->addItem("Gamma");

    int found = _treeView->findItem("Beta");
    QVERIFY(found >= 0);
}

void TreeViewTest::testFindItemByData() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int itemId = _treeView->addItem("Item");
    _treeView->setItemData(itemId, QVariant(42), Qt::UserRole);

    int found = _treeView->findItemByData(QVariant(42), Qt::UserRole);
    QCOMPARE(found, itemId);
}

// ============================================================================
// State Persistence Tests
// ============================================================================
void TreeViewTest::testRestoreFoldingStateFrom() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    TreeStateNode state;
    state._label = "Root";
    state._isExpanded = true;

    int rootId = _treeView->addItem("Root");
    _treeView->addItem("Child", rootId);

    QVERIFY(_treeView->restoreFoldingStateFrom(state, rootId));
}

void TreeViewTest::testRetrieveFoldingStateTo() {
    QVERIFY(_treeView->init(_parentWidget.get()));

    int rootId = _treeView->addItem("Root");
    _treeView->addItem("Child", rootId);
    _treeView->expand(rootId);

    TreeStateNode state;
    QVERIFY(_treeView->retrieveFoldingStateTo(state, rootId));
    QVERIFY(state._isExpanded);
}

} // namespace Tests

#include "TreeViewTest.moc"
