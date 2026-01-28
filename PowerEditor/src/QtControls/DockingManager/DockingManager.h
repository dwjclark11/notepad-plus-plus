// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QMainWindow>
#include <QDockWidget>
#include <QMap>
#include <QString>
#include <QByteArray>
#include <memory>

namespace QtControls {

namespace DockingManager {

enum class DockArea {
    Left,
    Right,
    Top,
    Bottom,
    Floating
};

struct PanelInfo {
    QString name;
    QString title;
    QWidget* widget = nullptr;
    QDockWidget* dockWidget = nullptr;
    DockArea area = DockArea::Right;
    bool visible = true;
    int id = 0;
};

class Manager : public Window
{
public:
    Manager();
    ~Manager() override;

    void init(QMainWindow* mainWindow);

    void destroy() override;

    void addPanel(const QString& name, QWidget* widget, DockArea area,
                  const QString& title = QString());
    void removePanel(const QString& name);

    void showPanel(const QString& name);
    void hidePanel(const QString& name);
    void togglePanel(const QString& name);

    bool isPanelVisible(const QString& name) const;
    bool hasPanel(const QString& name) const;

    void setPanelArea(const QString& name, DockArea area);
    DockArea getPanelArea(const QString& name) const;

    void setPanelTitle(const QString& name, const QString& title);
    QString getPanelTitle(const QString& name) const;

    QWidget* getPanelWidget(const QString& name) const;
    QDockWidget* getDockWidget(const QString& name) const;

    void setTabbedDocking(const QString& name1, const QString& name2);

    QByteArray saveLayout() const;
    void restoreLayout(const QByteArray& layout);

    void showAllPanels();
    void hideAllPanels();

    QStringList getPanelNames() const;
    QStringList getVisiblePanels() const;

    void setPanelFeatures(const QString& name, bool closable, bool movable, bool floatable);

    void raisePanel(const QString& name);

    int getPanelCount() const;

private:
    QMainWindow* _mainWindow = nullptr;
    QMap<QString, std::unique_ptr<PanelInfo>> _panels;
    int _nextId = 1;

    Qt::DockWidgetArea dockAreaToQt(DockArea area) const;
    DockArea qtToDockArea(Qt::DockWidgetArea area) const;
    void setupDockWidget(QDockWidget* dockWidget, DockArea area);
    PanelInfo* getPanelInfo(const QString& name) const;
};

} // namespace DockingManager

} // namespace QtControls
