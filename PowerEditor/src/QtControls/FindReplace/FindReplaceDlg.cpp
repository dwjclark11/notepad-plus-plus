// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FindReplaceDlg.h"
#include "../../Platform/Settings.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QMenu>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDirIterator>
#include <QtCore/QTextStream>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtGui/QKeyEvent>
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QHeaderView>

namespace NppFindReplace {

// Static members
FindOptions FindReplaceDlg::options;
FindOptions* FindReplaceDlg::env = &FindReplaceDlg::options;

FindReplaceDlg::FindReplaceDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Find / Replace"));
    resize(500, 400);
}

FindReplaceDlg::~FindReplaceDlg() {
    saveHistory();
}

void FindReplaceDlg::init(ScintillaEditView** ppEditView) {
    _ppEditView = ppEditView;
    loadHistory();
    setupUI();
    connectSignals();
    updateControlStates();
    _isCreated = true;
}

void FindReplaceDlg::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Tab widget
    _tabWidget = new QTabWidget(this);
    mainLayout->addWidget(_tabWidget);

    // Create tabs
    createFindTab();
    createReplaceTab();
    createFindInFilesTab();
    createFindInProjectsTab();
    createMarkTab();

    // Status label
    _statusLabel = new QLabel(this);
    _statusLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    _statusLabel->setMinimumHeight(24);
    mainLayout->addWidget(_statusLabel);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(_closeButton);

    mainLayout->addLayout(buttonLayout);
}

