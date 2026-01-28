// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DockingManager.h"
#include <QDebug>

namespace QtControls {
namespace DockingManager {

Manager::Manager() = default;

Manager::~Manager()
{
    destroy();
}

void Manager::init(QMainWindow* mainWindow)
{
    _mainWindow = mainWindow;
    if (_mainWindow) {
        _mainWindow->setDockNestingEnabled(true);
        _mainWindow->setDockOptions(QMainWindow::AnimatedDocks |
                                     QMainWindow::AllowNestedDocks |
                                     QMainWindow::AllowTabbedDocks);
    }
}

void Manager::destroy()
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        PanelInfo* info = it.value().get();
        if (info && info->dockWidget) {
            if (_mainWindow) {
                _mainWindow->removeDockWidget(info->dockWidget);
            }
            delete info->dockWidget;
        }
    }
    _panels.clear();
    _mainWindow = nullptr;
}

void Manager::addPanel(const QString& name, QWidget* widget, DockArea area,
                       const QString& title)
{
    if (!_mainWindow || !widget || name.isEmpty()) {
        return;
    }

    if (_panels.contains(name)) {
        removePanel(name);
    }

    auto info = std::make_unique<PanelInfo>();
    info->name = name;
    info->widget = widget;
    info->area = area;
    info->title = title.isEmpty() ? name : title;
    info->id = _nextId++;
    info->visible = true;

    QDockWidget* dockWidget = new QDockWidget(info->title, _mainWindow);
    dockWidget->setObjectName(name);
    dockWidget->setWidget(widget);

    setupDockWidget(dockWidget, area);

    info->dockWidget = dockWidget;

    if (area == DockArea::Floating) {
        dockWidget->setFloating(true);
        dockWidget->show();
    } else {
        Qt::DockWidgetArea qtArea = dockAreaToQt(area);
        _mainWindow->addDockWidget(qtArea, dockWidget);
    }

    _panels[name] = std::move(info);
}

void Manager::removePanel(const QString& name)
{
    if (!_panels.contains(name)) {
        return;
    }

    PanelInfo* info = _panels[name].get();
    if (info && info->dockWidget) {
        if (_mainWindow) {
            _mainWindow->removeDockWidget(info->dockWidget);
        }
        delete info->dockWidget;
    }

    _panels.remove(name);
}

void Manager::showPanel(const QString& name)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return;
    }

    info->dockWidget->show();
    info->visible = true;

    if (info->dockWidget->isFloating()) {
        info->dockWidget->raise();
        info->dockWidget->activateWindow();
    }
}

void Manager::hidePanel(const QString& name)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return;
    }

    info->dockWidget->hide();
    info->visible = false;
}

void Manager::togglePanel(const QString& name)
{
    if (isPanelVisible(name)) {
        hidePanel(name);
    } else {
        showPanel(name);
    }
}

bool Manager::isPanelVisible(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return false;
    }
    return info->dockWidget->isVisible();
}

bool Manager::hasPanel(const QString& name) const
{
    return _panels.contains(name);
}

void Manager::setPanelArea(const QString& name, DockArea area)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget || !_mainWindow) {
        return;
    }

    if (area == DockArea::Floating) {
        info->dockWidget->setFloating(true);
    } else {
        if (info->dockWidget->isFloating()) {
            info->dockWidget->setFloating(false);
        }
        Qt::DockWidgetArea qtArea = dockAreaToQt(area);
        _mainWindow->addDockWidget(qtArea, info->dockWidget);
    }

    info->area = area;
}

DockArea Manager::getPanelArea(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return DockArea::Floating;
    }

    if (info->dockWidget && info->dockWidget->isFloating()) {
        return DockArea::Floating;
    }

    return info->area;
}

void Manager::setPanelTitle(const QString& name, const QString& title)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return;
    }

    info->title = title;
    info->dockWidget->setWindowTitle(title);
}

