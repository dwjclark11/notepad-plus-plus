// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "preferenceDlg.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFrame>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtGui/QDesktopServices>

#include "../../Parameters.h"

namespace QtControls {

// ============================================================================
// GeneralSubDlg Implementation
// ============================================================================
GeneralSubDlg::GeneralSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void GeneralSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // General settings group
    _generalGroup = new QGroupBox(tr("General"), this);
    auto* generalLayout = new QVBoxLayout(_generalGroup);

    _hideStatusBarCheck = new QCheckBox(tr("Hide status bar"), _generalGroup);
    generalLayout->addWidget(_hideStatusBarCheck);

    _hideMenuBarCheck = new QCheckBox(tr("Hide menu bar (use Alt or F10 key to toggle)"), _generalGroup);
    generalLayout->addWidget(_hideMenuBarCheck);

    _hideMenuShortcutsCheck = new QCheckBox(tr("Hide right shortcuts from menu bar (e.g. +F, +O, +S...)"), _generalGroup);
    generalLayout->addWidget(_hideMenuShortcutsCheck);

    mainLayout->addWidget(_generalGroup);

    // Localization group
    _localizationGroup = new QGroupBox(tr("Localization"), this);
    auto* localizationLayout = new QHBoxLayout(_localizationGroup);

    _languageLabel = new QLabel(tr("Language:"), _localizationGroup);
    localizationLayout->addWidget(_languageLabel);

    _languageCombo = new QComboBox(_localizationGroup);
    _languageCombo->setMinimumWidth(200);
    // Add common languages
    QStringList languages;
    languages << "English" << "French" << "German" << "Spanish" << "Italian"
              << "Portuguese" << "Russian" << "Chinese (Simplified)" << "Chinese (Traditional)"
              << "Japanese" << "Korean" << "Arabic" << "Dutch" << "Polish"
              << "Turkish" << "Czech" << "Hungarian" << "Romanian" << "Vietnamese";
    _languageCombo->addItems(languages);
    localizationLayout->addWidget(_languageCombo);
    localizationLayout->addStretch();

    mainLayout->addWidget(_localizationGroup);
    mainLayout->addStretch();
}

void GeneralSubDlg::connectSignals()
{
    connect(_hideStatusBarCheck, &QCheckBox::toggled, this, &GeneralSubDlg::onStatusBarToggled);
    connect(_hideMenuBarCheck, &QCheckBox::toggled, this, &GeneralSubDlg::onMenuBarToggled);
    connect(_hideMenuShortcutsCheck, &QCheckBox::toggled, this, &GeneralSubDlg::onHideMenuShortcutsToggled);
    connect(_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GeneralSubDlg::onLanguageChanged);
}

void GeneralSubDlg::loadSettings()
{
    // Load from PlatformLayer::Settings
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _statusBarShow = settings.readBool(L"General", L"StatusBarShow", true);
    _menuBarShow = settings.readBool(L"General", L"MenuBarShow", true);
    _hideMenuRightShortcuts = settings.readBool(L"General", L"HideMenuRightShortcuts", false);
    _currentLanguage = QString::fromStdWString(settings.readString(L"General", L"Language", L"English"));

    _hideStatusBarCheck->setChecked(!_statusBarShow);
    _hideMenuBarCheck->setChecked(!_menuBarShow);
    _hideMenuShortcutsCheck->setChecked(_hideMenuRightShortcuts);

    int langIndex = _languageCombo->findText(_currentLanguage);
    if (langIndex >= 0) {
        _languageCombo->setCurrentIndex(langIndex);
    }
}

void GeneralSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"General", L"StatusBarShow", _statusBarShow);
    settings.writeBool(L"General", L"MenuBarShow", _menuBarShow);
    settings.writeBool(L"General", L"HideMenuRightShortcuts", _hideMenuRightShortcuts);
    settings.writeString(L"General", L"Language", _currentLanguage.toStdWString());
}

bool GeneralSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void GeneralSubDlg::onStatusBarToggled(bool checked)
{
    _statusBarShow = !checked;
}

void GeneralSubDlg::onMenuBarToggled(bool checked)
{
    _menuBarShow = !checked;
}

void GeneralSubDlg::onHideMenuShortcutsToggled(bool checked)
{
    _hideMenuRightShortcuts = checked;
}

void GeneralSubDlg::onLanguageChanged(int index)
{
    if (index >= 0) {
        _currentLanguage = _languageCombo->currentText();
    }
}

// ============================================================================
// ToolbarSubDlg Implementation
// ============================================================================
ToolbarSubDlg::ToolbarSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void ToolbarSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Toolbar visibility
    _toolbarGroup = new QGroupBox(tr("Toolbar"), this);
    auto* toolbarLayout = new QVBoxLayout(_toolbarGroup);

    _hideToolbarCheck = new QCheckBox(tr("Hide toolbar"), _toolbarGroup);
    toolbarLayout->addWidget(_hideToolbarCheck);

    mainLayout->addWidget(_toolbarGroup);

    // Icon set
    _iconSetGroup = new QGroupBox(tr("Icon Set"), this);
    auto* iconSetLayout = new QHBoxLayout(_iconSetGroup);

    auto* iconSetLabel = new QLabel(tr("Icon size:"), _iconSetGroup);
    iconSetLayout->addWidget(iconSetLabel);

    _iconSetCombo = new QComboBox(_iconSetGroup);
    _iconSetCombo->addItem(tr("Small icons"));
    _iconSetCombo->addItem(tr("Large icons"));
    _iconSetCombo->addItem(tr("Small icons (set 2)"));
    _iconSetCombo->addItem(tr("Large icons (set 2)"));
    _iconSetCombo->addItem(tr("Standard icons"));
    iconSetLayout->addWidget(_iconSetCombo);
    iconSetLayout->addStretch();

    mainLayout->addWidget(_iconSetGroup);

    // Icon color
    _iconColorGroup = new QGroupBox(tr("Icon Color"), this);
    auto* iconColorLayout = new QHBoxLayout(_iconColorGroup);

    auto* colorLabel = new QLabel(tr("Color:"), _iconColorGroup);
    iconColorLayout->addWidget(colorLabel);

    _iconColorCombo = new QComboBox(_iconColorGroup);
    _iconColorCombo->addItem(tr("Default"));
    _iconColorCombo->addItem(tr("Red"));
    _iconColorCombo->addItem(tr("Green"));
    _iconColorCombo->addItem(tr("Blue"));
    _iconColorCombo->addItem(tr("Purple"));
    _iconColorCombo->addItem(tr("Cyan"));
    _iconColorCombo->addItem(tr("Olive"));
    _iconColorCombo->addItem(tr("Yellow"));
    iconColorLayout->addWidget(_iconColorCombo);
    iconColorLayout->addStretch();

    mainLayout->addWidget(_iconColorGroup);
    mainLayout->addStretch();
}

void ToolbarSubDlg::connectSignals()
{
    connect(_hideToolbarCheck, &QCheckBox::toggled, this, &ToolbarSubDlg::onToolbarHideToggled);
    connect(_iconSetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolbarSubDlg::onIconSetChanged);
    connect(_iconColorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolbarSubDlg::onIconColorChanged);
}

void ToolbarSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _toolbarShow = settings.readBool(L"Toolbar", L"Show", true);
    _iconSet = settings.readInt(L"Toolbar", L"IconSet", 0);
    _iconColor = settings.readInt(L"Toolbar", L"IconColor", 0);

    _hideToolbarCheck->setChecked(!_toolbarShow);
    _iconSetCombo->setCurrentIndex(_iconSet);
    _iconColorCombo->setCurrentIndex(_iconColor);
}

void ToolbarSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Toolbar", L"Show", _toolbarShow);
    settings.writeInt(L"Toolbar", L"IconSet", _iconSet);
    settings.writeInt(L"Toolbar", L"IconColor", _iconColor);
}

bool ToolbarSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void ToolbarSubDlg::onToolbarHideToggled(bool checked)
{
    _toolbarShow = !checked;
}

void ToolbarSubDlg::onIconSetChanged(int index)
{
    _iconSet = index;
}

void ToolbarSubDlg::onIconColorChanged(int index)
{
    _iconColor = index;
}

// ============================================================================
// TabbarSubDlg Implementation
// ============================================================================
TabbarSubDlg::TabbarSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void TabbarSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Look & Feel
    _lookFeelGroup = new QGroupBox(tr("Look && Feel"), this);
    auto* lookFeelLayout = new QVBoxLayout(_lookFeelGroup);

    _reduceCheck = new QCheckBox(tr("Reduce tab bar"), _lookFeelGroup);
    lookFeelLayout->addWidget(_reduceCheck);

    _lockCheck = new QCheckBox(tr("Lock (disable dragging)"), _lookFeelGroup);
    lookFeelLayout->addWidget(_lockCheck);

    _drawTopBarCheck = new QCheckBox(tr("Draw colored top bar on active tab"), _lookFeelGroup);
    lookFeelLayout->addWidget(_drawTopBarCheck);

    _drawInactiveCheck = new QCheckBox(tr("Draw inactive tabs"), _lookFeelGroup);
    lookFeelLayout->addWidget(_drawInactiveCheck);

    mainLayout->addWidget(_lookFeelGroup);

    // Behavior
    _behaviorGroup = new QGroupBox(tr("Behavior"), this);
    auto* behaviorLayout = new QVBoxLayout(_behaviorGroup);

    _showCloseButtonCheck = new QCheckBox(tr("Show close button on each tab"), _behaviorGroup);
    behaviorLayout->addWidget(_showCloseButtonCheck);

    _showPinButtonCheck = new QCheckBox(tr("Show pin button on each tab"), _behaviorGroup);
    behaviorLayout->addWidget(_showPinButtonCheck);

    _doubleClickCloseCheck = new QCheckBox(tr("Double-click to close tab"), _behaviorGroup);
    behaviorLayout->addWidget(_doubleClickCloseCheck);

    _multiLineCheck = new QCheckBox(tr("Multi-line tabs"), _behaviorGroup);
    behaviorLayout->addWidget(_multiLineCheck);

    _verticalCheck = new QCheckBox(tr("Vertical tabs"), _behaviorGroup);
    behaviorLayout->addWidget(_verticalCheck);

    _hideTabBarCheck = new QCheckBox(tr("Hide tab bar"), _behaviorGroup);
    behaviorLayout->addWidget(_hideTabBarCheck);

    _quitOnEmptyCheck = new QCheckBox(tr("Quit on closing last tab"), _behaviorGroup);
    behaviorLayout->addWidget(_quitOnEmptyCheck);

    mainLayout->addWidget(_behaviorGroup);
    mainLayout->addStretch();
}

void TabbarSubDlg::connectSignals()
{
    connect(_reduceCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onReduceToggled);
    connect(_lockCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onLockToggled);
    connect(_drawTopBarCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onDrawTopBarToggled);
    connect(_drawInactiveCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onDrawInactiveToggled);
    connect(_showCloseButtonCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onShowCloseButtonToggled);
    connect(_showPinButtonCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onShowPinButtonToggled);
    connect(_doubleClickCloseCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onDoubleClickCloseToggled);
    connect(_multiLineCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onMultiLineToggled);
    connect(_verticalCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onVerticalToggled);
    connect(_hideTabBarCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onHideTabBarToggled);
    connect(_quitOnEmptyCheck, &QCheckBox::toggled, this, &TabbarSubDlg::onQuitOnEmptyToggled);
}

void TabbarSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _reduce = settings.readBool(L"TabBar", L"Reduce", false);
    _lock = settings.readBool(L"TabBar", L"Lock", false);
    _drawTopBar = settings.readBool(L"TabBar", L"DrawTopBar", false);
    _drawInactive = settings.readBool(L"TabBar", L"DrawInactive", false);
    _showCloseButton = settings.readBool(L"TabBar", L"ShowCloseButton", true);
    _showPinButton = settings.readBool(L"TabBar", L"ShowPinButton", false);
    _doubleClickClose = settings.readBool(L"TabBar", L"DoubleClickClose", false);
    _multiLine = settings.readBool(L"TabBar", L"MultiLine", false);
    _vertical = settings.readBool(L"TabBar", L"Vertical", false);
    _hideTabBar = settings.readBool(L"TabBar", L"Hide", false);
    _quitOnEmpty = settings.readBool(L"TabBar", L"QuitOnEmpty", false);

    _reduceCheck->setChecked(_reduce);
    _lockCheck->setChecked(_lock);
    _drawTopBarCheck->setChecked(_drawTopBar);
    _drawInactiveCheck->setChecked(_drawInactive);
    _showCloseButtonCheck->setChecked(_showCloseButton);
    _showPinButtonCheck->setChecked(_showPinButton);
    _doubleClickCloseCheck->setChecked(_doubleClickClose);
    _multiLineCheck->setChecked(_multiLine);
    _verticalCheck->setChecked(_vertical);
    _hideTabBarCheck->setChecked(_hideTabBar);
    _quitOnEmptyCheck->setChecked(_quitOnEmpty);
}

void TabbarSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"TabBar", L"Reduce", _reduce);
    settings.writeBool(L"TabBar", L"Lock", _lock);
    settings.writeBool(L"TabBar", L"DrawTopBar", _drawTopBar);
    settings.writeBool(L"TabBar", L"DrawInactive", _drawInactive);
    settings.writeBool(L"TabBar", L"ShowCloseButton", _showCloseButton);
    settings.writeBool(L"TabBar", L"ShowPinButton", _showPinButton);
    settings.writeBool(L"TabBar", L"DoubleClickClose", _doubleClickClose);
    settings.writeBool(L"TabBar", L"MultiLine", _multiLine);
    settings.writeBool(L"TabBar", L"Vertical", _vertical);
    settings.writeBool(L"TabBar", L"Hide", _hideTabBar);
    settings.writeBool(L"TabBar", L"QuitOnEmpty", _quitOnEmpty);
}

bool TabbarSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void TabbarSubDlg::onReduceToggled(bool checked) { _reduce = checked; }
void TabbarSubDlg::onLockToggled(bool checked) { _lock = checked; }
void TabbarSubDlg::onDrawTopBarToggled(bool checked) { _drawTopBar = checked; }
void TabbarSubDlg::onDrawInactiveToggled(bool checked) { _drawInactive = checked; }
void TabbarSubDlg::onShowCloseButtonToggled(bool checked) { _showCloseButton = checked; }
void TabbarSubDlg::onShowPinButtonToggled(bool checked) { _showPinButton = checked; }
void TabbarSubDlg::onDoubleClickCloseToggled(bool checked) { _doubleClickClose = checked; }
void TabbarSubDlg::onMultiLineToggled(bool checked) { _multiLine = checked; }
void TabbarSubDlg::onVerticalToggled(bool checked) { _vertical = checked; }
void TabbarSubDlg::onHideTabBarToggled(bool checked) { _hideTabBar = checked; }
void TabbarSubDlg::onQuitOnEmptyToggled(bool checked) { _quitOnEmpty = checked; }

// ============================================================================
// EditingSubDlg Implementation
// ============================================================================
EditingSubDlg::EditingSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void EditingSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Line Numbering group
    _lineNumberGroup = new QGroupBox(tr("Line Numbering"), this);
    auto* lineNumberLayout = new QVBoxLayout(_lineNumberGroup);

    _lineNumberCheck = new QCheckBox(tr("Display line number"), _lineNumberGroup);
    lineNumberLayout->addWidget(_lineNumberCheck);

    _lineNumberDynamicWidthCheck = new QCheckBox(tr("Dynamic line number width"), _lineNumberGroup);
    lineNumberLayout->addWidget(_lineNumberDynamicWidthCheck);

    mainLayout->addWidget(_lineNumberGroup);

    // Current Line group
    _currentLineGroup = new QGroupBox(tr("Current Line"), this);
    auto* currentLineLayout = new QHBoxLayout(_currentLineGroup);

    _currentLineLabel = new QLabel(tr("Highlighting mode:"), _currentLineGroup);
    currentLineLayout->addWidget(_currentLineLabel);

    _currentLineHighlightCombo = new QComboBox(_currentLineGroup);
    _currentLineHighlightCombo->addItem(tr("None"));
    _currentLineHighlightCombo->addItem(tr("Highlight background"));
    _currentLineHighlightCombo->addItem(tr("Frame"));
    currentLineLayout->addWidget(_currentLineHighlightCombo);
    currentLineLayout->addStretch();

    mainLayout->addWidget(_currentLineGroup);

    // Caret group
    _caretGroup = new QGroupBox(tr("Caret (Cursor)"), this);
    auto* caretLayout = new QGridLayout(_caretGroup);

    _caretBlinkRateLabel = new QLabel(tr("Blink rate:"), _caretGroup);
    caretLayout->addWidget(_caretBlinkRateLabel, 0, 0);

    _caretBlinkRateSlider = new QSlider(Qt::Horizontal, _caretGroup);
    _caretBlinkRateSlider->setRange(50, 2500);
    _caretBlinkRateSlider->setSingleStep(50);
    _caretBlinkRateSlider->setValue(600);
    caretLayout->addWidget(_caretBlinkRateSlider, 0, 1);

    _caretBlinkRateValue = new QLabel(tr("600 ms"), _caretGroup);
    caretLayout->addWidget(_caretBlinkRateValue, 0, 2);

    _caretWidthLabel = new QLabel(tr("Width:"), _caretGroup);
    caretLayout->addWidget(_caretWidthLabel, 1, 0);

    _caretWidthCombo = new QComboBox(_caretGroup);
    _caretWidthCombo->addItem(tr("0 - Block"));
    _caretWidthCombo->addItem(tr("1"));
    _caretWidthCombo->addItem(tr("2"));
    _caretWidthCombo->addItem(tr("3"));
    _caretWidthCombo->addItem(tr("Block After"));
    caretLayout->addWidget(_caretWidthCombo, 1, 1, 1, 2);

    mainLayout->addWidget(_caretGroup);

    // Scintilla Options group
    _scintillaGroup = new QGroupBox(tr("Scintilla Options"), this);
    auto* scintillaLayout = new QVBoxLayout(_scintillaGroup);

    _smoothFontCheck = new QCheckBox(tr("Enable smooth font"), _scintillaGroup);
    scintillaLayout->addWidget(_smoothFontCheck);

    _virtualSpaceCheck = new QCheckBox(tr("Enable virtual space"), _scintillaGroup);
    scintillaLayout->addWidget(_virtualSpaceCheck);

    _scrollBeyondLastLineCheck = new QCheckBox(tr("Scroll beyond last line"), _scintillaGroup);
    scintillaLayout->addWidget(_scrollBeyondLastLineCheck);

    _rightClickKeepsSelectionCheck = new QCheckBox(tr("Right click keeps selection"), _scintillaGroup);
    scintillaLayout->addWidget(_rightClickKeepsSelectionCheck);

    _lineCopyCutWithoutSelectionCheck = new QCheckBox(tr("Copy/Cut line without selection"), _scintillaGroup);
    scintillaLayout->addWidget(_lineCopyCutWithoutSelectionCheck);

    mainLayout->addWidget(_scintillaGroup);

    // Line Wrap group
    _lineWrapGroup = new QGroupBox(tr("Line Wrap"), this);
    auto* lineWrapLayout = new QHBoxLayout(_lineWrapGroup);

    _lineWrapCombo = new QComboBox(_lineWrapGroup);
    _lineWrapCombo->addItem(tr("Default"));
    _lineWrapCombo->addItem(tr("Aligned"));
    _lineWrapCombo->addItem(tr("Indent"));
    lineWrapLayout->addWidget(_lineWrapCombo);
    lineWrapLayout->addStretch();

    mainLayout->addWidget(_lineWrapGroup);
    mainLayout->addStretch();
}

