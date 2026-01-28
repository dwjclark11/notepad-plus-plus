// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ClipboardHistoryPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMimeData>

#include "../../ScintillaComponent/ScintillaEditView.h"

namespace QtControls {

// ============================================================================
// Constructor/Destructor
// ============================================================================

ClipboardHistoryPanel::ClipboardHistoryPanel(QWidget* parent)
    : StaticDialog()
{
    _parent = parent;
    _clipboard = QApplication::clipboard();
}

ClipboardHistoryPanel::~ClipboardHistoryPanel()
{
    saveHistory();
    destroy();
}

// ============================================================================
// Initialization
// ============================================================================

void ClipboardHistoryPanel::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;

    // Create the dialog widget
    create(tr("Clipboard History"), false);

    // Setup UI
    setupUI();
    connectSignals();

    // Load saved history
    loadHistory();

    // Initial update
    updateList();
}

void ClipboardHistoryPanel::doDialog()
{
    display(true);
}

// ============================================================================
// UI Setup
// ============================================================================

void ClipboardHistoryPanel::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setMinimumSize(300, 400);

    // Main layout
    _mainLayout = new QVBoxLayout(dialog);
    _mainLayout->setSpacing(6);
    _mainLayout->setContentsMargins(8, 8, 8, 8);

    // Filter/search box
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel(tr("Filter:"), dialog);
    _filterEdit = new QLineEdit(dialog);
    _filterEdit->setPlaceholderText(tr("Search clipboard items..."));
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(_filterEdit);
    _mainLayout->addLayout(filterLayout);

    // List view for clipboard items
    _listView = new ListView();
    _listView->init(dialog);
    _listView->setSelectionMode(ListViewSelectionMode::Single);
    _listView->getListWidget()->setAlternatingRowColors(true);

    // Set the list widget to expand
    QListWidget* listWidget = _listView->getListWidget();
    if (listWidget) {
        listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    _mainLayout->addWidget(_listView->getWidget(), 1);

    // Preview pane
    QLabel* previewLabel = new QLabel(tr("Preview:"), dialog);
    _mainLayout->addWidget(previewLabel);

    _preview = new QTextEdit(dialog);
    _preview->setReadOnly(true);
    _preview->setMaximumHeight(100);
    _preview->setPlaceholderText(tr("Select an item to preview"));
    _mainLayout->addWidget(_preview);

    // Button layout
    _buttonLayout = new QHBoxLayout();

    _pasteBtn = new QPushButton(tr("Paste"), dialog);
    _pasteBtn->setToolTip(tr("Paste selected item at cursor"));
    _pasteBtn->setEnabled(false);

    _deleteBtn = new QPushButton(tr("Delete"), dialog);
    _deleteBtn->setToolTip(tr("Delete selected item"));
    _deleteBtn->setEnabled(false);

    _clearBtn = new QPushButton(tr("Clear All"), dialog);
    _clearBtn->setToolTip(tr("Clear all clipboard history"));

    _buttonLayout->addWidget(_pasteBtn);
    _buttonLayout->addWidget(_deleteBtn);
    _buttonLayout->addStretch();
    _buttonLayout->addWidget(_clearBtn);

    _mainLayout->addLayout(_buttonLayout);

    // Status label
    _statusLabel = new QLabel(dialog);
    _statusLabel->setText(tr("Items: 0"));
    _mainLayout->addWidget(_statusLabel);

    dialog->setLayout(_mainLayout);
}

void ClipboardHistoryPanel::connectSignals()
{
    if (!_listView) return;

    // List view signals
    connect(_listView, &ListView::itemClicked, this, &ClipboardHistoryPanel::onItemClicked);
    connect(_listView, &ListView::itemDoubleClicked, this, &ClipboardHistoryPanel::onItemDoubleClicked);
    connect(_listView, &ListView::selectionChanged, this, &ClipboardHistoryPanel::onSelectionChanged);

    // Button signals
    connect(_pasteBtn, &QPushButton::clicked, this, &ClipboardHistoryPanel::onPasteClicked);
    connect(_deleteBtn, &QPushButton::clicked, this, &ClipboardHistoryPanel::onDeleteItem);
    connect(_clearBtn, &QPushButton::clicked, this, &ClipboardHistoryPanel::onClearAll);

    // Filter signal
    connect(_filterEdit, &QLineEdit::textChanged, this, &ClipboardHistoryPanel::onFilterChanged);

    // Clipboard monitoring
    if (_clipboard) {
        connect(_clipboard, &QClipboard::dataChanged, this, &ClipboardHistoryPanel::onClipboardChanged);
    }
}

// ============================================================================
// Event Handling
// ============================================================================

bool ClipboardHistoryPanel::run_dlgProc(QEvent* event)
{
    // Handle any custom events if needed
    (void)event;
    return false;
}

// ============================================================================
// Clipboard Operations
// ============================================================================

