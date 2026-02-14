// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "WindowsDlg.h"
#include "../DocTabView/DocTabView.h"
#include "../../Notepad_plus.h"
#include "../../Parameters.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtCore/QFileInfo>

#include <algorithm>

namespace QtControls {

// Column indices
enum Column
{
	COL_NAME = 0,
	COL_PATH = 1,
	COL_TYPE = 2,
	COL_SIZE = 3,
	COL_COUNT = 4
};

// ============================================================================
// Comparator for natural (numeric-aware) string sorting
// ============================================================================
static int naturalCompare(const QString& a, const QString& b)
{
	return a.compare(b, Qt::CaseInsensitive);
}

// ============================================================================
// WindowsDlg implementation
// ============================================================================

WindowsDlg::WindowsDlg(QWidget* parent)
	: StaticDialog(parent)
{
}

WindowsDlg::~WindowsDlg() = default;

void WindowsDlg::init(Notepad_plus* pNotepad, DocTabView* pTab)
{
	_pNotepad = pNotepad;
	_pTab = pTab;
}

void WindowsDlg::doDialog()
{
	if (!isCreated())
	{
		create(tr("Windows"), false);
		setupUI();
		connectSignals();
	}

	_currentColumn = -1;
	_lastSort = -1;
	_reverseSort = false;

	populateList();
	goToCenter();
	display(true, true);
}

void WindowsDlg::setupUI()
{
	QDialog* dialog = getDialog();
	if (!dialog)
		return;

	dialog->setWindowTitle(tr("Windows"));
	dialog->resize(700, 400);
	dialog->setMinimumSize(400, 250);

	auto* mainLayout = new QHBoxLayout(dialog);
	mainLayout->setSpacing(8);
	mainLayout->setContentsMargins(8, 8, 8, 8);

	// Left side: table
	auto* leftLayout = new QVBoxLayout();

	_table = new QTableWidget(dialog);
	_table->setColumnCount(COL_COUNT);
	_table->setHorizontalHeaderLabels(
		QStringList() << tr("Name") << tr("Path") << tr("Type") << tr("Size"));
	_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_table->setAlternatingRowColors(true);
	_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_table->setSortingEnabled(false); // We handle sorting ourselves
	_table->verticalHeader()->setVisible(false);
	_table->horizontalHeader()->setStretchLastSection(false);
	_table->horizontalHeader()->setSectionsClickable(true);

	// Set initial column widths
	_table->horizontalHeader()->setSectionResizeMode(COL_NAME, QHeaderView::Interactive);
	_table->horizontalHeader()->setSectionResizeMode(COL_PATH, QHeaderView::Stretch);
	_table->horizontalHeader()->setSectionResizeMode(COL_TYPE, QHeaderView::Interactive);
	_table->horizontalHeader()->setSectionResizeMode(COL_SIZE, QHeaderView::Interactive);
	_table->setColumnWidth(COL_NAME, 150);
	_table->setColumnWidth(COL_TYPE, 80);
	_table->setColumnWidth(COL_SIZE, 80);

	leftLayout->addWidget(_table, 1);

	_docCountLabel = new QLabel(dialog);
	leftLayout->addWidget(_docCountLabel);

	mainLayout->addLayout(leftLayout, 1);

	// Right side: buttons
	auto* buttonLayout = new QVBoxLayout();
	buttonLayout->setSpacing(6);

	_activateBtn = new QPushButton(tr("&Activate"), dialog);
	_activateBtn->setDefault(true);
	buttonLayout->addWidget(_activateBtn);

	_saveBtn = new QPushButton(tr("&Save"), dialog);
	buttonLayout->addWidget(_saveBtn);

	_closeBtn = new QPushButton(tr("&Close Document(s)"), dialog);
	buttonLayout->addWidget(_closeBtn);

	_sortTabsBtn = new QPushButton(tr("Sort &Tabs"), dialog);
	buttonLayout->addWidget(_sortTabsBtn);

	buttonLayout->addStretch(1);

	_closeDialogBtn = new QPushButton(tr("Cancel"), dialog);
	buttonLayout->addWidget(_closeDialogBtn);

	mainLayout->addLayout(buttonLayout);

	_rc = dialog->geometry();
}

void WindowsDlg::connectSignals()
{
	connect(_activateBtn, &QPushButton::clicked, this, &WindowsDlg::onActivateClicked);
	connect(_saveBtn, &QPushButton::clicked, this, &WindowsDlg::onSaveClicked);
	connect(_closeBtn, &QPushButton::clicked, this, &WindowsDlg::onCloseDocClicked);
	connect(_sortTabsBtn, &QPushButton::clicked, this, &WindowsDlg::onSortTabsClicked);
	connect(_closeDialogBtn, &QPushButton::clicked, this, &QDialog::reject);
	connect(_table, &QTableWidget::itemDoubleClicked,
		[this](QTableWidgetItem* item) { onItemDoubleClicked(item->row(), item->column()); });
	connect(_table->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &WindowsDlg::onSelectionChanged);
	connect(_table->horizontalHeader(), &QHeaderView::sectionClicked,
		this, &WindowsDlg::onHeaderClicked);
}

void WindowsDlg::populateList()
{
	if (!_pTab)
		return;

	_table->setRowCount(0);

	size_t count = _pTab->nbItem();
	_idxMap.resize(count);
	for (size_t i = 0; i < count; ++i)
		_idxMap[i] = static_cast<int>(i);

	_table->setRowCount(static_cast<int>(count));

	for (size_t i = 0; i < count; ++i)
	{
		BufferID bufID = _pTab->getBufferByIndex(i);
		Buffer* buf = MainFileManager.getBufferByID(bufID);
		if (!buf)
			continue;

		int row = static_cast<int>(i);

		// Name column - show dirty indicator
		QString name = buf->getFileNameQString();
		if (buf->isDirty())
			name += QStringLiteral("*");
		else if (buf->isReadOnly())
			name += tr(" [Read Only]");

		auto* nameItem = new QTableWidgetItem(name);
		_table->setItem(row, COL_NAME, nameItem);

		// Path column - directory only
		QString fullPath = buf->getFilePath();
		QFileInfo fi(fullPath);
		QString dirPath = fi.path();
		if (dirPath == QStringLiteral("."))
			dirPath.clear();
		auto* pathItem = new QTableWidgetItem(dirPath);
		_table->setItem(row, COL_PATH, pathItem);

		// Type column - language name
		QString typeName;
		NppParameters& nppParams = NppParameters::getInstance();
		Lang* lang = nppParams.getLangFromID(buf->getLangType());
		if (lang)
			typeName = QString::fromWCharArray(lang->getLangName());
		auto* typeItem = new QTableWidgetItem(typeName);
		typeItem->setTextAlignment(Qt::AlignCenter);
		_table->setItem(row, COL_TYPE, typeItem);

		// Size column
		size_t docSize = buf->docLength();
		auto* sizeItem = new QTableWidgetItem();
		sizeItem->setData(Qt::DisplayRole, QVariant::fromValue(static_cast<qulonglong>(docSize)));
		sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_table->setItem(row, COL_SIZE, sizeItem);
	}

	// Select the current tab's row
	if (_pTab->nbItem() > 0)
	{
		int curIdx = _pTab->getCurrentTabIndex();
		if (curIdx >= 0 && curIdx < _table->rowCount())
		{
			_table->selectRow(curIdx);
			_table->scrollToItem(_table->item(curIdx, 0));
		}
	}

	updateButtonState();
	updateDocCount();
	updateColumnHeaders();
}

void WindowsDlg::updateButtonState()
{
	int selCount = 0;
	if (_table->selectionModel())
	{
		QModelIndexList sel = _table->selectionModel()->selectedRows();
		selCount = sel.size();
	}

	_activateBtn->setEnabled(selCount == 1);
	_saveBtn->setEnabled(selCount > 0);
	_closeBtn->setEnabled(selCount > 0);
	_sortTabsBtn->setEnabled(_table->rowCount() > 0);
}

void WindowsDlg::onSelectionChanged()
{
	updateButtonState();
}

void WindowsDlg::onItemDoubleClicked(int row, int column)
{
	Q_UNUSED(column);
	if (row >= 0 && row < static_cast<int>(_idxMap.size()))
		onActivateClicked();
}

void WindowsDlg::onActivateClicked()
{
	QModelIndexList sel = _table->selectionModel()->selectedRows();
	if (sel.size() != 1)
		return;

	int row = sel.first().row();
	if (row < 0 || row >= static_cast<int>(_idxMap.size()))
		return;

	int tabIdx = _idxMap[row];
	if (!_pTab || !_pNotepad)
		return;

	BufferID bufID = _pTab->getBufferByIndex(static_cast<size_t>(tabIdx));
	_pNotepad->switchToFile(bufID);
	accept();
}

void WindowsDlg::onSaveClicked()
{
	if (!_pNotepad || !_pTab)
		return;

	QModelIndexList sel = _table->selectionModel()->selectedRows();
	for (const auto& idx : sel)
	{
		int row = idx.row();
		if (row < 0 || row >= static_cast<int>(_idxMap.size()))
			continue;

		int tabIdx = _idxMap[row];
		BufferID bufID = _pTab->getBufferByIndex(static_cast<size_t>(tabIdx));
		_pNotepad->fileSave(bufID);
	}

	// Refresh the list to update dirty indicators
	populateList();
}

void WindowsDlg::onCloseDocClicked()
{
	if (!_pNotepad || !_pTab)
		return;

	// Collect selected indices in reverse order to avoid index shifting issues
	QModelIndexList sel = _table->selectionModel()->selectedRows();
	std::vector<int> tabIndices;
	for (const auto& idx : sel)
	{
		int row = idx.row();
		if (row >= 0 && row < static_cast<int>(_idxMap.size()))
			tabIndices.push_back(_idxMap[row]);
	}

	// Sort in descending order so closing higher indices first doesn't shift lower ones
	std::sort(tabIndices.begin(), tabIndices.end(), std::greater<int>());

	for (int tabIdx : tabIndices)
	{
		BufferID bufID = _pTab->getBufferByIndex(static_cast<size_t>(tabIdx));
		_pNotepad->fileClose(bufID, -1);
	}

	if (_pTab->nbItem() == 0)
	{
		reject();
		return;
	}

	// Refresh the list
	populateList();
}

void WindowsDlg::onSortTabsClicked()
{
	if (!_pTab || !_pNotepad)
		return;

	// If no column was selected for sorting, default to filename
	if (_currentColumn == -1)
	{
		_currentColumn = COL_NAME;
		_reverseSort = false;
		_lastSort = _currentColumn;
		updateColumnHeaders();
	}

	// Reorder tabs to match the current sort order
	doColumnSort();
}

void WindowsDlg::onHeaderClicked(int logicalIndex)
{
	if (logicalIndex < 0 || logicalIndex >= COL_COUNT)
		return;

	_currentColumn = logicalIndex;

	if (_lastSort == _currentColumn)
	{
		_reverseSort = true;
		_lastSort = -1;
	}
	else
	{
		_reverseSort = false;
		_lastSort = _currentColumn;
	}

	updateColumnHeaders();
	doColumnSort();
}

void WindowsDlg::doColumnSort()
{
	if (_currentColumn < 0 || !_pTab)
		return;

	// Save selection state
	QSet<int> selectedOrigIdx;
	QModelIndexList sel = _table->selectionModel()->selectedRows();
	for (const auto& idx : sel)
	{
		int row = idx.row();
		if (row >= 0 && row < static_cast<int>(_idxMap.size()))
			selectedOrigIdx.insert(_idxMap[row]);
	}

	// Sort the index map
	int col = _currentColumn;
	bool rev = _reverseSort;
	DocTabView* pTab = _pTab;

	std::stable_sort(_idxMap.begin(), _idxMap.end(),
		[col, rev, pTab](int i1, int i2) -> bool
		{
			if (i1 == i2)
				return false;
			if (rev)
				std::swap(i1, i2);

			BufferID bid1 = pTab->getBufferByIndex(static_cast<size_t>(i1));
			BufferID bid2 = pTab->getBufferByIndex(static_cast<size_t>(i2));
			Buffer* b1 = MainFileManager.getBufferByID(bid1);
			Buffer* b2 = MainFileManager.getBufferByID(bid2);
			if (!b1 || !b2)
				return false;

			if (col == COL_NAME)
			{
				int result = naturalCompare(b1->getFileNameQString(), b2->getFileNameQString());
				if (result != 0)
					return result < 0;
			}
			else if (col == COL_TYPE)
			{
				NppParameters& nppParams = NppParameters::getInstance();
				QString s1, s2;
				Lang* lang1 = nppParams.getLangFromID(b1->getLangType());
				if (lang1)
					s1 = QString::fromWCharArray(lang1->getLangName());
				Lang* lang2 = nppParams.getLangFromID(b2->getLangType());
				if (lang2)
					s2 = QString::fromWCharArray(lang2->getLangName());
				int result = naturalCompare(s1, s2);
				if (result != 0)
					return result < 0;
			}
			else if (col == COL_SIZE)
			{
				size_t sz1 = b1->docLength();
				size_t sz2 = b2->docLength();
				if (sz1 != sz2)
					return sz1 < sz2;
			}

			// Default: sort by path
			return naturalCompare(b1->getFilePath(), b2->getFilePath()) < 0;
		});

	// Re-populate table rows according to new order
	_table->setRowCount(0);
	int count = static_cast<int>(_idxMap.size());
	_table->setRowCount(count);

	for (int row = 0; row < count; ++row)
	{
		int tabIdx = _idxMap[row];
		BufferID bufID = pTab->getBufferByIndex(static_cast<size_t>(tabIdx));
		Buffer* buf = MainFileManager.getBufferByID(bufID);
		if (!buf)
			continue;

		// Name
		QString name = buf->getFileNameQString();
		if (buf->isDirty())
			name += QStringLiteral("*");
		else if (buf->isReadOnly())
			name += tr(" [Read Only]");
		_table->setItem(row, COL_NAME, new QTableWidgetItem(name));

		// Path
		QString fullPath = buf->getFilePath();
		QFileInfo fi(fullPath);
		QString dirPath = fi.path();
		if (dirPath == QStringLiteral("."))
			dirPath.clear();
		_table->setItem(row, COL_PATH, new QTableWidgetItem(dirPath));

		// Type
		QString typeName;
		NppParameters& nppParams = NppParameters::getInstance();
		Lang* lang = nppParams.getLangFromID(buf->getLangType());
		if (lang)
			typeName = QString::fromWCharArray(lang->getLangName());
		auto* typeItem = new QTableWidgetItem(typeName);
		typeItem->setTextAlignment(Qt::AlignCenter);
		_table->setItem(row, COL_TYPE, typeItem);

		// Size
		auto* sizeItem = new QTableWidgetItem();
		sizeItem->setData(Qt::DisplayRole, QVariant::fromValue(static_cast<qulonglong>(buf->docLength())));
		sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_table->setItem(row, COL_SIZE, sizeItem);
	}

	// Restore selection
	for (int row = 0; row < count; ++row)
	{
		if (selectedOrigIdx.contains(_idxMap[row]))
			_table->selectRow(row);
	}

	updateButtonState();
}

void WindowsDlg::updateColumnHeaders()
{
	QStringList headers;
	QStringList baseNames = {tr("Name"), tr("Path"), tr("Type"), tr("Size")};

	for (int i = 0; i < COL_COUNT; ++i)
	{
		QString prefix;
		if (_currentColumn != i)
			prefix = QString::fromUtf8("\xe2\x87\xb5 "); // up-down arrow
		else if (_reverseSort)
			prefix = QString::fromUtf8("\xe2\x96\xb3 "); // up triangle
		else
			prefix = QString::fromUtf8("\xe2\x96\xbd "); // down triangle

		headers << (prefix + baseNames[i]);
	}

	_table->setHorizontalHeaderLabels(headers);
}

void WindowsDlg::updateDocCount()
{
	_docCountLabel->setText(tr("Total documents: %1").arg(_idxMap.size()));
}

} // namespace QtControls