void FindReplaceDlg::createFindTab() {
    auto* findWidget = new QWidget();
    auto* layout = new QGridLayout(findWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    _findWhatCombo = new QComboBox(this);
    _findWhatCombo->setEditable(true);
    _findWhatCombo->setMaxCount(20);
    _findWhatEdit = _findWhatCombo->lineEdit();
    layout->addWidget(_findWhatCombo, 0, 1, 1, 2);

    // Swap button
    _swapButton = new QPushButton(tr("⇄"), this);
    _swapButton->setToolTip(tr("Swap Find and Replace text"));
    layout->addWidget(_swapButton, 0, 3);

    // Match options
    _matchWholeWordCheck = new QCheckBox(tr("Match &whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 1, 0, 1, 2);

    _matchCaseCheck = new QCheckBox(tr("Match &case"), this);
    layout->addWidget(_matchCaseCheck, 2, 0, 1, 2);

    _wrapAroundCheck = new QCheckBox(tr("Wrap ar&ound"), this);
    _wrapAroundCheck->setChecked(true);
    layout->addWidget(_wrapAroundCheck, 3, 0, 1, 2);

    // Direction group
    _directionGroup = new QGroupBox(tr("Direction"), this);
    auto* dirLayout = new QHBoxLayout(_directionGroup);
    _dirUpRadio = new QRadioButton(tr("&Up"), this);
    _dirDownRadio = new QRadioButton(tr("&Down"), this);
    _dirDownRadio->setChecked(true);
    dirLayout->addWidget(_dirUpRadio);
    dirLayout->addWidget(_dirDownRadio);
    dirLayout->addStretch();
    layout->addWidget(_directionGroup, 1, 2, 3, 2);

    // Search mode group
    _searchModeGroup = new QGroupBox(tr("Search Mode"), this);
    auto* modeLayout = new QVBoxLayout(_searchModeGroup);
    _modeNormalRadio = new QRadioButton(tr("&Normal"), this);
    _modeExtendedRadio = new QRadioButton(tr("E&xtended (\\n, \\r, \\t, \\0, \\x...)"), this);
    _modeRegexRadio = new QRadioButton(tr("Re&gular expression"), this);
    _modeNormalRadio->setChecked(true);
    modeLayout->addWidget(_modeNormalRadio);
    modeLayout->addWidget(_modeExtendedRadio);
    modeLayout->addWidget(_modeRegexRadio);

    _dotMatchesNewlineCheck = new QCheckBox(tr(". matches &newline"), this);
    _dotMatchesNewlineCheck->setEnabled(false);
    modeLayout->addWidget(_dotMatchesNewlineCheck);
    modeLayout->addStretch();
    layout->addWidget(_searchModeGroup, 4, 0, 4, 2);

    // Scope
    _inSelectionCheck = new QCheckBox(tr("In select&ion"), this);
    layout->addWidget(_inSelectionCheck, 8, 0, 1, 2);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _findNextButton = new QPushButton(tr("Find &Next"), this);
    _findNextButton->setDefault(true);
    actionLayout->addWidget(_findNextButton);

    _findPreviousButton = new QPushButton(tr("Find &Previous"), this);
    actionLayout->addWidget(_findPreviousButton);

    _countButton = new QPushButton(tr("&Count"), this);
    actionLayout->addWidget(_countButton);

    _findAllButton = new QPushButton(tr("Find All in Current &Document"), this);
    actionLayout->addWidget(_findAllButton);

    _findAllInOpenDocsButton = new QPushButton(tr("Find All in All &Open Documents"), this);
    actionLayout->addWidget(_findAllInOpenDocsButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 4, 2, 5, 2);

    layout->setRowStretch(9, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(findWidget, tr("Find"));
}

void FindReplaceDlg::createReplaceTab() {
    auto* replaceWidget = new QWidget();
    auto* layout = new QGridLayout(replaceWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1);

    // Replace with
    layout->addWidget(new QLabel(tr("Replace with:")), 1, 0);
    _replaceWithCombo = new QComboBox(this);
    _replaceWithCombo->setEditable(true);
    _replaceWithCombo->setMaxCount(20);
    _replaceWithEdit = _replaceWithCombo->lineEdit();
    layout->addWidget(_replaceWithCombo, 1, 1);

    // Use local variables for the Replace tab's option checkboxes.
    // Previously these overwrote the member pointers set by createFindTab(),
    // so getCurrentOptions() would always read from the Replace tab's
    // widgets regardless of which tab was active.
    auto* matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(matchWholeWordCheck, 2, 0, 1, 2);

    auto* matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(matchCaseCheck, 3, 0, 1, 2);

    auto* wrapAroundCheck = new QCheckBox(tr("Wrap around"), this);
    wrapAroundCheck->setChecked(true);
    layout->addWidget(wrapAroundCheck, 4, 0, 1, 2);

    auto* inSelectionCheck = new QCheckBox(tr("In selection"), this);
    layout->addWidget(inSelectionCheck, 5, 0, 1, 2);

    // Sync Replace tab checkboxes with Find tab checkboxes
    if (_matchWholeWordCheck)
    {
        connect(_matchWholeWordCheck, &QCheckBox::toggled,
                matchWholeWordCheck, &QCheckBox::setChecked);
        connect(matchWholeWordCheck, &QCheckBox::toggled,
                _matchWholeWordCheck, &QCheckBox::setChecked);
    }
    if (_matchCaseCheck)
    {
        connect(_matchCaseCheck, &QCheckBox::toggled,
                matchCaseCheck, &QCheckBox::setChecked);
        connect(matchCaseCheck, &QCheckBox::toggled,
                _matchCaseCheck, &QCheckBox::setChecked);
    }
    if (_wrapAroundCheck)
    {
        connect(_wrapAroundCheck, &QCheckBox::toggled,
                wrapAroundCheck, &QCheckBox::setChecked);
        connect(wrapAroundCheck, &QCheckBox::toggled,
                _wrapAroundCheck, &QCheckBox::setChecked);
    }
    if (_inSelectionCheck)
    {
        connect(_inSelectionCheck, &QCheckBox::toggled,
                inSelectionCheck, &QCheckBox::setChecked);
        connect(inSelectionCheck, &QCheckBox::toggled,
                _inSelectionCheck, &QCheckBox::setChecked);
    }

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    auto* findNextButton = new QPushButton(tr("Find Next"), this);
    findNextButton->setDefault(true);
    actionLayout->addWidget(findNextButton);
    connect(findNextButton, &QPushButton::clicked,
            this, &FindReplaceDlg::onFindNextClicked);

    _replaceButton = new QPushButton(tr("&Replace"), this);
    actionLayout->addWidget(_replaceButton);

    _replaceAllButton = new QPushButton(tr("Replace &All"), this);
    actionLayout->addWidget(_replaceAllButton);

    _replaceAllInOpenDocsButton = new QPushButton(tr("Replace All in &Open Documents"), this);
    actionLayout->addWidget(_replaceAllInOpenDocsButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 2, 6, 1);

    layout->setRowStretch(6, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(replaceWidget, tr("Replace"));
}

void FindReplaceDlg::createFindInFilesTab() {
    auto* fifWidget = new QWidget();
    auto* layout = new QGridLayout(fifWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1);

    // Filters
    layout->addWidget(new QLabel(tr("Filters:")), 1, 0);
    _filtersCombo = new QComboBox(this);
    _filtersCombo->setEditable(true);
    _filtersCombo->setToolTip(tr("e.g. *.cpp *.h"));
    layout->addWidget(_filtersCombo, 1, 1);

    // Directory
    layout->addWidget(new QLabel(tr("Directory:")), 2, 0);
    _directoryCombo = new QComboBox(this);
    _directoryCombo->setEditable(true);
    layout->addWidget(_directoryCombo, 2, 1);

    auto* dirButtonLayout = new QVBoxLayout();
    _browseButton = new QPushButton(tr("..."), this);
    _browseButton->setToolTip(tr("Browse..."));
    dirButtonLayout->addWidget(_browseButton);

    _dirFromActiveDocButton = new QPushButton(tr("⬇"), this);
    _dirFromActiveDocButton->setToolTip(tr("Get directory from active document"));
    dirButtonLayout->addWidget(_dirFromActiveDocButton);
    dirButtonLayout->addStretch();
    layout->addLayout(dirButtonLayout, 2, 2);

    // Options
    _recursiveCheck = new QCheckBox(tr("Re&cursive"), this);
    _recursiveCheck->setChecked(true);
    layout->addWidget(_recursiveCheck, 3, 0);

    _hiddenDirCheck = new QCheckBox(tr("In &hidden folders"), this);
    layout->addWidget(_hiddenDirCheck, 3, 1);

    _matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(_matchCaseCheck, 4, 0);

    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 4, 1);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _findAllButton = new QPushButton(tr("Find All"), this);
    _findAllButton->setDefault(true);
    actionLayout->addWidget(_findAllButton);

    _replaceAllButton = new QPushButton(tr("Replace in Files"), this);
    actionLayout->addWidget(_replaceAllButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 3, 5, 1);

    layout->setRowStretch(5, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(fifWidget, tr("Find in Files"));
}

void FindReplaceDlg::createFindInProjectsTab()
{
    auto* fipWidget = new QWidget();
    auto* layout = new QGridLayout(fipWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1);

    // Filters
    layout->addWidget(new QLabel(tr("Filters:")), 1, 0);
    auto* filtersCombo = new QComboBox(this);
    filtersCombo->setEditable(true);
    filtersCombo->setToolTip(tr("e.g. *.cpp *.h"));
    layout->addWidget(filtersCombo, 1, 1);

    // Project panel checkboxes
    auto* projectGroup = new QGroupBox(tr("Project Panels"), this);
    auto* projLayout = new QVBoxLayout(projectGroup);
    _projectPanel1Check = new QCheckBox(tr("Project Panel &1"), this);
    _projectPanel1Check->setChecked(true);
    projLayout->addWidget(_projectPanel1Check);
    _projectPanel2Check = new QCheckBox(tr("Project Panel &2"), this);
    projLayout->addWidget(_projectPanel2Check);
    _projectPanel3Check = new QCheckBox(tr("Project Panel &3"), this);
    projLayout->addWidget(_projectPanel3Check);
    layout->addWidget(projectGroup, 2, 0, 1, 2);

    // Match options
    auto* matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(matchCaseCheck, 3, 0);
    if (_matchCaseCheck)
    {
        connect(_matchCaseCheck, &QCheckBox::toggled,
                matchCaseCheck, &QCheckBox::setChecked);
        connect(matchCaseCheck, &QCheckBox::toggled,
                _matchCaseCheck, &QCheckBox::setChecked);
    }

    auto* matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(matchWholeWordCheck, 3, 1);
    if (_matchWholeWordCheck)
    {
        connect(_matchWholeWordCheck, &QCheckBox::toggled,
                matchWholeWordCheck, &QCheckBox::setChecked);
        connect(matchWholeWordCheck, &QCheckBox::toggled,
                _matchWholeWordCheck, &QCheckBox::setChecked);
    }

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    auto* findAllButton = new QPushButton(tr("Find All"), this);
    findAllButton->setDefault(true);
    actionLayout->addWidget(findAllButton);
    connect(findAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindAllClicked);

    auto* replaceInProjectsButton = new QPushButton(tr("Replace in Projects"), this);
    actionLayout->addWidget(replaceInProjectsButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 2, 4, 1);

    layout->setRowStretch(4, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(fipWidget, tr("Find in Projects"));
}

void FindReplaceDlg::createMarkTab() {
    auto* markWidget = new QWidget();
    auto* layout = new QGridLayout(markWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1, 1, 2);

    // Options
    _bookmarkLineCheck = new QCheckBox(tr("Bookmark &line"), this);
    _bookmarkLineCheck->setChecked(true);
    layout->addWidget(_bookmarkLineCheck, 1, 0);

    _purgeCheck = new QCheckBox(tr("Purge for each search"), this);
    layout->addWidget(_purgeCheck, 1, 1);

    _matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(_matchCaseCheck, 2, 0);

    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 2, 1);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _markAllButton = new QPushButton(tr("Mark &All"), this);
    _markAllButton->setDefault(true);
    actionLayout->addWidget(_markAllButton);

    _clearMarksButton = new QPushButton(tr("Clear &all marks"), this);
    actionLayout->addWidget(_clearMarksButton);

    _copyMarkedTextButton = new QPushButton(tr("&Copy Marked Text"), this);
    actionLayout->addWidget(_copyMarkedTextButton);

    _findAllButton = new QPushButton(tr("Find All"), this);
    actionLayout->addWidget(_findAllButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 3, 5, 1);

    layout->setRowStretch(5, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(markWidget, tr("Mark"));
}

void FindReplaceDlg::connectSignals() {
    // Tab changes
    connect(_tabWidget, &QTabWidget::currentChanged, this, &FindReplaceDlg::onTabChanged);

    // Buttons
    connect(_findNextButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindNextClicked);
    connect(_findPreviousButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindPreviousClicked);
    connect(_replaceButton, &QPushButton::clicked, this, &FindReplaceDlg::onReplaceClicked);
    connect(_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onReplaceAllClicked);
    connect(_replaceAllInOpenDocsButton, &QPushButton::clicked, this, &FindReplaceDlg::onReplaceAllInOpenDocsClicked);
    connect(_findAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindAllClicked);
    connect(_findAllInOpenDocsButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindAllInOpenDocsClicked);
    connect(_countButton, &QPushButton::clicked, this, &FindReplaceDlg::onCountClicked);
    connect(_markAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onMarkAllClicked);
    connect(_clearMarksButton, &QPushButton::clicked, this, &FindReplaceDlg::onClearMarksClicked);
    connect(_closeButton, &QPushButton::clicked, this, &FindReplaceDlg::onCloseClicked);
    connect(_swapButton, &QPushButton::clicked, this, &FindReplaceDlg::onSwapFindReplaceClicked);

    // Browse
    connect(_browseButton, &QPushButton::clicked, this, &FindReplaceDlg::onBrowseDirectoryClicked);
    connect(_dirFromActiveDocButton, &QPushButton::clicked, this, &FindReplaceDlg::onDirFromActiveDocClicked);

    // Search mode
    connect(_modeRegexRadio, &QRadioButton::toggled, _dotMatchesNewlineCheck, &QWidget::setEnabled);

    // Text changes
    connect(_findWhatEdit, &QLineEdit::textChanged, this, &FindReplaceDlg::onSearchTextChanged);
    connect(_findWhatEdit, &QLineEdit::editingFinished, this, &FindReplaceDlg::onSearchTextEdited);
}

void FindReplaceDlg::showDialog(FindDialogType type) {
    // Guard against use before init() has been called
    if (!_tabWidget)
    {
        return;
    }

    _currentType = type;
    _tabWidget->setCurrentIndex(static_cast<int>(type));
    updateControlStates();
    show();
    raise();
    activateWindow();

    if (_findWhatEdit) {
        _findWhatEdit->setFocus();
        _findWhatEdit->selectAll();
    }
}

void FindReplaceDlg::setSearchText(const QString& text) {
    if (_findWhatEdit) {
        _findWhatEdit->setText(text);
    }
}

QString FindReplaceDlg::getSearchText() const {
    return _findWhatEdit ? _findWhatEdit->text() : QString();
}

QString FindReplaceDlg::getReplaceText() const {
    return _replaceWithEdit ? _replaceWithEdit->text() : QString();
}

FindOptions FindReplaceDlg::getCurrentOptions() const {
    FindOptions opt;

    // Search text
    opt.str2Search = getSearchText();
    opt.str4Replace = getReplaceText();

    // Match options
    opt.isMatchCase = _matchCaseCheck && _matchCaseCheck->isChecked();
    opt.isWholeWord = _matchWholeWordCheck && _matchWholeWordCheck->isChecked();
    opt.isWrapAround = _wrapAroundCheck && _wrapAroundCheck->isChecked();

    // Direction
    if (_dirUpRadio && _dirDownRadio) {
        opt.direction = _dirDownRadio->isChecked() ? SearchDirection::Down : SearchDirection::Up;
    }

    // Search type
    if (_modeNormalRadio && _modeNormalRadio->isChecked()) {
        opt.searchType = SearchType::Normal;
    } else if (_modeExtendedRadio && _modeExtendedRadio->isChecked()) {
        opt.searchType = SearchType::Extended;
    } else if (_modeRegexRadio && _modeRegexRadio->isChecked()) {
        opt.searchType = SearchType::Regex;
    }

    // Other options
    opt.isInSelection = _inSelectionCheck && _inSelectionCheck->isChecked();
    opt.dotMatchesNewline = _dotMatchesNewlineCheck && _dotMatchesNewlineCheck->isChecked();
    opt.isRecursive = _recursiveCheck && _recursiveCheck->isChecked();
    opt.isInHiddenDir = _hiddenDirCheck && _hiddenDirCheck->isChecked();
    opt.doPurge = _purgeCheck && _purgeCheck->isChecked();
    opt.doMarkLine = _bookmarkLineCheck && _bookmarkLineCheck->isChecked();

    // Filters and directory
    if (_filtersCombo) {
        opt.filters = _filtersCombo->currentText();
    }
    if (_directoryCombo) {
        opt.directory = _directoryCombo->currentText();
    }

    return opt;
}

void FindReplaceDlg::setOptions(const FindOptions& options) {
    if (_matchCaseCheck) _matchCaseCheck->setChecked(options.isMatchCase);
    if (_matchWholeWordCheck) _matchWholeWordCheck->setChecked(options.isWholeWord);
    if (_wrapAroundCheck) _wrapAroundCheck->setChecked(options.isWrapAround);
    if (_inSelectionCheck) _inSelectionCheck->setChecked(options.isInSelection);
    if (_dotMatchesNewlineCheck) _dotMatchesNewlineCheck->setChecked(options.dotMatchesNewline);
    if (_recursiveCheck) _recursiveCheck->setChecked(options.isRecursive);
    if (_hiddenDirCheck) _hiddenDirCheck->setChecked(options.isInHiddenDir);

    // Direction
    if (_dirUpRadio && _dirDownRadio) {
        if (options.direction == SearchDirection::Up) {
            _dirUpRadio->setChecked(true);
        } else {
            _dirDownRadio->setChecked(true);
        }
    }

    // Search type
    if (_modeNormalRadio) _modeNormalRadio->setChecked(options.searchType == SearchType::Normal);
    if (_modeExtendedRadio) _modeExtendedRadio->setChecked(options.searchType == SearchType::Extended);
    if (_modeRegexRadio) _modeRegexRadio->setChecked(options.searchType == SearchType::Regex);
}

bool FindReplaceDlg::findNext() {
    return processFindNext(getSearchText(), getCurrentOptions());
}

bool FindReplaceDlg::findPrevious() {
    auto options = getCurrentOptions();
    options.direction = SearchDirection::Up;
    return processFindNext(getSearchText(), options);
}

bool FindReplaceDlg::replace() {
    return processReplace(getSearchText(), getReplaceText(), getCurrentOptions());
}

bool FindReplaceDlg::replaceAll() {
    int count = processReplaceAll(getSearchText(), getReplaceText(), getCurrentOptions());
    return count > 0;
}

void FindReplaceDlg::onFindNextClicked() {
    if (findNext()) {
        setStatusMessage(tr("Match found"), FindStatus::Found);
    } else {
        setStatusMessage(tr("Match not found"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onFindPreviousClicked() {
    if (findPrevious()) {
        setStatusMessage(tr("Match found"), FindStatus::Found);
    } else {
        setStatusMessage(tr("Match not found"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onReplaceClicked() {
    if (replace()) {
        setStatusMessage(tr("Replaced"), FindStatus::Found);
    } else {
        setStatusMessage(tr("No match found to replace"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onReplaceAllClicked() {
    int count = processReplaceAll(getSearchText(), getReplaceText(), getCurrentOptions());
    if (count > 0) {
        setStatusMessage(tr("Replaced %1 occurrence(s)").arg(count), FindStatus::Found);
    } else {
        setStatusMessage(tr("No occurrences found"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onReplaceAllInOpenDocsClicked() {
    if (!_getOpenBuffersCb || !_activateBufferCb)
    {
        // Fallback to current document only
        int count = processReplaceAll(getSearchText(), getReplaceText(), getCurrentOptions());
        if (count > 0)
        {
            setStatusMessage(tr("Replaced %1 occurrence(s) in current document").arg(count), FindStatus::Found);
        }
        else
        {
            setStatusMessage(tr("No occurrences found"), FindStatus::NotFound);
        }
        return;
    }

    QString findText = getSearchText();
    QString replaceText = getReplaceText();
    FindOptions opts = getCurrentOptions();

    if (findText.isEmpty())
    {
        return;
    }

    std::vector<BufferInfo> buffers = _getOpenBuffersCb();
    void* originalBuffer = nullptr;
    if (_ppEditView && *_ppEditView)
    {
        originalBuffer = (*_ppEditView)->getCurrentBufferID();
    }

    int totalCount = 0;
    int docsWithReplacements = 0;

    for (const auto& bufInfo : buffers)
    {
        if (!_activateBufferCb(bufInfo.first))
        {
            continue;
        }

        int count = processReplaceAll(findText, replaceText, opts);
        totalCount += count;
        if (count > 0)
        {
            docsWithReplacements++;
        }
    }

    if (originalBuffer)
    {
        _activateBufferCb(originalBuffer);
    }

    if (totalCount > 0)
    {
        setStatusMessage(tr("Replaced %1 occurrence(s) in %2 document(s)")
                         .arg(totalCount).arg(docsWithReplacements), FindStatus::Found);
    }
    else
    {
        setStatusMessage(tr("No occurrences found in any open document"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onFindAllClicked() {
    if (_currentType == FindDialogType::FindInFiles) {
        int hits = processFindInFiles(getSearchText(), getCurrentOptions());
        if (hits > 0)
            setStatusMessage(tr("Found %1 hit(s)").arg(hits), FindStatus::Found);
        else
            setStatusMessage(tr("No matches found"), FindStatus::NotFound);
    } else if (_currentType == FindDialogType::FindInProjects) {
        findInProjects();
    } else {
        findAllInCurrentDoc();
    }
}

void FindReplaceDlg::onFindAllInOpenDocsClicked()
{
    findAllInOpenDocs();
}

void FindReplaceDlg::onCountClicked() {
    int count = countMatches();
    setStatusMessage(tr("Count: %1 match(es)").arg(count), FindStatus::Message);
}

void FindReplaceDlg::onMarkAllClicked() {
    int count = markAll();
    setStatusMessage(tr("Marked %1 occurrence(s)").arg(count), FindStatus::Found);
}

void FindReplaceDlg::onClearMarksClicked() {
    clearMarks();
    setStatusMessage(tr("Marks cleared"), FindStatus::Message);
}

void FindReplaceDlg::onCloseClicked() {
    hide();
}

void FindReplaceDlg::onSwapFindReplaceClicked() {
    QString findText = getSearchText();
    QString replaceText = getReplaceText();
    setSearchText(replaceText);
    if (_replaceWithEdit) {
        _replaceWithEdit->setText(findText);
    }
}

void FindReplaceDlg::onSearchTextChanged(const QString& text) {
    (void)text;
    // Update button states based on whether search text is empty
    bool hasText = !_findWhatEdit->text().isEmpty();
    if (_findNextButton) _findNextButton->setEnabled(hasText);
    if (_replaceButton) _replaceButton->setEnabled(hasText);
    if (_replaceAllButton) _replaceAllButton->setEnabled(hasText);
}

void FindReplaceDlg::onSearchTextEdited() {
    addToHistory(getSearchText(), _findHistory);
    updateComboHistory(_findWhatCombo, _findHistory);
}

void FindReplaceDlg::onTabChanged(int index) {
    _currentType = static_cast<FindDialogType>(index);
    updateControlStates();
}

void FindReplaceDlg::onBrowseDirectoryClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                    _directoryCombo ? _directoryCombo->currentText() : QString());
    if (!dir.isEmpty() && _directoryCombo) {
        _directoryCombo->setCurrentText(dir);
    }
}

void FindReplaceDlg::onDirFromActiveDocClicked() {
    if (_getActiveFilePathCb)
    {
        QString filePath = _getActiveFilePathCb();
        if (!filePath.isEmpty())
        {
            QFileInfo fi(filePath);
            QString dir = fi.absolutePath();
            if (_directoryCombo)
            {
                _directoryCombo->setCurrentText(dir);
            }
            return;
        }
    }
    QMessageBox::information(this, tr("Find"), tr("No active document with a file path"));
}

void FindReplaceDlg::updateControlStates() {
    // Enable/disable controls based on current tab
    switch (_currentType) {
        case FindDialogType::Find:
            enableFindControls(true);
            break;
        case FindDialogType::Replace:
            enableReplaceControls(true);
            break;
        case FindDialogType::FindInFiles:
            enableFindInFilesControls(true);
            break;
        case FindDialogType::FindInProjects:
            enableFindInFilesControls(true);
            break;
        case FindDialogType::Mark:
            enableMarkControls(true);
            break;
        default:
            break;
    }
}

void FindReplaceDlg::enableFindControls(bool enable) {
    (void)enable;
    // Update UI state for find tab
}

void FindReplaceDlg::enableReplaceControls(bool enable) {
    (void)enable;
    // Update UI state for replace tab
}

void FindReplaceDlg::enableFindInFilesControls(bool enable) {
    (void)enable;
    // Update UI state for find in files tab
}

void FindReplaceDlg::enableMarkControls(bool enable) {
    (void)enable;
    // Update UI state for mark tab
}

void FindReplaceDlg::setStatusMessage(const QString& msg, FindStatus status) {
    _statusMessage = msg;
    _findStatus = status;
    if (_statusLabel) {
        _statusLabel->setText(msg);

        // Set color based on status
        QPalette palette = _statusLabel->palette();
        switch (status) {
            case FindStatus::Found:
                palette.setColor(QPalette::WindowText, Qt::darkGreen);
                break;
            case FindStatus::NotFound:
            case FindStatus::Warning:
                palette.setColor(QPalette::WindowText, Qt::red);
                break;
            default:
                palette.setColor(QPalette::WindowText, palette.color(QPalette::Text));
                break;
        }
        _statusLabel->setPalette(palette);
    }
}

void FindReplaceDlg::loadHistory() {
    QSettings settings;
    settings.beginGroup("FindReplace");

    int findCount = settings.beginReadArray("FindHistory");
    for (int i = 0; i < findCount; ++i)
    {
        settings.setArrayIndex(i);
        QString text = settings.value("text").toString();
        if (!text.isEmpty())
        {
            _findHistory.push_back(text);
        }
    }
    settings.endArray();

    int replaceCount = settings.beginReadArray("ReplaceHistory");
    for (int i = 0; i < replaceCount; ++i)
    {
        settings.setArrayIndex(i);
        QString text = settings.value("text").toString();
        if (!text.isEmpty())
        {
            _replaceHistory.push_back(text);
        }
    }
    settings.endArray();

    int filterCount = settings.beginReadArray("FilterHistory");
    for (int i = 0; i < filterCount; ++i)
    {
        settings.setArrayIndex(i);
        QString text = settings.value("text").toString();
        if (!text.isEmpty())
        {
            _filterHistory.push_back(text);
        }
    }
    settings.endArray();

    int dirCount = settings.beginReadArray("DirectoryHistory");
    for (int i = 0; i < dirCount; ++i)
    {
        settings.setArrayIndex(i);
        QString text = settings.value("text").toString();
        if (!text.isEmpty())
        {
            _directoryHistory.push_back(text);
        }
    }
    settings.endArray();

    settings.endGroup();

    // Populate combo boxes
    updateComboHistory(_findWhatCombo, _findHistory);
    updateComboHistory(_replaceWithCombo, _replaceHistory);
    updateComboHistory(_filtersCombo, _filterHistory);
    updateComboHistory(_directoryCombo, _directoryHistory);
}

void FindReplaceDlg::saveHistory() {
    // Capture current text from combos before saving
    saveComboHistory(_findWhatCombo, _findHistory, 20);
    saveComboHistory(_replaceWithCombo, _replaceHistory, 20);
    saveComboHistory(_filtersCombo, _filterHistory, 20);
    saveComboHistory(_directoryCombo, _directoryHistory, 20);

    QSettings settings;
    settings.beginGroup("FindReplace");

    settings.beginWriteArray("FindHistory", static_cast<int>(_findHistory.size()));
    for (size_t i = 0; i < _findHistory.size(); ++i)
    {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("text", _findHistory[i]);
    }
    settings.endArray();

    settings.beginWriteArray("ReplaceHistory", static_cast<int>(_replaceHistory.size()));
    for (size_t i = 0; i < _replaceHistory.size(); ++i)
    {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("text", _replaceHistory[i]);
    }
    settings.endArray();

    settings.beginWriteArray("FilterHistory", static_cast<int>(_filterHistory.size()));
    for (size_t i = 0; i < _filterHistory.size(); ++i)
    {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("text", _filterHistory[i]);
    }
    settings.endArray();

    settings.beginWriteArray("DirectoryHistory", static_cast<int>(_directoryHistory.size()));
    for (size_t i = 0; i < _directoryHistory.size(); ++i)
    {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("text", _directoryHistory[i]);
    }
    settings.endArray();

    settings.endGroup();
}

void FindReplaceDlg::addToHistory(const QString& text, std::vector<QString>& history) {
    if (text.isEmpty()) return;

    // Remove if already exists
    auto it = std::find(history.begin(), history.end(), text);
    if (it != history.end()) {
        history.erase(it);
    }

    // Add to front
    history.insert(history.begin(), text);

    // Limit size
    if (history.size() > 20) {
        history.resize(20);
    }
}

void FindReplaceDlg::updateComboHistory(QComboBox* combo, const std::vector<QString>& history) {
    if (!combo) return;

    QString currentText = combo->currentText();
    combo->clear();

    for (const auto& item : history) {
        combo->addItem(item);
    }

    combo->setCurrentText(currentText);
}

void FindReplaceDlg::saveComboHistory(QComboBox* combo, std::vector<QString>& history, int maxCount) {
    if (!combo) return;
    QString text = combo->currentText();
    if (text.isEmpty()) return;

    // Remove if already exists
    auto it = std::find(history.begin(), history.end(), text);
    if (it != history.end()) {
        history.erase(it);
    }

    // Add to front
    history.insert(history.begin(), text);

    // Limit size
    if (history.size() > static_cast<size_t>(maxCount)) {
        history.resize(maxCount);
    }
}

bool FindReplaceDlg::processFindNext(const QString& text, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;
    if (!view->isVisible()) return false;

    // Build search flags
    int flags = buildSearchFlags(options);

    // Get current position
    intptr_t currentPos = view->execute(SCI_GETCURRENTPOS);
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    // Set search range based on direction
    intptr_t startPos, endPos;
    if (options.direction == SearchDirection::Down) {
        startPos = currentPos;
        endPos = docLength;
    } else {
        startPos = currentPos;
        endPos = 0;
    }

    // Apply extended string conversion if needed
    QString processedText = text;
    if (options.searchType == SearchType::Extended)
    {
        processedText = convertExtendedString(text);
    }

    // Convert text to proper encoding
    QByteArray searchText = processedText.toUtf8();

    // Set target
    view->execute(SCI_SETSEARCHFLAGS, flags);
    view->execute(SCI_SETTARGETSTART, startPos);
    view->execute(SCI_SETTARGETEND, endPos);

    // Perform search
    intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                    reinterpret_cast<LPARAM>(searchText.constData()));

    if (result == -1) {
        // Not found - try wrap around if enabled
        if (options.isWrapAround) {
            if (options.direction == SearchDirection::Down) {
                startPos = 0;
                endPos = currentPos;
            } else {
                startPos = docLength;
                endPos = currentPos;
            }

            view->execute(SCI_SETTARGETSTART, startPos);
            view->execute(SCI_SETTARGETEND, endPos);

            result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                   reinterpret_cast<LPARAM>(searchText.constData()));
        }
    }

    if (result != -1) {
        // Found - select and show
        intptr_t matchEnd = view->execute(SCI_GETTARGETEND);
        view->execute(SCI_SETSEL, result, matchEnd);
        displaySectionCentered(result, matchEnd, options.direction == SearchDirection::Down);
        return true;
    }

    return false;
}

bool FindReplaceDlg::processReplace(const QString& findText, const QString& replaceText, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;

    // Check if there's a selection that matches
    intptr_t selStart = view->execute(SCI_GETSELECTIONSTART);
    intptr_t selEnd = view->execute(SCI_GETSELECTIONEND);

    if (selStart == selEnd) {
        // No selection - find first
        if (!processFindNext(findText, options)) {
            return false;
        }
        selStart = view->execute(SCI_GETSELECTIONSTART);
        selEnd = view->execute(SCI_GETSELECTIONEND);
    }

    // Apply extended string conversion to replacement text if needed
    QString processedReplace = replaceText;
    if (options.searchType == SearchType::Extended)
    {
        processedReplace = convertExtendedString(replaceText);
    }

    // Replace the selection
    QByteArray replacement = processedReplace.toUtf8();
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(replacement.constData()));

    // Find next occurrence
    processFindNext(findText, options);

    return true;
}

int FindReplaceDlg::processReplaceAll(const QString& findText, const QString& replaceText, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;
    int flags = buildSearchFlags(options);

    // Apply extended string conversion if needed
    QString processedFind = findText;
    QString processedReplace = replaceText;
    if (options.searchType == SearchType::Extended)
    {
        processedFind = convertExtendedString(findText);
        processedReplace = convertExtendedString(replaceText);
    }

    QByteArray searchText = processedFind.toUtf8();
    QByteArray replacement = processedReplace.toUtf8();

    view->execute(SCI_BEGINUNDOACTION);

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));

        if (result == -1) {
            break;
        }

        // Replace
        view->execute(SCI_REPLACETARGET, replacement.length(),
                      reinterpret_cast<LPARAM>(replacement.constData()));

        count++;

        // Update position for next search
        startPos = result + replacement.length();
        docLength = view->execute(SCI_GETLENGTH);
    }

    view->execute(SCI_ENDUNDOACTION);

    return count;
}

int FindReplaceDlg::countMatches() {
    if (!_ppEditView || !*_ppEditView) return 0;

    auto* view = *_ppEditView;
    auto options = getCurrentOptions();
    int flags = buildSearchFlags(options);

    QString rawSearch = getSearchText();
    if (options.searchType == SearchType::Extended)
    {
        rawSearch = convertExtendedString(rawSearch);
    }

    QByteArray searchText = rawSearch.toUtf8();

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));

        if (result == -1) {
            break;
        }

        count++;
        startPos = view->execute(SCI_GETTARGETEND);
    }

    return count;
}

int FindReplaceDlg::markAll(int styleId) {
    if (!_ppEditView || !*_ppEditView) return 0;

    auto* view = *_ppEditView;
    auto options = getCurrentOptions();
    int flags = buildSearchFlags(options);

    QByteArray searchText = getSearchText().toUtf8();
    if (searchText.isEmpty()) return 0;

    // Determine the indicator style to use
    int indicatorId = (styleId < 0) ? SCE_UNIVERSAL_FOUND_STYLE : styleId;

    // Check if bookmark line option is enabled
    bool doBookmarkLine = _bookmarkLineCheck && _bookmarkLineCheck->isChecked();

    // Check if purge option is enabled
    bool doPurge = _purgeCheck && _purgeCheck->isChecked();

    // Purge previous marks if requested
    if (doPurge) {
        view->clearIndicator(indicatorId);
        if (doBookmarkLine) {
            view->execute(SCI_MARKERDELETEALL, MARK_BOOKMARK);
        }
    }

    // Set the indicator for text highlighting
    view->execute(SCI_SETINDICATORCURRENT, indicatorId);

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));

        if (result == -1) {
            break;
        }

        intptr_t targetEnd = view->execute(SCI_GETTARGETEND);
        intptr_t matchLen = targetEnd - result;

        // Highlight the matched text using indicator
        if (matchLen > 0) {
            view->execute(SCI_INDICATORFILLRANGE, result, matchLen);
        }

        // Optionally bookmark the line
        if (doBookmarkLine) {
            intptr_t line = view->execute(SCI_LINEFROMPOSITION, result);
            view->execute(SCI_MARKERADD, line, MARK_BOOKMARK);
        }

        ++count;
        startPos = targetEnd;

        // Prevent infinite loop on zero-length matches
        if (targetEnd == result) {
            ++startPos;
            if (startPos >= docLength)
                break;
        }
    }

    return count;
}

void FindReplaceDlg::clearMarks() {
    if (!_ppEditView || !*_ppEditView) return;

    auto* view = *_ppEditView;

    // Clear the default mark indicator (used by Mark tab)
    view->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE);

    // Clear bookmarks added by Mark tab
    view->execute(SCI_MARKERDELETEALL, MARK_BOOKMARK);
}

bool FindReplaceDlg::findAllInCurrentDoc() {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;
    auto opts = getCurrentOptions();
    int flags = buildSearchFlags(opts);

    QString rawSearch = getSearchText();
    if (rawSearch.isEmpty()) return false;

    // Apply extended string conversion if needed
    if (opts.searchType == SearchType::Extended)
    {
        rawSearch = convertExtendedString(rawSearch);
    }

    QByteArray searchText = rawSearch.toUtf8();

    view->execute(SCI_SETSEARCHFLAGS, flags);

    // Get file path for results panel
    QString filePath;
    if (_getActiveFilePathCb)
    {
        filePath = _getActiveFilePathCb();
    }
    if (filePath.isEmpty())
    {
        filePath = tr("Untitled");
    }

    // Set up finder panel
    if (_pFinderPanel)
    {
        _pFinderPanel->addSearchLine(rawSearch);
        _pFinderPanel->addFileNameTitle(filePath);
    }

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));
        if (result == -1)
            break;

        intptr_t matchEnd = view->execute(SCI_GETTARGETEND);
        intptr_t line = view->execute(SCI_LINEFROMPOSITION, result);

        // Get the line text
        intptr_t lineStart = view->execute(SCI_POSITIONFROMLINE, line);
        intptr_t lineEnd = view->execute(SCI_GETLINEENDPOSITION, line);
        intptr_t lineLen = lineEnd - lineStart;

        QByteArray lineBuf(static_cast<int>(lineLen) + 1, '\0');

        // Get text range using SCI_GETTEXTRANGE-like approach
        Sci_TextRangeFull tr;
        tr.chrg.cpMin = static_cast<Sci_Position>(lineStart);
        tr.chrg.cpMax = static_cast<Sci_Position>(lineEnd);
        tr.lpstrText = lineBuf.data();
        view->execute(SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<LPARAM>(&tr));

        QString lineText = QString::fromUtf8(lineBuf.constData());

        // Add result to finder panel
        if (_pFinderPanel)
        {
            _pFinderPanel->addSearchResult(lineText, static_cast<int>(line) + 1, result, matchEnd);
        }

        count++;
        startPos = matchEnd;
    }

    // Finalize the file results
    if (_pFinderPanel)
    {
        _pFinderPanel->addFileHitCount(count);
        _pFinderPanel->show();
    }

    setStatusMessage(tr("Found %1 match(es)").arg(count), count > 0 ? FindStatus::Found : FindStatus::NotFound);
    return count > 0;
}

bool FindReplaceDlg::findAllInOpenDocs() {
    if (!_ppEditView || !*_ppEditView) return false;

    // If no open-buffers callback, fall back to current doc
    if (!_getOpenBuffersCb || !_activateBufferCb)
    {
        return findAllInCurrentDoc();
    }

    auto opts = getCurrentOptions();
    int flags = buildSearchFlags(opts);

    QString rawSearch = getSearchText();
    if (rawSearch.isEmpty()) return false;

    // Apply extended string conversion if needed
    if (opts.searchType == SearchType::Extended)
    {
        rawSearch = convertExtendedString(rawSearch);
    }

    QByteArray searchText = rawSearch.toUtf8();

    // Save original buffer to restore later
    void* originalBuffer = (*_ppEditView)->getCurrentBufferID();

    std::vector<BufferInfo> buffers = _getOpenBuffersCb();

    // Set up finder panel for multi-file search
    if (_pFinderPanel)
    {
        _pFinderPanel->addSearchLine(rawSearch);
    }

    int totalCount = 0;
    int filesWithHits = 0;

    for (const auto& bufInfo : buffers)
    {
        if (!_activateBufferCb(bufInfo.first))
        {
            continue;
        }

        auto* view = *_ppEditView;
        view->execute(SCI_SETSEARCHFLAGS, flags);

        QString filePath = bufInfo.second;
        if (filePath.isEmpty())
        {
            filePath = tr("Untitled");
        }

        int fileCount = 0;
        intptr_t startPos = 0;
        intptr_t docLength = view->execute(SCI_GETLENGTH);

        // Track whether we've added the file header
        bool fileHeaderAdded = false;

        while (true)
        {
            view->execute(SCI_SETTARGETSTART, startPos);
            view->execute(SCI_SETTARGETEND, docLength);

            intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                            reinterpret_cast<LPARAM>(searchText.constData()));
            if (result == -1)
            {
                break;
            }

            // Add file header on first hit
            if (!fileHeaderAdded && _pFinderPanel)
            {
                _pFinderPanel->addFileNameTitle(filePath);
                fileHeaderAdded = true;
            }

            intptr_t matchEnd = view->execute(SCI_GETTARGETEND);
            intptr_t line = view->execute(SCI_LINEFROMPOSITION, result);

            // Get the line text
            intptr_t lineStart = view->execute(SCI_POSITIONFROMLINE, line);
            intptr_t lineEnd = view->execute(SCI_GETLINEENDPOSITION, line);
            intptr_t lineLen = lineEnd - lineStart;

            QByteArray lineBuf(static_cast<int>(lineLen) + 1, '\0');
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = static_cast<Sci_Position>(lineStart);
            tr.chrg.cpMax = static_cast<Sci_Position>(lineEnd);
            tr.lpstrText = lineBuf.data();
            view->execute(SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<LPARAM>(&tr));

            QString lineText = QString::fromUtf8(lineBuf.constData());

            if (_pFinderPanel)
            {
                _pFinderPanel->addSearchResult(lineText, static_cast<int>(line) + 1, result, matchEnd);
            }

            fileCount++;
            startPos = matchEnd;
        }

        if (fileCount > 0)
        {
            if (_pFinderPanel)
            {
                _pFinderPanel->addFileHitCount(fileCount);
            }
            filesWithHits++;
            totalCount += fileCount;
        }
    }

    // Restore original buffer
    if (originalBuffer)
    {
        _activateBufferCb(originalBuffer);
    }

    if (_pFinderPanel && totalCount > 0)
    {
        _pFinderPanel->show();
    }

    setStatusMessage(tr("Found %1 hit(s) in %2 of %3 document(s)")
                     .arg(totalCount).arg(filesWithHits).arg(buffers.size()),
                     totalCount > 0 ? FindStatus::Found : FindStatus::NotFound);
    return totalCount > 0;
}

bool FindReplaceDlg::findAllInFiles() {
    auto opts = getCurrentOptions();
    int hits = processFindInFiles(getSearchText(), opts);
    return hits > 0;
}

bool FindReplaceDlg::findInProjects()
{
    if (!_getProjectFilesCb)
    {
        setStatusMessage(tr("No project panel available"), FindStatus::Warning);
        return false;
    }

    QString rawSearch = getSearchText();
    if (rawSearch.isEmpty())
    {
        return false;
    }

    auto opts = getCurrentOptions();

    // Apply extended string conversion if needed
    if (opts.searchType == SearchType::Extended)
    {
        rawSearch = convertExtendedString(rawSearch);
    }

    bool isRegex = (opts.searchType == SearchType::Regex);
    Qt::CaseSensitivity cs = opts.isMatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;

    QRegularExpression regex;
    if (isRegex)
    {
        QRegularExpression::PatternOptions regexOpts = QRegularExpression::NoPatternOption;
        if (!opts.isMatchCase)
            regexOpts |= QRegularExpression::CaseInsensitiveOption;
        if (opts.dotMatchesNewline)
            regexOpts |= QRegularExpression::DotMatchesEverythingOption;
        regex = QRegularExpression(rawSearch, regexOpts);
        if (!regex.isValid())
        {
            setStatusMessage(tr("Invalid regex: %1").arg(regex.errorString()), FindStatus::Warning);
            return false;
        }
    }

    // Collect files from checked project panels (1, 2, 3)
    QStringList allFiles;
    for (int i = 1; i <= 3; ++i)
    {
        bool checked = false;
        if (i == 1 && _projectPanel1Check) checked = _projectPanel1Check->isChecked();
        if (i == 2 && _projectPanel2Check) checked = _projectPanel2Check->isChecked();
        if (i == 3 && _projectPanel3Check) checked = _projectPanel3Check->isChecked();

        if (checked)
        {
            QStringList files = _getProjectFilesCb(i);
            allFiles.append(files);
        }
    }

    if (allFiles.isEmpty())
    {
        setStatusMessage(tr("No project files found. Check that a project panel is selected."), FindStatus::Warning);
        return false;
    }

    // Set up finder panel
    if (_pFinderPanel)
    {
        _pFinderPanel->addSearchLine(rawSearch);
    }

    int totalHits = 0;
    int filesWithHits = 0;
    int filesSearched = 0;

    for (const QString& filePath : allFiles)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }

        QTextStream stream(&file);
        int lineNumber = 0;
        int fileHits = 0;
        bool fileHeaderAdded = false;

        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            lineNumber++;

            bool found = false;
            if (isRegex)
            {
                found = regex.match(line).hasMatch();
            }
            else if (opts.isWholeWord)
            {
                int idx = 0;
                while (true)
                {
                    idx = line.indexOf(rawSearch, idx, cs);
                    if (idx < 0) break;
                    bool leftBound = (idx == 0) || !line[idx - 1].isLetterOrNumber();
                    bool rightBound = (idx + rawSearch.length() >= line.length()) ||
                                      !line[idx + rawSearch.length()].isLetterOrNumber();
                    if (leftBound && rightBound)
                    {
                        found = true;
                        break;
                    }
                    idx++;
                }
            }
            else
            {
                found = line.contains(rawSearch, cs);
            }

            if (found)
            {
                if (!fileHeaderAdded && _pFinderPanel)
                {
                    _pFinderPanel->addFileNameTitle(filePath);
                    fileHeaderAdded = true;
                }

                if (_pFinderPanel)
                {
                    _pFinderPanel->addSearchResult(line, lineNumber, 0, 0);
                }

                fileHits++;
                totalHits++;
            }
        }

        filesSearched++;
        if (fileHits > 0)
        {
            if (_pFinderPanel)
            {
                _pFinderPanel->addFileHitCount(fileHits);
            }
            filesWithHits++;
        }
    }

    if (_pFinderPanel && totalHits > 0)
    {
        _pFinderPanel->show();
    }

    setStatusMessage(tr("Found %1 hit(s) in %2 file(s) (searched %3 files)")
                     .arg(totalHits).arg(filesWithHits).arg(filesSearched),
                     totalHits > 0 ? FindStatus::Found : FindStatus::NotFound);

    return totalHits > 0;
}

bool FindReplaceDlg::replaceAllInOpenDocs() {
    if (!_getOpenBuffersCb || !_activateBufferCb)
    {
        // Fallback: replace in current document only
        int count = processReplaceAll(getSearchText(), getReplaceText(), getCurrentOptions());
        return count > 0;
    }

    QString findText = getSearchText();
    QString replaceText = getReplaceText();
    FindOptions opts = getCurrentOptions();

    if (findText.isEmpty())
    {
        return false;
    }

    // Remember current buffer so we can switch back
    std::vector<BufferInfo> buffers = _getOpenBuffersCb();
    if (buffers.empty())
    {
        return false;
    }

    // Save the current buffer pointer
    void* originalBuffer = nullptr;
    if (!buffers.empty())
    {
        // Find which buffer is currently active by checking the current view
        if (_ppEditView && *_ppEditView)
        {
            originalBuffer = (*_ppEditView)->getCurrentBufferID();
        }
    }

    int totalCount = 0;
    int docsWithReplacements = 0;

    for (const auto& bufInfo : buffers)
    {
        void* bufID = bufInfo.first;

        // Switch to this buffer
        if (!_activateBufferCb(bufID))
        {
            continue;
        }

        // Perform replace all in this document
        int count = processReplaceAll(findText, replaceText, opts);
        totalCount += count;
        if (count > 0)
        {
            docsWithReplacements++;
        }
    }

    // Switch back to original buffer
    if (originalBuffer)
    {
        _activateBufferCb(originalBuffer);
    }

    return totalCount > 0;
}

int FindReplaceDlg::processCount(const QString& findText, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return 0;

    auto* view = *_ppEditView;
    int flags = buildSearchFlags(options);

    QByteArray searchText = findText.toUtf8();
    if (searchText.isEmpty()) return 0;

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));
        if (result == -1)
            break;

        count++;
        startPos = view->execute(SCI_GETTARGETEND);
    }

    return count;
}

int FindReplaceDlg::processMarkAll(const QString& findText, int styleId, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return 0;

    auto* view = *_ppEditView;
    int flags = buildSearchFlags(options);

    QByteArray searchText = findText.toUtf8();
    if (searchText.isEmpty()) return 0;

    // Determine the indicator style to use
    int indicatorId = (styleId < 0) ? SCE_UNIVERSAL_FOUND_STYLE : styleId;

    // Set the indicator for text highlighting
    view->execute(SCI_SETINDICATORCURRENT, indicatorId);

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));
        if (result == -1)
            break;

        intptr_t targetEnd = view->execute(SCI_GETTARGETEND);
        intptr_t matchLen = targetEnd - result;

        // Highlight the matched text using indicator
        if (matchLen > 0) {
            view->execute(SCI_INDICATORFILLRANGE, result, matchLen);
        }

        // Also bookmark the line if requested via options
        if (options.doMarkLine) {
            intptr_t line = view->execute(SCI_LINEFROMPOSITION, result);
            view->execute(SCI_MARKERADD, line, MARK_BOOKMARK);
        }

        ++count;
        startPos = targetEnd;

        // Prevent infinite loop on zero-length matches
        if (targetEnd == result) {
            ++startPos;
            if (startPos >= docLength)
                break;
        }
    }

    return count;
}