void EditingSubDlg::connectSignals()
{
    connect(_lineNumberCheck, &QCheckBox::toggled, this, &EditingSubDlg::onLineNumberToggled);
    connect(_lineNumberDynamicWidthCheck, &QCheckBox::toggled, this, &EditingSubDlg::onLineNumberDynamicWidthToggled);
    connect(_currentLineHighlightCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditingSubDlg::onCurrentLineHighlightChanged);
    connect(_caretBlinkRateSlider, &QSlider::valueChanged, this, &EditingSubDlg::onCaretBlinkRateChanged);
    connect(_caretWidthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditingSubDlg::onCaretWidthChanged);
    connect(_smoothFontCheck, &QCheckBox::toggled, this, &EditingSubDlg::onSmoothFontToggled);
    connect(_virtualSpaceCheck, &QCheckBox::toggled, this, &EditingSubDlg::onVirtualSpaceToggled);
    connect(_scrollBeyondLastLineCheck, &QCheckBox::toggled, this, &EditingSubDlg::onScrollBeyondLastLineToggled);
    connect(_rightClickKeepsSelectionCheck, &QCheckBox::toggled, this, &EditingSubDlg::onRightClickKeepsSelectionToggled);
    connect(_lineCopyCutWithoutSelectionCheck, &QCheckBox::toggled, this, &EditingSubDlg::onLineCopyCutWithoutSelectionToggled);
    connect(_lineWrapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditingSubDlg::onLineWrapMethodChanged);
}

void EditingSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _lineNumberShow = settings.readBool(L"Scintillas", L"LineNumberShow", true);
    _lineNumberDynamicWidth = settings.readBool(L"Scintillas", L"LineNumberDynamicWidth", true);
    _currentLineHighlightMode = settings.readInt(L"Scintillas", L"CurrentLineHighlightMode", 1);
    _caretBlinkRate = settings.readInt(L"Scintillas", L"CaretBlinkRate", 600);
    _caretWidth = settings.readInt(L"Scintillas", L"CaretWidth", 1);
    _doSmoothFont = settings.readBool(L"Scintillas", L"DoSmoothFont", false);
    _virtualSpace = settings.readBool(L"Scintillas", L"VirtualSpace", false);
    _scrollBeyondLastLine = settings.readBool(L"Scintillas", L"ScrollBeyondLastLine", true);
    _rightClickKeepsSelection = settings.readBool(L"Scintillas", L"RightClickKeepsSelection", false);
    _lineCopyCutWithoutSelection = settings.readBool(L"Scintillas", L"LineCopyCutWithoutSelection", false);
    _lineWrapMethod = settings.readInt(L"Scintillas", L"LineWrapMethod", 0);

    _lineNumberCheck->setChecked(_lineNumberShow);
    _lineNumberDynamicWidthCheck->setChecked(_lineNumberDynamicWidth);
    _currentLineHighlightCombo->setCurrentIndex(_currentLineHighlightMode);
    _caretBlinkRateSlider->setValue(_caretBlinkRate);
    _caretBlinkRateValue->setText(QString::number(_caretBlinkRate) + " ms");
    _caretWidthCombo->setCurrentIndex(_caretWidth);
    _smoothFontCheck->setChecked(_doSmoothFont);
    _virtualSpaceCheck->setChecked(_virtualSpace);
    _scrollBeyondLastLineCheck->setChecked(_scrollBeyondLastLine);
    _rightClickKeepsSelectionCheck->setChecked(_rightClickKeepsSelection);
    _lineCopyCutWithoutSelectionCheck->setChecked(_lineCopyCutWithoutSelection);
    _lineWrapCombo->setCurrentIndex(_lineWrapMethod);
}

void EditingSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Scintillas", L"LineNumberShow", _lineNumberShow);
    settings.writeBool(L"Scintillas", L"LineNumberDynamicWidth", _lineNumberDynamicWidth);
    settings.writeInt(L"Scintillas", L"CurrentLineHighlightMode", _currentLineHighlightMode);
    settings.writeInt(L"Scintillas", L"CaretBlinkRate", _caretBlinkRate);
    settings.writeInt(L"Scintillas", L"CaretWidth", _caretWidth);
    settings.writeBool(L"Scintillas", L"DoSmoothFont", _doSmoothFont);
    settings.writeBool(L"Scintillas", L"VirtualSpace", _virtualSpace);
    settings.writeBool(L"Scintillas", L"ScrollBeyondLastLine", _scrollBeyondLastLine);
    settings.writeBool(L"Scintillas", L"RightClickKeepsSelection", _rightClickKeepsSelection);
    settings.writeBool(L"Scintillas", L"LineCopyCutWithoutSelection", _lineCopyCutWithoutSelection);
    settings.writeInt(L"Scintillas", L"LineWrapMethod", _lineWrapMethod);
}

bool EditingSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void EditingSubDlg::onLineNumberToggled(bool checked)
{
    _lineNumberShow = checked;
}

void EditingSubDlg::onLineNumberDynamicWidthToggled(bool checked)
{
    _lineNumberDynamicWidth = checked;
}

void EditingSubDlg::onCurrentLineHighlightChanged(int index)
{
    _currentLineHighlightMode = index;
}

void EditingSubDlg::onCaretBlinkRateChanged(int value)
{
    _caretBlinkRate = value;
    _caretBlinkRateValue->setText(QString::number(value) + " ms");
}

void EditingSubDlg::onCaretWidthChanged(int index)
{
    _caretWidth = index;
}

void EditingSubDlg::onSmoothFontToggled(bool checked)
{
    _doSmoothFont = checked;
}

void EditingSubDlg::onVirtualSpaceToggled(bool checked)
{
    _virtualSpace = checked;
}

void EditingSubDlg::onScrollBeyondLastLineToggled(bool checked)
{
    _scrollBeyondLastLine = checked;
}

void EditingSubDlg::onRightClickKeepsSelectionToggled(bool checked)
{
    _rightClickKeepsSelection = checked;
}

void EditingSubDlg::onLineCopyCutWithoutSelectionToggled(bool checked)
{
    _lineCopyCutWithoutSelection = checked;
}

void EditingSubDlg::onLineWrapMethodChanged(int index)
{
    _lineWrapMethod = index;
}

void EditingSubDlg::initScintParam()
{
    // Additional initialization if needed
}

// ============================================================================
// Editing2SubDlg Implementation
// ============================================================================
Editing2SubDlg::Editing2SubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void Editing2SubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Non-Printing Characters
    _npcGroup = new QGroupBox(tr("Non-Printing Characters"), this);
    auto* npcLayout = new QVBoxLayout(_npcGroup);

    auto* npcModeLayout = new QHBoxLayout();
    _npcModeLabel = new QLabel(tr("Appearance:"), _npcGroup);
    npcModeLayout->addWidget(_npcModeLabel);

    _npcModeCombo = new QComboBox(_npcGroup);
    _npcModeCombo->addItem(tr("Abbreviation"));
    _npcModeCombo->addItem(tr("Codepoint"));
    npcModeLayout->addWidget(_npcModeCombo);
    npcModeLayout->addStretch();
    npcLayout->addLayout(npcModeLayout);

    _npcCustomColorCheck = new QCheckBox(tr("Use custom color"), _npcGroup);
    npcLayout->addWidget(_npcCustomColorCheck);

    _npcIncludeCcUniEolCheck = new QCheckBox(tr("Include C0/C1 control and Unicode EOL characters"), _npcGroup);
    npcLayout->addWidget(_npcIncludeCcUniEolCheck);

    _npcNoInputC0Check = new QCheckBox(tr("Do not input C0 control characters"), _npcGroup);
    npcLayout->addWidget(_npcNoInputC0Check);

    mainLayout->addWidget(_npcGroup);

    // CR/LF Display
    _crlfGroup = new QGroupBox(tr("CR/LF Display"), this);
    auto* crlfLayout = new QVBoxLayout(_crlfGroup);

    auto* crlfModeLayout = new QHBoxLayout();
    _crlfModeLabel = new QLabel(tr("Style:"), _crlfGroup);
    crlfModeLayout->addWidget(_crlfModeLabel);

    _crlfModeCombo = new QComboBox(_crlfGroup);
    _crlfModeCombo->addItem(tr("Round corner"));
    _crlfModeCombo->addItem(tr("Plain text"));
    crlfModeLayout->addWidget(_crlfModeCombo);
    crlfModeLayout->addStretch();
    crlfLayout->addLayout(crlfModeLayout);

    _crlfCustomColorCheck = new QCheckBox(tr("Use custom color"), _crlfGroup);
    crlfLayout->addWidget(_crlfCustomColorCheck);

    mainLayout->addWidget(_crlfGroup);
    mainLayout->addStretch();
}

void Editing2SubDlg::connectSignals()
{
    connect(_npcModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Editing2SubDlg::onNpcModeChanged);
    connect(_npcCustomColorCheck, &QCheckBox::toggled, this, &Editing2SubDlg::onNpcCustomColorToggled);
    connect(_npcIncludeCcUniEolCheck, &QCheckBox::toggled, this, &Editing2SubDlg::onNpcIncludeCcUniEolToggled);
    connect(_npcNoInputC0Check, &QCheckBox::toggled, this, &Editing2SubDlg::onNpcNoInputC0Toggled);
    connect(_crlfModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Editing2SubDlg::onCrlfDisplayModeChanged);
    connect(_crlfCustomColorCheck, &QCheckBox::toggled, this, &Editing2SubDlg::onCrlfCustomColorToggled);
}

void Editing2SubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _npcMode = settings.readInt(L"Editing2", L"NpcMode", 0);
    _npcCustomColor = settings.readBool(L"Editing2", L"NpcCustomColor", false);
    _npcIncludeCcUniEol = settings.readBool(L"Editing2", L"NpcIncludeCcUniEol", false);
    _npcNoInputC0 = settings.readBool(L"Editing2", L"NpcNoInputC0", false);
    _crlfDisplayMode = settings.readInt(L"Editing2", L"CrlfDisplayMode", 0);
    _crlfCustomColor = settings.readBool(L"Editing2", L"CrlfCustomColor", false);

    _npcModeCombo->setCurrentIndex(_npcMode);
    _npcCustomColorCheck->setChecked(_npcCustomColor);
    _npcIncludeCcUniEolCheck->setChecked(_npcIncludeCcUniEol);
    _npcNoInputC0Check->setChecked(_npcNoInputC0);
    _crlfModeCombo->setCurrentIndex(_crlfDisplayMode);
    _crlfCustomColorCheck->setChecked(_crlfCustomColor);
}

void Editing2SubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"Editing2", L"NpcMode", _npcMode);
    settings.writeBool(L"Editing2", L"NpcCustomColor", _npcCustomColor);
    settings.writeBool(L"Editing2", L"NpcIncludeCcUniEol", _npcIncludeCcUniEol);
    settings.writeBool(L"Editing2", L"NpcNoInputC0", _npcNoInputC0);
    settings.writeInt(L"Editing2", L"CrlfDisplayMode", _crlfDisplayMode);
    settings.writeBool(L"Editing2", L"CrlfCustomColor", _crlfCustomColor);
}

bool Editing2SubDlg::applySettings()
{
    saveSettings();
    return true;
}

void Editing2SubDlg::onNpcModeChanged(int index) { _npcMode = index; }
void Editing2SubDlg::onNpcCustomColorToggled(bool checked) { _npcCustomColor = checked; }
void Editing2SubDlg::onNpcIncludeCcUniEolToggled(bool checked) { _npcIncludeCcUniEol = checked; }
void Editing2SubDlg::onNpcNoInputC0Toggled(bool checked) { _npcNoInputC0 = checked; }
void Editing2SubDlg::onCrlfDisplayModeChanged(int index) { _crlfDisplayMode = index; }
void Editing2SubDlg::onCrlfCustomColorToggled(bool checked) { _crlfCustomColor = checked; }

// ============================================================================
// DarkModeSubDlg Implementation
// ============================================================================
DarkModeSubDlg::DarkModeSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void DarkModeSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Dark Mode
    _darkModeGroup = new QGroupBox(tr("Dark Mode"), this);
    auto* darkModeLayout = new QVBoxLayout(_darkModeGroup);

    _enableDarkModeCheck = new QCheckBox(tr("Enable dark mode"), _darkModeGroup);
    darkModeLayout->addWidget(_enableDarkModeCheck);

    mainLayout->addWidget(_darkModeGroup);

    // Theme
    _themeGroup = new QGroupBox(tr("Theme"), this);
    auto* themeLayout = new QHBoxLayout(_themeGroup);

    _themeLabel = new QLabel(tr("Theme:"), _themeGroup);
    themeLayout->addWidget(_themeLabel);

    _themeCombo = new QComboBox(_themeGroup);
    _themeCombo->addItem(tr("Default Dark"));
    _themeCombo->addItem(tr("Dark Black"));
    _themeCombo->addItem(tr("Dark Red"));
    _themeCombo->addItem(tr("Dark Green"));
    _themeCombo->addItem(tr("Dark Blue"));
    _themeCombo->addItem(tr("Dark Purple"));
    _themeCombo->addItem(tr("Dark Cyan"));
    _themeCombo->addItem(tr("Dark Olive"));
    _themeCombo->addItem(tr("Customized"));
    themeLayout->addWidget(_themeCombo);
    themeLayout->addStretch();

    mainLayout->addWidget(_themeGroup);
    mainLayout->addStretch();
}

void DarkModeSubDlg::connectSignals()
{
    connect(_enableDarkModeCheck, &QCheckBox::toggled, this, &DarkModeSubDlg::onDarkModeToggled);
    connect(_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DarkModeSubDlg::onThemeChanged);
}

void DarkModeSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _darkModeEnabled = settings.readBool(L"DarkMode", L"Enabled", false);
    _themeIndex = settings.readInt(L"DarkMode", L"Theme", 0);

    _enableDarkModeCheck->setChecked(_darkModeEnabled);
    _themeCombo->setCurrentIndex(_themeIndex);
}

void DarkModeSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"DarkMode", L"Enabled", _darkModeEnabled);
    settings.writeInt(L"DarkMode", L"Theme", _themeIndex);
}

bool DarkModeSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void DarkModeSubDlg::onDarkModeToggled(bool checked) { _darkModeEnabled = checked; }
void DarkModeSubDlg::onThemeChanged(int index) { _themeIndex = index; }