QString Manager::getPanelTitle(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return QString();
    }
    return info->title;
}

QWidget* Manager::getPanelWidget(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return nullptr;
    }
    return info->widget;
}

QDockWidget* Manager::getDockWidget(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return nullptr;
    }
    return info->dockWidget;
}

void Manager::setTabbedDocking(const QString& name1, const QString& name2)
{
    if (!_mainWindow) {
        return;
    }

    QDockWidget* dock1 = getDockWidget(name1);
    QDockWidget* dock2 = getDockWidget(name2);

    if (!dock1 || !dock2) {
        return;
    }

    _mainWindow->tabifyDockWidget(dock1, dock2);
}

QByteArray Manager::saveLayout() const
{
    if (!_mainWindow) {
        return QByteArray();
    }

    return _mainWindow->saveState();
}

void Manager::restoreLayout(const QByteArray& layout)
{
    if (!_mainWindow || layout.isEmpty()) {
        return;
    }

    _mainWindow->restoreState(layout);

    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        PanelInfo* info = it.value().get();
        if (info && info->dockWidget) {
            info->visible = info->dockWidget->isVisible();
        }
    }
}

void Manager::showAllPanels()
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        showPanel(it.key());
    }
}

void Manager::hideAllPanels()
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        hidePanel(it.key());
    }
}

QStringList Manager::getPanelNames() const
{
    return _panels.keys();
}

QStringList Manager::getVisiblePanels() const
{
    QStringList visible;
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        if (isPanelVisible(it.key())) {
            visible.append(it.key());
        }
    }
    return visible;
}

void Manager::setPanelFeatures(const QString& name, bool closable, bool movable, bool floatable)
{
    QDockWidget* dock = getDockWidget(name);
    if (!dock) {
        return;
    }

    QDockWidget::DockWidgetFeatures features = QDockWidget::NoDockWidgetFeatures;

    if (closable) {
        features |= QDockWidget::DockWidgetClosable;
    }
    if (movable) {
        features |= QDockWidget::DockWidgetMovable;
    }
    if (floatable) {
        features |= QDockWidget::DockWidgetFloatable;
    }

    dock->setFeatures(features);
}

void Manager::raisePanel(const QString& name)
{
    QDockWidget* dock = getDockWidget(name);
    if (!dock) {
        return;
    }

    dock->raise();

    if (dock->isFloating()) {
        dock->activateWindow();
    }
}

int Manager::getPanelCount() const
{
    return _panels.size();
}

Qt::DockWidgetArea Manager::dockAreaToQt(DockArea area) const
{
    switch (area) {
        case DockArea::Left:
            return Qt::LeftDockWidgetArea;
        case DockArea::Right:
            return Qt::RightDockWidgetArea;
        case DockArea::Top:
            return Qt::TopDockWidgetArea;
        case DockArea::Bottom:
            return Qt::BottomDockWidgetArea;
        case DockArea::Floating:
        default:
            return Qt::NoDockWidgetArea;
    }
}

DockArea Manager::qtToDockArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            return DockArea::Left;
        case Qt::RightDockWidgetArea:
            return DockArea::Right;
        case Qt::TopDockWidgetArea:
            return DockArea::Top;
        case Qt::BottomDockWidgetArea:
            return DockArea::Bottom;
        default:
            return DockArea::Floating;
    }
}

void Manager::setupDockWidget(QDockWidget* dockWidget, DockArea area)
{
    if (!dockWidget) {
        return;
    }

    QDockWidget::DockWidgetFeatures features =
        QDockWidget::DockWidgetClosable |
        QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable;

    dockWidget->setFeatures(features);

    if (area != DockArea::Floating) {
        dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    }
}

PanelInfo* Manager::getPanelInfo(const QString& name) const
{
    if (!_panels.contains(name)) {
        return nullptr;
    }
    return _panels[name].get();
}

} // namespace DockingManager
} // namespace QtControls