static bool matchesFileFilter(const QString& fileName, const QString& filters)
{
    if (filters.isEmpty())
        return true;

    QStringList filterList = filters.split(QRegularExpression("[;\\s]+"), Qt::SkipEmptyParts);
    for (const auto& filter : filterList)
    {
        // Convert glob pattern to regex
        QString pattern = QRegularExpression::wildcardToRegularExpression(filter.trimmed());
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        if (re.match(fileName).hasMatch())
            return true;
    }
    return false;
}

int FindReplaceDlg::processFindInFiles(const QString& findText, const FindOptions& options) {
    if (findText.isEmpty())
        return 0;

    QString directory = options.directory;
    if (directory.isEmpty())
        return 0;

    QDir dir(directory);
    if (!dir.exists())
        return 0;

    bool isRegex = (options.searchType == SearchType::Regex);
    Qt::CaseSensitivity cs = options.isMatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;

    QRegularExpression regex;
    if (isRegex) {
        QRegularExpression::PatternOptions regexOpts = QRegularExpression::NoPatternOption;
        if (!options.isMatchCase)
            regexOpts |= QRegularExpression::CaseInsensitiveOption;
        if (options.dotMatchesNewline)
            regexOpts |= QRegularExpression::DotMatchesEverythingOption;
        regex = QRegularExpression(findText, regexOpts);
        if (!regex.isValid())
            return 0;
    }

    QDirIterator::IteratorFlags iterFlags = QDirIterator::NoIteratorFlags;
    if (options.isRecursive)
        iterFlags |= QDirIterator::Subdirectories;

    int totalHits = 0;
    int filesSearched = 0;
    int filesWithHits = 0;

    QDirIterator it(directory, QDir::Files | QDir::Readable, iterFlags);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString fileName = QFileInfo(filePath).fileName();

        // Skip hidden files/dirs unless requested
        if (!options.isInHiddenDir) {
            if (fileName.startsWith('.'))
                continue;
            // Check if any path component is hidden
            QString relPath = dir.relativeFilePath(filePath);
            if (relPath.contains("/."))
                continue;
        }

        // Check file filter
        if (!matchesFileFilter(fileName, options.filters))
            continue;

        // Read file and search
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QTextStream stream(&file);
        int lineNumber = 0;
        int fileHits = 0;

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            lineNumber++;

            bool found = false;
            if (isRegex) {
                found = regex.match(line).hasMatch();
            } else if (options.isWholeWord) {
                // Simple whole-word matching
                int idx = 0;
                while (true) {
                    idx = line.indexOf(findText, idx, cs);
                    if (idx < 0) break;
                    bool leftBound = (idx == 0) || !line[idx - 1].isLetterOrNumber();
                    bool rightBound = (idx + findText.length() >= line.length()) ||
                                      !line[idx + findText.length()].isLetterOrNumber();
                    if (leftBound && rightBound) {
                        found = true;
                        break;
                    }
                    idx++;
                }
            } else {
                found = line.contains(findText, cs);
            }

            if (found) {
                fileHits++;
                totalHits++;
            }
        }

        filesSearched++;
        if (fileHits > 0)
            filesWithHits++;
    }

    setStatusMessage(tr("Found %1 hit(s) in %2 file(s) (searched %3 files)")
                     .arg(totalHits).arg(filesWithHits).arg(filesSearched),
                     totalHits > 0 ? FindStatus::Found : FindStatus::NotFound);

    return totalHits;
}