// ============================================================================
// MarginsBorderEdgeSubDlg Implementation
// ============================================================================
MarginsBorderEdgeSubDlg::MarginsBorderEdgeSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void MarginsBorderEdgeSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Margins
    _marginsGroup = new QGroupBox(tr("Margins"), this);
    auto* marginsLayout = new QVBoxLayout(_marginsGroup);

    _bookmarkMarginCheck = new QCheckBox(tr("Show bookmark margin"), _marginsGroup);
    marginsLayout->addWidget(_bookmarkMarginCheck);

    _changeHistoryMarginCheck = new QCheckBox(tr("Show change history margin"), _marginsGroup);
    marginsLayout->addWidget(_changeHistoryMarginCheck);

    auto* folderStyleLayout = new QHBoxLayout();
    _folderMarkStyleLabel = new QLabel(tr("Folder mark style:"), _marginsGroup);
    folderStyleLayout->addWidget(_folderMarkStyleLabel);

    _folderMarkStyleCombo = new QComboBox(_marginsGroup);
    _folderMarkStyleCombo->addItem(tr("Simple"));
    _folderMarkStyleCombo->addItem(tr("Arrow"));
    _folderMarkStyleCombo->addItem(tr("Circle"));
    _folderMarkStyleCombo->addItem(tr("Box"));
    _folderMarkStyleCombo->addItem(tr("None"));
    folderStyleLayout->addWidget(_folderMarkStyleCombo);
    folderStyleLayout->addStretch();
    marginsLayout->addLayout(folderStyleLayout);

    mainLayout->addWidget(_marginsGroup);

    // Border
    _borderGroup = new QGroupBox(tr("Border Width"), this);
    auto* borderLayout = new QHBoxLayout(_borderGroup);

    _borderWidthLabel = new QLabel(tr("Width:"), _borderGroup);
    borderLayout->addWidget(_borderWidthLabel);

    _borderWidthSlider = new QSlider(Qt::Horizontal, _borderGroup);
    _borderWidthSlider->setRange(0, 5);
    _borderWidthSlider->setPageStep(1);
    borderLayout->addWidget(_borderWidthSlider);

    _borderWidthValue = new QLabel("2", _borderGroup);
    _borderWidthValue->setMinimumWidth(20);
    borderLayout->addWidget(_borderWidthValue);

    mainLayout->addWidget(_borderGroup);

    // Padding
    _paddingGroup = new QGroupBox(tr("Padding"), this);
    auto* paddingLayout = new QGridLayout(_paddingGroup);

    _paddingLeftLabel = new QLabel(tr("Left:"), _paddingGroup);
    paddingLayout->addWidget(_paddingLeftLabel, 0, 0);

    _paddingLeftSlider = new QSlider(Qt::Horizontal, _paddingGroup);
    _paddingLeftSlider->setRange(0, 30);
    _paddingLeftSlider->setPageStep(1);
    paddingLayout->addWidget(_paddingLeftSlider, 0, 1);

    _paddingLeftValue = new QLabel("0", _paddingGroup);
    _paddingLeftValue->setMinimumWidth(20);
    paddingLayout->addWidget(_paddingLeftValue, 0, 2);

    _paddingRightLabel = new QLabel(tr("Right:"), _paddingGroup);
    paddingLayout->addWidget(_paddingRightLabel, 1, 0);

    _paddingRightSlider = new QSlider(Qt::Horizontal, _paddingGroup);
    _paddingRightSlider->setRange(0, 30);
    _paddingRightSlider->setPageStep(1);
    paddingLayout->addWidget(_paddingRightSlider, 1, 1);

    _paddingRightValue = new QLabel("0", _paddingGroup);
    _paddingRightValue->setMinimumWidth(20);
    paddingLayout->addWidget(_paddingRightValue, 1, 2);

    _distractionFreeLabel = new QLabel(tr("Distraction Free:"), _paddingGroup);
    paddingLayout->addWidget(_distractionFreeLabel, 2, 0);

    _distractionFreeSlider = new QSlider(Qt::Horizontal, _paddingGroup);
    _distractionFreeSlider->setRange(1, 9);
    _distractionFreeSlider->setPageStep(1);
    paddingLayout->addWidget(_distractionFreeSlider, 2, 1);

    _distractionFreeValue = new QLabel("3", _paddingGroup);
    _distractionFreeValue->setMinimumWidth(20);
    paddingLayout->addWidget(_distractionFreeValue, 2, 2);

    mainLayout->addWidget(_paddingGroup);

    // Vertical Edge
    _verticalEdgeGroup = new QGroupBox(tr("Vertical Edge"), this);
    auto* verticalEdgeLayout = new QVBoxLayout(_verticalEdgeGroup);

    _verticalEdgeCheck = new QCheckBox(tr("Show vertical edge"), _verticalEdgeGroup);
    verticalEdgeLayout->addWidget(_verticalEdgeCheck);

    auto* columnsLayout = new QHBoxLayout();
    _verticalEdgeColumnsLabel = new QLabel(tr("Column position(s):"), _verticalEdgeGroup);
    columnsLayout->addWidget(_verticalEdgeColumnsLabel);

    _verticalEdgeColumnsEdit = new QLineEdit(_verticalEdgeGroup);
    _verticalEdgeColumnsEdit->setPlaceholderText(tr("e.g. 80 120"));
    columnsLayout->addWidget(_verticalEdgeColumnsEdit);
    verticalEdgeLayout->addLayout(columnsLayout);

    mainLayout->addWidget(_verticalEdgeGroup);
    mainLayout->addStretch();
}

void MarginsBorderEdgeSubDlg::connectSignals()
{
    connect(_borderWidthSlider, &QSlider::valueChanged, this, &MarginsBorderEdgeSubDlg::onBorderWidthChanged);
    connect(_paddingLeftSlider, &QSlider::valueChanged, this, &MarginsBorderEdgeSubDlg::onPaddingLeftChanged);
    connect(_paddingRightSlider, &QSlider::valueChanged, this, &MarginsBorderEdgeSubDlg::onPaddingRightChanged);
    connect(_distractionFreeSlider, &QSlider::valueChanged, this, &MarginsBorderEdgeSubDlg::onDistractionFreeChanged);
    connect(_folderMarkStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MarginsBorderEdgeSubDlg::onFolderMarkStyleChanged);
    connect(_bookmarkMarginCheck, &QCheckBox::toggled, this, &MarginsBorderEdgeSubDlg::onBookmarkMarginToggled);
    connect(_changeHistoryMarginCheck, &QCheckBox::toggled, this, &MarginsBorderEdgeSubDlg::onChangeHistoryMarginToggled);
    connect(_verticalEdgeCheck, &QCheckBox::toggled, this, &MarginsBorderEdgeSubDlg::onVerticalEdgeToggled);
    connect(_verticalEdgeColumnsEdit, &QLineEdit::textChanged, this, &MarginsBorderEdgeSubDlg::onVerticalEdgeColumnsChanged);
}

void MarginsBorderEdgeSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _borderWidth = settings.readInt(L"MarginsBorderEdge", L"BorderWidth", 2);
    _paddingLeft = settings.readInt(L"MarginsBorderEdge", L"PaddingLeft", 0);
    _paddingRight = settings.readInt(L"MarginsBorderEdge", L"PaddingRight", 0);
    _distractionFree = settings.readInt(L"MarginsBorderEdge", L"DistractionFree", 3);
    _folderMarkStyle = settings.readInt(L"MarginsBorderEdge", L"FolderMarkStyle", 0);
    _bookmarkMargin = settings.readBool(L"MarginsBorderEdge", L"BookmarkMargin", true);
    _changeHistoryMargin = settings.readBool(L"MarginsBorderEdge", L"ChangeHistoryMargin", false);
    _verticalEdge = settings.readBool(L"MarginsBorderEdge", L"VerticalEdge", false);
    _verticalEdgeColumns = QString::fromStdWString(settings.readString(L"MarginsBorderEdge", L"VerticalEdgeColumns", L""));

    _borderWidthSlider->setValue(_borderWidth);
    _borderWidthValue->setText(QString::number(_borderWidth));
    _paddingLeftSlider->setValue(_paddingLeft);
    _paddingLeftValue->setText(QString::number(_paddingLeft));
    _paddingRightSlider->setValue(_paddingRight);
    _paddingRightValue->setText(QString::number(_paddingRight));
    _distractionFreeSlider->setValue(_distractionFree);
    _distractionFreeValue->setText(QString::number(_distractionFree));
    _folderMarkStyleCombo->setCurrentIndex(_folderMarkStyle);
    _bookmarkMarginCheck->setChecked(_bookmarkMargin);
    _changeHistoryMarginCheck->setChecked(_changeHistoryMargin);
    _verticalEdgeCheck->setChecked(_verticalEdge);
    _verticalEdgeColumnsEdit->setText(_verticalEdgeColumns);
}

void MarginsBorderEdgeSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"MarginsBorderEdge", L"BorderWidth", _borderWidth);
    settings.writeInt(L"MarginsBorderEdge", L"PaddingLeft", _paddingLeft);
    settings.writeInt(L"MarginsBorderEdge", L"PaddingRight", _paddingRight);
    settings.writeInt(L"MarginsBorderEdge", L"DistractionFree", _distractionFree);
    settings.writeInt(L"MarginsBorderEdge", L"FolderMarkStyle", _folderMarkStyle);
    settings.writeBool(L"MarginsBorderEdge", L"BookmarkMargin", _bookmarkMargin);
    settings.writeBool(L"MarginsBorderEdge", L"ChangeHistoryMargin", _changeHistoryMargin);
    settings.writeBool(L"MarginsBorderEdge", L"VerticalEdge", _verticalEdge);
    settings.writeString(L"MarginsBorderEdge", L"VerticalEdgeColumns", _verticalEdgeColumns.toStdWString());
}

bool MarginsBorderEdgeSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void MarginsBorderEdgeSubDlg::onBorderWidthChanged(int value)
{
    _borderWidth = value;
    _borderWidthValue->setText(QString::number(value));
}

void MarginsBorderEdgeSubDlg::onPaddingLeftChanged(int value)
{
    _paddingLeft = value;
    _paddingLeftValue->setText(QString::number(value));
}

void MarginsBorderEdgeSubDlg::onPaddingRightChanged(int value)
{
    _paddingRight = value;
    _paddingRightValue->setText(QString::number(value));
}

void MarginsBorderEdgeSubDlg::onDistractionFreeChanged(int value)
{
    _distractionFree = value;
    _distractionFreeValue->setText(QString::number(value));
}

void MarginsBorderEdgeSubDlg::onFolderMarkStyleChanged(int index) { _folderMarkStyle = index; }
void MarginsBorderEdgeSubDlg::onBookmarkMarginToggled(bool checked) { _bookmarkMargin = checked; }
void MarginsBorderEdgeSubDlg::onChangeHistoryMarginToggled(bool checked) { _changeHistoryMargin = checked; }

void MarginsBorderEdgeSubDlg::onVerticalEdgeToggled(bool checked)
{
    _verticalEdge = checked;
    _verticalEdgeColumnsEdit->setEnabled(checked);
}

void MarginsBorderEdgeSubDlg::onVerticalEdgeColumnsChanged(const QString& text) { _verticalEdgeColumns = text; }

// ============================================================================
// NewDocumentSubDlg Implementation
// ============================================================================
NewDocumentSubDlg::NewDocumentSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void NewDocumentSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Encoding group
    _encodingGroup = new QGroupBox(tr("Encoding"), this);
    auto* encodingLayout = new QVBoxLayout(_encodingGroup);

    _encodingCombo = new QComboBox(_encodingGroup);
    _encodingCombo->addItem(tr("UTF-8"));
    _encodingCombo->addItem(tr("UTF-8 BOM"));
    _encodingCombo->addItem(tr("UTF-16 Little Endian"));
    _encodingCombo->addItem(tr("UTF-16 Big Endian"));
    _encodingCombo->addItem(tr("ANSI"));
    _encodingCombo->setMinimumWidth(200);
    encodingLayout->addWidget(_encodingCombo);

    _ansiAsUtf8Check = new QCheckBox(tr("Open ANSI files as UTF-8 (without BOM)"), _encodingGroup);
    encodingLayout->addWidget(_ansiAsUtf8Check);

    mainLayout->addWidget(_encodingGroup);

    // Format group
    _formatGroup = new QGroupBox(tr("Format (Line ending)"), this);
    auto* formatLayout = new QVBoxLayout(_formatGroup);

    _formatCombo = new QComboBox(_formatGroup);
    _formatCombo->addItem(tr("Windows (CR LF)"));
    _formatCombo->addItem(tr("Unix (LF)"));
    _formatCombo->addItem(tr("Macintosh (CR)"));
    formatLayout->addWidget(_formatCombo);

    mainLayout->addWidget(_formatGroup);

    // Language group
    _languageGroup = new QGroupBox(tr("Default Language"), this);
    auto* languageLayout = new QVBoxLayout(_languageGroup);

    _languageCombo = new QComboBox(_languageGroup);
    _languageCombo->addItem(tr("Text"));
    _languageCombo->addItem(tr("C"));
    _languageCombo->addItem(tr("C++"));
    _languageCombo->addItem(tr("Java"));
    _languageCombo->addItem(tr("Python"));
    _languageCombo->addItem(tr("JavaScript"));
    _languageCombo->addItem(tr("HTML"));
    _languageCombo->addItem(tr("XML"));
    _languageCombo->addItem(tr("CSS"));
    _languageCombo->addItem(tr("PHP"));
    languageLayout->addWidget(_languageCombo);

    mainLayout->addWidget(_languageGroup);

    // Defaults group
    _defaultsGroup = new QGroupBox(tr("Apply to Opened ANSI Files"), this);
    auto* defaultsLayout = new QVBoxLayout(_defaultsGroup);

    _applyToOpenedAnsiFilesCheck = new QCheckBox(tr("Apply the above settings to opened ANSI files"), _defaultsGroup);
    defaultsLayout->addWidget(_applyToOpenedAnsiFilesCheck);

    mainLayout->addWidget(_defaultsGroup);
    mainLayout->addStretch();
}

void NewDocumentSubDlg::connectSignals()
{
    connect(_encodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewDocumentSubDlg::onEncodingChanged);
    connect(_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewDocumentSubDlg::onFormatChanged);
    connect(_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewDocumentSubDlg::onLanguageChanged);
    connect(_ansiAsUtf8Check, &QCheckBox::toggled, this, &NewDocumentSubDlg::onAnsiAsUtf8Toggled);
}

void NewDocumentSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _defaultEncoding = settings.readInt(L"NewDoc", L"DefaultEncoding", 0);
    _defaultFormat = settings.readInt(L"NewDoc", L"DefaultFormat", 0);
    _defaultLanguage = settings.readInt(L"NewDoc", L"DefaultLanguage", 0);
    _openAnsiAsUtf8 = settings.readBool(L"NewDoc", L"OpenAnsiAsUtf8", true);

    _encodingCombo->setCurrentIndex(_defaultEncoding);
    _formatCombo->setCurrentIndex(_defaultFormat);
    _languageCombo->setCurrentIndex(_defaultLanguage);
    _ansiAsUtf8Check->setChecked(_openAnsiAsUtf8);
}

void NewDocumentSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"NewDoc", L"DefaultEncoding", _defaultEncoding);
    settings.writeInt(L"NewDoc", L"DefaultFormat", _defaultFormat);
    settings.writeInt(L"NewDoc", L"DefaultLanguage", _defaultLanguage);
    settings.writeBool(L"NewDoc", L"OpenAnsiAsUtf8", _openAnsiAsUtf8);
}

bool NewDocumentSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void NewDocumentSubDlg::onEncodingChanged(int index)
{
    _defaultEncoding = index;
}

void NewDocumentSubDlg::onFormatChanged(int index)
{
    _defaultFormat = index;
}

void NewDocumentSubDlg::onLanguageChanged(int index)
{
    _defaultLanguage = index;
}

void NewDocumentSubDlg::onAnsiAsUtf8Toggled(bool checked)
{
    _openAnsiAsUtf8 = checked;
}

// ============================================================================
// DefaultDirectorySubDlg Implementation
// ============================================================================
DefaultDirectorySubDlg::DefaultDirectorySubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void DefaultDirectorySubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Directory group
    _directoryGroup = new QGroupBox(tr("Default Directory"), this);
    auto* directoryLayout = new QVBoxLayout(_directoryGroup);

    _directoryTypeCombo = new QComboBox(_directoryGroup);
    _directoryTypeCombo->addItem(tr("Follow current document"));
    _directoryTypeCombo->addItem(tr("Remember last used directory"));
    _directoryTypeCombo->addItem(tr("Custom directory:"));
    directoryLayout->addWidget(_directoryTypeCombo);

    auto* customPathLayout = new QHBoxLayout();
    _customPathEdit = new QLineEdit(_directoryGroup);
    _customPathEdit->setPlaceholderText(tr("Enter custom directory path..."));
    customPathLayout->addWidget(_customPathEdit);

    _browseButton = new QPushButton(tr("Browse..."), _directoryGroup);
    customPathLayout->addWidget(_browseButton);

    directoryLayout->addLayout(customPathLayout);
    mainLayout->addWidget(_directoryGroup);
    mainLayout->addStretch();
}

void DefaultDirectorySubDlg::connectSignals()
{
    connect(_directoryTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DefaultDirectorySubDlg::onDirectoryTypeChanged);
    connect(_browseButton, &QPushButton::clicked, this, &DefaultDirectorySubDlg::onBrowseClicked);
    connect(_customPathEdit, &QLineEdit::textChanged, this, &DefaultDirectorySubDlg::onCustomPathChanged);
}

void DefaultDirectorySubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _defaultDirectoryType = settings.readInt(L"DefaultDir", L"DefaultDirectoryType", 0);
    _customDefaultDirectory = QString::fromStdWString(settings.readString(L"DefaultDir", L"CustomDefaultDirectory", L""));

    _directoryTypeCombo->setCurrentIndex(_defaultDirectoryType);
    _customPathEdit->setText(_customDefaultDirectory);
    _customPathEdit->setEnabled(_defaultDirectoryType == 2);
    _browseButton->setEnabled(_defaultDirectoryType == 2);
}

void DefaultDirectorySubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"DefaultDir", L"DefaultDirectoryType", _defaultDirectoryType);
    settings.writeString(L"DefaultDir", L"CustomDefaultDirectory", _customDefaultDirectory.toStdWString());
}

bool DefaultDirectorySubDlg::applySettings()
{
    saveSettings();
    return true;
}

