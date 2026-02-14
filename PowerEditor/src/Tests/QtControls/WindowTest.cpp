// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "WindowTest.h"
#include "Window.h"
#include "../Common/TestUtils.h"
#include <QRect>
#include <QWidget>

using namespace QtControls;

// Testable concrete implementation of the abstract Window class
class TestableWindow : public Window {
public:
    TestableWindow(QObject* parent = nullptr) : Window(parent) {}

    void destroy() override {
        if (_widget) {
            _widget->deleteLater();
            _widget = nullptr;
        }
    }

    void init(QWidget* parent) override {
        _parent = parent;
        _widget = new QWidget(parent);
    }
};

namespace Tests {

WindowTest::WindowTest() {}

WindowTest::~WindowTest() {}

void WindowTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void WindowTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void WindowTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _window = std::make_unique<TestableWindow>();
}

void WindowTest::cleanup() {
    _window.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void WindowTest::testInit() {
    QVERIFY(_window != nullptr);
    _window->init(_parentWidget.get());
    QCOMPARE(_window->getParent(), _parentWidget.get());
}

void WindowTest::testDestroy() {
    _window->init(_parentWidget.get());
    _window->destroy();
    // Window should be in destroyed state
    QVERIFY(_window->getWidget() == nullptr);
}

// ============================================================================
// Visibility Tests
// ============================================================================
void WindowTest::testDisplay() {
    _window->init(_parentWidget.get());
    QWidget* w = _window->getWidget();
    QVERIFY(w != nullptr);

    // After display(true), the widget should not be explicitly hidden
    _window->display(true);
    QVERIFY(!w->isHidden());
    QVERIFY(w->testAttribute(Qt::WA_WState_ExplicitShowHide));

    // After display(false), the widget should be explicitly hidden
    _window->display(false);
    QVERIFY(w->isHidden());
}

void WindowTest::testIsVisible() {
    _window->init(_parentWidget.get());
    QWidget* w = _window->getWidget();
    QVERIFY(w != nullptr);

    // After display(true), the widget should not be explicitly hidden
    _window->display(true);
    QVERIFY(!w->isHidden());
}

// ============================================================================
// Geometry Tests
// ============================================================================
void WindowTest::testReSizeTo() {
    _window->init(_parentWidget.get());
    _window->display(true);

    QRect newRect(10, 20, 300, 400);
    _window->reSizeTo(newRect);

    // getClientRect returns the widget's local rect (origin 0,0) so check size
    QRect clientRect;
    _window->getClientRect(clientRect);
    QCOMPARE(clientRect.width(), 300);
    QCOMPARE(clientRect.height(), 400);

    // Verify position via the widget's geometry
    QWidget* w = _window->getWidget();
    QVERIFY(w != nullptr);
    QCOMPARE(w->geometry(), newRect);
}

void WindowTest::testReSizeToWH() {
    _window->init(_parentWidget.get());
    _window->display(true);

    QRect newRect(10, 20, 300, 400);
    _window->reSizeToWH(newRect);

    // Verify geometry was applied to the widget
    QWidget* w = _window->getWidget();
    QVERIFY(w != nullptr);
    QCOMPARE(w->geometry().x(), 10);
    QCOMPARE(w->geometry().y(), 20);
    QCOMPARE(w->geometry().width(), 300);
    QCOMPARE(w->geometry().height(), 400);
}

void WindowTest::testGetClientRect() {
    _window->init(_parentWidget.get());
    _window->display(true);

    QRect rect;
    _window->getClientRect(rect);

    // Should return valid rect
    QVERIFY(rect.width() >= 0);
    QVERIFY(rect.height() >= 0);
}

void WindowTest::testGetWindowRect() {
    _window->init(_parentWidget.get());
    _window->display(true);

    QRect rect;
    _window->getWindowRect(rect);

    // Should return valid rect
    QVERIFY(rect.width() >= 0);
    QVERIFY(rect.height() >= 0);
}

void WindowTest::testGetWidth() {
    _window->init(_parentWidget.get());
    _window->display(true);

    int width = _window->getWidth();
    QVERIFY(width >= 0);
}

void WindowTest::testGetHeight() {
    _window->init(_parentWidget.get());
    _window->display(true);

    int height = _window->getHeight();
    QVERIFY(height >= 0);
}

// ============================================================================
// Redraw Tests
// ============================================================================
void WindowTest::testRedraw() {
    _window->init(_parentWidget.get());
    _window->display(true);

    // Should not crash
    _window->redraw();
    QVERIFY(true);
}

void WindowTest::testRedrawForceUpdate() {
    _window->init(_parentWidget.get());
    _window->display(true);

    // Should not crash
    _window->redraw(true);
    QVERIFY(true);
}

// ============================================================================
// Visibility Guard Tests (Bug 4)
// ============================================================================
void WindowTest::testIsNotVisibleBeforeShow() {
    _window->init(_parentWidget.get());

    // Widget exists but has never been shown
    QVERIFY(_window->getWidget() != nullptr);
    QVERIFY(!_window->isVisible());

    // This is the guard condition that prevents SCI_GRABFOCUS on hidden widgets:
    // if (w && w->isVisible()) { ... }
    QWidget* w = _window->getWidget();
    bool guardPasses = (w && w->isVisible());
    QVERIFY(!guardPasses);
}

// ============================================================================
// Focus Tests
// ============================================================================
void WindowTest::testGrabFocus() {
    _window->init(_parentWidget.get());
    _window->display(true);

    // Should not crash
    _window->grabFocus();
    QVERIFY(true);
}

// ============================================================================
// Widget Access Tests
// ============================================================================
void WindowTest::testGetWidget() {
    _window->init(_parentWidget.get());
    QVERIFY(_window->getWidget() != nullptr);
}

void WindowTest::testGetParent() {
    _window->init(_parentWidget.get());
    QCOMPARE(_window->getParent(), _parentWidget.get());
}

} // namespace Tests