intptr_t FindReplaceDlg::searchInTarget(const QString& text, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return -1;

    auto* view = *_ppEditView;
    int flags = buildSearchFlags(options);

    QByteArray searchText = text.toUtf8();
    view->execute(SCI_SETSEARCHFLAGS, flags);

    return view->execute(SCI_SEARCHINTARGET, searchText.length(),
                         reinterpret_cast<LPARAM>(searchText.constData()));
}

QString FindReplaceDlg::convertExtendedString(const QString& input) {
    QString result;
    for (int i = 0; i < input.length(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.length())
        {
            ++i;
            switch (input[i].toLatin1())
            {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case '0': result += '\0'; break;
                case '\\': result += '\\'; break;
                case 'x':
                case 'X':
                {
                    // Parse hex value \xNN
                    if (i + 2 < input.length())
                    {
                        QString hex = input.mid(i + 1, 2);
                        bool ok = false;
                        int value = hex.toInt(&ok, 16);
                        if (ok)
                        {
                            result += QChar(value);
                            i += 2;
                        }
                        else
                        {
                            result += '\\';
                            result += input[i];
                        }
                    }
                    else
                    {
                        result += '\\';
                        result += input[i];
                    }
                    break;
                }
                case 'u':
                case 'U':
                {
                    // Parse unicode value \uNNNN
                    if (i + 4 < input.length())
                    {
                        QString hex = input.mid(i + 1, 4);
                        bool ok = false;
                        int value = hex.toInt(&ok, 16);
                        if (ok)
                        {
                            result += QChar(value);
                            i += 4;
                        }
                        else
                        {
                            result += '\\';
                            result += input[i];
                        }
                    }
                    else
                    {
                        result += '\\';
                        result += input[i];
                    }
                    break;
                }
                default:
                    result += '\\';
                    result += input[i];
                    break;
            }
        }
        else
        {
            result += input[i];
        }
    }
    return result;
}