void DefaultDirectorySubDlg::onDirectoryTypeChanged(int index)
{
    _defaultDirectoryType = index;
    bool isCustom = (index == 2);
    _customPathEdit->setEnabled(isCustom);
    _browseButton->setEnabled(isCustom);
}

void DefaultDirectorySubDlg::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Default Directory"),
                                                    _customDefaultDirectory.isEmpty() ? QDir::homePath() : _customDefaultDirectory);
    if (!dir.isEmpty()) {
        _customDefaultDirectory = dir;
        _customPathEdit->setText(dir);
    }
}

void DefaultDirectorySubDlg::onCustomPathChanged(const QString& path)
{
    _customDefaultDirectory = path;
}

// ============================================================================
// RecentFilesHistorySubDlg Implementation
// ============================================================================
RecentFilesHistorySubDlg::RecentFilesHistorySubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void RecentFilesHistorySubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Recent files group
    _recentFilesGroup = new QGroupBox(tr("Recent Files History"), this);
    auto* recentFilesLayout = new QGridLayout(_recentFilesGroup);

    _maxFilesLabel = new QLabel(tr("Max number of recent files:"), _recentFilesGroup);
    recentFilesLayout->addWidget(_maxFilesLabel, 0, 0);

    _maxFilesSpin = new QSpinBox(_recentFilesGroup);
    _maxFilesSpin->setRange(0, 30);
    _maxFilesSpin->setValue(10);
    recentFilesLayout->addWidget(_maxFilesSpin, 0, 1);

    _customLengthLabel = new QLabel(tr("Custom length (0 = unlimited):"), _recentFilesGroup);
    recentFilesLayout->addWidget(_customLengthLabel, 1, 0);

    _customLengthSpin = new QSpinBox(_recentFilesGroup);
    _customLengthSpin->setRange(0, 100);
    _customLengthSpin->setValue(0);
    recentFilesLayout->addWidget(_customLengthSpin, 1, 1);

    recentFilesLayout->setColumnStretch(2, 1);

    _dontCheckAtStartupCheck = new QCheckBox(tr("Don't check at startup"), _recentFilesGroup);
    recentFilesLayout->addWidget(_dontCheckAtStartupCheck, 2, 0, 1, 3);

    mainLayout->addWidget(_recentFilesGroup);
    mainLayout->addStretch();
}

void RecentFilesHistorySubDlg::connectSignals()
{
    connect(_maxFilesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RecentFilesHistorySubDlg::onMaxFilesChanged);
    connect(_customLengthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RecentFilesHistorySubDlg::onCustomLengthChanged);
    connect(_dontCheckAtStartupCheck, &QCheckBox::toggled, this, &RecentFilesHistorySubDlg::onDontCheckAtStartupToggled);
}

void RecentFilesHistorySubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _maxRecentFiles = settings.readInt(L"RecentFilesHistory", L"MaxRecentFiles", 10);
    _customLength = settings.readInt(L"RecentFilesHistory", L"CustomLength", 0);
    _dontCheckAtStartup = settings.readBool(L"RecentFilesHistory", L"DontCheckAtStartup", false);

    _maxFilesSpin->setValue(_maxRecentFiles);
    _customLengthSpin->setValue(_customLength);
    _dontCheckAtStartupCheck->setChecked(_dontCheckAtStartup);
}

void RecentFilesHistorySubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"RecentFilesHistory", L"MaxRecentFiles", _maxRecentFiles);
    settings.writeInt(L"RecentFilesHistory", L"CustomLength", _customLength);
    settings.writeBool(L"RecentFilesHistory", L"DontCheckAtStartup", _dontCheckAtStartup);
}

bool RecentFilesHistorySubDlg::applySettings()
{
    saveSettings();
    return true;
}

void RecentFilesHistorySubDlg::onMaxFilesChanged(int value)
{
    _maxRecentFiles = value;
}

void RecentFilesHistorySubDlg::onCustomLengthChanged(int value)
{
    _customLength = value;
}

void RecentFilesHistorySubDlg::onDontCheckAtStartupToggled(bool checked)
{
    _dontCheckAtStartup = checked;
}

// ============================================================================
// FileAssocSubDlg Implementation
// ============================================================================
FileAssocSubDlg::FileAssocSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void FileAssocSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    _fileAssocGroup = new QGroupBox(tr("File Associations"), this);
    auto* assocLayout = new QHBoxLayout(_fileAssocGroup);

    // Left side - supported extensions
    auto* leftLayout = new QVBoxLayout();
    _supportedExtLabel = new QLabel(tr("Supported extensions:"), _fileAssocGroup);
    leftLayout->addWidget(_supportedExtLabel);

    _extensionList = new QListWidget(_fileAssocGroup);
    leftLayout->addWidget(_extensionList);

    assocLayout->addLayout(leftLayout);

    // Middle - buttons
    auto* buttonLayout = new QVBoxLayout();
    buttonLayout->addStretch();
    _registerButton = new QPushButton(tr(">>"), _fileAssocGroup);
    _registerButton->setFixedWidth(40);
    buttonLayout->addWidget(_registerButton);

    _unregisterButton = new QPushButton(tr("<<"), _fileAssocGroup);
    _unregisterButton->setFixedWidth(40);
    buttonLayout->addWidget(_unregisterButton);
    buttonLayout->addStretch();

    assocLayout->addLayout(buttonLayout);

    // Right side - registered extensions
    auto* rightLayout = new QVBoxLayout();
    _registeredExtLabel = new QLabel(tr("Registered extensions:"), _fileAssocGroup);
    rightLayout->addWidget(_registeredExtLabel);

    _registeredList = new QListWidget(_fileAssocGroup);
    rightLayout->addWidget(_registeredList);

    assocLayout->addLayout(rightLayout);

    mainLayout->addWidget(_fileAssocGroup);
    mainLayout->addStretch();

    populateExtensions();
}

void FileAssocSubDlg::connectSignals()
{
    connect(_registerButton, &QPushButton::clicked, this, &FileAssocSubDlg::onRegisterClicked);
    connect(_unregisterButton, &QPushButton::clicked, this, &FileAssocSubDlg::onUnregisterClicked);
    connect(_extensionList, &QListWidget::currentRowChanged, this, &FileAssocSubDlg::onExtensionSelected);
}

void FileAssocSubDlg::populateExtensions()
{
    QStringList extensions;
    extensions << ".txt" << ".log" << ".ini" << ".cfg" << ".conf"
               << ".xml" << ".html" << ".htm" << ".css" << ".js"
               << ".json" << ".yaml" << ".yml" << ".md" << ".py"
               << ".c" << ".cpp" << ".h" << ".hpp" << ".java"
               << ".cs" << ".php" << ".rb" << ".rs" << ".go"
               << ".sh" << ".bash" << ".sql" << ".lua" << ".pl"
               << ".ts" << ".tsx" << ".jsx" << ".vue" << ".svelte";
    _extensionList->addItems(extensions);
}

void FileAssocSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    QString registered = QString::fromStdWString(settings.readString(L"FileAssoc", L"RegisteredExtensions", L""));
    _registeredExtensions = registered.split(';', Qt::SkipEmptyParts);

    _registeredList->clear();
    _registeredList->addItems(_registeredExtensions);
}

void FileAssocSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeString(L"FileAssoc", L"RegisteredExtensions", _registeredExtensions.join(';').toStdWString());
}

bool FileAssocSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void FileAssocSubDlg::onExtensionSelected(int /*index*/)
{
    // Enable/disable register button based on selection
}

void FileAssocSubDlg::onRegisterClicked()
{
    auto* item = _extensionList->currentItem();
    if (!item)
        return;

    QString ext = item->text();
    if (!_registeredExtensions.contains(ext))
    {
        _registeredExtensions.append(ext);
        _registeredList->addItem(ext);
    }
}

void FileAssocSubDlg::onUnregisterClicked()
{
    int row = _registeredList->currentRow();
    if (row < 0)
        return;

    auto* item = _registeredList->takeItem(row);
    if (item)
    {
        _registeredExtensions.removeAll(item->text());
        delete item;
    }
}

// ============================================================================
// LanguageSubDlg Implementation
// ============================================================================
LanguageSubDlg::LanguageSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void LanguageSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Language list group
    _languageListGroup = new QGroupBox(tr("Language Menu"), this);
    auto* languageListLayout = new QVBoxLayout(_languageListGroup);

    _languageList = new QListWidget(_languageListGroup);
    populateLanguageList();
    languageListLayout->addWidget(_languageList);

    mainLayout->addWidget(_languageListGroup);

    // Indentation group
    _indentationGroup = new QGroupBox(tr("Tab Settings"), this);
    auto* indentationLayout = new QGridLayout(_indentationGroup);

    _tabSizeLabel = new QLabel(tr("Tab size:"), _indentationGroup);
    indentationLayout->addWidget(_tabSizeLabel, 0, 0);

    _tabSizeSpin = new QSpinBox(_indentationGroup);
    _tabSizeSpin->setRange(1, 16);
    _tabSizeSpin->setValue(4);
    indentationLayout->addWidget(_tabSizeSpin, 0, 1);

    _replaceBySpaceCheck = new QCheckBox(tr("Replace by space"), _indentationGroup);
    indentationLayout->addWidget(_replaceBySpaceCheck, 1, 0, 1, 2);

    indentationLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_indentationGroup);
    mainLayout->addStretch();
}

void LanguageSubDlg::populateLanguageList()
{
    QStringList languages;
    languages << "Text" << "ActionScript" << "Ada" << "ASP" << "Assembly" << "AutoIt"
              << "Batch" << "C" << "C++" << "C#" << "CSS" << "D" << "Diff"
              << "Fortran" << "HTML" << "INI file" << "Java" << "JavaScript"
              << "JSON" << "JSP" << "Lua" << "Makefile" << "Markdown" << "Matlab"
              << "Objective-C" << "Pascal" << "Perl" << "PHP" << "PowerShell"
              << "Python" << "R" << "Ruby" << "Rust" << "Shell" << "SQL"
              << "Tcl" << "TypeScript" << "VB" << "VBScript" << "Verilog"
              << "VHDL" << "XML" << "YAML";
    _languageList->addItems(languages);
}

void LanguageSubDlg::connectSignals()
{
    connect(_languageList, &QListWidget::currentRowChanged, this, &LanguageSubDlg::onLanguageSelected);
    connect(_tabSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &LanguageSubDlg::onTabSizeChanged);
    connect(_replaceBySpaceCheck, &QCheckBox::toggled, this, &LanguageSubDlg::onReplaceBySpaceToggled);
}

void LanguageSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _selectedLanguage = settings.readInt(L"Language", L"SelectedLanguage", 0);
    _tabSize = settings.readInt(L"Language", L"TabSize", 4);
    _replaceBySpace = settings.readBool(L"Language", L"ReplaceBySpace", false);

    if (_selectedLanguage < _languageList->count()) {
        _languageList->setCurrentRow(_selectedLanguage);
    }
    _tabSizeSpin->setValue(_tabSize);
    _replaceBySpaceCheck->setChecked(_replaceBySpace);
}

void LanguageSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"Language", L"SelectedLanguage", _selectedLanguage);
    settings.writeInt(L"Language", L"TabSize", _tabSize);
    settings.writeBool(L"Language", L"ReplaceBySpace", _replaceBySpace);
}

bool LanguageSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void LanguageSubDlg::onLanguageSelected(int index)
{
    _selectedLanguage = index;
}

void LanguageSubDlg::onTabSizeChanged(int value)
{
    _tabSize = value;
}

void LanguageSubDlg::onReplaceBySpaceToggled(bool checked)
{
    _replaceBySpace = checked;
}

// ============================================================================
// IndentationSubDlg Implementation
// ============================================================================
IndentationSubDlg::IndentationSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void IndentationSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Tab Settings
    _tabSettingsGroup = new QGroupBox(tr("Tab Settings"), this);
    auto* tabLayout = new QVBoxLayout(_tabSettingsGroup);

    auto* tabSizeLayout = new QHBoxLayout();
    _tabSizeLabel = new QLabel(tr("Tab size:"), _tabSettingsGroup);
    tabSizeLayout->addWidget(_tabSizeLabel);

    _tabSizeSpin = new QSpinBox(_tabSettingsGroup);
    _tabSizeSpin->setRange(1, 16);
    _tabSizeSpin->setValue(4);
    tabSizeLayout->addWidget(_tabSizeSpin);
    tabSizeLayout->addStretch();
    tabLayout->addLayout(tabSizeLayout);

    _replaceBySpaceCheck = new QCheckBox(tr("Replace by space"), _tabSettingsGroup);
    tabLayout->addWidget(_replaceBySpaceCheck);

    _backspaceUnindentCheck = new QCheckBox(tr("Backspace unindent"), _tabSettingsGroup);
    tabLayout->addWidget(_backspaceUnindentCheck);

    mainLayout->addWidget(_tabSettingsGroup);

    // Per-language tab settings
    _tabPerLanguageGroup = new QGroupBox(tr("Per-Language Tab Settings"), this);
    auto* perLangLayout = new QVBoxLayout(_tabPerLanguageGroup);

    _tabSettingLanguageList = new QListWidget(_tabPerLanguageGroup);
    _tabSettingLanguageList->setMaximumHeight(150);
    perLangLayout->addWidget(_tabSettingLanguageList);
    populateLanguageList();

    mainLayout->addWidget(_tabPerLanguageGroup);

    // Auto-Indent
    _autoIndentGroup = new QGroupBox(tr("Auto-Indent"), this);
    auto* autoIndentLayout = new QHBoxLayout(_autoIndentGroup);

    _autoIndentLabel = new QLabel(tr("Mode:"), _autoIndentGroup);
    autoIndentLayout->addWidget(_autoIndentLabel);

    _autoIndentCombo = new QComboBox(_autoIndentGroup);
    _autoIndentCombo->addItem(tr("None"));
    _autoIndentCombo->addItem(tr("Basic"));
    _autoIndentCombo->addItem(tr("Advanced (C-like languages and Python)"));
    autoIndentLayout->addWidget(_autoIndentCombo);
    autoIndentLayout->addStretch();

    mainLayout->addWidget(_autoIndentGroup);
    mainLayout->addStretch();
}

void IndentationSubDlg::connectSignals()
{
    connect(_tabSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &IndentationSubDlg::onTabSizeChanged);
    connect(_replaceBySpaceCheck, &QCheckBox::toggled, this, &IndentationSubDlg::onReplaceBySpaceToggled);
    connect(_backspaceUnindentCheck, &QCheckBox::toggled, this, &IndentationSubDlg::onBackspaceUnindentToggled);
    connect(_autoIndentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IndentationSubDlg::onAutoIndentModeChanged);
    connect(_tabSettingLanguageList, &QListWidget::currentRowChanged, this, &IndentationSubDlg::onTabSettingLanguageChanged);
}

void IndentationSubDlg::populateLanguageList()
{
    _tabSettingLanguageList->addItem(tr("[Default]"));
    QStringList languages;
    languages << "C" << "C++" << "Java" << "C#" << "Python" << "JavaScript"
              << "TypeScript" << "PHP" << "Rust" << "Go" << "Ruby" << "Perl"
              << "HTML" << "XML" << "CSS" << "SQL" << "Bash" << "Makefile"
              << "Lua" << "JSON" << "YAML" << "Markdown";
    _tabSettingLanguageList->addItems(languages);
    _tabSettingLanguageList->setCurrentRow(0);
}

void IndentationSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _tabSize = settings.readInt(L"Indentation", L"TabSize", 4);
    _replaceBySpace = settings.readBool(L"Indentation", L"ReplaceBySpace", false);
    _backspaceUnindent = settings.readBool(L"Indentation", L"BackspaceUnindent", false);
    _autoIndentMode = settings.readInt(L"Indentation", L"AutoIndentMode", 2);

    _tabSizeSpin->setValue(_tabSize);
    _replaceBySpaceCheck->setChecked(_replaceBySpace);
    _backspaceUnindentCheck->setChecked(_backspaceUnindent);
    _autoIndentCombo->setCurrentIndex(_autoIndentMode);
}

void IndentationSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"Indentation", L"TabSize", _tabSize);
    settings.writeBool(L"Indentation", L"ReplaceBySpace", _replaceBySpace);
    settings.writeBool(L"Indentation", L"BackspaceUnindent", _backspaceUnindent);
    settings.writeInt(L"Indentation", L"AutoIndentMode", _autoIndentMode);
}

bool IndentationSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void IndentationSubDlg::onTabSizeChanged(int value) { _tabSize = value; }
void IndentationSubDlg::onReplaceBySpaceToggled(bool checked) { _replaceBySpace = checked; }
void IndentationSubDlg::onBackspaceUnindentToggled(bool checked) { _backspaceUnindent = checked; }
void IndentationSubDlg::onAutoIndentModeChanged(int index) { _autoIndentMode = index; }

void IndentationSubDlg::onTabSettingLanguageChanged(int /*index*/)
{
    // When a specific language is selected from the list, we could load
    // per-language tab settings. For now, we use the default settings.
}

