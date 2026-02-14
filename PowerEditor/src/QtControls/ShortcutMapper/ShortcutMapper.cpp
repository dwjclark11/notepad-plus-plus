// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ShortcutMapper.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QInputDialog>
#include <QtCore/QStringList>
#include <QtGui/QKeyEvent>

#include <algorithm>
#include <cctype>

#include "../../MISC/Common/Common.h"
#include "../../Parameters.h"
#include "../../NppXml.h"
#include "../ShortcutManager/ShortcutManager.h"

namespace QtControls {

namespace ShortcutMapper {

// ============================================================================
// Helper functions
// ============================================================================

static QString toLower(const QString& str) {
    return str.toLower();
}

static std::vector<QString> splitFilter(const QString& filter) {
    std::vector<QString> result;
    QStringList parts = filter.split(' ', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        QString trimmed = part.trimmed();
        if (!trimmed.isEmpty()) {
            result.push_back(toLower(trimmed));
        }
    }
    return result;
}

// ============================================================================
// ShortcutMapper implementation
// ============================================================================

ShortcutMapper::ShortcutMapper(QWidget* parent)
    : StaticDialog(parent)
    , _currentState(GridState::STATE_MENU)
{
    _lastHomeRow.resize(_nbTab, 0);
    _lastCursorRow.resize(_nbTab, 0);
}

ShortcutMapper::~ShortcutMapper() = default;

void ShortcutMapper::doDialog(GridState initState) {
    if (!isCreated()) {
        create(tr("Shortcut Mapper"), false);
        setupUI();
        connectSignals();
    }

    // Load shortcuts from NppParameters
    loadShortcutsFromParameters();

    _currentState = initState;
    _tabWidget->setCurrentIndex(static_cast<int>(initState));

    fillGrid();
    QtControls::StaticDialog::goToCenter();
    QtControls::StaticDialog::display(true, true);
}

void ShortcutMapper::loadShortcutsFromParameters() {
    NppParameters& nppParams = NppParameters::getInstance();

    // Clear existing shortcuts
    _menuShortcuts.clear();
    _macroShortcuts.clear();
    _userShortcuts.clear();
    _pluginShortcuts.clear();
    _scintillaShortcuts.clear();

    // Load main menu shortcuts
    std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
    for (size_t i = 0; i < shortcuts.size(); ++i) {
        const CommandShortcut& sc = shortcuts[i];
        ShortcutData data;
        data.name = QString::fromUtf8(sc.getName());
        data.keyCombo = sc.getKeyCombo();
        data.shortcut = keyComboToString(data.keyCombo);
        data.isEnabled = sc.isValid();
        data.originalIndex = i;
        // TODO: Set category based on command ID
        _menuShortcuts.push_back(data);
    }

    // Load macro shortcuts
    std::vector<MacroShortcut>& macros = nppParams.getMacroList();
    for (size_t i = 0; i < macros.size(); ++i) {
        const MacroShortcut& sc = macros[i];
        ShortcutData data;
        data.name = QString::fromUtf8(sc.getName());
        data.keyCombo = sc.getKeyCombo();
        data.shortcut = keyComboToString(data.keyCombo);
        data.isEnabled = sc.isValid();
        data.originalIndex = i;
        _macroShortcuts.push_back(data);
    }

    // Load user command shortcuts
    std::vector<UserCommand>& userCommands = nppParams.getUserCommandList();
    for (size_t i = 0; i < userCommands.size(); ++i) {
        const UserCommand& sc = userCommands[i];
        ShortcutData data;
        data.name = QString::fromUtf8(sc.getName());
        data.keyCombo = sc.getKeyCombo();
        data.shortcut = keyComboToString(data.keyCombo);
        data.isEnabled = sc.isValid();
        data.originalIndex = i;
        _userShortcuts.push_back(data);
    }

    // Load plugin command shortcuts
    std::vector<PluginCmdShortcut>& pluginCommands = nppParams.getPluginCommandList();
    for (size_t i = 0; i < pluginCommands.size(); ++i) {
        const PluginCmdShortcut& sc = pluginCommands[i];
        ShortcutData data;
        data.name = QString::fromUtf8(sc.getName());
        data.keyCombo = sc.getKeyCombo();
        data.shortcut = keyComboToString(data.keyCombo);
        data.isEnabled = sc.isValid();
        data.originalIndex = i;
        data.pluginName = QString::fromUtf8(sc.getModuleName());
        _pluginShortcuts.push_back(data);
    }

    // Load Scintilla shortcuts
    std::vector<ScintillaKeyMap>& scintillaKeys = nppParams.getScintillaKeyList();
    for (size_t i = 0; i < scintillaKeys.size(); ++i)
    {
        const ScintillaKeyMap& skm = scintillaKeys[i];
        ShortcutData data;
        data.name = QString::fromUtf8(skm.getName());
        // Use the first key combo for display
        if (skm.getSize() > 0)
        {
            data.keyCombo = skm.getKeyComboByIndex(0);
        }
        else
        {
            data.keyCombo = skm.getKeyCombo();
        }
        data.shortcut = keyComboToString(data.keyCombo);
        data.isEnabled = (data.keyCombo._key != 0);
        data.originalIndex = i;
        _scintillaShortcuts.push_back(data);
    }
}

int ShortcutMapper::getCommandIdForShortcut(size_t index) const {
    NppParameters& nppParams = NppParameters::getInstance();

    switch (_currentState) {
        case GridState::STATE_MENU: {
            std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
            if (index < shortcuts.size()) {
                return shortcuts[index].getID();
            }
            break;
        }
        case GridState::STATE_MACRO: {
            std::vector<MacroShortcut>& macros = nppParams.getMacroList();
            if (index < macros.size()) {
                return macros[index].getID();
            }
            break;
        }
        case GridState::STATE_USER: {
            std::vector<UserCommand>& userCommands = nppParams.getUserCommandList();
            if (index < userCommands.size()) {
                return userCommands[index].getID();
            }
            break;
        }
        case GridState::STATE_PLUGIN: {
            std::vector<PluginCmdShortcut>& pluginCommands = nppParams.getPluginCommandList();
            if (index < pluginCommands.size()) {
                return static_cast<int>(pluginCommands[index].getID());
            }
            break;
        }
        case GridState::STATE_SCINTILLA:
        {
            std::vector<ScintillaKeyMap>& scintKeys = nppParams.getScintillaKeyList();
            if (index < scintKeys.size()) {
                return scintKeys[index].getMenuCmdID();
            }
            break;
        }
    }

    return -1;
}

void ShortcutMapper::setupUI() {
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Shortcut Mapper"));
    dialog->resize(750, 550);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Tab widget
    _tabWidget = new QTabWidget(dialog);
    _tabNames[0] = tr("Main menu");
    _tabNames[1] = tr("Macros");
    _tabNames[2] = tr("Run commands");
    _tabNames[3] = tr("Plugin commands");
    _tabNames[4] = tr("Scintilla commands");

    for (int i = 0; i < _nbTab; ++i) {
        _tabWidget->addTab(new QWidget(_tabWidget), _tabNames[i]);
    }
    mainLayout->addWidget(_tabWidget);

    // Filter row
    auto* filterLayout = new QHBoxLayout();
    _filterLabel = new QLabel(tr("Filter:"), dialog);
    filterLayout->addWidget(_filterLabel);

    _filterEdit = new QLineEdit(dialog);
    _filterEdit->setPlaceholderText(tr("Type to filter shortcuts..."));
    filterLayout->addWidget(_filterEdit, 1);

    _filterClearButton = new QPushButton(tr("Clear"), dialog);
    _filterClearButton->setFixedWidth(60);
    _filterClearButton->setToolTip(tr("Clear filter"));
    filterLayout->addWidget(_filterClearButton);

    mainLayout->addLayout(filterLayout);

    // Grid/table
    _grid = new QTableWidget(dialog);
    _grid->setColumnCount(3);
    _grid->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Shortcut") << tr("Category"));
    _grid->horizontalHeader()->setStretchLastSection(true);
    _grid->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _grid->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    _grid->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    _grid->setSelectionBehavior(QAbstractItemView::SelectRows);
    _grid->setSelectionMode(QAbstractItemView::SingleSelection);
    _grid->setAlternatingRowColors(true);
    _grid->setContextMenuPolicy(Qt::CustomContextMenu);
    mainLayout->addWidget(_grid, 1);

    // Conflict info label
    _conflictLabel = new QLabel(dialog);
    _conflictLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _conflictLabel->setMinimumHeight(40);
    _conflictLabel->setWordWrap(true);
    _conflictInfoOk = tr("No shortcut conflicts for this item.");
    _conflictInfoEditing = tr("No conflicts...");
    _conflictLabel->setText(_conflictInfoOk);
    mainLayout->addWidget(_conflictLabel);

    // Button rows
    auto* buttonLayout1 = new QHBoxLayout();
    buttonLayout1->addStretch();

    _modifyButton = new QPushButton(tr("Modify"), dialog);
    _modifyButton->setToolTip(tr("Edit the selected shortcut"));
    buttonLayout1->addWidget(_modifyButton);

    _deleteButton = new QPushButton(tr("Delete"), dialog);
    _deleteButton->setToolTip(tr("Delete the selected macro or command"));
    buttonLayout1->addWidget(_deleteButton);

    _clearButton = new QPushButton(tr("Clear"), dialog);
    _clearButton->setToolTip(tr("Clear the shortcut for this item"));
    buttonLayout1->addWidget(_clearButton);

    _clearAllButton = new QPushButton(tr("Clear All"), dialog);
    _clearAllButton->setToolTip(tr("Clear all shortcuts in this category"));
    buttonLayout1->addWidget(_clearAllButton);

    buttonLayout1->addSpacing(20);

    _okButton = new QPushButton(tr("OK"), dialog);
    _okButton->setDefault(true);
    buttonLayout1->addWidget(_okButton);

    mainLayout->addLayout(buttonLayout1);

    auto* buttonLayout2 = new QHBoxLayout();
    buttonLayout2->addStretch();

    _importButton = new QPushButton(tr("Import..."), dialog);
    _importButton->setToolTip(tr("Import shortcuts from file"));
    buttonLayout2->addWidget(_importButton);

    _exportButton = new QPushButton(tr("Export..."), dialog);
    _exportButton->setToolTip(tr("Export shortcuts to file"));
    buttonLayout2->addWidget(_exportButton);

    _resetAllButton = new QPushButton(tr("Reset All"), dialog);
    _resetAllButton->setToolTip(tr("Reset all shortcuts to default"));
    buttonLayout2->addWidget(_resetAllButton);

    mainLayout->addLayout(buttonLayout2);

    // Store initial rect
    _rc = dialog->geometry();

    // Initialize button states
    updateButtonStates();
}

void ShortcutMapper::connectSignals() {
    // Tab changes
    connect(_tabWidget, &QTabWidget::currentChanged, this, &ShortcutMapper::onTabChanged);

    // Filter
    connect(_filterEdit, &QLineEdit::textChanged, this, &ShortcutMapper::onFilterChanged);
    connect(_filterClearButton, &QPushButton::clicked, this, &ShortcutMapper::onFilterClearClicked);

    // Grid interactions
    connect(_grid, &QTableWidget::cellDoubleClicked, this, &ShortcutMapper::onItemDoubleClicked);
    connect(_grid->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ShortcutMapper::onItemSelectionChanged);
    connect(_grid, &QTableWidget::customContextMenuRequested, this, &ShortcutMapper::onContextMenu);

    // Buttons
    connect(_modifyButton, &QPushButton::clicked, this, &ShortcutMapper::onModifyClicked);
    connect(_deleteButton, &QPushButton::clicked, this, &ShortcutMapper::onDeleteClicked);
    connect(_clearButton, &QPushButton::clicked, this, &ShortcutMapper::onClearClicked);
    connect(_clearAllButton, &QPushButton::clicked, this, &ShortcutMapper::onClearAllClicked);
    connect(_importButton, &QPushButton::clicked, this, &ShortcutMapper::onImportClicked);
    connect(_exportButton, &QPushButton::clicked, this, &ShortcutMapper::onExportClicked);
    connect(_resetAllButton, &QPushButton::clicked, this, &ShortcutMapper::onResetAllClicked);
    connect(_okButton, &QPushButton::clicked, this, &ShortcutMapper::onOkClicked);
}

void ShortcutMapper::getClientRect(QRect& rc) const {
    if (_grid) {
        rc = _grid->geometry();
    }
}

QString ShortcutMapper::getTabString(int index) const {
    if (index >= 0 && index < _nbTab) {
        return _tabNames[index];
    }
    return QString();
}

void ShortcutMapper::onTabChanged(int index) {
    // Save current view state
    saveViewState();

    // Update current state
    switch (index) {
        case 0: _currentState = GridState::STATE_MENU; break;
        case 1: _currentState = GridState::STATE_MACRO; break;
        case 2: _currentState = GridState::STATE_USER; break;
        case 3: _currentState = GridState::STATE_PLUGIN; break;
        case 4: _currentState = GridState::STATE_SCINTILLA; break;
        default: _currentState = GridState::STATE_MENU; break;
    }

    // Refresh grid
    fillGrid();

    // Restore view state for this tab
    restoreViewState();
}

void ShortcutMapper::onFilterChanged(const QString& text) {
    (void)text;
    fillGrid();
}

void ShortcutMapper::onItemDoubleClicked(int row, int column) {
    (void)column;
    if (row >= 0) {
        onModifyClicked();
    }
}

void ShortcutMapper::onItemSelectionChanged() {
    updateButtonStates();

    int currentRow = _grid->currentRow();
    if (currentRow >= 0 && currentRow < static_cast<int>(_shortcutIndex.size())) {
        size_t originalIndex = _shortcutIndex[currentRow];

        // Get the appropriate shortcuts list
        const std::vector<ShortcutData>* shortcuts = nullptr;
        switch (_currentState) {
            case GridState::STATE_MENU: shortcuts = &_menuShortcuts; break;
            case GridState::STATE_MACRO: shortcuts = &_macroShortcuts; break;
            case GridState::STATE_USER: shortcuts = &_userShortcuts; break;
            case GridState::STATE_PLUGIN: shortcuts = &_pluginShortcuts; break;
            case GridState::STATE_SCINTILLA: shortcuts = &_scintillaShortcuts; break;
        }

        if (shortcuts && originalIndex < shortcuts->size()) {
            showConflictInfo((*shortcuts)[originalIndex]);
        }
    } else {
        clearConflictInfo();
    }
}

void ShortcutMapper::onModifyClicked() {
    int currentRow = _grid->currentRow();
    if (currentRow < 0 || currentRow >= static_cast<int>(_shortcutIndex.size())) {
        return;
    }

    size_t originalIndex = _shortcutIndex[currentRow];

    // Get the command ID for this shortcut
    int commandId = getCommandIdForShortcut(originalIndex);
    if (commandId < 0) {
        return;
    }

    // TODO: Open a proper shortcut edit dialog that captures key input
    // For now, show a simple input dialog as a placeholder
    bool ok;
    QString text = QInputDialog::getText(getDialog(), tr("Modify Shortcut"),
                                         tr("Enter new shortcut (e.g., Ctrl+N):"),
                                         QLineEdit::Normal, QString(), &ok);
    if (!ok || text.isEmpty()) {
        return;
    }

    // Parse the shortcut string (simplified parsing)
    KeyCombo newCombo;
    QString lowerText = text.toLower();
    newCombo._isCtrl = lowerText.contains("ctrl");
    newCombo._isAlt = lowerText.contains("alt");
    newCombo._isShift = lowerText.contains("shift");

    // Extract the key
    QKeySequence seq(text);
    if (!seq.isEmpty()) {
        newCombo = ShortcutManager::qKeySequenceToKeyCombo(seq);
    }

    // Check for conflicts
    QString conflictLocation;
    if (findKeyConflicts(newCombo, originalIndex, &conflictLocation)) {
        int result = QMessageBox::warning(getDialog(), tr("Shortcut Conflict"),
            tr("This shortcut conflicts with:\n%1\n\nDo you want to continue?").arg(conflictLocation),
            QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes) {
            return;
        }
    }

    // Update the shortcut via ShortcutManager
    ShortcutManager* manager = ShortcutManager::getInstance();
    manager->updateCommandShortcut(commandId, newCombo);

    // Refresh the grid
    loadShortcutsFromParameters();
    fillGrid();
}

void ShortcutMapper::onDeleteClicked() {
    int currentRow = _grid->currentRow();
    if (currentRow < 0 || currentRow >= static_cast<int>(_shortcutIndex.size())) {
        return;
    }

    // Only macros and user commands can be deleted
    if (_currentState != GridState::STATE_MACRO && _currentState != GridState::STATE_USER) {
        return;
    }

    size_t originalIndex = _shortcutIndex[currentRow];

    int result = QMessageBox::question(getDialog(), tr("Confirm Delete"),
        tr("Are you sure you want to delete this item?"),
        QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    NppParameters& nppParams = NppParameters::getInstance();

    if (_currentState == GridState::STATE_MACRO)
    {
        std::vector<MacroShortcut>& macros = nppParams.getMacroList();
        if (originalIndex < macros.size())
        {
            macros.erase(macros.begin() + originalIndex);
            nppParams.setShortcutDirty();
        }
    }
    else if (_currentState == GridState::STATE_USER)
    {
        std::vector<UserCommand>& userCmds = nppParams.getUserCommandList();
        if (originalIndex < userCmds.size())
        {
            userCmds.erase(userCmds.begin() + originalIndex);
            nppParams.setShortcutDirty();
        }
    }

    // Refresh the grid
    loadShortcutsFromParameters();
    fillGrid();
}

void ShortcutMapper::onClearClicked() {
    int currentRow = _grid->currentRow();
    if (currentRow < 0 || currentRow >= static_cast<int>(_shortcutIndex.size())) {
        return;
    }

    // Scintilla shortcuts cannot be cleared
    if (_currentState == GridState::STATE_SCINTILLA) {
        return;
    }

    size_t originalIndex = _shortcutIndex[currentRow];

    // Get the command ID for this shortcut
    int commandId = getCommandIdForShortcut(originalIndex);
    if (commandId < 0) {
        return;
    }

    // Clear the shortcut via ShortcutManager
    ShortcutManager* manager = ShortcutManager::getInstance();
    manager->clearCommandShortcut(commandId);

    // Refresh the grid
    loadShortcutsFromParameters();
    fillGrid();
}

void ShortcutMapper::onClearAllClicked() {
    int result = QMessageBox::question(getDialog(), tr("Confirm Clear All"),
        tr("Are you sure you want to clear all shortcuts in this category?"),
        QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    // Clear all shortcuts in current category
    ShortcutManager* manager = ShortcutManager::getInstance();
    std::vector<ShortcutData>* shortcuts = nullptr;

    switch (_currentState) {
        case GridState::STATE_MENU:
            shortcuts = &_menuShortcuts;
            break;
        case GridState::STATE_MACRO:
            shortcuts = &_macroShortcuts;
            break;
        case GridState::STATE_USER:
            shortcuts = &_userShortcuts;
            break;
        case GridState::STATE_PLUGIN:
            shortcuts = &_pluginShortcuts;
            break;
        case GridState::STATE_SCINTILLA:
            // Scintilla shortcuts cannot be cleared
            return;
    }

    if (shortcuts) {
        for (size_t i = 0; i < shortcuts->size(); ++i) {
            int commandId = getCommandIdForShortcut(i);
            if (commandId >= 0) {
                manager->clearCommandShortcut(commandId);
            }
        }
    }

    // Refresh the grid
    loadShortcutsFromParameters();
    fillGrid();
}

void ShortcutMapper::onImportClicked() {
    QString fileName = QFileDialog::getOpenFileName(getDialog(),
        tr("Import Shortcuts"),
        QString(),
        tr("XML Files (*.xml);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    // Load the XML file
    NppXml::NewDocument doc;
    std::wstring wFileName = fileName.toStdWString();
    if (!NppXml::loadFileShortcut(&doc, wFileName.c_str()))
    {
        QMessageBox::warning(getDialog(), tr("Import Error"),
            tr("Failed to load shortcuts file:\n%1").arg(fileName));
        return;
    }

    // Find the root NotepadPlus element
    NppXml::Element root = NppXml::firstChildElement(&doc, "NotepadPlus");
    if (!root)
    {
        QMessageBox::warning(getDialog(), tr("Import Error"),
            tr("Invalid shortcuts file format."));
        return;
    }

    NppParameters& nppParams = NppParameters::getInstance();

    // Helper: parse a shortcut element into a KeyCombo and name
    auto parseShortcutElement = [](const NppXml::Element& elem, KeyCombo& combo, std::string& name) -> bool
    {
        const char* nameAttr = NppXml::attribute(elem, "name", "");
        name = nameAttr;

        const char* ctrlAttr = NppXml::attribute(elem, "Ctrl", "no");
        const char* altAttr = NppXml::attribute(elem, "Alt", "no");
        const char* shiftAttr = NppXml::attribute(elem, "Shift", "no");
        int key = NppXml::intAttribute(elem, "Key", -1);
        if (key == -1) return false;

        combo._isCtrl = (strcmp(ctrlAttr, "yes") == 0);
        combo._isAlt = (strcmp(altAttr, "yes") == 0);
        combo._isShift = (strcmp(shiftAttr, "yes") == 0);
        combo._key = static_cast<unsigned char>(key);
        return true;
    };

    // Import main menu shortcut customizations
    NppXml::Element internalCmdsRoot = NppXml::firstChildElement(root, "InternalCommands");
    if (internalCmdsRoot)
    {
        std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
        for (NppXml::Element childNode = NppXml::firstChildElement(internalCmdsRoot, "Shortcut");
            childNode;
            childNode = NppXml::nextSiblingElement(childNode, "Shortcut"))
        {
            int cmdId = NppXml::intAttribute(childNode, "id", -1);
            if (cmdId < 0) continue;

            KeyCombo combo;
            std::string name;
            if (!parseShortcutElement(childNode, combo, name)) continue;

            for (size_t i = 0; i < shortcuts.size(); ++i)
            {
                if (shortcuts[i].getID() == cmdId)
                {
                    shortcuts[i].setKeyCombo(combo);
                    nppParams.addUserModifiedIndex(i);
                    break;
                }
            }
        }
    }

    // Import macro shortcuts
    NppXml::Element macrosRoot = NppXml::firstChildElement(root, "Macros");
    if (macrosRoot)
    {
        std::vector<MacroShortcut>& macros = nppParams.getMacroList();
        for (NppXml::Element childNode = NppXml::firstChildElement(macrosRoot, "Macro");
            childNode;
            childNode = NppXml::nextSiblingElement(childNode, "Macro"))
        {
            KeyCombo combo;
            std::string name;
            if (!parseShortcutElement(childNode, combo, name)) continue;

            for (auto& macro : macros)
            {
                if (name == macro.getName())
                {
                    macro.setKeyCombo(combo);
                    break;
                }
            }
        }
    }

    // Import user command shortcuts
    NppXml::Element userCmdsRoot = NppXml::firstChildElement(root, "UserDefinedCommands");
    if (userCmdsRoot)
    {
        std::vector<UserCommand>& userCmds = nppParams.getUserCommandList();
        for (NppXml::Element childNode = NppXml::firstChildElement(userCmdsRoot, "Command");
            childNode;
            childNode = NppXml::nextSiblingElement(childNode, "Command"))
        {
            KeyCombo combo;
            std::string name;
            if (!parseShortcutElement(childNode, combo, name)) continue;

            for (auto& cmd : userCmds)
            {
                if (name == cmd.getName())
                {
                    cmd.setKeyCombo(combo);
                    break;
                }
            }
        }
    }

    nppParams.setShortcutDirty();

    // Refresh the grid
    loadShortcutsFromParameters();
    fillGrid();

    QMessageBox::information(getDialog(), tr("Import Complete"),
        tr("Shortcuts have been imported successfully."));
}

void ShortcutMapper::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(getDialog(),
        tr("Export Shortcuts"),
        QStringLiteral("shortcuts.xml"),
        tr("XML Files (*.xml);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    NppParameters& nppParams = NppParameters::getInstance();

    // Build an XML document
    NppXml::NewDocument docStorage;
    NppXml::Document doc = &docStorage;
    NppXml::createNewDeclaration(doc);
    NppXml::Element root = NppXml::createChildElement(doc, "NotepadPlus");

    auto writeShortcutAttribs = [](NppXml::Element& elem, const Shortcut& sc)
    {
        NppXml::setAttribute(elem, "name", sc.getName());
        NppXml::setAttribute(elem, "Ctrl", sc.getKeyCombo()._isCtrl ? "yes" : "no");
        NppXml::setAttribute(elem, "Alt", sc.getKeyCombo()._isAlt ? "yes" : "no");
        NppXml::setAttribute(elem, "Shift", sc.getKeyCombo()._isShift ? "yes" : "no");
        NppXml::setAttribute(elem, "Key", static_cast<int>(sc.getKeyCombo()._key));
    };

    // Export main menu shortcuts (only modified ones)
    NppXml::Element internalCmdsNode = NppXml::createChildElement(root, "InternalCommands");
    std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
    for (size_t i = 0; i < shortcuts.size(); ++i)
    {
        NppXml::Element scNode = NppXml::createChildElement(internalCmdsNode, "Shortcut");
        NppXml::setAttribute(scNode, "id", shortcuts[i].getID());
        writeShortcutAttribs(scNode, shortcuts[i]);
    }

    // Export macro shortcuts
    NppXml::Element macrosNode = NppXml::createChildElement(root, "Macros");
    std::vector<MacroShortcut>& macros = nppParams.getMacroList();
    for (auto& macro : macros)
    {
        NppXml::Element macroNode = NppXml::createChildElement(macrosNode, "Macro");
        writeShortcutAttribs(macroNode, macro);
    }

    // Export user command shortcuts
    NppXml::Element userCmdsNode = NppXml::createChildElement(root, "UserDefinedCommands");
    std::vector<UserCommand>& userCmds = nppParams.getUserCommandList();
    for (auto& cmd : userCmds)
    {
        NppXml::Element cmdNode = NppXml::createChildElement(userCmdsNode, "Command");
        writeShortcutAttribs(cmdNode, cmd);
        NppXml::createChildText(cmdNode, cmd.getCmd());
    }

    // Export plugin command shortcuts
    NppXml::Element pluginCmdsNode = NppXml::createChildElement(root, "PluginCommands");
    std::vector<PluginCmdShortcut>& pluginCmds = nppParams.getPluginCommandList();
    for (auto& pc : pluginCmds)
    {
        NppXml::Element pcNode = NppXml::createChildElement(pluginCmdsNode, "PluginCommand");
        writeShortcutAttribs(pcNode, pc);
        NppXml::setAttribute(pcNode, "moduleName", pc.getModuleName());
        NppXml::setAttribute(pcNode, "internalID", static_cast<int>(pc.getInternalID()));
    }

    // Save the file
    std::wstring wFileName = fileName.toStdWString();
    if (NppXml::saveFileShortcut(doc, wFileName.c_str()))
    {
        QMessageBox::information(getDialog(), tr("Export Complete"),
            tr("Shortcuts have been exported to:\n%1").arg(fileName));
    }
    else
    {
        QMessageBox::warning(getDialog(), tr("Export Error"),
            tr("Failed to save shortcuts file:\n%1").arg(fileName));
    }
}

void ShortcutMapper::onResetAllClicked() {
    int result = QMessageBox::question(getDialog(), tr("Confirm Reset All"),
        tr("Are you sure you want to reset ALL shortcuts to their default values?\n"
           "This action cannot be undone."),
        QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    NppParameters& nppParams = NppParameters::getInstance();

    // Reload shortcuts from the shortcuts.xml file on disk
    // This discards all in-memory changes and reloads defaults
    if (nppParams.reloadShortcutsFromFile())
    {
        nppParams.setShortcutDirty();

        // Refresh the grid
        loadShortcutsFromParameters();
        fillGrid();

        // Apply the reset shortcuts
        ShortcutManager* manager = ShortcutManager::getInstance();
        manager->updateShortcutsFromParameters();

        QMessageBox::information(getDialog(), tr("Reset Complete"),
            tr("All shortcuts have been reset to their default values."));
    }
    else
    {
        QMessageBox::warning(getDialog(), tr("Reset Failed"),
            tr("Failed to reload shortcuts from configuration file."));
    }
}

void ShortcutMapper::onOkClicked() {
    // Save shortcuts to configuration
    NppParameters& nppParams = NppParameters::getInstance();
    nppParams.writeShortcuts();

    // Apply shortcuts to all registered actions
    ShortcutManager* manager = ShortcutManager::getInstance();
    manager->applyShortcuts();

    hide();
}

void ShortcutMapper::onFilterClearClicked() {
    _filterEdit->clear();
    _filterEdit->setFocus();
}

void ShortcutMapper::onContextMenu(const QPoint& pos) {
    QMenu menu(_grid);

    QAction* modifyAction = menu.addAction(tr("Modify"));
    QAction* deleteAction = menu.addAction(tr("Delete"));
    QAction* clearAction = menu.addAction(tr("Clear"));

    // Enable/disable actions based on current state
    bool hasSelection = _grid->currentRow() >= 0;
    bool hasRows = _grid->rowCount() > 0;

    modifyAction->setEnabled(hasRows);
    clearAction->setEnabled(hasRows && _currentState != GridState::STATE_SCINTILLA);

    // Only macros and user commands can be deleted
    bool canDelete = (_currentState == GridState::STATE_MACRO ||
                      _currentState == GridState::STATE_USER) && hasSelection;
    deleteAction->setEnabled(canDelete);

    QAction* selectedAction = menu.exec(_grid->mapToGlobal(pos));

    if (selectedAction == modifyAction) {
        onModifyClicked();
    } else if (selectedAction == deleteAction) {
        onDeleteClicked();
    } else if (selectedAction == clearAction) {
        onClearClicked();
    }
}

void ShortcutMapper::initGrid() {
    _grid->clearContents();
    _grid->setRowCount(0);
}

void ShortcutMapper::fillGrid() {
    initGrid();
    _shortcutIndex.clear();

    // Parse filter text
    QString filterText = _filterEdit->text();
    std::vector<QString> filterWords = splitFilter(filterText);

    // Get the appropriate shortcuts list based on current state
    const std::vector<ShortcutData>* shortcuts = nullptr;
    int categoryColumn = -1;

    switch (_currentState) {
        case GridState::STATE_MENU:
            shortcuts = &_menuShortcuts;
            categoryColumn = 2;
            break;
        case GridState::STATE_MACRO:
            shortcuts = &_macroShortcuts;
            categoryColumn = -1;
            break;
        case GridState::STATE_USER:
            shortcuts = &_userShortcuts;
            categoryColumn = -1;
            break;
        case GridState::STATE_PLUGIN:
            shortcuts = &_pluginShortcuts;
            categoryColumn = 2;
            break;
        case GridState::STATE_SCINTILLA:
            shortcuts = &_scintillaShortcuts;
            categoryColumn = -1;
            break;
    }

    if (!shortcuts) return;

    // Set column count based on category visibility
    int colCount = (categoryColumn >= 0) ? 3 : 2;
    _grid->setColumnCount(colCount);

    QStringList headers;
    headers << tr("Name") << tr("Shortcut");
    if (categoryColumn >= 0) {
        headers << tr(_currentState == GridState::STATE_PLUGIN ? "Plugin" : "Category");
    }
    _grid->setHorizontalHeaderLabels(headers);

    // Populate grid
    int row = 0;
    for (size_t i = 0; i < shortcuts->size(); ++i) {
        const ShortcutData& sc = (*shortcuts)[i];

        // Apply filter
        if (!filterWords.empty()) {
            QString nameLower = toLower(sc.name);
            QString shortcutLower = toLower(sc.shortcut);
            QString categoryLower = toLower(sc.category);

            bool matches = true;
            for (const QString& word : filterWords) {
                if (!nameLower.contains(word) &&
                    !shortcutLower.contains(word) &&
                    !categoryLower.contains(word)) {
                    matches = false;
                    break;
                }
            }
            if (!matches) continue;
        }

        _grid->insertRow(row);
        _shortcutIndex.push_back(i);

        // Name column
        QTableWidgetItem* nameItem = new QTableWidgetItem(sc.name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        if (sc.hasConflict) {
            nameItem->setBackground(Qt::red);
        }
        _grid->setItem(row, 0, nameItem);

        // Shortcut column
        QTableWidgetItem* shortcutItem = new QTableWidgetItem(sc.shortcut);
        shortcutItem->setFlags(shortcutItem->flags() & ~Qt::ItemIsEditable);
        _grid->setItem(row, 1, shortcutItem);

        // Category column (if applicable)
        if (colCount > 2) {
            QTableWidgetItem* categoryItem = new QTableWidgetItem(
                _currentState == GridState::STATE_PLUGIN ? sc.pluginName : sc.category);
            categoryItem->setFlags(categoryItem->flags() & ~Qt::ItemIsEditable);
            _grid->setItem(row, 2, categoryItem);
        }

        ++row;
    }

    // Adjust column widths
    _grid->resizeColumnsToContents();
    _grid->horizontalHeader()->setStretchLastSection(true);
    if (_grid->columnCount() > 0) {
        _grid->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    }

    updateButtonStates();
}

bool ShortcutMapper::isFilterValid(const ShortcutData& sc) {
    // This is checked during fillGrid now
    (void)sc;
    return true;
}

bool ShortcutMapper::isConflict(const KeyCombo& lhs, const KeyCombo& rhs) const {
    return (lhs._isCtrl == rhs._isCtrl) &&
           (lhs._isAlt == rhs._isAlt) &&
           (lhs._isShift == rhs._isShift) &&
           (lhs._key == rhs._key);
}

bool ShortcutMapper::findKeyConflicts(const KeyCombo& combo, size_t itemIndex, QString* conflictLocation) {
    if (combo._key == 0) {
        return false;
    }

    bool hasConflict = false;

    // Check all shortcut categories
    const std::vector<ShortcutData>* allShortcuts[] = {
        &_menuShortcuts,
        &_macroShortcuts,
        &_userShortcuts,
        &_pluginShortcuts,
        &_scintillaShortcuts
    };

    for (int cat = 0; cat < 5; ++cat) {
        const std::vector<ShortcutData>* shortcuts = allShortcuts[cat];
        GridState catState = static_cast<GridState>(cat);

        for (size_t i = 0; i < shortcuts->size(); ++i) {
            const ShortcutData& sc = (*shortcuts)[i];

            if (!sc.isEnabled) continue;

            // Skip the item being tested
            if (i == itemIndex && catState == _currentState) continue;

            if (isConflict(sc.keyCombo, combo)) {
                hasConflict = true;
                if (conflictLocation) {
                    if (!conflictLocation->isEmpty()) {
                        *conflictLocation += "\n";
                    }
                    *conflictLocation += _tabNames[cat] + "  |  " +
                                        QString::number(i + 1) + "   " +
                                        sc.name + "  ( " + sc.shortcut + " )";
                } else {
                    return true;
                }
            }
        }
    }

    return hasConflict;
}

void ShortcutMapper::updateButtonStates() {
    bool hasRows = _grid->rowCount() > 0;
    bool hasSelection = _grid->currentRow() >= 0;

    _modifyButton->setEnabled(hasRows);
    _clearButton->setEnabled(hasRows && _currentState != GridState::STATE_SCINTILLA);

    // Only macros and user commands can be deleted
    bool canDelete = (_currentState == GridState::STATE_MACRO ||
                      _currentState == GridState::STATE_USER) && hasSelection;
    _deleteButton->setEnabled(canDelete);

    _clearAllButton->setEnabled(hasRows);
}

void ShortcutMapper::saveViewState() {
    int stateIdx = static_cast<int>(_currentState);
    if (stateIdx >= 0 && stateIdx < _nbTab) {
        _lastHomeRow[stateIdx] = _grid->verticalScrollBar() ? _grid->verticalScrollBar()->value() : 0;
        _lastCursorRow[stateIdx] = _grid->currentRow();
    }
}

void ShortcutMapper::restoreViewState() {
    int stateIdx = static_cast<int>(_currentState);
    if (stateIdx >= 0 && stateIdx < _nbTab) {
        if (_grid->verticalScrollBar()) {
            _grid->verticalScrollBar()->setValue(_lastHomeRow[stateIdx]);
        }
        if (_lastCursorRow[stateIdx] >= 0 && _lastCursorRow[stateIdx] < _grid->rowCount()) {
            _grid->selectRow(_lastCursorRow[stateIdx]);
        }
    }
}

void ShortcutMapper::showConflictInfo(const ShortcutData& data) {
    if (!data.isEnabled) {
        _conflictLabel->setText(_conflictInfoOk);
        return;
    }

    QString conflictInfo;
    if (findKeyConflicts(data.keyCombo, data.originalIndex, &conflictInfo)) {
        _conflictLabel->setText(conflictInfo);
        _conflictLabel->setStyleSheet("QLabel { background-color: #FFE4E1; }");
    } else {
        _conflictLabel->setText(_conflictInfoOk);
        _conflictLabel->setStyleSheet(QString());
    }
}

void ShortcutMapper::clearConflictInfo() {
    _conflictLabel->setText(_conflictInfoOk);
    _conflictLabel->setStyleSheet(QString());
}

QString ShortcutMapper::keyComboToString(const KeyCombo& combo) const {
    if (combo._key == 0) {
        return QString();
    }

    QStringList parts;
    if (combo._isCtrl) parts << "Ctrl";
    if (combo._isAlt) parts << "Alt";
    if (combo._isShift) parts << "Shift";

    // Convert key code to string
    QString keyStr;
    if (combo._key >= 'A' && combo._key <= 'Z') {
        keyStr = QString(QChar::fromLatin1(combo._key));
    } else if (combo._key >= '0' && combo._key <= '9') {
        keyStr = QString(QChar::fromLatin1(combo._key));
    } else if (combo._key >= VK_F1 && combo._key <= VK_F24) {
        keyStr = QString("F%1").arg(combo._key - VK_F1 + 1);
    } else {
        switch (combo._key) {
            case VK_SPACE: keyStr = QString("Space"); break;
            case VK_RETURN: keyStr = QString("Enter"); break;
            case VK_ESCAPE: keyStr = QString("Esc"); break;
            case VK_TAB: keyStr = QString("Tab"); break;
            case VK_BACK: keyStr = QString("Backspace"); break;
            case VK_DELETE: keyStr = QString("Delete"); break;
            case VK_INSERT: keyStr = QString("Insert"); break;
            case VK_HOME: keyStr = QString("Home"); break;
            case VK_END: keyStr = QString("End"); break;
            case VK_PRIOR: keyStr = QString("PageUp"); break;
            case VK_NEXT: keyStr = QString("PageDown"); break;
            case VK_LEFT: keyStr = QString("Left"); break;
            case VK_RIGHT: keyStr = QString("Right"); break;
            case VK_UP: keyStr = QString("Up"); break;
            case VK_DOWN: keyStr = QString("Down"); break;
            default: keyStr = QString("Key%1").arg(combo._key); break;
        }
    }

    parts << keyStr;
    return parts.join("+");
}

} // namespace ShortcutMapper

} // namespace QtControls