void FindReplaceDlg::savePosition() {
    // Save window geometry to settings if needed
}

void FindReplaceDlg::restorePosition() {
    // Restore window geometry from settings if needed
}

void FindReplaceDlg::showWarning(const QString& msg) {
    QMessageBox::warning(this, tr("Find"), msg);
}

void FindReplaceDlg::showInfo(const QString& msg) {
    QMessageBox::information(this, tr("Find"), msg);
}

int FindReplaceDlg::buildSearchFlags(const FindOptions& options) const {
    int flags = 0;

    if (options.isWholeWord) {
        flags |= SCFIND_WHOLEWORD;
    }
    if (options.isMatchCase) {
        flags |= SCFIND_MATCHCASE;
    }
    if (options.searchType == SearchType::Regex) {
        flags |= SCFIND_REGEXP | SCFIND_POSIX;
        if (options.dotMatchesNewline) {
            flags |= SCFIND_REGEXP_DOTMATCHESNL;
        }
    }

    return flags;
}

void FindReplaceDlg::displaySectionCentered(intptr_t start, intptr_t end, bool isDownwards) {
    (void)end;
    (void)isDownwards;

    if (!_ppEditView || !*_ppEditView) return;

    auto* view = *_ppEditView;

    // Make sure the position is visible
    view->execute(SCI_ENSUREVISIBLE, view->execute(SCI_LINEFROMPOSITION, start));
    view->execute(SCI_SCROLLCARET);
}

