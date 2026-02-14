// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>

#include <QMap>

class QMenu;
class QAction;
class QWidget;

struct MenuItemUnit final
{
	unsigned long _cmdID = 0;
	std::wstring _itemName;
	std::wstring _parentFolderName;

	MenuItemUnit() = default;
	MenuItemUnit(unsigned long cmdID, const std::wstring& itemName, const std::wstring& parentFolderName = std::wstring())
		: _cmdID(cmdID), _itemName(itemName), _parentFolderName(parentFolderName) {}
	MenuItemUnit(unsigned long cmdID, const wchar_t* itemName, const wchar_t* parentFolderName = nullptr);
};

class ContextMenu final
{
public:
	ContextMenu() = default;
	~ContextMenu();

	void create(void* hParent, const std::vector<MenuItemUnit>& menuItemArray, const void* mainMenuHandle = nullptr, bool copyLink = false);
	bool isCreated() const;

	void display(const void* p) const;
	void display(void* hwnd) const;

	void enableItem(int cmdID, bool doEnable) const;
	void checkItem(int cmdID, bool doCheck) const;

	QMenu* getMenuHandle() const { return _menu; }

	void destroy();

private:
	QWidget* _hParent = nullptr;
	QMenu* _menu = nullptr;
	QMap<int, QAction*> _actionMap;
	std::vector<QMenu*> _subMenus;
	bool _isCreated = false;
};