// ============================================================================
// HighlightingSubDlg Implementation
// ============================================================================
HighlightingSubDlg::HighlightingSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void HighlightingSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Smart Highlighting group
    _smartHighlightGroup = new QGroupBox(tr("Smart Highlighting"), this);
    auto* smartHighlightLayout = new QVBoxLayout(_smartHighlightGroup);

    _enableSmartHighlightCheck = new QCheckBox(tr("Enable smart highlighting"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_enableSmartHighlightCheck);

    _matchCaseCheck = new QCheckBox(tr("Match case"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_matchCaseCheck);

    _wholeWordCheck = new QCheckBox(tr("Match whole word only"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_wholeWordCheck);

    _matchSelectionCheck = new QCheckBox(tr("Use selection highlighting on another view"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_matchSelectionCheck);

    mainLayout->addWidget(_smartHighlightGroup);

    // Matching group
    _matchingGroup = new QGroupBox(tr("Brace and Tag Matching"), this);
    auto* matchingLayout = new QVBoxLayout(_matchingGroup);

    _braceHighlightCheck = new QCheckBox(tr("Enable brace matching"), _matchingGroup);
    matchingLayout->addWidget(_braceHighlightCheck);

    _tagHighlightCheck = new QCheckBox(tr("Enable tag attribute matching (HTML/XML)"), _matchingGroup);
    matchingLayout->addWidget(_tagHighlightCheck);

    mainLayout->addWidget(_matchingGroup);
    mainLayout->addStretch();
}

void HighlightingSubDlg::connectSignals()
{
    connect(_enableSmartHighlightCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onSmartHighlightingToggled);
    connect(_matchCaseCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onMatchCaseToggled);
    connect(_wholeWordCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onWholeWordToggled);
    connect(_matchSelectionCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onMatchSelectionToggled);
    connect(_braceHighlightCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onBraceHighlightingToggled);
    connect(_tagHighlightCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onTagHighlightingToggled);
}

void HighlightingSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _enableSmartHighlight = settings.readBool(L"Highlighting", L"EnableSmartHighlight", true);
    _smartHighlightMatchCase = settings.readBool(L"Highlighting", L"SmartHighlightMatchCase", false);
    _smartHighlightWholeWord = settings.readBool(L"Highlighting", L"SmartHighlightWholeWord", true);
    _smartHighlightUseSelection = settings.readBool(L"Highlighting", L"SmartHighlightUseSelection", false);
    _enableBraceHighlight = settings.readBool(L"Highlighting", L"EnableBraceHighlight", true);
    _enableTagHighlight = settings.readBool(L"Highlighting", L"EnableTagHighlight", true);

    _enableSmartHighlightCheck->setChecked(_enableSmartHighlight);
    _matchCaseCheck->setChecked(_smartHighlightMatchCase);
    _wholeWordCheck->setChecked(_smartHighlightWholeWord);
    _matchSelectionCheck->setChecked(_smartHighlightUseSelection);
    _braceHighlightCheck->setChecked(_enableBraceHighlight);
    _tagHighlightCheck->setChecked(_enableTagHighlight);
}

void HighlightingSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Highlighting", L"EnableSmartHighlight", _enableSmartHighlight);
    settings.writeBool(L"Highlighting", L"SmartHighlightMatchCase", _smartHighlightMatchCase);
    settings.writeBool(L"Highlighting", L"SmartHighlightWholeWord", _smartHighlightWholeWord);
    settings.writeBool(L"Highlighting", L"SmartHighlightUseSelection", _smartHighlightUseSelection);
    settings.writeBool(L"Highlighting", L"EnableBraceHighlight", _enableBraceHighlight);
    settings.writeBool(L"Highlighting", L"EnableTagHighlight", _enableTagHighlight);
}

bool HighlightingSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void HighlightingSubDlg::onSmartHighlightingToggled(bool checked)
{
    _enableSmartHighlight = checked;
    _matchCaseCheck->setEnabled(checked);
    _wholeWordCheck->setEnabled(checked);
    _matchSelectionCheck->setEnabled(checked);
}

void HighlightingSubDlg::onMatchCaseToggled(bool checked)
{
    _smartHighlightMatchCase = checked;
}

void HighlightingSubDlg::onWholeWordToggled(bool checked)
{
    _smartHighlightWholeWord = checked;
}

void HighlightingSubDlg::onMatchSelectionToggled(bool checked)
{
    _smartHighlightUseSelection = checked;
}

void HighlightingSubDlg::onBraceHighlightingToggled(bool checked)
{
    _enableBraceHighlight = checked;
}

void HighlightingSubDlg::onTagHighlightingToggled(bool checked)
{
    _enableTagHighlight = checked;
}

// ============================================================================
// PrintSubDlg Implementation
// ============================================================================
PrintSubDlg::PrintSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void PrintSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Print Options group
    _printOptionsGroup = new QGroupBox(tr("Print Options"), this);
    auto* printOptionsLayout = new QVBoxLayout(_printOptionsGroup);

    _printLineNumberCheck = new QCheckBox(tr("Print line number"), _printOptionsGroup);
    printOptionsLayout->addWidget(_printLineNumberCheck);

    _printSelectionCheck = new QCheckBox(tr("Print selection only (when printing selected text)"), _printOptionsGroup);
    printOptionsLayout->addWidget(_printSelectionCheck);

    _noBackgroundCheck = new QCheckBox(tr("No background color"), _printOptionsGroup);
    printOptionsLayout->addWidget(_noBackgroundCheck);

    mainLayout->addWidget(_printOptionsGroup);

    // Header/Footer group
    _headerFooterGroup = new QGroupBox(tr("Header and Footer"), this);
    auto* headerFooterLayout = new QGridLayout(_headerFooterGroup);

    _enableHeaderFooterCheck = new QCheckBox(tr("Enable header and footer"), _headerFooterGroup);
    headerFooterLayout->addWidget(_enableHeaderFooterCheck, 0, 0, 1, 3);

    // Header
    _headerLeftLabel = new QLabel(tr("Header left:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_headerLeftLabel, 1, 0);
    _headerLeftEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_headerLeftEdit, 1, 1, 1, 2);

    _headerCenterLabel = new QLabel(tr("Header center:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_headerCenterLabel, 2, 0);
    _headerCenterEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_headerCenterEdit, 2, 1, 1, 2);

    _headerRightLabel = new QLabel(tr("Header right:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_headerRightLabel, 3, 0);
    _headerRightEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_headerRightEdit, 3, 1, 1, 2);

    // Footer
    _footerLeftLabel = new QLabel(tr("Footer left:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_footerLeftLabel, 4, 0);
    _footerLeftEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_footerLeftEdit, 4, 1, 1, 2);

    _footerCenterLabel = new QLabel(tr("Footer center:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_footerCenterLabel, 5, 0);
    _footerCenterEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_footerCenterEdit, 5, 1, 1, 2);

    _footerRightLabel = new QLabel(tr("Footer right:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_footerRightLabel, 6, 0);
    _footerRightEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_footerRightEdit, 6, 1, 1, 2);

    headerFooterLayout->setColumnStretch(1, 1);

    mainLayout->addWidget(_headerFooterGroup);
    mainLayout->addStretch();
}

void PrintSubDlg::connectSignals()
{
    connect(_printLineNumberCheck, &QCheckBox::toggled, this, &PrintSubDlg::onPrintLineNumberToggled);
    connect(_printSelectionCheck, &QCheckBox::toggled, this, &PrintSubDlg::onPrintSelectionToggled);
    connect(_noBackgroundCheck, &QCheckBox::toggled, this, &PrintSubDlg::onNoBackgroundToggled);
    connect(_enableHeaderFooterCheck, &QCheckBox::toggled, this, &PrintSubDlg::onHeaderFooterToggled);
    connect(_headerLeftEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onHeaderLeftChanged);
    connect(_headerCenterEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onHeaderCenterChanged);
    connect(_headerRightEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onHeaderRightChanged);
    connect(_footerLeftEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onFooterLeftChanged);
    connect(_footerCenterEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onFooterCenterChanged);
    connect(_footerRightEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onFooterRightChanged);
}

void PrintSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _printLineNumber = settings.readBool(L"Print", L"PrintLineNumber", true);
    _printSelection = settings.readBool(L"Print", L"PrintSelection", false);
    _noBackground = settings.readBool(L"Print", L"NoBackground", true);
    _headerFooterEnabled = settings.readBool(L"Print", L"HeaderFooterEnabled", true);
    _headerLeft = QString::fromStdWString(settings.readString(L"Print", L"HeaderLeft", L""));
    _headerCenter = QString::fromStdWString(settings.readString(L"Print", L"HeaderCenter", L""));
    _headerRight = QString::fromStdWString(settings.readString(L"Print", L"HeaderRight", L""));
    _footerLeft = QString::fromStdWString(settings.readString(L"Print", L"FooterLeft", L""));
    _footerCenter = QString::fromStdWString(settings.readString(L"Print", L"FooterCenter", L""));
    _footerRight = QString::fromStdWString(settings.readString(L"Print", L"FooterRight", L""));

    _printLineNumberCheck->setChecked(_printLineNumber);
    _printSelectionCheck->setChecked(_printSelection);
    _noBackgroundCheck->setChecked(_noBackground);
    _enableHeaderFooterCheck->setChecked(_headerFooterEnabled);
    _headerLeftEdit->setText(_headerLeft);
    _headerCenterEdit->setText(_headerCenter);
    _headerRightEdit->setText(_headerRight);
    _footerLeftEdit->setText(_footerLeft);
    _footerCenterEdit->setText(_footerCenter);
    _footerRightEdit->setText(_footerRight);
}

void PrintSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Print", L"PrintLineNumber", _printLineNumber);
    settings.writeBool(L"Print", L"PrintSelection", _printSelection);
    settings.writeBool(L"Print", L"NoBackground", _noBackground);
    settings.writeBool(L"Print", L"HeaderFooterEnabled", _headerFooterEnabled);
    settings.writeString(L"Print", L"HeaderLeft", _headerLeft.toStdWString());
    settings.writeString(L"Print", L"HeaderCenter", _headerCenter.toStdWString());
    settings.writeString(L"Print", L"HeaderRight", _headerRight.toStdWString());
    settings.writeString(L"Print", L"FooterLeft", _footerLeft.toStdWString());
    settings.writeString(L"Print", L"FooterCenter", _footerCenter.toStdWString());
    settings.writeString(L"Print", L"FooterRight", _footerRight.toStdWString());
}

bool PrintSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void PrintSubDlg::onPrintLineNumberToggled(bool checked)
{
    _printLineNumber = checked;
}

void PrintSubDlg::onPrintSelectionToggled(bool checked)
{
    _printSelection = checked;
}

void PrintSubDlg::onNoBackgroundToggled(bool checked)
{
    _noBackground = checked;
}

void PrintSubDlg::onHeaderFooterToggled(bool checked)
{
    _headerFooterEnabled = checked;
}

void PrintSubDlg::onHeaderLeftChanged(const QString& text)
{
    _headerLeft = text;
}

void PrintSubDlg::onHeaderCenterChanged(const QString& text)
{
    _headerCenter = text;
}

void PrintSubDlg::onHeaderRightChanged(const QString& text)
{
    _headerRight = text;
}

void PrintSubDlg::onFooterLeftChanged(const QString& text)
{
    _footerLeft = text;
}

void PrintSubDlg::onFooterCenterChanged(const QString& text)
{
    _footerCenter = text;
}

void PrintSubDlg::onFooterRightChanged(const QString& text)
{
    _footerRight = text;
}

// ============================================================================
// SearchingSubDlg Implementation
// ============================================================================
SearchingSubDlg::SearchingSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void SearchingSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Search Options group
    _searchOptionsGroup = new QGroupBox(tr("Search Options"), this);
    auto* searchOptionsLayout = new QVBoxLayout(_searchOptionsGroup);

    _stopAtFirstMatchCheck = new QCheckBox(tr("Stop at first match (incremental search)"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_stopAtFirstMatchCheck);

    _wrapAroundCheck = new QCheckBox(tr("Wrap around"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_wrapAroundCheck);

    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only (find only)"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_matchWholeWordCheck);

    _matchCaseCheck = new QCheckBox(tr("Match case (find only)"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_matchCaseCheck);

    mainLayout->addWidget(_searchOptionsGroup);

    // Advanced group
    _advancedGroup = new QGroupBox(tr("Advanced"), this);
    auto* advancedLayout = new QGridLayout(_advancedGroup);

    _inSelectionThresholdLabel = new QLabel(tr("In-selection threshold (characters):"), _advancedGroup);
    advancedLayout->addWidget(_inSelectionThresholdLabel, 0, 0);

    _inSelectionThresholdSpin = new QSpinBox(_advancedGroup);
    _inSelectionThresholdSpin->setRange(0, 10000);
    _inSelectionThresholdSpin->setSingleStep(128);
    _inSelectionThresholdSpin->setValue(1024);
    advancedLayout->addWidget(_inSelectionThresholdSpin, 0, 1);

    _fillFindWhatCheck = new QCheckBox(tr("Fill Find what field with selected text"), _advancedGroup);
    advancedLayout->addWidget(_fillFindWhatCheck, 1, 0, 1, 2);

    advancedLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_advancedGroup);
    mainLayout->addStretch();
}

void SearchingSubDlg::connectSignals()
{
    connect(_stopAtFirstMatchCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onStopAtFirstMatchToggled);
    connect(_wrapAroundCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onWrapAroundToggled);
    connect(_matchWholeWordCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onMatchWholeWordToggled);
    connect(_matchCaseCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onMatchCaseToggled);
    connect(_inSelectionThresholdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SearchingSubDlg::onInSelectionThresholdChanged);
    connect(_fillFindWhatCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onFillFindWhatToggled);
}

void SearchingSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _stopAtFirstMatch = settings.readBool(L"Searching", L"StopAtFirstMatch", false);
    _wrapAround = settings.readBool(L"Searching", L"WrapAround", true);
    _matchWholeWord = settings.readBool(L"Searching", L"MatchWholeWord", false);
    _matchCase = settings.readBool(L"Searching", L"MatchCase", false);
    _inSelectionThreshold = settings.readInt(L"Searching", L"InSelectionThreshold", 1024);
    _fillFindWhatWithSelection = settings.readBool(L"Searching", L"FillFindWhatWithSelection", true);

    _stopAtFirstMatchCheck->setChecked(_stopAtFirstMatch);
    _wrapAroundCheck->setChecked(_wrapAround);
    _matchWholeWordCheck->setChecked(_matchWholeWord);
    _matchCaseCheck->setChecked(_matchCase);
    _inSelectionThresholdSpin->setValue(_inSelectionThreshold);
    _fillFindWhatCheck->setChecked(_fillFindWhatWithSelection);
}

void SearchingSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Searching", L"StopAtFirstMatch", _stopAtFirstMatch);
    settings.writeBool(L"Searching", L"WrapAround", _wrapAround);
    settings.writeBool(L"Searching", L"MatchWholeWord", _matchWholeWord);
    settings.writeBool(L"Searching", L"MatchCase", _matchCase);
    settings.writeInt(L"Searching", L"InSelectionThreshold", _inSelectionThreshold);
    settings.writeBool(L"Searching", L"FillFindWhatWithSelection", _fillFindWhatWithSelection);
}

bool SearchingSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void SearchingSubDlg::onStopAtFirstMatchToggled(bool checked)
{
    _stopAtFirstMatch = checked;
}

void SearchingSubDlg::onWrapAroundToggled(bool checked)
{
    _wrapAround = checked;
}

void SearchingSubDlg::onMatchWholeWordToggled(bool checked)
{
    _matchWholeWord = checked;
}

void SearchingSubDlg::onMatchCaseToggled(bool checked)
{
    _matchCase = checked;
}

void SearchingSubDlg::onInSelectionThresholdChanged(int value)
{
    _inSelectionThreshold = value;
}

void SearchingSubDlg::onFillFindWhatToggled(bool checked)
{
    _fillFindWhatWithSelection = checked;
}

// ============================================================================
// BackupSubDlg Implementation
// ============================================================================
BackupSubDlg::BackupSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void BackupSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Backup group
    _backupGroup = new QGroupBox(tr("Backup"), this);
    auto* backupLayout = new QVBoxLayout(_backupGroup);

    _backupModeLabel = new QLabel(tr("Backup mode:"), _backupGroup);
    backupLayout->addWidget(_backupModeLabel);

    _backupModeCombo = new QComboBox(_backupGroup);
    _backupModeCombo->addItem(tr("None"));
    _backupModeCombo->addItem(tr("Simple backup"));
    _backupModeCombo->addItem(tr("Verbose backup"));
    backupLayout->addWidget(_backupModeCombo);

    mainLayout->addWidget(_backupGroup);

    // Backup Directory group
    _backupDirectoryGroup = new QGroupBox(tr("Backup Directory"), this);
    auto* backupDirectoryLayout = new QVBoxLayout(_backupDirectoryGroup);

    _backupDirectoryCombo = new QComboBox(_backupDirectoryGroup);
    _backupDirectoryCombo->addItem(tr("Same directory as current file"));
    _backupDirectoryCombo->addItem(tr("Custom directory:"));
    backupDirectoryLayout->addWidget(_backupDirectoryCombo);

    auto* customDirLayout = new QHBoxLayout();
    _customDirectoryEdit = new QLineEdit(_backupDirectoryGroup);
    _customDirectoryEdit->setPlaceholderText(tr("Enter custom backup directory..."));
    customDirLayout->addWidget(_customDirectoryEdit);

    _browseButton = new QPushButton(tr("Browse..."), _backupDirectoryGroup);
    customDirLayout->addWidget(_browseButton);

    backupDirectoryLayout->addLayout(customDirLayout);
    mainLayout->addWidget(_backupDirectoryGroup);

    // Session Snapshot group
    _sessionGroup = new QGroupBox(tr("Session Snapshot"), this);
    auto* sessionLayout = new QGridLayout(_sessionGroup);

    _sessionSnapshotCheck = new QCheckBox(tr("Enable session snapshot and periodic backup"), _sessionGroup);
    sessionLayout->addWidget(_sessionSnapshotCheck, 0, 0, 1, 3);

    _snapshotIntervalLabel = new QLabel(tr("Backup interval (seconds):"), _sessionGroup);
    sessionLayout->addWidget(_snapshotIntervalLabel, 1, 0);

    _snapshotIntervalSpin = new QSpinBox(_sessionGroup);
    _snapshotIntervalSpin->setRange(1, 600);
    _snapshotIntervalSpin->setValue(7);
    sessionLayout->addWidget(_snapshotIntervalSpin, 1, 1);

    sessionLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_sessionGroup);
    mainLayout->addStretch();
}

void BackupSubDlg::connectSignals()
{
    connect(_backupModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackupSubDlg::onBackupModeChanged);
    connect(_backupDirectoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackupSubDlg::onBackupDirectoryTypeChanged);
    connect(_customDirectoryEdit, &QLineEdit::textChanged, this, &BackupSubDlg::onCustomDirectoryChanged);
    connect(_browseButton, &QPushButton::clicked, this, &BackupSubDlg::onBrowseDirectoryClicked);
    connect(_sessionSnapshotCheck, &QCheckBox::toggled, this, &BackupSubDlg::onSessionSnapshotToggled);
}

void BackupSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _backupMode = settings.readInt(L"Backup", L"BackupMode", 0);
    _backupDirectoryType = settings.readInt(L"Backup", L"BackupDirectoryType", 0);
    _customBackupDirectory = QString::fromStdWString(settings.readString(L"Backup", L"CustomBackupDirectory", L""));
    _sessionSnapshot = settings.readBool(L"Backup", L"SessionSnapshot", true);
    _snapshotInterval = settings.readInt(L"Backup", L"SnapshotInterval", 7);

    _backupModeCombo->setCurrentIndex(_backupMode);
    _backupDirectoryCombo->setCurrentIndex(_backupDirectoryType);
    _customDirectoryEdit->setText(_customBackupDirectory);
    _sessionSnapshotCheck->setChecked(_sessionSnapshot);
    _snapshotIntervalSpin->setValue(_snapshotInterval);

    updateBackupUI();
}

void BackupSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"Backup", L"BackupMode", _backupMode);
    settings.writeInt(L"Backup", L"BackupDirectoryType", _backupDirectoryType);
    settings.writeString(L"Backup", L"CustomBackupDirectory", _customBackupDirectory.toStdWString());
    settings.writeBool(L"Backup", L"SessionSnapshot", _sessionSnapshot);
    settings.writeInt(L"Backup", L"SnapshotInterval", _snapshotInterval);
}

bool BackupSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void BackupSubDlg::onBackupModeChanged(int index)
{
    _backupMode = index;
    updateBackupUI();
}

void BackupSubDlg::onBackupDirectoryTypeChanged(int index)
{
    _backupDirectoryType = index;
    bool isCustom = (index == 1);
    _customDirectoryEdit->setEnabled(isCustom);
    _browseButton->setEnabled(isCustom);
}

void BackupSubDlg::onCustomDirectoryChanged(const QString& path)
{
    _customBackupDirectory = path;
}

void BackupSubDlg::onBrowseDirectoryClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Backup Directory"),
                                                    _customBackupDirectory.isEmpty() ? QDir::homePath() : _customBackupDirectory);
    if (!dir.isEmpty()) {
        _customBackupDirectory = dir;
        _customDirectoryEdit->setText(dir);
    }
}

void BackupSubDlg::onSessionSnapshotToggled(bool checked)
{
    _sessionSnapshot = checked;
    _snapshotIntervalSpin->setEnabled(checked);
    _snapshotIntervalLabel->setEnabled(checked);
}

void BackupSubDlg::updateBackupUI()
{
    bool isBackupEnabled = (_backupMode != 0);
    _backupDirectoryGroup->setEnabled(isBackupEnabled);
    bool isCustom = (_backupDirectoryType == 1);
    _customDirectoryEdit->setEnabled(isBackupEnabled && isCustom);
    _browseButton->setEnabled(isBackupEnabled && isCustom);
}

// ============================================================================
// AutoCompletionSubDlg Implementation
// ============================================================================
AutoCompletionSubDlg::AutoCompletionSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void AutoCompletionSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Auto-Completion group
    _autoCompletionGroup = new QGroupBox(tr("Auto-Completion"), this);
    auto* autoCompletionLayout = new QGridLayout(_autoCompletionGroup);

    _enableAutoCompletionCheck = new QCheckBox(tr("Enable auto-completion on each input"), _autoCompletionGroup);
    autoCompletionLayout->addWidget(_enableAutoCompletionCheck, 0, 0, 1, 3);

    _thresholdLabel = new QLabel(tr("From the th character:"), _autoCompletionGroup);
    autoCompletionLayout->addWidget(_thresholdLabel, 1, 0);

    _thresholdSpin = new QSpinBox(_autoCompletionGroup);
    _thresholdSpin->setRange(1, 9);
    _thresholdSpin->setValue(1);
    autoCompletionLayout->addWidget(_thresholdSpin, 1, 1);

    autoCompletionLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_autoCompletionGroup);

    // Auto-Insert group
    _autoInsertGroup = new QGroupBox(tr("Auto-Insert"), this);
    auto* autoInsertLayout = new QVBoxLayout(_autoInsertGroup);

    _autoInsertBracketsCheck = new QCheckBox(tr("Auto-insert matching brackets ( ) [ ] { }"), _autoInsertGroup);
    autoInsertLayout->addWidget(_autoInsertBracketsCheck);

    _autoInsertQuotesCheck = new QCheckBox(tr("Auto-insert matching quotes ' \" `"), _autoInsertGroup);
    autoInsertLayout->addWidget(_autoInsertQuotesCheck);

    mainLayout->addWidget(_autoInsertGroup);

    // Auto-Indent group
    _autoIndentGroup = new QGroupBox(tr("Auto-Indent"), this);
    auto* autoIndentLayout = new QVBoxLayout(_autoIndentGroup);

    _enableAutoIndentCheck = new QCheckBox(tr("Enable auto-indent"), _autoIndentGroup);
    autoIndentLayout->addWidget(_enableAutoIndentCheck);

    mainLayout->addWidget(_autoIndentGroup);

    // Completion Source group
    _completionSourceGroup = new QGroupBox(tr("Completion Source"), this);
    auto* completionSourceLayout = new QVBoxLayout(_completionSourceGroup);

    _completionSourceCombo = new QComboBox(_completionSourceGroup);
    _completionSourceCombo->addItem(tr("Word and function completion"));
    _completionSourceCombo->addItem(tr("Word completion only"));
    _completionSourceCombo->addItem(tr("Function completion only"));
    completionSourceLayout->addWidget(_completionSourceCombo);

    _ignoreNumbersCheck = new QCheckBox(tr("Ignore numbers"), _completionSourceGroup);
    completionSourceLayout->addWidget(_ignoreNumbersCheck);

    mainLayout->addWidget(_completionSourceGroup);
    mainLayout->addStretch();
}

void AutoCompletionSubDlg::connectSignals()
{
    connect(_enableAutoCompletionCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoCompletionToggled);
    connect(_autoInsertBracketsCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoInsertBracketsToggled);
    connect(_autoInsertQuotesCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoInsertQuotesToggled);
    connect(_enableAutoIndentCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoIndentToggled);
    connect(_completionSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoCompletionSubDlg::onAutoCompletionFromChanged);
    connect(_thresholdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AutoCompletionSubDlg::onAutoCompletionThresholdChanged);
    connect(_ignoreNumbersCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoCompletionIgnoreNumbersToggled);
}

void AutoCompletionSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _enableAutoCompletion = settings.readBool(L"AutoCompletion", L"EnableAutoCompletion", true);
    _autoCompletionThreshold = settings.readInt(L"AutoCompletion", L"AutoCompletionThreshold", 1);
    _autoInsertBrackets = settings.readBool(L"AutoCompletion", L"AutoInsertBrackets", true);
    _autoInsertQuotes = settings.readBool(L"AutoCompletion", L"AutoInsertQuotes", true);
    _enableAutoIndent = settings.readBool(L"AutoCompletion", L"EnableAutoIndent", true);
    _autoCompletionSource = settings.readInt(L"AutoCompletion", L"AutoCompletionSource", 0);
    _autoCompletionIgnoreNumbers = settings.readBool(L"AutoCompletion", L"AutoCompletionIgnoreNumbers", false);

    _enableAutoCompletionCheck->setChecked(_enableAutoCompletion);
    _thresholdSpin->setValue(_autoCompletionThreshold);
    _autoInsertBracketsCheck->setChecked(_autoInsertBrackets);
    _autoInsertQuotesCheck->setChecked(_autoInsertQuotes);
    _enableAutoIndentCheck->setChecked(_enableAutoIndent);
    _completionSourceCombo->setCurrentIndex(_autoCompletionSource);
    _ignoreNumbersCheck->setChecked(_autoCompletionIgnoreNumbers);

    // Sync to NppGUI so AutoCompletion engine reads correct values
    NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();
    if (_enableAutoCompletion)
    {
        switch (_autoCompletionSource)
        {
            case 0: nppGUI._autocStatus = NppGUI::autoc_both; break;
            case 1: nppGUI._autocStatus = NppGUI::autoc_word; break;
            case 2: nppGUI._autocStatus = NppGUI::autoc_func; break;
            default: nppGUI._autocStatus = NppGUI::autoc_both; break;
        }
    }
    else
    {
        nppGUI._autocStatus = NppGUI::autoc_none;
    }
    nppGUI._autocFromLen = static_cast<UINT>(_autoCompletionThreshold);
    nppGUI._autocIgnoreNumbers = _autoCompletionIgnoreNumbers;
}

void AutoCompletionSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"AutoCompletion", L"EnableAutoCompletion", _enableAutoCompletion);
    settings.writeInt(L"AutoCompletion", L"AutoCompletionThreshold", _autoCompletionThreshold);
    settings.writeBool(L"AutoCompletion", L"AutoInsertBrackets", _autoInsertBrackets);
    settings.writeBool(L"AutoCompletion", L"AutoInsertQuotes", _autoInsertQuotes);
    settings.writeBool(L"AutoCompletion", L"EnableAutoIndent", _enableAutoIndent);
    settings.writeInt(L"AutoCompletion", L"AutoCompletionSource", _autoCompletionSource);
    settings.writeBool(L"AutoCompletion", L"AutoCompletionIgnoreNumbers", _autoCompletionIgnoreNumbers);
}

bool AutoCompletionSubDlg::applySettings()
{
    saveSettings();

    // Sync settings to NppGUI so the AutoCompletion engine reads correct values
    NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();
    if (_enableAutoCompletion)
    {
        // Map combo box index to autocomplete status
        // 0 = word + function, 1 = word only, 2 = function only
        switch (_autoCompletionSource)
        {
            case 0: nppGUI._autocStatus = NppGUI::autoc_both; break;
            case 1: nppGUI._autocStatus = NppGUI::autoc_word; break;
            case 2: nppGUI._autocStatus = NppGUI::autoc_func; break;
            default: nppGUI._autocStatus = NppGUI::autoc_both; break;
        }
    }
    else
    {
        nppGUI._autocStatus = NppGUI::autoc_none;
    }
    nppGUI._autocFromLen = static_cast<UINT>(_autoCompletionThreshold);
    nppGUI._autocIgnoreNumbers = _autoCompletionIgnoreNumbers;

    return true;
}

void AutoCompletionSubDlg::onAutoCompletionToggled(bool checked)
{
    _enableAutoCompletion = checked;
    _thresholdSpin->setEnabled(checked);
    _thresholdLabel->setEnabled(checked);
}

void AutoCompletionSubDlg::onAutoInsertBracketsToggled(bool checked)
{
    _autoInsertBrackets = checked;
}

void AutoCompletionSubDlg::onAutoInsertQuotesToggled(bool checked)
{
    _autoInsertQuotes = checked;
}

void AutoCompletionSubDlg::onAutoIndentToggled(bool checked)
{
    _enableAutoIndent = checked;
}

void AutoCompletionSubDlg::onAutoCompletionFromChanged(int index)
{
    _autoCompletionSource = index;
}

void AutoCompletionSubDlg::onAutoCompletionThresholdChanged(int value)
{
    _autoCompletionThreshold = value;
}

void AutoCompletionSubDlg::onAutoCompletionIgnoreNumbersToggled(bool checked)
{
    _autoCompletionIgnoreNumbers = checked;
}

// ============================================================================
// MultiInstanceSubDlg Implementation
// ============================================================================
MultiInstanceSubDlg::MultiInstanceSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void MultiInstanceSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Multi-Instance group
    _multiInstanceGroup = new QGroupBox(tr("Multi-Instance"), this);
    auto* multiInstanceLayout = new QVBoxLayout(_multiInstanceGroup);

    _multiInstanceLabel = new QLabel(tr("Open files in:"), _multiInstanceGroup);
    multiInstanceLayout->addWidget(_multiInstanceLabel);

    _multiInstanceCombo = new QComboBox(_multiInstanceGroup);
    _multiInstanceCombo->addItem(tr("Mono-instance (default) - Open all files in same instance"));
    _multiInstanceCombo->addItem(tr("Multi-instance - Each file opens in new instance"));
    _multiInstanceCombo->addItem(tr("Always in multi-instance mode"));
    multiInstanceLayout->addWidget(_multiInstanceCombo);

    mainLayout->addWidget(_multiInstanceGroup);

    // Date/Time Format group
    _dateTimeFormatGroup = new QGroupBox(tr("Date/Time Format"), this);
    auto* dateTimeFormatLayout = new QVBoxLayout(_dateTimeFormatGroup);

    _useCustomDateTimeCheck = new QCheckBox(tr("Use custom date/time format"), _dateTimeFormatGroup);
    dateTimeFormatLayout->addWidget(_useCustomDateTimeCheck);

    _dateTimeFormatEdit = new QLineEdit(_dateTimeFormatGroup);
    _dateTimeFormatEdit->setPlaceholderText(tr("yyyy-MM-dd HH:mm:ss"));
    dateTimeFormatLayout->addWidget(_dateTimeFormatEdit);

    _dateTimePreviewLabel = new QLabel(tr("Preview: "), _dateTimeFormatGroup);
    dateTimeFormatLayout->addWidget(_dateTimePreviewLabel);

    mainLayout->addWidget(_dateTimeFormatGroup);
    mainLayout->addStretch();
}

void MultiInstanceSubDlg::connectSignals()
{
    connect(_multiInstanceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MultiInstanceSubDlg::onMultiInstanceModeChanged);
    connect(_dateTimeFormatEdit, &QLineEdit::textChanged, this, &MultiInstanceSubDlg::onDateTimeFormatChanged);
    connect(_useCustomDateTimeCheck, &QCheckBox::toggled, this, &MultiInstanceSubDlg::onUseCustomDateTimeToggled);
}

void MultiInstanceSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _multiInstanceMode = settings.readInt(L"MultiInstance", L"MultiInstanceMode", 0);
    _useCustomDateTime = settings.readBool(L"MultiInstance", L"UseCustomDateTime", false);
    _dateTimeFormat = QString::fromStdWString(settings.readString(L"MultiInstance", L"DateTimeFormat", L"yyyy-MM-dd HH:mm:ss"));

    _multiInstanceCombo->setCurrentIndex(_multiInstanceMode);
    _useCustomDateTimeCheck->setChecked(_useCustomDateTime);
    _dateTimeFormatEdit->setText(_dateTimeFormat);
    _dateTimeFormatEdit->setEnabled(_useCustomDateTime);
}

void MultiInstanceSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"MultiInstance", L"MultiInstanceMode", _multiInstanceMode);
    settings.writeBool(L"MultiInstance", L"UseCustomDateTime", _useCustomDateTime);
    settings.writeString(L"MultiInstance", L"DateTimeFormat", _dateTimeFormat.toStdWString());
}

bool MultiInstanceSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void MultiInstanceSubDlg::onMultiInstanceModeChanged(int index)
{
    _multiInstanceMode = index;
}

void MultiInstanceSubDlg::onDateTimeFormatChanged(const QString& format)
{
    _dateTimeFormat = format;
    // Update preview
    if (_useCustomDateTime && !format.isEmpty()) {
        _dateTimePreviewLabel->setText(tr("Preview: ") + QDateTime::currentDateTime().toString(format));
    } else {
        _dateTimePreviewLabel->setText(tr("Preview: ") + QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat));
    }
}

void MultiInstanceSubDlg::onUseCustomDateTimeToggled(bool checked)
{
    _useCustomDateTime = checked;
    _dateTimeFormatEdit->setEnabled(checked);
}