void FindReplaceDlg::resizeEvent(QResizeEvent* event) {
    StaticDialog::resizeEvent(event);
    // Adjust layout if needed
}

bool FindReplaceDlg::run_dlgProc(QEvent* event) {
    Q_UNUSED(event);
    return true;
}

// ============================================================================
// Multi-cursor commands
// ============================================================================

void FindReplaceDlg::multiSelectNextOccurrence(int searchFlags) {
    if (!_ppEditView || !*_ppEditView)
    {
        return;
    }

    auto* view = *_ppEditView;

    // If no selection, expand to word first
    bool hasSelection = (view->execute(SCI_GETSELECTIONSTART) != view->execute(SCI_GETSELECTIONEND));
    if (!hasSelection)
    {
        view->expandWordSelection();
    }

    view->execute(SCI_TARGETWHOLEDOCUMENT);
    view->execute(SCI_SETSEARCHFLAGS, searchFlags);
    view->execute(SCI_MULTIPLESELECTADDNEXT);
}

void FindReplaceDlg::multiSelectAllOccurrences(int searchFlags) {
    if (!_ppEditView || !*_ppEditView)
    {
        return;
    }

    auto* view = *_ppEditView;

    // If no selection, expand to word first
    bool hasSelection = (view->execute(SCI_GETSELECTIONSTART) != view->execute(SCI_GETSELECTIONEND));
    if (!hasSelection)
    {
        view->expandWordSelection();
    }

    view->execute(SCI_TARGETWHOLEDOCUMENT);
    view->execute(SCI_SETSEARCHFLAGS, searchFlags);
    view->execute(SCI_MULTIPLESELECTADDEACH);
}