QString ClipboardHistoryPanel::getClipboardText() const
{
    if (!_clipboard) return QString();

    const QMimeData* mimeData = _clipboard->mimeData();
    if (!mimeData) return QString();

    // Check for text formats
    if (mimeData->hasText()) {
        return mimeData->text();
    }

    if (mimeData->hasHtml()) {
        return mimeData->html();
    }

    // Try to get any available text
    QStringList formats = mimeData->formats();
    for (const QString& format : formats) {
        if (format.contains("text", Qt::CaseInsensitive) ||
            format.contains("unicode", Qt::CaseInsensitive)) {
            QByteArray data = mimeData->data(format);
            if (!data.isEmpty()) {
                return QString::fromUtf8(data);
            }
        }
    }

    return QString();
}

void ClipboardHistoryPanel::onClipboardChanged()
{
    if (!_isTrackingClipboardOps || _isInternalClipboardChange) {
        _isInternalClipboardChange = false;
        return;
    }

    QString text = getClipboardText();
    if (!text.isEmpty()) {
        addToHistory(text);
    }
}

void ClipboardHistoryPanel::addToHistory(const QString& text)
{
    if (text.isEmpty()) return;

    // Check if item already exists
    int existingIndex = findItemIndex(text);
    if (existingIndex == 0) {
        // Item is already at the top, no change needed
        return;
    }

    if (existingIndex > 0) {
        // Move existing item to the top
        ClipboardItem item = std::move(_history[static_cast<size_t>(existingIndex)]);
        _history.erase(_history.begin() + existingIndex);
        _history.insert(_history.begin(), std::move(item));
    } else {
        // Add new item at the beginning
        _history.insert(_history.begin(), ClipboardItem(text));

        // Trim history if needed
        trimHistory();
    }

    // Clear filter and update display
    if (_filterEdit && !_filterEdit->text().isEmpty()) {
        _filterEdit->clear();
    }

    updateList();
}