// ============================================================================
// DelimiterSubDlg Implementation
// ============================================================================
DelimiterSubDlg::DelimiterSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void DelimiterSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Delimiter group
    _delimiterGroup = new QGroupBox(tr("Word Delimiters"), this);
    auto* delimiterLayout = new QVBoxLayout(_delimiterGroup);

    _delimiterListLabel = new QLabel(tr("Additional delimiters for word selection (double-click selection):"), _delimiterGroup);
    delimiterLayout->addWidget(_delimiterListLabel);

    _delimiterListEdit = new QLineEdit(_delimiterGroup);
    _delimiterListEdit->setPlaceholderText(tr("e.g., -+=~!@#$%^&*()[]{}|;':\",./<>?"));
    delimiterLayout->addWidget(_delimiterListEdit);

    mainLayout->addWidget(_delimiterGroup);

    // Word Character group
    _wordCharGroup = new QGroupBox(tr("Word Characters"), this);
    auto* wordCharLayout = new QVBoxLayout(_wordCharGroup);

    _defaultWordCharsRadio = new QRadioButton(tr("Use default word characters (alphanumeric and underscore)"), _wordCharGroup);
    wordCharLayout->addWidget(_defaultWordCharsRadio);

    _customWordCharsRadio = new QRadioButton(tr("Use custom word characters:"), _wordCharGroup);
    wordCharLayout->addWidget(_customWordCharsRadio);

    _customWordCharsEdit = new QLineEdit(_wordCharGroup);
    _customWordCharsEdit->setPlaceholderText(tr("e.g., _abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
    wordCharLayout->addWidget(_customWordCharsEdit);

    mainLayout->addWidget(_wordCharGroup);
    mainLayout->addStretch();
}

void DelimiterSubDlg::connectSignals()
{
    connect(_delimiterListEdit, &QLineEdit::textChanged, this, &DelimiterSubDlg::onDelimiterListChanged);
    connect(_customWordCharsEdit, &QLineEdit::textChanged, this, &DelimiterSubDlg::onWordCharListChanged);
    connect(_defaultWordCharsRadio, &QRadioButton::toggled, this, &DelimiterSubDlg::onDefaultWordCharsToggled);
}

void DelimiterSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _delimiterList = QString::fromStdWString(settings.readString(L"Delimiter", L"DelimiterList", L""));
    _useDefaultWordChars = settings.readBool(L"Delimiter", L"UseDefaultWordChars", true);
    _customWordChars = QString::fromStdWString(settings.readString(L"Delimiter", L"CustomWordChars", L""));

    _delimiterListEdit->setText(_delimiterList);
    _defaultWordCharsRadio->setChecked(_useDefaultWordChars);
    _customWordCharsRadio->setChecked(!_useDefaultWordChars);
    _customWordCharsEdit->setText(_customWordChars);
    _customWordCharsEdit->setEnabled(!_useDefaultWordChars);
}

void DelimiterSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeString(L"Delimiter", L"DelimiterList", _delimiterList.toStdWString());
    settings.writeBool(L"Delimiter", L"UseDefaultWordChars", _useDefaultWordChars);
    settings.writeString(L"Delimiter", L"CustomWordChars", _customWordChars.toStdWString());
}

bool DelimiterSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void DelimiterSubDlg::onDelimiterListChanged()
{
    _delimiterList = _delimiterListEdit->text();
}

void DelimiterSubDlg::onWordCharListChanged()
{
    _customWordChars = _customWordCharsEdit->text();
}

void DelimiterSubDlg::onDefaultWordCharsToggled(bool checked)
{
    _useDefaultWordChars = checked;
    _customWordCharsEdit->setEnabled(!checked);
}

// ============================================================================
// PerformanceSubDlg Implementation
// ============================================================================
PerformanceSubDlg::PerformanceSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void PerformanceSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Performance / Large File Restriction
    _performanceGroup = new QGroupBox(tr("Large File Restriction"), this);
    auto* perfLayout = new QVBoxLayout(_performanceGroup);

    _enableRestrictionCheck = new QCheckBox(tr("Enable large file restriction"), _performanceGroup);
    perfLayout->addWidget(_enableRestrictionCheck);

    auto* fileSizeLayout = new QHBoxLayout();
    _fileSizeLabel = new QLabel(tr("File size threshold:"), _performanceGroup);
    fileSizeLayout->addWidget(_fileSizeLabel);

    _fileSizeSpin = new QSpinBox(_performanceGroup);
    _fileSizeSpin->setRange(1, 4096);
    _fileSizeSpin->setValue(200);
    fileSizeLayout->addWidget(_fileSizeSpin);

    _fileSizeUnitLabel = new QLabel(tr("MB"), _performanceGroup);
    fileSizeLayout->addWidget(_fileSizeUnitLabel);
    fileSizeLayout->addStretch();
    perfLayout->addLayout(fileSizeLayout);

    mainLayout->addWidget(_performanceGroup);

    // Feature restrictions
    _restrictionsGroup = new QGroupBox(tr("Allowed Features for Large Files"), this);
    auto* restrictLayout = new QVBoxLayout(_restrictionsGroup);

    _allowBraceMatchCheck = new QCheckBox(tr("Allow brace matching"), _restrictionsGroup);
    restrictLayout->addWidget(_allowBraceMatchCheck);

    _allowAutoCompletionCheck = new QCheckBox(tr("Allow auto-completion"), _restrictionsGroup);
    restrictLayout->addWidget(_allowAutoCompletionCheck);

    _allowSmartHiliteCheck = new QCheckBox(tr("Allow smart highlighting"), _restrictionsGroup);
    restrictLayout->addWidget(_allowSmartHiliteCheck);

    _allowClickableLinkCheck = new QCheckBox(tr("Allow clickable links"), _restrictionsGroup);
    restrictLayout->addWidget(_allowClickableLinkCheck);

    _deactivateWordWrapCheck = new QCheckBox(tr("Deactivate word wrap globally"), _restrictionsGroup);
    restrictLayout->addWidget(_deactivateWordWrapCheck);

    mainLayout->addWidget(_restrictionsGroup);
    mainLayout->addStretch();
}

void PerformanceSubDlg::connectSignals()
{
    connect(_enableRestrictionCheck, &QCheckBox::toggled, this, &PerformanceSubDlg::onLargeFileRestrictionToggled);
    connect(_fileSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PerformanceSubDlg::onFileSizeChanged);
    connect(_allowBraceMatchCheck, &QCheckBox::toggled, this, &PerformanceSubDlg::onAllowBraceMatchToggled);
    connect(_allowAutoCompletionCheck, &QCheckBox::toggled, this, &PerformanceSubDlg::onAllowAutoCompletionToggled);
    connect(_allowSmartHiliteCheck, &QCheckBox::toggled, this, &PerformanceSubDlg::onAllowSmartHiliteToggled);
    connect(_allowClickableLinkCheck, &QCheckBox::toggled, this, &PerformanceSubDlg::onAllowClickableLinkToggled);
    connect(_deactivateWordWrapCheck, &QCheckBox::toggled, this, &PerformanceSubDlg::onDeactivateWordWrapToggled);
}

void PerformanceSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _largeFileRestrictionEnabled = settings.readBool(L"Performance", L"LargeFileRestriction", true);
    _largeFileSizeMB = settings.readInt(L"Performance", L"LargeFileSizeMB", 200);
    _allowBraceMatch = settings.readBool(L"Performance", L"AllowBraceMatch", false);
    _allowAutoCompletion = settings.readBool(L"Performance", L"AllowAutoCompletion", false);
    _allowSmartHilite = settings.readBool(L"Performance", L"AllowSmartHilite", false);
    _allowClickableLink = settings.readBool(L"Performance", L"AllowClickableLink", false);
    _deactivateWordWrap = settings.readBool(L"Performance", L"DeactivateWordWrap", true);

    _enableRestrictionCheck->setChecked(_largeFileRestrictionEnabled);
    _fileSizeSpin->setValue(_largeFileSizeMB);
    _allowBraceMatchCheck->setChecked(_allowBraceMatch);
    _allowAutoCompletionCheck->setChecked(_allowAutoCompletion);
    _allowSmartHiliteCheck->setChecked(_allowSmartHilite);
    _allowClickableLinkCheck->setChecked(_allowClickableLink);
    _deactivateWordWrapCheck->setChecked(_deactivateWordWrap);

    updateEnabledState();
}

void PerformanceSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Performance", L"LargeFileRestriction", _largeFileRestrictionEnabled);
    settings.writeInt(L"Performance", L"LargeFileSizeMB", _largeFileSizeMB);
    settings.writeBool(L"Performance", L"AllowBraceMatch", _allowBraceMatch);
    settings.writeBool(L"Performance", L"AllowAutoCompletion", _allowAutoCompletion);
    settings.writeBool(L"Performance", L"AllowSmartHilite", _allowSmartHilite);
    settings.writeBool(L"Performance", L"AllowClickableLink", _allowClickableLink);
    settings.writeBool(L"Performance", L"DeactivateWordWrap", _deactivateWordWrap);
}

bool PerformanceSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void PerformanceSubDlg::updateEnabledState()
{
    bool enabled = _largeFileRestrictionEnabled;
    _fileSizeSpin->setEnabled(enabled);
    _allowBraceMatchCheck->setEnabled(enabled);
    _allowAutoCompletionCheck->setEnabled(enabled);
    _allowSmartHiliteCheck->setEnabled(enabled);
    _allowClickableLinkCheck->setEnabled(enabled);
    _deactivateWordWrapCheck->setEnabled(enabled);
}

void PerformanceSubDlg::onLargeFileRestrictionToggled(bool checked)
{
    _largeFileRestrictionEnabled = checked;
    updateEnabledState();
}

void PerformanceSubDlg::onFileSizeChanged(int value) { _largeFileSizeMB = value; }
void PerformanceSubDlg::onAllowBraceMatchToggled(bool checked) { _allowBraceMatch = checked; }
void PerformanceSubDlg::onAllowAutoCompletionToggled(bool checked) { _allowAutoCompletion = checked; }
void PerformanceSubDlg::onAllowSmartHiliteToggled(bool checked) { _allowSmartHilite = checked; }
void PerformanceSubDlg::onAllowClickableLinkToggled(bool checked) { _allowClickableLink = checked; }
void PerformanceSubDlg::onDeactivateWordWrapToggled(bool checked) { _deactivateWordWrap = checked; }

// ============================================================================
// CloudLinkSubDlg Implementation
// ============================================================================
CloudLinkSubDlg::CloudLinkSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void CloudLinkSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Cloud group
    _cloudGroup = new QGroupBox(tr("Cloud Settings"), this);
    auto* cloudLayout = new QVBoxLayout(_cloudGroup);

    _enableCloudSyncCheck = new QCheckBox(tr("Enable cloud synchronization"), _cloudGroup);
    cloudLayout->addWidget(_enableCloudSyncCheck);

    _cloudPathLabel = new QLabel(tr("Cloud path:"), _cloudGroup);
    cloudLayout->addWidget(_cloudPathLabel);

    auto* cloudPathLayout = new QHBoxLayout();
    _cloudPathEdit = new QLineEdit(_cloudGroup);
    _cloudPathEdit->setPlaceholderText(tr("Path to cloud storage folder (e.g., OneDrive, Dropbox)"));
    cloudPathLayout->addWidget(_cloudPathEdit);

    _browseButton = new QPushButton(tr("Browse..."), _cloudGroup);
    cloudPathLayout->addWidget(_browseButton);

    cloudLayout->addLayout(cloudPathLayout);
    mainLayout->addWidget(_cloudGroup);
    mainLayout->addStretch();
}

void CloudLinkSubDlg::connectSignals()
{
    connect(_cloudPathEdit, &QLineEdit::textChanged, this, &CloudLinkSubDlg::onCloudPathChanged);
    connect(_browseButton, &QPushButton::clicked, this, &CloudLinkSubDlg::onBrowseCloudPathClicked);
    connect(_enableCloudSyncCheck, &QCheckBox::toggled, this, &CloudLinkSubDlg::onCloudSyncToggled);
}

void CloudLinkSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _cloudSyncEnabled = settings.readBool(L"Cloud", L"CloudSyncEnabled", false);
    _cloudPath = QString::fromStdWString(settings.readString(L"Cloud", L"CloudPath", L""));

    _enableCloudSyncCheck->setChecked(_cloudSyncEnabled);
    _cloudPathEdit->setText(_cloudPath);
    _cloudPathEdit->setEnabled(_cloudSyncEnabled);
    _browseButton->setEnabled(_cloudSyncEnabled);
    _cloudPathLabel->setEnabled(_cloudSyncEnabled);
}

void CloudLinkSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Cloud", L"CloudSyncEnabled", _cloudSyncEnabled);
    settings.writeString(L"Cloud", L"CloudPath", _cloudPath.toStdWString());
}

bool CloudLinkSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void CloudLinkSubDlg::onCloudPathChanged(const QString& path)
{
    _cloudPath = path;
}

void CloudLinkSubDlg::onBrowseCloudPathClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Cloud Directory"),
                                                    _cloudPath.isEmpty() ? QDir::homePath() : _cloudPath);
    if (!dir.isEmpty()) {
        _cloudPath = dir;
        _cloudPathEdit->setText(dir);
    }
}

void CloudLinkSubDlg::onCloudSyncToggled(bool checked)
{
    _cloudSyncEnabled = checked;
    _cloudPathEdit->setEnabled(checked);
    _browseButton->setEnabled(checked);
    _cloudPathLabel->setEnabled(checked);
}

// ============================================================================
// SearchEngineSubDlg Implementation
// ============================================================================
SearchEngineSubDlg::SearchEngineSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void SearchEngineSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Search Engine group
    _searchEngineGroup = new QGroupBox(tr("Search Engine"), this);
    auto* searchEngineLayout = new QVBoxLayout(_searchEngineGroup);

    _searchEngineCombo = new QComboBox(_searchEngineGroup);
    populateSearchEngines();
    searchEngineLayout->addWidget(_searchEngineCombo);

    _customUrlLabel = new QLabel(tr("Custom search URL (use %s for search term):"), _searchEngineGroup);
    searchEngineLayout->addWidget(_customUrlLabel);

    _customUrlEdit = new QLineEdit(_searchEngineGroup);
    _customUrlEdit->setPlaceholderText(tr("https://www.example.com/search?q=%s"));
    searchEngineLayout->addWidget(_customUrlEdit);

    _setDefaultButton = new QPushButton(tr("Set as Default"), _searchEngineGroup);
    searchEngineLayout->addWidget(_setDefaultButton);

    mainLayout->addWidget(_searchEngineGroup);
    mainLayout->addStretch();
}

void SearchEngineSubDlg::populateSearchEngines()
{
    _searchEngineCombo->addItem(tr("Google"), "https://www.google.com/search?q=%s");
    _searchEngineCombo->addItem(tr("Bing"), "https://www.bing.com/search?q=%s");
    _searchEngineCombo->addItem(tr("DuckDuckGo"), "https://duckduckgo.com/?q=%s");
    _searchEngineCombo->addItem(tr("Yahoo"), "https://search.yahoo.com/search?p=%s");
    _searchEngineCombo->addItem(tr("Stack Overflow"), "https://stackoverflow.com/search?q=%s");
    _searchEngineCombo->addItem(tr("GitHub"), "https://github.com/search?q=%s");
    _searchEngineCombo->addItem(tr("Custom..."), "");
}

void SearchEngineSubDlg::connectSignals()
{
    connect(_searchEngineCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SearchEngineSubDlg::onSearchEngineChanged);
    connect(_customUrlEdit, &QLineEdit::textChanged, this, &SearchEngineSubDlg::onCustomUrlChanged);
    connect(_setDefaultButton, &QPushButton::clicked, this, &SearchEngineSubDlg::onSetDefaultClicked);
}

void SearchEngineSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _selectedSearchEngine = settings.readInt(L"SearchEngine", L"SelectedSearchEngine", 0);
    _customSearchUrl = QString::fromStdWString(settings.readString(L"SearchEngine", L"CustomSearchUrl", L""));

    _searchEngineCombo->setCurrentIndex(_selectedSearchEngine);
    _customUrlEdit->setText(_customSearchUrl);
    bool isCustom = (_selectedSearchEngine == _searchEngineCombo->count() - 1);
    _customUrlEdit->setEnabled(isCustom);
    _customUrlLabel->setEnabled(isCustom);
}

void SearchEngineSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"SearchEngine", L"SelectedSearchEngine", _selectedSearchEngine);
    settings.writeString(L"SearchEngine", L"CustomSearchUrl", _customSearchUrl.toStdWString());
}

bool SearchEngineSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void SearchEngineSubDlg::onSearchEngineChanged(int index)
{
    _selectedSearchEngine = index;
    bool isCustom = (index == _searchEngineCombo->count() - 1);
    _customUrlEdit->setEnabled(isCustom);
    _customUrlLabel->setEnabled(isCustom);
}

void SearchEngineSubDlg::onCustomUrlChanged(const QString& url)
{
    _customSearchUrl = url;
}

void SearchEngineSubDlg::onSetDefaultClicked()
{
    // Save the current selection as default
    saveSettings();
    QMessageBox::information(this, tr("Search Engine"), tr("Search engine preference saved."));
}