void FindReplaceDlg::multiSelectUndo() {
    if (!_ppEditView || !*_ppEditView)
    {
        return;
    }

    auto* view = *_ppEditView;
    LRESULT n = view->execute(SCI_GETSELECTIONS);
    if (n > 0)
    {
        view->execute(SCI_DROPSELECTIONN, n - 1);
    }
}

void FindReplaceDlg::multiSelectSkip(int searchFlags) {
    if (!_ppEditView || !*_ppEditView)
    {
        return;
    }

    auto* view = *_ppEditView;

    view->execute(SCI_TARGETWHOLEDOCUMENT);
    view->execute(SCI_SETSEARCHFLAGS, searchFlags);
    view->execute(SCI_MULTIPLESELECTADDNEXT);

    LRESULT n = view->execute(SCI_GETSELECTIONS);
    if (n > 1)
    {
        view->execute(SCI_DROPSELECTIONN, n - 2);
    }
}

// ============================================================================
// Windows-compatible interface methods
// ============================================================================

void FindReplaceDlg::doDialog(DIALOG_TYPE type, bool isRTL, bool isDelete) {
    Q_UNUSED(isRTL);
    Q_UNUSED(isDelete);

    // Map DIALOG_TYPE to FindDialogType
    FindDialogType dlgType;
    switch (type) {
        case REPLACE_DLG:
            dlgType = FindDialogType::Replace;
            break;
        case FINDINFILES_DLG:
            dlgType = FindDialogType::FindInFiles;
            break;
        case FINDINPROJECTS_DLG:
            dlgType = FindDialogType::FindInProjects;
            break;
        case MARK_DLG:
            dlgType = FindDialogType::Mark;
            break;
        case FIND_DLG:
        default:
            dlgType = FindDialogType::Find;
            break;
    }

    showDialog(dlgType);
}

void FindReplaceDlg::setSearchText(const wchar_t* text) {
    if (text) {
        setSearchText(QString::fromWCharArray(text));
    }
}

void FindReplaceDlg::markAll(const wchar_t* text, int styleID) {
    if (text) {
        setSearchText(QString::fromWCharArray(text));
    }
    markAll(styleID);
}

void FindReplaceDlg::gotoNextFoundResult(int direction) const {
    if (_pFinderPanel)
    {
        if (direction >= 0)
        {
            _pFinderPanel->gotoNextResult();
        }
        else
        {
            _pFinderPanel->gotoPreviousResult();
        }
    }
}

bool FindReplaceDlg::processFindNext(const wchar_t* text, const FindOption* opt, ::FindStatus* status, FindNextType type) {
    Q_UNUSED(type);

    if (!text || !opt) {
        if (status) *status = FSNoMessage;
        return false;
    }

    // Convert wchar_t to QString
    QString searchText = QString::fromWCharArray(text);

    // Convert FindOption to FindOptions
    FindOptions options;
    options.isWholeWord = opt->_isWholeWord;
    options.isMatchCase = opt->_isMatchCase;
    options.isWrapAround = opt->_isWrapAround;
    options.direction = opt->_whichDirection ? SearchDirection::Down : SearchDirection::Up;
    options.searchType = static_cast<SearchType>(opt->_searchType);

    // Call the Qt implementation
    bool result = processFindNext(searchText, options);

    if (status) {
        *status = result ? FSFound : FSNotFound;
    }

    return result;
}

// Initialize the _env alias
FindOptions* FindReplaceDlg::_env = &FindReplaceDlg::options;

// ============================================================================
// FindIncrementDlg implementation
// ============================================================================

FindIncrementDlg::FindIncrementDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Incremental Search"));
}

FindIncrementDlg::~FindIncrementDlg() = default;

void FindIncrementDlg::init(FindReplaceDlg* pFRDlg, ScintillaEditView** ppEditView) {
    _pFRDlg = pFRDlg;
    _ppEditView = ppEditView;
    setupUI();
    connectSignals();
    _isCreated = true;
}

void FindIncrementDlg::setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(4, 2, 4, 2);

    _searchEdit = new QLineEdit(this);
    _searchEdit->setMinimumWidth(200);
    layout->addWidget(_searchEdit);

    _statusLabel = new QLabel(this);
    _statusLabel->setMinimumWidth(80);
    layout->addWidget(_statusLabel);

    _nextButton = new QPushButton(tr("▲"), this);
    _nextButton->setToolTip(tr("Find Next"));
    _nextButton->setFixedSize(24, 24);
    layout->addWidget(_nextButton);

    _prevButton = new QPushButton(tr("▼"), this);
    _prevButton->setToolTip(tr("Find Previous"));
    _prevButton->setFixedSize(24, 24);
    layout->addWidget(_prevButton);

    _highlightButton = new QPushButton(tr("*"), this);
    _highlightButton->setToolTip(tr("Highlight all"));
    _highlightButton->setFixedSize(24, 24);
    _highlightButton->setCheckable(true);
    layout->addWidget(_highlightButton);

    _caseSensitiveCheck = new QCheckBox(tr("Aa"), this);
    _caseSensitiveCheck->setToolTip(tr("Match case"));
    layout->addWidget(_caseSensitiveCheck);

    _wholeWordCheck = new QCheckBox(tr("\u00ab\u00bb"), this);
    _wholeWordCheck->setToolTip(tr("Match whole word only"));
    layout->addWidget(_wholeWordCheck);

    _regexCheck = new QCheckBox(tr(".*"), this);
    _regexCheck->setToolTip(tr("Regular expression"));
    layout->addWidget(_regexCheck);
}

void FindIncrementDlg::connectSignals() {
    connect(_searchEdit, &QLineEdit::textChanged, this, &FindIncrementDlg::onTextChanged);
    connect(_nextButton, &QPushButton::clicked, this, &FindIncrementDlg::onFindNext);
    connect(_prevButton, &QPushButton::clicked, this, &FindIncrementDlg::onFindPrevious);
    connect(_highlightButton, &QPushButton::toggled, this, &FindIncrementDlg::onHighlightAll);
    connect(_caseSensitiveCheck, &QCheckBox::toggled, this, &FindIncrementDlg::onCaseSensitiveToggled);
    connect(_wholeWordCheck, &QCheckBox::toggled, this, &FindIncrementDlg::onWholeWordToggled);
    connect(_regexCheck, &QCheckBox::toggled, this, &FindIncrementDlg::onRegexToggled);
}

void FindIncrementDlg::setSearchText(const QString& text) {
    if (_searchEdit) {
        _searchEdit->setText(text);
    }
}

void FindIncrementDlg::display(bool toShow) {
    if (toShow) {
        show();
        raise();
        activateWindow();
    } else {
        hide();
    }
}

void FindIncrementDlg::setFindStatus(FindStatus status, int count) {
    _findStatus = status;
    QString msg;
    switch (status) {
        case FindStatus::Found:
            msg = (count > 0) ? tr("%1 matches").arg(count) : tr("Found");
            break;
        case FindStatus::NotFound:
            msg = tr("Not found");
            break;
        case FindStatus::EndReached:
            msg = tr("Wrapped");
            break;
        default:
            msg = QString();
            break;
    }
    if (_statusLabel) {
        _statusLabel->setText(msg);
    }
}

void FindIncrementDlg::onTextChanged(const QString& text) {
    if (text.isEmpty()) {
        setFindStatus(FindStatus::NoMessage, 0);
        return;
    }

    // Perform incremental search
    if (_pFRDlg) {
        FindOptions opt = _pFRDlg->getCurrentOptions();
        opt.isMatchCase = _caseSensitiveCheck->isChecked();
        opt.isWholeWord = _wholeWordCheck->isChecked();
        opt.searchType = _regexCheck->isChecked() ? SearchType::Regex : SearchType::Normal;

        bool found = _pFRDlg->processFindNext(text, opt);
        setFindStatus(found ? FindStatus::Found : FindStatus::NotFound, 0);
    }
}

void FindIncrementDlg::onFindNext() {
    if (_pFRDlg) {
        _pFRDlg->findNext();
    }
}

void FindIncrementDlg::onFindPrevious() {
    if (_pFRDlg) {
        _pFRDlg->findPrevious();
    }
}

void FindIncrementDlg::onHighlightAll() {
    if (!_ppEditView || !*_ppEditView)
    {
        return;
    }

    auto* view = *_ppEditView;
    bool doHighlight = _highlightButton && _highlightButton->isChecked();

    // Indicator number for incremental search highlighting
    const int INDICATOR_INCREMENTAL = 31;

    // Clear previous highlights
    intptr_t docLength = view->execute(SCI_GETLENGTH);
    view->execute(SCI_SETINDICATORCURRENT, INDICATOR_INCREMENTAL);
    view->execute(SCI_INDICATORCLEARRANGE, 0, docLength);

    if (!doHighlight)
    {
        return;
    }

    QString searchText = _searchEdit ? _searchEdit->text() : QString();
    if (searchText.isEmpty())
    {
        return;
    }

    // Configure indicator style
    view->execute(SCI_INDICSETSTYLE, INDICATOR_INCREMENTAL, INDIC_ROUNDBOX);
    view->execute(SCI_INDICSETFORE, INDICATOR_INCREMENTAL, 0x0000FF);  // Red
    view->execute(SCI_INDICSETALPHA, INDICATOR_INCREMENTAL, 100);
    view->execute(SCI_INDICSETOUTLINEALPHA, INDICATOR_INCREMENTAL, 200);

    // Build search flags
    int flags = 0;
    if (_caseSensitiveCheck && _caseSensitiveCheck->isChecked())
    {
        flags |= SCFIND_MATCHCASE;
    }
    if (_wholeWordCheck && _wholeWordCheck->isChecked())
    {
        flags |= SCFIND_WHOLEWORD;
    }
    if (_regexCheck && _regexCheck->isChecked())
    {
        flags |= SCFIND_REGEXP | SCFIND_POSIX;
    }

    QByteArray searchBytes = searchText.toUtf8();
    view->execute(SCI_SETSEARCHFLAGS, flags);

    intptr_t startPos = 0;
    int count = 0;

    while (true)
    {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchBytes.length(),
                                        reinterpret_cast<LPARAM>(searchBytes.constData()));
        if (result == -1)
        {
            break;
        }

        intptr_t matchEnd = view->execute(SCI_GETTARGETEND);
        intptr_t matchLen = matchEnd - result;

        if (matchLen > 0)
        {
            view->execute(SCI_INDICATORFILLRANGE, result, matchLen);
            count++;
        }

        startPos = matchEnd;
        if (startPos >= docLength)
        {
            break;
        }
    }

    setFindStatus(count > 0 ? FindStatus::Found : FindStatus::NotFound, count);
}