void ClipboardHistoryPanel::clearHistory()
{
    if (_history.empty()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        getDialog(),
        tr("Clear Clipboard History"),
        tr("Are you sure you want to clear all clipboard history?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        _history.clear();
        _filteredHistory.clear();
        updateList();
        saveHistory();
    }
}

void ClipboardHistoryPanel::setMaxHistorySize(int size)
{
    if (size < 1) size = 1;
    if (size > 100) size = 100; // Reasonable upper limit

    _maxHistorySize = size;
    trimHistory();
    updateList();
}

void ClipboardHistoryPanel::trimHistory()
{
    while (static_cast<int>(_history.size()) > _maxHistorySize) {
        _history.pop_back();
    }
}

int ClipboardHistoryPanel::findItemIndex(const QString& text) const
{
    for (size_t i = 0; i < _history.size(); ++i) {
        if (_history[i].text == text) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ============================================================================
// List Management
// ============================================================================

void ClipboardHistoryPanel::updateList()
{
    if (!_listView) return;

    _listView->clear();

    const std::vector<ClipboardItem>* itemsToShow = _filterEdit && !_filterEdit->text().isEmpty()
        ? &_filteredHistory
        : &_history;

    for (const auto& item : *itemsToShow) {
        _listView->addItem(item.displayText);
    }

    // Update status
    if (_statusLabel) {
        int total = static_cast<int>(_history.size());
        int shown = static_cast<int>(itemsToShow->size());
        if (shown != total) {
            _statusLabel->setText(tr("Items: %1 (filtered from %2)").arg(shown).arg(total));
        } else {
            _statusLabel->setText(tr("Items: %1").arg(total));
        }
    }

    // Update button states
    onSelectionChanged();
}

void ClipboardHistoryPanel::updatePreview()
{
    if (!_preview || !_listView) return;

    int index = _listView->getSelectedIndex();
    if (index < 0 || index >= static_cast<int>(_filteredHistory.size())) {
        if (_filterEdit && _filterEdit->text().isEmpty()) {
            // Using full history
            if (index >= 0 && index < static_cast<int>(_history.size())) {
                QString text = _history[static_cast<size_t>(index)].text;
                if (text.length() > MAX_PREVIEW_LENGTH) {
                    text = text.left(MAX_PREVIEW_LENGTH) + "\n\n[... " +
                           tr("truncated") + " ...]";
                }
                _preview->setPlainText(text);
                return;
            }
        }
        _preview->clear();
        return;
    }

    QString text = _filteredHistory[static_cast<size_t>(index)].text;
    if (text.length() > MAX_PREVIEW_LENGTH) {
        text = text.left(MAX_PREVIEW_LENGTH) + "\n\n[... " +
               tr("truncated") + " ...]";
    }
    _preview->setPlainText(text);
}

void ClipboardHistoryPanel::onFilterChanged(const QString& text)
{
    _filteredHistory.clear();

    if (text.isEmpty()) {
        updateList();
        return;
    }

    // Filter history items
    for (const auto& item : _history) {
        if (item.text.contains(text, Qt::CaseInsensitive) ||
            item.displayText.contains(text, Qt::CaseInsensitive)) {
            _filteredHistory.push_back(item);
        }
    }

    updateList();
}

// ============================================================================
// Item Actions
// ============================================================================

void ClipboardHistoryPanel::onItemClicked(int index)
{
    (void)index;
    updatePreview();
}

void ClipboardHistoryPanel::onItemDoubleClicked(int index)
{
    pasteItem(index);
}

void ClipboardHistoryPanel::onSelectionChanged()
{
    if (!_listView || !_pasteBtn || !_deleteBtn) return;

    int index = _listView->getSelectedIndex();
    bool hasSelection = (index >= 0);

    _pasteBtn->setEnabled(hasSelection && _ppEditView != nullptr);
    _deleteBtn->setEnabled(hasSelection);

    updatePreview();
}

void ClipboardHistoryPanel::onPasteClicked()
{
    if (!_listView) return;

    int index = _listView->getSelectedIndex();
    if (index >= 0) {
        pasteItem(index);
    }
}

void ClipboardHistoryPanel::pasteItem(int index)
{
    if (!_ppEditView || !*_ppEditView) return;

    const ClipboardItem* item = nullptr;

    if (_filterEdit && !_filterEdit->text().isEmpty()) {
        // Using filtered list
        if (index < 0 || index >= static_cast<int>(_filteredHistory.size())) return;
        item = &_filteredHistory[static_cast<size_t>(index)];
    } else {
        // Using full history
        if (index < 0 || index >= static_cast<int>(_history.size())) return;
        item = &_history[static_cast<size_t>(index)];
    }

    if (!item) return;

    // Get the Scintilla edit view
    ScintillaEditView* editView = *_ppEditView;
    if (!editView) return;

    // Convert QString to std::string for Scintilla
    std::string text = item->text.toStdString();

    try {
        // Replace selection with clipboard text
        editView->replaceSelWith(text.c_str());

        // Set focus to the editor
        editView->grabFocus();

        // Move the pasted item to the top of history
        addToHistory(item->text);

    } catch (...) {
        QMessageBox::warning(
            getDialog(),
            tr("Clipboard Error"),
            tr("Cannot paste clipboard data. The data may be too large or invalid.")
        );
    }
}

void ClipboardHistoryPanel::onDeleteItem()
{
    if (!_listView) return;

    int index = _listView->getSelectedIndex();
    if (index >= 0) {
        deleteItem(index);
    }
}

void ClipboardHistoryPanel::deleteItem(int index)
{
    if (_filterEdit && !_filterEdit->text().isEmpty()) {
        // Deleting from filtered list - find original index
        if (index < 0 || index >= static_cast<int>(_filteredHistory.size())) return;

        const QString& textToDelete = _filteredHistory[static_cast<size_t>(index)].text;
        int originalIndex = findItemIndex(textToDelete);

        if (originalIndex >= 0) {
            _history.erase(_history.begin() + originalIndex);
        }

        _filteredHistory.erase(_filteredHistory.begin() + index);
    } else {
        // Deleting from full history
        if (index < 0 || index >= static_cast<int>(_history.size())) return;
        _history.erase(_history.begin() + index);
    }

    updateList();
    saveHistory();
}

void ClipboardHistoryPanel::onClearAll()
{
    clearHistory();
}

// ============================================================================
// Persistence
// ============================================================================

void ClipboardHistoryPanel::loadHistory()
{
    QSettings settings;
    settings.beginGroup("ClipboardHistory");

    _maxHistorySize = settings.value("MaxHistorySize", 20).toInt();

    QByteArray data = settings.value("History").toByteArray();
    if (!data.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const auto& val : array) {
                if (val.isObject()) {
                    QJsonObject obj = val.toObject();
                    QString text = obj["text"].toString();
                    if (!text.isEmpty()) {
                        ClipboardItem item;
                        item.text = text;
                        item.timestamp = QDateTime::fromString(
                            obj["timestamp"].toString(), Qt::ISODate);
                        item.updateDisplayText();
                        _history.push_back(std::move(item));
                    }
                }
            }
        }
    }

    settings.endGroup();

    trimHistory();
}

void ClipboardHistoryPanel::saveHistory()
{
    QSettings settings;
    settings.beginGroup("ClipboardHistory");

    settings.setValue("MaxHistorySize", _maxHistorySize);

    QJsonArray array;
    for (const auto& item : _history) {
        QJsonObject obj;
        obj["text"] = item.text;
        obj["timestamp"] = item.timestamp.toString(Qt::ISODate);
        array.append(obj);
    }

    QJsonDocument doc(array);
    settings.setValue("History", doc.toJson(QJsonDocument::Compact));

    settings.endGroup();
}

} // namespace QtControls
