// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "Splitter.h"

namespace QtControls {

void Splitter::init(QWidget* parent)
{
    init(parent, Orientation::Horizontal);
}

void Splitter::init(QWidget* parent, Orientation orientation)
{
    if (!parent) return;

    _parent = parent;
    _widget = new QSplitter(toQtOrientation(orientation), parent);
}

void Splitter::destroy()
{
    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

void Splitter::setOrientation(Orientation orientation)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    splitter->setOrientation(toQtOrientation(orientation));
}

Splitter::Orientation Splitter::getOrientation() const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return Orientation::Horizontal;

    return fromQtOrientation(splitter->orientation());
}

void Splitter::addWidget(QWidget* widget)
{
    QSplitter* splitter = getSplitter();
    if (!splitter || !widget) return;

    splitter->addWidget(widget);
}

void Splitter::insertWidget(int index, QWidget* widget)
{
    QSplitter* splitter = getSplitter();
    if (!splitter || !widget) return;

    splitter->insertWidget(index, widget);
}

void Splitter::removeWidget(QWidget* widget)
{
    if (!widget) return;

    widget->setParent(nullptr);
}

void Splitter::setSizes(const std::vector<int>& sizes)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    QList<int> qtSizes;
    for (int size : sizes) {
        qtSizes.append(size);
    }
    splitter->setSizes(qtSizes);
}

std::vector<int> Splitter::getSizes() const
{
    std::vector<int> result;
    QSplitter* splitter = getSplitter();
    if (!splitter) return result;

    QList<int> qtSizes = splitter->sizes();
    result.reserve(qtSizes.size());
    for (int size : qtSizes) {
        result.push_back(size);
    }
    return result;
}

void Splitter::setCollapsible(int index, bool collapsible)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    splitter->setCollapsible(index, collapsible);
}

bool Splitter::isCollapsible(int index) const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return false;

    // QSplitter doesn't have a direct isCollapsible getter
    // We need to check the handle's enabled state
    QSplitterHandle* handle = splitter->handle(index);
    if (!handle) return false;

    return handle->isEnabled();
}

void Splitter::setStretchFactor(int index, int stretch)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    splitter->setStretchFactor(index, stretch);
}

int Splitter::getStretchFactor(int index) const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return 0;

    // QSplitter doesn't expose stretch factor directly
    // Return 0 as default (equal stretch)
    (void)index;
    return 0;
}

void Splitter::setHandleWidth(int width)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    splitter->setHandleWidth(width);
}

int Splitter::getHandleWidth() const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return 0;

    return splitter->handleWidth();
}

void Splitter::setOpaqueResize(bool opaque)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    splitter->setOpaqueResize(opaque);
}

bool Splitter::getOpaqueResize() const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return true;

    return splitter->opaqueResize();
}

int Splitter::count() const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return 0;

    return splitter->count();
}

QWidget* Splitter::widget(int index) const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return nullptr;

    return splitter->widget(index);
}

int Splitter::indexOf(QWidget* widget) const
{
    QSplitter* splitter = getSplitter();
    if (!splitter || !widget) return -1;

    return splitter->indexOf(widget);
}

QByteArray Splitter::saveState() const
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return QByteArray();

    return splitter->saveState();
}

void Splitter::restoreState(const QByteArray& state)
{
    QSplitter* splitter = getSplitter();
    if (!splitter) return;

    splitter->restoreState(state);
}

Qt::Orientation Splitter::toQtOrientation(Orientation orientation)
{
    return (orientation == Orientation::Horizontal)
        ? Qt::Horizontal
        : Qt::Vertical;
}

Splitter::Orientation Splitter::fromQtOrientation(Qt::Orientation orientation)
{
    return (orientation == Qt::Horizontal)
        ? Orientation::Horizontal
        : Orientation::Vertical;
}

} // namespace QtControls
