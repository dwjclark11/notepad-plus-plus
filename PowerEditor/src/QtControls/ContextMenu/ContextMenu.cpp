// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ContextMenu.h"
#include "ShortcutManager/ShortcutManager.h"
#include "Common.h"

#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QCursor>
#include <QPoint>

MenuItemUnit::MenuItemUnit(unsigned long cmdID, const wchar_t* itemName, const wchar_t* parentFolderName) : _cmdID(cmdID)
{
	if (!itemName)
		_itemName.clear();
	else
		_itemName = itemName;

	if (!parentFolderName)
		_parentFolderName.clear();
	else
		_parentFolderName = parentFolderName;
}

ContextMenu::~ContextMenu()
{
	destroy();
}

void ContextMenu::create(void* hParent, const std::vector<MenuItemUnit>& menuItemArray, const void* mainMenuHandle, bool copyLink)
{
	(void)mainMenuHandle;
	(void)copyLink;

	destroy();

	_hParent = static_cast<QWidget*>(hParent);

	_menu = new QMenu(_hParent);

	bool lastIsSep = false;
	QMenu* hParentFolder = nullptr;
	std::wstring currentParentFolderStr;

	for (size_t i = 0, len = menuItemArray.size(); i < len; ++i)
	{
		const MenuItemUnit& item = menuItemArray[i];
		bool isSeparator = (item._cmdID == 0);

		// Determine the target menu (submenu or top-level)
		if (item._parentFolderName.empty())
		{
			currentParentFolderStr.clear();
			hParentFolder = nullptr;
		}
		else
		{
			if (item._parentFolderName != currentParentFolderStr)
			{
				currentParentFolderStr = item._parentFolderName;
				hParentFolder = _menu->addMenu(QString::fromStdWString(currentParentFolderStr));
				_subMenus.push_back(hParentFolder);
			}
		}

		QMenu* targetMenu = hParentFolder ? hParentFolder : _menu;

		if (isSeparator)
		{
			// Skip leading/trailing separators at top level
			if (!hParentFolder && (i == 0 || i == len - 1))
			{
				lastIsSep = true;
				continue;
			}
			// Skip consecutive separators
			if (!hParentFolder && lastIsSep)
			{
				continue;
			}
			targetMenu->addSeparator();
			lastIsSep = true;
		}
		else
		{
			QString actionText = QString::fromStdWString(item._itemName);
			if (actionText.isEmpty())
			{
				// Try to get the name from ShortcutManager's registered command
				QtControls::ShortcutManager* sm = QtControls::ShortcutManager::getInstance();
				if (sm)
				{
					auto cmdInfo = sm->getCommand(static_cast<int>(item._cmdID));
					if (!cmdInfo.name.isEmpty())
						actionText = cmdInfo.name;
					else
						actionText = QString("Command %1").arg(item._cmdID);
				}
				else
				{
					actionText = QString("Command %1").arg(item._cmdID);
				}
			}

			QAction* action = targetMenu->addAction(actionText);
			int cmdID = static_cast<int>(item._cmdID);
			action->setData(cmdID);
			_actionMap[cmdID] = action;

			QObject::connect(action, &QAction::triggered, action, [cmdID]()
			{
				QtControls::ShortcutManager* sm = QtControls::ShortcutManager::getInstance();
				if (sm)
				{
					sm->executeCommand(cmdID);
				}
			});

			lastIsSep = false;
		}
	}

	_isCreated = true;
}

bool ContextMenu::isCreated() const
{
	return _isCreated;
}

void ContextMenu::display(const void* p) const
{
	if (!_menu || !p)
		return;

	const auto* pt = static_cast<const POINT*>(p);
	_menu->exec(QPoint(static_cast<int>(pt->x), static_cast<int>(pt->y)));
}

void ContextMenu::display(void* hwnd) const
{
	if (!_menu)
		return;

	// Display at current cursor position when given a widget handle
	QWidget* widget = static_cast<QWidget*>(hwnd);
	if (widget)
	{
		QRect rect = widget->rect();
		QPoint bottomLeft = widget->mapToGlobal(rect.bottomLeft());
		_menu->exec(bottomLeft);
	}
	else
	{
		_menu->exec(QCursor::pos());
	}
}

void ContextMenu::enableItem(int cmdID, bool doEnable) const
{
	auto it = _actionMap.find(cmdID);
	if (it != _actionMap.end())
	{
		it.value()->setEnabled(doEnable);
	}
}

void ContextMenu::checkItem(int cmdID, bool doCheck) const
{
	auto it = _actionMap.find(cmdID);
	if (it != _actionMap.end())
	{
		it.value()->setCheckable(true);
		it.value()->setChecked(doCheck);
	}
}

void ContextMenu::destroy()
{
	if (_isCreated)
	{
		_actionMap.clear();
		_subMenus.clear();
		delete _menu;
		_menu = nullptr;
		_hParent = nullptr;
		_isCreated = false;
	}
}
