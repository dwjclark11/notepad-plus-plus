// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QSplitter>
#include <QByteArray>
#include <vector>

namespace QtControls {

class Splitter : public Window
{
public:
    enum class Orientation { Horizontal, Vertical };

    Splitter() = default;
    ~Splitter() override = default;

    void init(QWidget* parent) override;
    void init(QWidget* parent, Orientation orientation);

    void destroy() override;

    void setOrientation(Orientation orientation);
    Orientation getOrientation() const;

    void addWidget(QWidget* widget);
    void insertWidget(int index, QWidget* widget);
    void removeWidget(QWidget* widget);

    void setSizes(const std::vector<int>& sizes);
    std::vector<int> getSizes() const;

    void setCollapsible(int index, bool collapsible);
    bool isCollapsible(int index) const;

    void setStretchFactor(int index, int stretch);
    int getStretchFactor(int index) const;

    void setHandleWidth(int width);
    int getHandleWidth() const;

    void setOpaqueResize(bool opaque);
    bool getOpaqueResize() const;

    int count() const;
    QWidget* widget(int index) const;
    int indexOf(QWidget* widget) const;

    QByteArray saveState() const;
    void restoreState(const QByteArray& state);

    QSplitter* getSplitter() const { return qobject_cast<QSplitter*>(_widget); }

private:
    static Qt::Orientation toQtOrientation(Orientation orientation);
    static Orientation fromQtOrientation(Qt::Orientation orientation);
};

} // namespace QtControls