// ============================================================================
// MISCSubDlg Implementation
// ============================================================================
MISCSubDlg::MISCSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void MISCSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // General Misc group
    _generalMiscGroup = new QGroupBox(tr("General"), this);
    auto* generalMiscLayout = new QVBoxLayout(_generalMiscGroup);

    _minimizeToTrayCheck = new QCheckBox(tr("Minimize to system tray"), _generalMiscGroup);
    generalMiscLayout->addWidget(_minimizeToTrayCheck);

    _autoUpdateCheck = new QCheckBox(tr("Enable auto-updater"), _generalMiscGroup);
    generalMiscLayout->addWidget(_autoUpdateCheck);

    _enableNotificationsCheck = new QCheckBox(tr("Enable notifications"), _generalMiscGroup);
    generalMiscLayout->addWidget(_enableNotificationsCheck);

    _muteSoundsCheck = new QCheckBox(tr("Mute all sounds"), _generalMiscGroup);
    generalMiscLayout->addWidget(_muteSoundsCheck);

    _confirmExitCheck = new QCheckBox(tr("Confirm exit when there are unsaved changes"), _generalMiscGroup);
    generalMiscLayout->addWidget(_confirmExitCheck);

    _confirmDeleteCheck = new QCheckBox(tr("Confirm move to Recycle Bin"), _generalMiscGroup);
    generalMiscLayout->addWidget(_confirmDeleteCheck);

    mainLayout->addWidget(_generalMiscGroup);

    // File Detection group
    _fileDetectionGroup = new QGroupBox(tr("File Change Detection"), this);
    auto* fileDetectionLayout = new QVBoxLayout(_fileDetectionGroup);

    _fileAutoDetectionLabel = new QLabel(tr("File auto-detection:"), _fileDetectionGroup);
    fileDetectionLayout->addWidget(_fileAutoDetectionLabel);

    _fileAutoDetectionCombo = new QComboBox(_fileDetectionGroup);
    _fileAutoDetectionCombo->addItem(tr("Disable"));
    _fileAutoDetectionCombo->addItem(tr("Enable"));
    _fileAutoDetectionCombo->addItem(tr("Enable and update silently"));
    fileDetectionLayout->addWidget(_fileAutoDetectionCombo);

    mainLayout->addWidget(_fileDetectionGroup);
    mainLayout->addStretch();
}

void MISCSubDlg::connectSignals()
{
    connect(_minimizeToTrayCheck, &QCheckBox::toggled, this, &MISCSubDlg::onMinimizeToTrayToggled);
    connect(_autoUpdateCheck, &QCheckBox::toggled, this, &MISCSubDlg::onAutoUpdateToggled);
    connect(_enableNotificationsCheck, &QCheckBox::toggled, this, &MISCSubDlg::onEnableNotificationsToggled);
    connect(_fileAutoDetectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MISCSubDlg::onFileAutoDetectionChanged);
    connect(_muteSoundsCheck, &QCheckBox::toggled, this, &MISCSubDlg::onMuteSoundsToggled);
    connect(_confirmExitCheck, &QCheckBox::toggled, this, &MISCSubDlg::onConfirmExitToggled);
    connect(_confirmDeleteCheck, &QCheckBox::toggled, this, &MISCSubDlg::onConfirmDeleteToggled);
}

void MISCSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _minimizeToTray = settings.readBool(L"MISC", L"MinimizeToTray", false);
    _autoUpdate = settings.readBool(L"MISC", L"AutoUpdate", true);
    _enableNotifications = settings.readBool(L"MISC", L"EnableNotifications", true);
    _fileAutoDetection = settings.readInt(L"MISC", L"FileAutoDetection", 1);
    _muteSounds = settings.readBool(L"MISC", L"MuteSounds", false);
    _confirmExit = settings.readBool(L"MISC", L"ConfirmExit", true);
    _confirmDelete = settings.readBool(L"MISC", L"ConfirmDelete", true);

    _minimizeToTrayCheck->setChecked(_minimizeToTray);
    _autoUpdateCheck->setChecked(_autoUpdate);
    _enableNotificationsCheck->setChecked(_enableNotifications);
    _fileAutoDetectionCombo->setCurrentIndex(_fileAutoDetection);
    _muteSoundsCheck->setChecked(_muteSounds);
    _confirmExitCheck->setChecked(_confirmExit);
    _confirmDeleteCheck->setChecked(_confirmDelete);
}

void MISCSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"MISC", L"MinimizeToTray", _minimizeToTray);
    settings.writeBool(L"MISC", L"AutoUpdate", _autoUpdate);
    settings.writeBool(L"MISC", L"EnableNotifications", _enableNotifications);
    settings.writeInt(L"MISC", L"FileAutoDetection", _fileAutoDetection);
    settings.writeBool(L"MISC", L"MuteSounds", _muteSounds);
    settings.writeBool(L"MISC", L"ConfirmExit", _confirmExit);
    settings.writeBool(L"MISC", L"ConfirmDelete", _confirmDelete);
}

bool MISCSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void MISCSubDlg::onMinimizeToTrayToggled(bool checked)
{
    _minimizeToTray = checked;
}

void MISCSubDlg::onAutoUpdateToggled(bool checked)
{
    _autoUpdate = checked;
}

void MISCSubDlg::onEnableNotificationsToggled(bool checked)
{
    _enableNotifications = checked;
}

void MISCSubDlg::onFileAutoDetectionChanged(int index)
{
    _fileAutoDetection = index;
}

void MISCSubDlg::onMuteSoundsToggled(bool checked)
{
    _muteSounds = checked;
}

void MISCSubDlg::onConfirmExitToggled(bool checked)
{
    _confirmExit = checked;
}

void MISCSubDlg::onConfirmDeleteToggled(bool checked)
{
    _confirmDelete = checked;
}

// ============================================================================
// PreferenceDlg Implementation
// ============================================================================
PreferenceDlg::PreferenceDlg(QWidget* parent) : StaticDialog(parent)
{
    setupUI();
    _settings = &PlatformLayer::ISettings::getInstance();
}

PreferenceDlg::~PreferenceDlg()
{
    // Sub-pages are owned by the stacked widget and will be deleted automatically
}

void PreferenceDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Preferences"));
    dialog->setMinimumSize(700, 500);
    dialog->resize(750, 550);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create horizontal splitter-like layout
    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(10);

    // Left side - Category list
    _categoryList = new QListWidget(dialog);
    _categoryList->setMaximumWidth(200);
    _categoryList->setMinimumWidth(150);
    contentLayout->addWidget(_categoryList);

    // Right side - Stacked widget for pages
    _pagesStack = new QStackedWidget(dialog);
    contentLayout->addWidget(_pagesStack, 1);

    mainLayout->addLayout(contentLayout);

    // Separator line
    auto* line = new QFrame(dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _okButton = new QPushButton(tr("OK"), dialog);
    _okButton->setDefault(true);
    buttonLayout->addWidget(_okButton);

    _cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_cancelButton);

    _applyButton = new QPushButton(tr("Apply"), dialog);
    buttonLayout->addWidget(_applyButton);

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void PreferenceDlg::createSubPages()
{
    // Create all sub-pages in Windows order (24 pages total)
    _generalPage = new GeneralSubDlg(_pagesStack);
    _pagesStack->addWidget(_generalPage);
    _categories.append(CategoryInfo(tr("General"), "Global", _generalPage));

    _toolbarPage = new ToolbarSubDlg(_pagesStack);
    _pagesStack->addWidget(_toolbarPage);
    _categories.append(CategoryInfo(tr("Toolbar"), "Toolbar", _toolbarPage));

    _tabbarPage = new TabbarSubDlg(_pagesStack);
    _pagesStack->addWidget(_tabbarPage);
    _categories.append(CategoryInfo(tr("Tab Bar"), "Tabbar", _tabbarPage));

    _editingPage = new EditingSubDlg(_pagesStack);
    _pagesStack->addWidget(_editingPage);
    _categories.append(CategoryInfo(tr("Editing 1"), "Scintillas", _editingPage));

    _editing2Page = new Editing2SubDlg(_pagesStack);
    _pagesStack->addWidget(_editing2Page);
    _categories.append(CategoryInfo(tr("Editing 2"), "Scintillas2", _editing2Page));

    _darkModePage = new DarkModeSubDlg(_pagesStack);
    _pagesStack->addWidget(_darkModePage);
    _categories.append(CategoryInfo(tr("Dark Mode"), "DarkMode", _darkModePage));

    _marginsBorderEdgePage = new MarginsBorderEdgeSubDlg(_pagesStack);
    _pagesStack->addWidget(_marginsBorderEdgePage);
    _categories.append(CategoryInfo(tr("Margins/Border/Edge"), "MarginsBorderEdge", _marginsBorderEdgePage));

    _newDocumentPage = new NewDocumentSubDlg(_pagesStack);
    _pagesStack->addWidget(_newDocumentPage);
    _categories.append(CategoryInfo(tr("New Document"), "NewDoc", _newDocumentPage));

    _defaultDirectoryPage = new DefaultDirectorySubDlg(_pagesStack);
    _pagesStack->addWidget(_defaultDirectoryPage);
    _categories.append(CategoryInfo(tr("Default Directory"), "DefaultDir", _defaultDirectoryPage));

    _recentFilesHistoryPage = new RecentFilesHistorySubDlg(_pagesStack);
    _pagesStack->addWidget(_recentFilesHistoryPage);
    _categories.append(CategoryInfo(tr("Recent Files History"), "RecentFilesHistory", _recentFilesHistoryPage));

    _fileAssocPage = new FileAssocSubDlg(_pagesStack);
    _pagesStack->addWidget(_fileAssocPage);
    _categories.append(CategoryInfo(tr("File Association"), "FileAssoc", _fileAssocPage));

    _languagePage = new LanguageSubDlg(_pagesStack);
    _pagesStack->addWidget(_languagePage);
    _categories.append(CategoryInfo(tr("Language"), "Language", _languagePage));

    _indentationPage = new IndentationSubDlg(_pagesStack);
    _pagesStack->addWidget(_indentationPage);
    _categories.append(CategoryInfo(tr("Indentation"), "Indentation", _indentationPage));

    _highlightingPage = new HighlightingSubDlg(_pagesStack);
    _pagesStack->addWidget(_highlightingPage);
    _categories.append(CategoryInfo(tr("Highlighting"), "Highlighting", _highlightingPage));

    _printPage = new PrintSubDlg(_pagesStack);
    _pagesStack->addWidget(_printPage);
    _categories.append(CategoryInfo(tr("Print"), "Print", _printPage));

    _searchingPage = new SearchingSubDlg(_pagesStack);
    _pagesStack->addWidget(_searchingPage);
    _categories.append(CategoryInfo(tr("Searching"), "Searching", _searchingPage));

    _backupPage = new BackupSubDlg(_pagesStack);
    _pagesStack->addWidget(_backupPage);
    _categories.append(CategoryInfo(tr("Backup"), "Backup", _backupPage));

    _autoCompletionPage = new AutoCompletionSubDlg(_pagesStack);
    _pagesStack->addWidget(_autoCompletionPage);
    _categories.append(CategoryInfo(tr("Auto-Completion"), "AutoCompletion", _autoCompletionPage));

    _multiInstancePage = new MultiInstanceSubDlg(_pagesStack);
    _pagesStack->addWidget(_multiInstancePage);
    _categories.append(CategoryInfo(tr("Multi-Instance & Date"), "MultiInstance", _multiInstancePage));

    _delimiterPage = new DelimiterSubDlg(_pagesStack);
    _pagesStack->addWidget(_delimiterPage);
    _categories.append(CategoryInfo(tr("Delimiter"), "Delimiter", _delimiterPage));

    _performancePage = new PerformanceSubDlg(_pagesStack);
    _pagesStack->addWidget(_performancePage);
    _categories.append(CategoryInfo(tr("Performance"), "Performance", _performancePage));

    _cloudLinkPage = new CloudLinkSubDlg(_pagesStack);
    _pagesStack->addWidget(_cloudLinkPage);
    _categories.append(CategoryInfo(tr("Cloud & Link"), "Cloud", _cloudLinkPage));

    _searchEnginePage = new SearchEngineSubDlg(_pagesStack);
    _pagesStack->addWidget(_searchEnginePage);
    _categories.append(CategoryInfo(tr("Search Engine"), "SearchEngine", _searchEnginePage));

    _miscPage = new MISCSubDlg(_pagesStack);
    _pagesStack->addWidget(_miscPage);
    _categories.append(CategoryInfo(tr("MISC."), "MISC", _miscPage));

    // Populate category list
    for (const auto& category : _categories) {
        _categoryList->addItem(category.name);
    }

    // Select first item
    if (_categoryList->count() > 0) {
        _categoryList->setCurrentRow(0);
        _pagesStack->setCurrentIndex(0);
    }
}

void PreferenceDlg::loadSettings()
{
    // Load settings for all pages
    if (_generalPage) _generalPage->loadSettings();
    if (_toolbarPage) _toolbarPage->loadSettings();
    if (_tabbarPage) _tabbarPage->loadSettings();
    if (_editingPage) _editingPage->loadSettings();
    if (_editing2Page) _editing2Page->loadSettings();
    if (_darkModePage) _darkModePage->loadSettings();
    if (_marginsBorderEdgePage) _marginsBorderEdgePage->loadSettings();
    if (_newDocumentPage) _newDocumentPage->loadSettings();
    if (_defaultDirectoryPage) _defaultDirectoryPage->loadSettings();
    if (_recentFilesHistoryPage) _recentFilesHistoryPage->loadSettings();
    if (_fileAssocPage) _fileAssocPage->loadSettings();
    if (_languagePage) _languagePage->loadSettings();
    if (_indentationPage) _indentationPage->loadSettings();
    if (_highlightingPage) _highlightingPage->loadSettings();
    if (_printPage) _printPage->loadSettings();
    if (_searchingPage) _searchingPage->loadSettings();
    if (_backupPage) _backupPage->loadSettings();
    if (_autoCompletionPage) _autoCompletionPage->loadSettings();
    if (_multiInstancePage) _multiInstancePage->loadSettings();
    if (_delimiterPage) _delimiterPage->loadSettings();
    if (_performancePage) _performancePage->loadSettings();
    if (_cloudLinkPage) _cloudLinkPage->loadSettings();
    if (_searchEnginePage) _searchEnginePage->loadSettings();
    if (_miscPage) _miscPage->loadSettings();
}

bool PreferenceDlg::saveSettings()
{
    // Apply settings from all pages
    bool success = true;

    if (_generalPage && !_generalPage->applySettings()) success = false;
    if (_toolbarPage && !_toolbarPage->applySettings()) success = false;
    if (_tabbarPage && !_tabbarPage->applySettings()) success = false;
    if (_editingPage && !_editingPage->applySettings()) success = false;
    if (_editing2Page && !_editing2Page->applySettings()) success = false;
    if (_darkModePage && !_darkModePage->applySettings()) success = false;
    if (_marginsBorderEdgePage && !_marginsBorderEdgePage->applySettings()) success = false;
    if (_newDocumentPage && !_newDocumentPage->applySettings()) success = false;
    if (_defaultDirectoryPage && !_defaultDirectoryPage->applySettings()) success = false;
    if (_recentFilesHistoryPage && !_recentFilesHistoryPage->applySettings()) success = false;
    if (_fileAssocPage && !_fileAssocPage->applySettings()) success = false;
    if (_languagePage && !_languagePage->applySettings()) success = false;
    if (_indentationPage && !_indentationPage->applySettings()) success = false;
    if (_highlightingPage && !_highlightingPage->applySettings()) success = false;
    if (_printPage && !_printPage->applySettings()) success = false;
    if (_searchingPage && !_searchingPage->applySettings()) success = false;
    if (_backupPage && !_backupPage->applySettings()) success = false;
    if (_autoCompletionPage && !_autoCompletionPage->applySettings()) success = false;
    if (_multiInstancePage && !_multiInstancePage->applySettings()) success = false;
    if (_delimiterPage && !_delimiterPage->applySettings()) success = false;
    if (_performancePage && !_performancePage->applySettings()) success = false;
    if (_cloudLinkPage && !_cloudLinkPage->applySettings()) success = false;
    if (_searchEnginePage && !_searchEnginePage->applySettings()) success = false;
    if (_miscPage && !_miscPage->applySettings()) success = false;

    // Save to persistent storage
    if (_settings) {
        _settings->saveConfig();
    }

    return success;
}

void PreferenceDlg::connectSignals()
{
    connect(_categoryList, &QListWidget::currentRowChanged, this, &PreferenceDlg::onCategoryChanged);
    connect(_okButton, &QPushButton::clicked, this, &PreferenceDlg::onOKClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &PreferenceDlg::onCancelClicked);
    connect(_applyButton, &QPushButton::clicked, this, &PreferenceDlg::onApplyClicked);
}

void PreferenceDlg::doDialog()
{
    doDialog(false);
}

void PreferenceDlg::doDialog(bool /*isRTL*/)
{
    if (!isCreated()) {
        create(tr("Preferences"), false);
        setupUI();
        createSubPages();
        loadSettings();
        connectSignals();
    }

    goToCenter();
    display(true, true);
}

void PreferenceDlg::showPage(const QString& internalName)
{
    for (int i = 0; i < _categories.size(); ++i) {
        if (_categories[i].internalName == internalName) {
            showPage(i);
            return;
        }
    }
}

void PreferenceDlg::showPage(int index)
{
    if (index >= 0 && index < _categoryList->count()) {
        _categoryList->setCurrentRow(index);
        _pagesStack->setCurrentIndex(index);
    }
}

int PreferenceDlg::getCurrentPageIndex() const
{
    return _pagesStack->currentIndex();
}

void PreferenceDlg::onCategoryChanged(int index)
{
    if (index >= 0 && index < _pagesStack->count()) {
        _pagesStack->setCurrentIndex(index);
    }
}

void PreferenceDlg::onCategoryItemClicked(QListWidgetItem* item)
{
    if (!item)
        return;

    int index = _categoryList->row(item);
    if (index >= 0 && index < _pagesStack->count()) {
        _pagesStack->setCurrentIndex(index);
    }
}

void PreferenceDlg::onOKClicked()
{
    saveSettings();
    display(false);
}

void PreferenceDlg::onCancelClicked()
{
    // Discard changes and close
    display(false);
}

void PreferenceDlg::onApplyClicked()
{
    saveSettings();
}

bool PreferenceDlg::run_dlgProc(QEvent* /*event*/)
{
    // Handle any custom event processing here
    // Return true if event was handled, false otherwise
    return false;
}

} // namespace QtControls
