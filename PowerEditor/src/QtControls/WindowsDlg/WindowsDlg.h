// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"

#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

class Notepad_plus;

namespace QtControls {

class DocTabView;

class WindowsDlg : public StaticDialog
{
	Q_OBJECT

public:
	explicit WindowsDlg(QWidget* parent = nullptr);
	~WindowsDlg() override;

	void init(Notepad_plus* pNotepad, DocTabView* pTab);
	void doDialog();

private slots:
	void onActivateClicked();
	void onSaveClicked();
	void onCloseDocClicked();
	void onSortTabsClicked();
	void onSelectionChanged();
	void onItemDoubleClicked(int row, int column);
	void onHeaderClicked(int logicalIndex);

private:
	void setupUI() override;
	void connectSignals() override;
	void populateList();
	void updateButtonState();
	void doColumnSort();
	void updateColumnHeaders();
	void updateDocCount();

	Notepad_plus* _pNotepad = nullptr;
	DocTabView* _pTab = nullptr;

	QTableWidget* _table = nullptr;
	QPushButton* _activateBtn = nullptr;
	QPushButton* _saveBtn = nullptr;
	QPushButton* _closeBtn = nullptr;
	QPushButton* _sortTabsBtn = nullptr;
	QPushButton* _closeDialogBtn = nullptr;
	QLabel* _docCountLabel = nullptr;

	std::vector<int> _idxMap;
	int _currentColumn = -1;
	int _lastSort = -1;
	bool _reverseSort = false;
};

class WindowsMenu
{
public:
	WindowsMenu() = default;
	~WindowsMenu() = default;
};

} // namespace QtControls

using QtControls::WindowsDlg;
using QtControls::WindowsMenu;