void FindIncrementDlg::onCaseSensitiveToggled(bool checked) {
    (void)checked;
    // Update search
    onTextChanged(_searchEdit->text());
}

void FindIncrementDlg::onWholeWordToggled(bool checked) {
    (void)checked;
    // Update search
    onTextChanged(_searchEdit->text());
}

void FindIncrementDlg::onRegexToggled(bool checked) {
    (void)checked;
    // Update search
    onTextChanged(_searchEdit->text());
}

bool FindIncrementDlg::run_dlgProc(QEvent* event) {
    Q_UNUSED(event);
    return true;
}

// ============================================================================
// FindProgressDlg implementation
// ============================================================================

FindProgressDlg::FindProgressDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Search Progress"));
    setModal(true);
}

FindProgressDlg::~FindProgressDlg() = default;

void FindProgressDlg::setupUI() {
    auto* layout = new QVBoxLayout(this);

    _headerLabel = new QLabel(this);
    _headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(_headerLabel);

    _infoLabel = new QLabel(this);
    layout->addWidget(_infoLabel);

    _progressBar = new QProgressBar(this);
    _progressBar->setRange(0, 100);
    layout->addWidget(_progressBar);

    _hitsLabel = new QLabel(tr("Hits: 0"), this);
    layout->addWidget(_hitsLabel);

    _cancelButton = new QPushButton(tr("Cancel"), this);
    layout->addWidget(_cancelButton, 0, Qt::AlignCenter);
}

void FindProgressDlg::connectSignals() {
    connect(_cancelButton, &QPushButton::clicked, this, &FindProgressDlg::onCancel);
}

void FindProgressDlg::setHeader(const QString& header) {
    if (_headerLabel) {
        _headerLabel->setText(header);
    }
}

void FindProgressDlg::setPercent(int percent) {
    if (_progressBar) {
        _progressBar->setValue(percent);
    }
}

void FindProgressDlg::setInfo(const QString& info) {
    if (_infoLabel) {
        _infoLabel->setText(info);
    }
}

void FindProgressDlg::setHitsCount(int count) {
    if (_hitsLabel) {
        _hitsLabel->setText(tr("Hits: %1").arg(count));
    }
}

void FindProgressDlg::onCancel() {
    _cancelled = true;
    hide();
}

void FindProgressDlg::closeEvent(QCloseEvent* event) {
    _cancelled = true;
    StaticDialog::closeEvent(event);
}

// ============================================================================
// FinderPanel implementation
// ============================================================================

FinderPanel::FinderPanel(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Find result"));
}

FinderPanel::~FinderPanel() = default;

void FinderPanel::init(ScintillaEditView** ppEditView) {
    _ppEditView = ppEditView;
    setupUI();
    connectSignals();
}

void FinderPanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    _resultsTree = new QTreeWidget(this);
    _resultsTree->setHeaderLabels({tr("Search Results")});
    _resultsTree->setRootIsDecorated(true);
    _resultsTree->setAlternatingRowColors(true);
    _resultsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    _resultsTree->header()->setStretchLastSection(true);
    _resultsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    _resultsTree->setFont(QFont("monospace", 9));
    layout->addWidget(_resultsTree);
}

void FinderPanel::connectSignals() {
    if (_resultsTree) {
        connect(_resultsTree, &QTreeWidget::itemDoubleClicked,
                this, &FinderPanel::onResultDoubleClicked);
        connect(_resultsTree, &QTreeWidget::customContextMenuRequested,
                this, &FinderPanel::onContextMenu);
    }
}

void FinderPanel::addSearchLine(const QString& searchText) {
    if (!_resultsTree) return;

    _currentSearchItem = new QTreeWidgetItem(_resultsTree);
    _currentSearchItem->setText(0, tr("Search \"%1\"").arg(searchText));
    QFont font = _currentSearchItem->font(0);
    font.setBold(true);
    _currentSearchItem->setFont(0, font);
    _currentSearchItem->setExpanded(true);
    _currentFileItem = nullptr;
}

void FinderPanel::addFileNameTitle(const QString& fileName) {
    if (!_currentSearchItem) return;

    _currentFileItem = new QTreeWidgetItem(_currentSearchItem);
    _currentFileItem->setText(0, fileName);
    QFont font = _currentFileItem->font(0);
    font.setBold(true);
    _currentFileItem->setFont(0, font);
    _currentFileItem->setExpanded(true);
    _currentFileCount++;
}

void FinderPanel::addFileHitCount(int count) {
    if (!_currentFileItem) return;

    QString text = _currentFileItem->text(0);
    _currentFileItem->setText(0, tr("%1 (%2 hits)").arg(text).arg(count));
}

void FinderPanel::addSearchResult(const QString& line, int lineNumber, intptr_t start, intptr_t end) {
    QTreeWidgetItem* parent = _currentFileItem ? _currentFileItem : _currentSearchItem;
    if (!parent) return;

    auto* item = new QTreeWidgetItem(parent);
    item->setText(0, tr("  Line %1: %2").arg(lineNumber).arg(line.trimmed()));

    // Store the index into _foundInfos as item data
    int idx = static_cast<int>(_foundInfos.size());
    item->setData(0, Qt::UserRole, idx);

    FoundInfo info;
    info.start = start;
    info.end = end;
    info.lineNumber = lineNumber;
    info.lineText = line;
    if (_currentFileItem)
        info.filePath = _currentFileItem->text(0).section(" (", 0, 0);
    _foundInfos.push_back(info);
}

void FinderPanel::beginNewFilesSearch() {
    _foundInfos.clear();
    _currentFileCount = 0;
    _currentSearchItem = nullptr;
    _currentFileItem = nullptr;
    _currentResultIndex = -1;
}

void FinderPanel::finishFilesSearch(int count, int searchedCount) {
    if (_currentSearchItem) {
        QString text = _currentSearchItem->text(0);
        _currentSearchItem->setText(0, tr("%1 (%2 hits in %3 files)")
                                       .arg(text).arg(count).arg(searchedCount));
    }
}

void FinderPanel::removeAll() {
    _foundInfos.clear();
    _currentFileCount = 0;
    _currentSearchItem = nullptr;
    _currentFileItem = nullptr;
    _currentResultIndex = -1;
    if (_resultsTree)
        _resultsTree->clear();
}

void FinderPanel::openAll() {
    // Collect unique file paths from results
    // Opening files requires the main app integration - signal to Notepad_plus
}

void FinderPanel::copy() {
    if (!_resultsTree) return;

    QString text;
    for (int i = 0; i < _resultsTree->topLevelItemCount(); i++) {
        QTreeWidgetItem* searchItem = _resultsTree->topLevelItem(i);
        text += searchItem->text(0) + "\n";
        for (int j = 0; j < searchItem->childCount(); j++) {
            QTreeWidgetItem* fileItem = searchItem->child(j);
            text += "  " + fileItem->text(0) + "\n";
            for (int k = 0; k < fileItem->childCount(); k++) {
                text += "    " + fileItem->child(k)->text(0) + "\n";
            }
        }
    }

    QApplication::clipboard()->setText(text);
}

void FinderPanel::copyPathnames() {
    if (!_resultsTree) return;

    QStringList paths;
    for (const auto& info : _foundInfos) {
        if (!info.filePath.isEmpty() && !paths.contains(info.filePath))
            paths.append(info.filePath);
    }
    QApplication::clipboard()->setText(paths.join("\n"));
}

void FinderPanel::gotoNextResult() {
    if (_foundInfos.empty()) return;

    _currentResultIndex++;
    if (_currentResultIndex >= static_cast<int>(_foundInfos.size()))
        _currentResultIndex = 0;

    gotoFoundLine(static_cast<size_t>(_currentResultIndex));
}

void FinderPanel::gotoPreviousResult() {
    if (_foundInfos.empty()) return;

    _currentResultIndex--;
    if (_currentResultIndex < 0)
        _currentResultIndex = static_cast<int>(_foundInfos.size()) - 1;

    gotoFoundLine(static_cast<size_t>(_currentResultIndex));
}

void FinderPanel::setFinderStyle() {
    if (!_resultsTree) return;
    // Apply the current theme colors to the results tree
    // This is a simplified version - full theming would read from stylers.xml
}

void FinderPanel::onResultDoubleClicked(QTreeWidgetItem* item, int column) {
    (void)column;
    if (!item) return;

    QVariant data = item->data(0, Qt::UserRole);
    if (data.isValid()) {
        int idx = data.toInt();
        gotoFoundLine(static_cast<size_t>(idx));
    }
}

void FinderPanel::onContextMenu(const QPoint& pos) {
    if (!_resultsTree) return;

    QMenu menu;
    menu.addAction(tr("Copy"), this, &FinderPanel::copy);
    menu.addAction(tr("Copy Pathnames"), this, &FinderPanel::copyPathnames);
    menu.addSeparator();
    menu.addAction(tr("Clear All"), this, &FinderPanel::removeAll);
    menu.exec(_resultsTree->mapToGlobal(pos));
}

void FinderPanel::gotoFoundLine(size_t index) {
    if (index >= _foundInfos.size()) return;

    const auto& info = _foundInfos[index];
    if (!_ppEditView || !*_ppEditView) return;

    auto* view = *_ppEditView;
    view->execute(SCI_ENSUREVISIBLE, info.lineNumber - 1);
    view->execute(SCI_GOTOPOS, info.start);
    view->execute(SCI_SETSEL, info.start, info.end);
}

} // namespace NppFindReplace
