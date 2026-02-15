// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../Platform/Settings.h"

#include <QWidget>
#include <QString>
#include <QHash>

// Forward declarations
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QPushButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QSlider;
class QLabel;
class QGroupBox;
class QRadioButton;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTextEdit;
class QTabWidget;

namespace QtControls {

// ============================================================================
// Forward declarations for all sub-dialogs
// ============================================================================
class GeneralSubDlg;
class ToolbarSubDlg;
class TabbarSubDlg;
class EditingSubDlg;
class Editing2SubDlg;
class DarkModeSubDlg;
class MarginsBorderEdgeSubDlg;
class NewDocumentSubDlg;
class DefaultDirectorySubDlg;
class RecentFilesHistorySubDlg;
class FileAssocSubDlg;
class LanguageSubDlg;
class IndentationSubDlg;
class HighlightingSubDlg;
class PrintSubDlg;
class SearchingSubDlg;
class BackupSubDlg;
class AutoCompletionSubDlg;
class MultiInstanceSubDlg;
class DelimiterSubDlg;
class PerformanceSubDlg;
class CloudLinkSubDlg;
class SearchEngineSubDlg;
class MISCSubDlg;

// ============================================================================
// Category Info Structure
// ============================================================================
struct CategoryInfo {
    QString name;
    QString internalName;
    QWidget* page = nullptr;

    CategoryInfo() = default;
    CategoryInfo(const QString& n, const QString& internal, QWidget* p)
        : name(n), internalName(internal), page(p) {}
};

// ============================================================================
// GeneralSubDlg - General settings (language, toolbar, status bar)
// ============================================================================
class GeneralSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit GeneralSubDlg(QWidget* parent = nullptr);
    ~GeneralSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onStatusBarToggled(bool checked);
    void onMenuBarToggled(bool checked);
    void onHideMenuShortcutsToggled(bool checked);
    void onLanguageChanged(int index);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _generalGroup = nullptr;
    QCheckBox* _hideStatusBarCheck = nullptr;
    QCheckBox* _hideMenuBarCheck = nullptr;
    QCheckBox* _hideMenuShortcutsCheck = nullptr;

    QGroupBox* _localizationGroup = nullptr;
    QComboBox* _languageCombo = nullptr;
    QLabel* _languageLabel = nullptr;

    // Settings reference
    bool _statusBarShow = true;
    bool _menuBarShow = true;
    bool _hideMenuRightShortcuts = false;
    QString _currentLanguage = "English";
};

// ============================================================================
// ToolbarSubDlg - Toolbar settings (icon set, size, color)
// ============================================================================
class ToolbarSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit ToolbarSubDlg(QWidget* parent = nullptr);
    ~ToolbarSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onToolbarHideToggled(bool checked);
    void onIconSetChanged(int index);
    void onIconColorChanged(int index);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _toolbarGroup = nullptr;
    QCheckBox* _hideToolbarCheck = nullptr;

    QGroupBox* _iconSetGroup = nullptr;
    QComboBox* _iconSetCombo = nullptr;

    QGroupBox* _iconColorGroup = nullptr;
    QComboBox* _iconColorCombo = nullptr;

    // Settings
    bool _toolbarShow = true;
    int _iconSet = 0; // 0=small, 1=large, 2=small2, 3=large2, 4=standard
    int _iconColor = 0; // 0=default, 1=red, 2=green, 3=blue, 4=purple, 5=cyan, 6=olive, 7=yellow
};

// ============================================================================
// TabbarSubDlg - Tab bar settings (appearance, behavior)
// ============================================================================
class TabbarSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit TabbarSubDlg(QWidget* parent = nullptr);
    ~TabbarSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onReduceToggled(bool checked);
    void onLockToggled(bool checked);
    void onDrawTopBarToggled(bool checked);
    void onDrawInactiveToggled(bool checked);
    void onShowCloseButtonToggled(bool checked);
    void onDoubleClickCloseToggled(bool checked);
    void onMultiLineToggled(bool checked);
    void onVerticalToggled(bool checked);
    void onHideTabBarToggled(bool checked);
    void onShowPinButtonToggled(bool checked);
    void onQuitOnEmptyToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _lookFeelGroup = nullptr;
    QCheckBox* _reduceCheck = nullptr;
    QCheckBox* _lockCheck = nullptr;
    QCheckBox* _drawTopBarCheck = nullptr;
    QCheckBox* _drawInactiveCheck = nullptr;

    QGroupBox* _behaviorGroup = nullptr;
    QCheckBox* _showCloseButtonCheck = nullptr;
    QCheckBox* _doubleClickCloseCheck = nullptr;
    QCheckBox* _multiLineCheck = nullptr;
    QCheckBox* _verticalCheck = nullptr;
    QCheckBox* _hideTabBarCheck = nullptr;
    QCheckBox* _showPinButtonCheck = nullptr;
    QCheckBox* _quitOnEmptyCheck = nullptr;

    // Settings
    bool _reduce = false;
    bool _lock = false;
    bool _drawTopBar = false;
    bool _drawInactive = false;
    bool _showCloseButton = true;
    bool _doubleClickClose = false;
    bool _multiLine = false;
    bool _vertical = false;
    bool _hideTabBar = false;
    bool _showPinButton = false;
    bool _quitOnEmpty = false;
};

// ============================================================================
// EditingSubDlg - Editing settings (line numbering, indentation, caret)
// ============================================================================
class EditingSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit EditingSubDlg(QWidget* parent = nullptr);
    ~EditingSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onLineNumberToggled(bool checked);
    void onLineNumberDynamicWidthToggled(bool checked);
    void onCurrentLineHighlightChanged(int index);
    void onCaretBlinkRateChanged(int value);
    void onCaretWidthChanged(int index);
    void onSmoothFontToggled(bool checked);
    void onVirtualSpaceToggled(bool checked);
    void onScrollBeyondLastLineToggled(bool checked);
    void onRightClickKeepsSelectionToggled(bool checked);
    void onLineCopyCutWithoutSelectionToggled(bool checked);
    void onLineWrapMethodChanged(int index);

private:
    void setupUI();
    void connectSignals();
    void initScintParam();

    // UI Components - Line Numbering
    QGroupBox* _lineNumberGroup = nullptr;
    QCheckBox* _lineNumberCheck = nullptr;
    QCheckBox* _lineNumberDynamicWidthCheck = nullptr;

    // UI Components - Current Line
    QGroupBox* _currentLineGroup = nullptr;
    QComboBox* _currentLineHighlightCombo = nullptr;
    QLabel* _currentLineLabel = nullptr;

    // UI Components - Caret
    QGroupBox* _caretGroup = nullptr;
    QLabel* _caretBlinkRateLabel = nullptr;
    QSlider* _caretBlinkRateSlider = nullptr;
    QLabel* _caretBlinkRateValue = nullptr;
    QLabel* _caretWidthLabel = nullptr;
    QComboBox* _caretWidthCombo = nullptr;

    // UI Components - Scintilla Options
    QGroupBox* _scintillaGroup = nullptr;
    QCheckBox* _smoothFontCheck = nullptr;
    QCheckBox* _virtualSpaceCheck = nullptr;
    QCheckBox* _scrollBeyondLastLineCheck = nullptr;
    QCheckBox* _rightClickKeepsSelectionCheck = nullptr;
    QCheckBox* _lineCopyCutWithoutSelectionCheck = nullptr;

    // UI Components - Line Wrap
    QGroupBox* _lineWrapGroup = nullptr;
    QComboBox* _lineWrapCombo = nullptr;

    // Settings
    bool _lineNumberShow = true;
    bool _lineNumberDynamicWidth = true;
    int _currentLineHighlightMode = 1; // 0=none, 1=hilite, 2=frame
    int _caretBlinkRate = 600;
    int _caretWidth = 1;
    bool _doSmoothFont = false;
    bool _virtualSpace = false;
    bool _scrollBeyondLastLine = true;
    bool _rightClickKeepsSelection = false;
    bool _lineCopyCutWithoutSelection = false;
    int _lineWrapMethod = 0; // 0=default, 1=aligned, 2=indent
};

// ============================================================================
// Editing2SubDlg - NPC display, Unicode control chars, CR/LF display
// ============================================================================
class Editing2SubDlg : public QWidget {
    Q_OBJECT

public:
    explicit Editing2SubDlg(QWidget* parent = nullptr);
    ~Editing2SubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onNpcModeChanged(int index);
    void onNpcCustomColorToggled(bool checked);
    void onNpcIncludeCcUniEolToggled(bool checked);
    void onNpcNoInputC0Toggled(bool checked);
    void onCrlfDisplayModeChanged(int index);
    void onCrlfCustomColorToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components - Non-Printing Characters
    QGroupBox* _npcGroup = nullptr;
    QLabel* _npcModeLabel = nullptr;
    QComboBox* _npcModeCombo = nullptr;
    QCheckBox* _npcCustomColorCheck = nullptr;
    QCheckBox* _npcIncludeCcUniEolCheck = nullptr;
    QCheckBox* _npcNoInputC0Check = nullptr;

    // UI Components - CR/LF Display
    QGroupBox* _crlfGroup = nullptr;
    QLabel* _crlfModeLabel = nullptr;
    QComboBox* _crlfModeCombo = nullptr;
    QCheckBox* _crlfCustomColorCheck = nullptr;

    // Settings
    int _npcMode = 0; // 0=abbreviation, 1=codepoint
    bool _npcCustomColor = false;
    bool _npcIncludeCcUniEol = false;
    bool _npcNoInputC0 = false;
    int _crlfDisplayMode = 0; // 0=round corner, 1=plain text
    bool _crlfCustomColor = false;
};

// ============================================================================
// DarkModeSubDlg - Dark mode / theme settings
// ============================================================================
class DarkModeSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit DarkModeSubDlg(QWidget* parent = nullptr);
    ~DarkModeSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

signals:
    void darkModeChanged(bool enabled);

private slots:
    void onDarkModeToggled(bool checked);
    void onThemeChanged(int index);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _darkModeGroup = nullptr;
    QCheckBox* _enableDarkModeCheck = nullptr;

    QGroupBox* _themeGroup = nullptr;
    QLabel* _themeLabel = nullptr;
    QComboBox* _themeCombo = nullptr;

    // Settings
    bool _darkModeEnabled = false;
    int _themeIndex = 0;
};

// ============================================================================
// MarginsBorderEdgeSubDlg - Margins, border, vertical edge
// ============================================================================
class MarginsBorderEdgeSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit MarginsBorderEdgeSubDlg(QWidget* parent = nullptr);
    ~MarginsBorderEdgeSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onBorderWidthChanged(int value);
    void onPaddingLeftChanged(int value);
    void onPaddingRightChanged(int value);
    void onDistractionFreeChanged(int value);
    void onFolderMarkStyleChanged(int index);
    void onBookmarkMarginToggled(bool checked);
    void onChangeHistoryMarginToggled(bool checked);
    void onVerticalEdgeToggled(bool checked);
    void onVerticalEdgeColumnsChanged(const QString& text);

private:
    void setupUI();
    void connectSignals();

    // UI Components - Margins
    QGroupBox* _marginsGroup = nullptr;
    QCheckBox* _bookmarkMarginCheck = nullptr;
    QCheckBox* _changeHistoryMarginCheck = nullptr;
    QLabel* _folderMarkStyleLabel = nullptr;
    QComboBox* _folderMarkStyleCombo = nullptr;

    // UI Components - Border
    QGroupBox* _borderGroup = nullptr;
    QLabel* _borderWidthLabel = nullptr;
    QSlider* _borderWidthSlider = nullptr;
    QLabel* _borderWidthValue = nullptr;

    // UI Components - Padding
    QGroupBox* _paddingGroup = nullptr;
    QLabel* _paddingLeftLabel = nullptr;
    QSlider* _paddingLeftSlider = nullptr;
    QLabel* _paddingLeftValue = nullptr;
    QLabel* _paddingRightLabel = nullptr;
    QSlider* _paddingRightSlider = nullptr;
    QLabel* _paddingRightValue = nullptr;

    // UI Components - Distraction Free
    QLabel* _distractionFreeLabel = nullptr;
    QSlider* _distractionFreeSlider = nullptr;
    QLabel* _distractionFreeValue = nullptr;

    // UI Components - Vertical Edge
    QGroupBox* _verticalEdgeGroup = nullptr;
    QCheckBox* _verticalEdgeCheck = nullptr;
    QLabel* _verticalEdgeColumnsLabel = nullptr;
    QLineEdit* _verticalEdgeColumnsEdit = nullptr;

    // Settings
    int _borderWidth = 2;
    int _paddingLeft = 0;
    int _paddingRight = 0;
    int _distractionFree = 3;
    int _folderMarkStyle = 0; // 0=simple, 1=arrow, 2=circle, 3=box, 4=none
    bool _bookmarkMargin = true;
    bool _changeHistoryMargin = false;
    bool _verticalEdge = false;
    QString _verticalEdgeColumns;
};

// ============================================================================
// NewDocumentSubDlg - New document defaults
// ============================================================================
class NewDocumentSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit NewDocumentSubDlg(QWidget* parent = nullptr);
    ~NewDocumentSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onEncodingChanged(int index);
    void onFormatChanged(int index);
    void onLanguageChanged(int index);
    void onAnsiAsUtf8Toggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _encodingGroup = nullptr;
    QComboBox* _encodingCombo = nullptr;
    QCheckBox* _ansiAsUtf8Check = nullptr;

    QGroupBox* _formatGroup = nullptr;
    QComboBox* _formatCombo = nullptr;

    QGroupBox* _languageGroup = nullptr;
    QComboBox* _languageCombo = nullptr;

    QGroupBox* _defaultsGroup = nullptr;
    QCheckBox* _applyToOpenedAnsiFilesCheck = nullptr;

    // Settings
    int _defaultEncoding = 0; // UTF-8
    int _defaultFormat = 0; // Windows (CR LF)
    int _defaultLanguage = 0; // Text
    bool _openAnsiAsUtf8 = true;
};

// ============================================================================
// DefaultDirectorySubDlg - Default file paths
// ============================================================================
class DefaultDirectorySubDlg : public QWidget {
    Q_OBJECT

public:
    explicit DefaultDirectorySubDlg(QWidget* parent = nullptr);
    ~DefaultDirectorySubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onDirectoryTypeChanged(int index);
    void onBrowseClicked();
    void onCustomPathChanged(const QString& path);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _directoryGroup = nullptr;
    QComboBox* _directoryTypeCombo = nullptr;
    QLineEdit* _customPathEdit = nullptr;
    QPushButton* _browseButton = nullptr;

    // Settings
    int _defaultDirectoryType = 0; // 0=follow current doc, 1=last used, 2=custom
    QString _customDefaultDirectory;
};

// ============================================================================
// RecentFilesHistorySubDlg - Recent files settings
// ============================================================================
class RecentFilesHistorySubDlg : public QWidget {
    Q_OBJECT

public:
    explicit RecentFilesHistorySubDlg(QWidget* parent = nullptr);
    ~RecentFilesHistorySubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onMaxFilesChanged(int value);
    void onCustomLengthChanged(int value);
    void onDontCheckAtStartupToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _recentFilesGroup = nullptr;
    QLabel* _maxFilesLabel = nullptr;
    QSpinBox* _maxFilesSpin = nullptr;

    QLabel* _customLengthLabel = nullptr;
    QSpinBox* _customLengthSpin = nullptr;

    QCheckBox* _dontCheckAtStartupCheck = nullptr;

    // Settings
    int _maxRecentFiles = 10;
    int _customLength = 0;
    bool _dontCheckAtStartup = false;
};

// ============================================================================
// FileAssocSubDlg - File association / MIME type settings (Linux)
// ============================================================================
class FileAssocSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit FileAssocSubDlg(QWidget* parent = nullptr);
    ~FileAssocSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onExtensionSelected(int index);
    void onRegisterClicked();
    void onUnregisterClicked();

private:
    void setupUI();
    void connectSignals();
    void populateExtensions();

    // UI Components
    QGroupBox* _fileAssocGroup = nullptr;
    QLabel* _supportedExtLabel = nullptr;
    QListWidget* _extensionList = nullptr;
    QLabel* _registeredExtLabel = nullptr;
    QListWidget* _registeredList = nullptr;
    QPushButton* _registerButton = nullptr;
    QPushButton* _unregisterButton = nullptr;

    // Settings
    QStringList _registeredExtensions;
};

// ============================================================================
// LanguageSubDlg - Language-specific settings
// ============================================================================
class LanguageSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit LanguageSubDlg(QWidget* parent = nullptr);
    ~LanguageSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onLanguageSelected(int index);
    void onTabSizeChanged(int value);
    void onReplaceBySpaceToggled(bool checked);

private:
    void setupUI();
    void connectSignals();
    void populateLanguageList();

    // UI Components
    QGroupBox* _languageListGroup = nullptr;
    QListWidget* _languageList = nullptr;

    QGroupBox* _indentationGroup = nullptr;
    QLabel* _tabSizeLabel = nullptr;
    QSpinBox* _tabSizeSpin = nullptr;
    QCheckBox* _replaceBySpaceCheck = nullptr;

    // Settings
    int _selectedLanguage = 0;
    int _tabSize = 4;
    bool _replaceBySpace = false;
};

// ============================================================================
// IndentationSubDlg - Auto-indent and tab settings
// ============================================================================
class IndentationSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit IndentationSubDlg(QWidget* parent = nullptr);
    ~IndentationSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onTabSizeChanged(int value);
    void onReplaceBySpaceToggled(bool checked);
    void onBackspaceUnindentToggled(bool checked);
    void onAutoIndentModeChanged(int index);
    void onTabSettingLanguageChanged(int index);

private:
    void setupUI();
    void connectSignals();
    void populateLanguageList();

    // UI Components - Tab Settings
    QGroupBox* _tabSettingsGroup = nullptr;
    QLabel* _tabSizeLabel = nullptr;
    QSpinBox* _tabSizeSpin = nullptr;
    QCheckBox* _replaceBySpaceCheck = nullptr;
    QCheckBox* _backspaceUnindentCheck = nullptr;

    QGroupBox* _tabPerLanguageGroup = nullptr;
    QListWidget* _tabSettingLanguageList = nullptr;

    // UI Components - Auto-Indent
    QGroupBox* _autoIndentGroup = nullptr;
    QComboBox* _autoIndentCombo = nullptr;
    QLabel* _autoIndentLabel = nullptr;

    // Settings
    int _tabSize = 4;
    bool _replaceBySpace = false;
    bool _backspaceUnindent = false;
    int _autoIndentMode = 2; // 0=none, 1=basic, 2=advanced
};

// ============================================================================
// HighlightingSubDlg - Smart highlighting, matching
// ============================================================================
class HighlightingSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit HighlightingSubDlg(QWidget* parent = nullptr);
    ~HighlightingSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onSmartHighlightingToggled(bool checked);
    void onMatchCaseToggled(bool checked);
    void onWholeWordToggled(bool checked);
    void onMatchSelectionToggled(bool checked);
    void onBraceHighlightingToggled(bool checked);
    void onTagHighlightingToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components - Smart Highlighting
    QGroupBox* _smartHighlightGroup = nullptr;
    QCheckBox* _enableSmartHighlightCheck = nullptr;
    QCheckBox* _matchCaseCheck = nullptr;
    QCheckBox* _wholeWordCheck = nullptr;
    QCheckBox* _matchSelectionCheck = nullptr;

    // UI Components - Brace/Tag Matching
    QGroupBox* _matchingGroup = nullptr;
    QCheckBox* _braceHighlightCheck = nullptr;
    QCheckBox* _tagHighlightCheck = nullptr;

    // Settings
    bool _enableSmartHighlight = true;
    bool _smartHighlightMatchCase = false;
    bool _smartHighlightWholeWord = true;
    bool _smartHighlightUseSelection = false;
    bool _enableBraceHighlight = true;
    bool _enableTagHighlight = true;
};

// ============================================================================
// PrintSubDlg - Print settings
// ============================================================================
class PrintSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit PrintSubDlg(QWidget* parent = nullptr);
    ~PrintSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onPrintLineNumberToggled(bool checked);
    void onPrintSelectionToggled(bool checked);
    void onNoBackgroundToggled(bool checked);
    void onHeaderFooterToggled(bool checked);
    void onHeaderLeftChanged(const QString& text);
    void onHeaderCenterChanged(const QString& text);
    void onHeaderRightChanged(const QString& text);
    void onFooterLeftChanged(const QString& text);
    void onFooterCenterChanged(const QString& text);
    void onFooterRightChanged(const QString& text);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _printOptionsGroup = nullptr;
    QCheckBox* _printLineNumberCheck = nullptr;
    QCheckBox* _printSelectionCheck = nullptr;
    QCheckBox* _noBackgroundCheck = nullptr;

    QGroupBox* _headerFooterGroup = nullptr;
    QCheckBox* _enableHeaderFooterCheck = nullptr;

    QLabel* _headerLeftLabel = nullptr;
    QLineEdit* _headerLeftEdit = nullptr;
    QLabel* _headerCenterLabel = nullptr;
    QLineEdit* _headerCenterEdit = nullptr;
    QLabel* _headerRightLabel = nullptr;
    QLineEdit* _headerRightEdit = nullptr;

    QLabel* _footerLeftLabel = nullptr;
    QLineEdit* _footerLeftEdit = nullptr;
    QLabel* _footerCenterLabel = nullptr;
    QLineEdit* _footerCenterEdit = nullptr;
    QLabel* _footerRightLabel = nullptr;
    QLineEdit* _footerRightEdit = nullptr;

    // Settings
    bool _printLineNumber = true;
    bool _printSelection = false;
    bool _noBackground = true;
    bool _headerFooterEnabled = true;
    QString _headerLeft;
    QString _headerCenter;
    QString _headerRight;
    QString _footerLeft;
    QString _footerCenter;
    QString _footerRight;
};

// ============================================================================
// SearchingSubDlg - Search behavior
// ============================================================================
class SearchingSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit SearchingSubDlg(QWidget* parent = nullptr);
    ~SearchingSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onStopAtFirstMatchToggled(bool checked);
    void onWrapAroundToggled(bool checked);
    void onMatchWholeWordToggled(bool checked);
    void onMatchCaseToggled(bool checked);
    void onInSelectionThresholdChanged(int value);
    void onFillFindWhatToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _searchOptionsGroup = nullptr;
    QCheckBox* _stopAtFirstMatchCheck = nullptr;
    QCheckBox* _wrapAroundCheck = nullptr;
    QCheckBox* _matchWholeWordCheck = nullptr;
    QCheckBox* _matchCaseCheck = nullptr;

    QGroupBox* _advancedGroup = nullptr;
    QLabel* _inSelectionThresholdLabel = nullptr;
    QSpinBox* _inSelectionThresholdSpin = nullptr;
    QCheckBox* _fillFindWhatCheck = nullptr;

    // Settings
    bool _stopAtFirstMatch = false;
    bool _wrapAround = true;
    bool _matchWholeWord = false;
    bool _matchCase = false;
    int _inSelectionThreshold = 1024;
    bool _fillFindWhatWithSelection = true;
};

// ============================================================================
// BackupSubDlg - Auto-backup settings
// ============================================================================
class BackupSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit BackupSubDlg(QWidget* parent = nullptr);
    ~BackupSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onBackupModeChanged(int index);
    void onBackupDirectoryTypeChanged(int index);
    void onCustomDirectoryChanged(const QString& path);
    void onBrowseDirectoryClicked();
    void onSessionSnapshotToggled(bool checked);

private:
    void setupUI();
    void connectSignals();
    void updateBackupUI();

    // UI Components
    QGroupBox* _backupGroup = nullptr;
    QComboBox* _backupModeCombo = nullptr;
    QLabel* _backupModeLabel = nullptr;

    QGroupBox* _backupDirectoryGroup = nullptr;
    QComboBox* _backupDirectoryCombo = nullptr;
    QLineEdit* _customDirectoryEdit = nullptr;
    QPushButton* _browseButton = nullptr;

    QGroupBox* _sessionGroup = nullptr;
    QCheckBox* _sessionSnapshotCheck = nullptr;
    QLabel* _snapshotIntervalLabel = nullptr;
    QSpinBox* _snapshotIntervalSpin = nullptr;

    // Settings
    int _backupMode = 0; // 0=none, 1=simple, 2=verbose
    int _backupDirectoryType = 0; // 0=same dir, 1=custom
    QString _customBackupDirectory;
    bool _sessionSnapshot = true;
    int _snapshotInterval = 7;
};

// ============================================================================
// AutoCompletionSubDlg - Auto-completion settings
// ============================================================================
class AutoCompletionSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit AutoCompletionSubDlg(QWidget* parent = nullptr);
    ~AutoCompletionSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onAutoCompletionToggled(bool checked);
    void onAutoInsertBracketsToggled(bool checked);
    void onAutoInsertQuotesToggled(bool checked);
    void onAutoIndentToggled(bool checked);
    void onAutoCompletionIgnoreNumbersToggled(bool checked);
    void onAutoCompletionFromChanged(int index);
    void onAutoCompletionThresholdChanged(int value);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _autoCompletionGroup = nullptr;
    QCheckBox* _enableAutoCompletionCheck = nullptr;
    QLabel* _thresholdLabel = nullptr;
    QSpinBox* _thresholdSpin = nullptr;

    QGroupBox* _autoInsertGroup = nullptr;
    QCheckBox* _autoInsertBracketsCheck = nullptr;
    QCheckBox* _autoInsertQuotesCheck = nullptr;

    QGroupBox* _autoIndentGroup = nullptr;
    QCheckBox* _enableAutoIndentCheck = nullptr;

    QGroupBox* _completionSourceGroup = nullptr;
    QComboBox* _completionSourceCombo = nullptr;
    QCheckBox* _ignoreNumbersCheck = nullptr;

    // Settings
    bool _enableAutoCompletion = true;
    int _autoCompletionThreshold = 1;
    bool _autoInsertBrackets = true;
    bool _autoInsertQuotes = true;
    bool _enableAutoIndent = true;
    int _autoCompletionSource = 0; // 0=all, 1=words, 2=functions, 3=words+functions
    bool _autoCompletionIgnoreNumbers = false;
};

// ============================================================================
// MultiInstanceSubDlg - Multi-instance behavior
// ============================================================================
class MultiInstanceSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit MultiInstanceSubDlg(QWidget* parent = nullptr);
    ~MultiInstanceSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onMultiInstanceModeChanged(int index);
    void onDateTimeFormatChanged(const QString& format);
    void onUseCustomDateTimeToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _multiInstanceGroup = nullptr;
    QComboBox* _multiInstanceCombo = nullptr;
    QLabel* _multiInstanceLabel = nullptr;

    QGroupBox* _dateTimeFormatGroup = nullptr;
    QCheckBox* _useCustomDateTimeCheck = nullptr;
    QLineEdit* _dateTimeFormatEdit = nullptr;
    QLabel* _dateTimePreviewLabel = nullptr;

    // Settings
    int _multiInstanceMode = 0; // 0=mono, 1=multi, 2=session
    bool _useCustomDateTime = false;
    QString _dateTimeFormat;
};

// ============================================================================
// DelimiterSubDlg - Word delimiter settings
// ============================================================================
class DelimiterSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit DelimiterSubDlg(QWidget* parent = nullptr);
    ~DelimiterSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onDelimiterListChanged();
    void onWordCharListChanged();
    void onDefaultWordCharsToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _delimiterGroup = nullptr;
    QLabel* _delimiterListLabel = nullptr;
    QLineEdit* _delimiterListEdit = nullptr;

    QGroupBox* _wordCharGroup = nullptr;
    QRadioButton* _defaultWordCharsRadio = nullptr;
    QRadioButton* _customWordCharsRadio = nullptr;
    QLineEdit* _customWordCharsEdit = nullptr;

    // Settings
    QString _delimiterList;
    bool _useDefaultWordChars = true;
    QString _customWordChars;
};

// ============================================================================
// PerformanceSubDlg - Large file restrictions
// ============================================================================
class PerformanceSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit PerformanceSubDlg(QWidget* parent = nullptr);
    ~PerformanceSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onLargeFileRestrictionToggled(bool checked);
    void onFileSizeChanged(int value);
    void onAllowBraceMatchToggled(bool checked);
    void onAllowAutoCompletionToggled(bool checked);
    void onAllowSmartHiliteToggled(bool checked);
    void onAllowClickableLinkToggled(bool checked);
    void onDeactivateWordWrapToggled(bool checked);

private:
    void setupUI();
    void connectSignals();
    void updateEnabledState();

    // UI Components
    QGroupBox* _performanceGroup = nullptr;
    QCheckBox* _enableRestrictionCheck = nullptr;
    QLabel* _fileSizeLabel = nullptr;
    QSpinBox* _fileSizeSpin = nullptr;
    QLabel* _fileSizeUnitLabel = nullptr;

    QGroupBox* _restrictionsGroup = nullptr;
    QCheckBox* _allowBraceMatchCheck = nullptr;
    QCheckBox* _allowAutoCompletionCheck = nullptr;
    QCheckBox* _allowSmartHiliteCheck = nullptr;
    QCheckBox* _allowClickableLinkCheck = nullptr;
    QCheckBox* _deactivateWordWrapCheck = nullptr;

    // Settings
    bool _largeFileRestrictionEnabled = true;
    int _largeFileSizeMB = 200;
    bool _allowBraceMatch = false;
    bool _allowAutoCompletion = false;
    bool _allowSmartHilite = false;
    bool _allowClickableLink = false;
    bool _deactivateWordWrap = true;
};

// ============================================================================
// CloudLinkSubDlg - Cloud/OneDrive settings
// ============================================================================
class CloudLinkSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit CloudLinkSubDlg(QWidget* parent = nullptr);
    ~CloudLinkSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onCloudPathChanged(const QString& path);
    void onBrowseCloudPathClicked();
    void onCloudSyncToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _cloudGroup = nullptr;
    QCheckBox* _enableCloudSyncCheck = nullptr;
    QLabel* _cloudPathLabel = nullptr;
    QLineEdit* _cloudPathEdit = nullptr;
    QPushButton* _browseButton = nullptr;

    // Settings
    bool _cloudSyncEnabled = false;
    QString _cloudPath;
};

// ============================================================================
// SearchEngineSubDlg - Search engine preferences
// ============================================================================
class SearchEngineSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit SearchEngineSubDlg(QWidget* parent = nullptr);
    ~SearchEngineSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onSearchEngineChanged(int index);
    void onCustomUrlChanged(const QString& url);
    void onSetDefaultClicked();

private:
    void setupUI();
    void connectSignals();
    void populateSearchEngines();

    // UI Components
    QGroupBox* _searchEngineGroup = nullptr;
    QComboBox* _searchEngineCombo = nullptr;
    QLabel* _customUrlLabel = nullptr;
    QLineEdit* _customUrlEdit = nullptr;
    QPushButton* _setDefaultButton = nullptr;

    // Settings
    int _selectedSearchEngine = 0; // 0=Google, 1=Bing, 2=DuckDuckGo, 3=custom
    QString _customSearchUrl;
};

// ============================================================================
// MISCSubDlg - Miscellaneous settings
// ============================================================================
class MISCSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit MISCSubDlg(QWidget* parent = nullptr);
    ~MISCSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onMinimizeToTrayToggled(bool checked);
    void onAutoUpdateToggled(bool checked);
    void onEnableNotificationsToggled(bool checked);
    void onFileAutoDetectionChanged(int index);
    void onMuteSoundsToggled(bool checked);
    void onConfirmExitToggled(bool checked);
    void onConfirmDeleteToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _generalMiscGroup = nullptr;
    QCheckBox* _minimizeToTrayCheck = nullptr;
    QCheckBox* _autoUpdateCheck = nullptr;
    QCheckBox* _enableNotificationsCheck = nullptr;
    QCheckBox* _muteSoundsCheck = nullptr;
    QCheckBox* _confirmExitCheck = nullptr;
    QCheckBox* _confirmDeleteCheck = nullptr;

    QGroupBox* _fileDetectionGroup = nullptr;
    QComboBox* _fileAutoDetectionCombo = nullptr;
    QLabel* _fileAutoDetectionLabel = nullptr;

    // Settings
    bool _minimizeToTray = false;
    bool _autoUpdate = true;
    bool _enableNotifications = true;
    int _fileAutoDetection = 1; // 0=disable, 1=enable, 2=update silently
    bool _muteSounds = false;
    bool _confirmExit = true;
    bool _confirmDelete = true;
};

// ============================================================================
// PreferenceDlg - Main Preferences Dialog
// ============================================================================
class PreferenceDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit PreferenceDlg(QWidget* parent = nullptr);
    ~PreferenceDlg() override;

    void doDialog();
    void doDialog(bool isRTL);

    // Navigation methods
    void showPage(const QString& internalName);
    void showPage(int index);
    int getCurrentPageIndex() const;

private slots:
    void onCategoryChanged(int index);
    void onCategoryItemClicked(QListWidgetItem* item);
    void onOKClicked();
    void onCancelClicked();
    void onApplyClicked();

private:
    void setupUI();
    void createSubPages();
    void loadSettings();
    bool saveSettings();
    void connectSignals();
    bool run_dlgProc(QEvent* event) override;

    // UI Components
    QWidget* _mainWidget = nullptr;
    QListWidget* _categoryList = nullptr;
    QStackedWidget* _pagesStack = nullptr;

    QPushButton* _okButton = nullptr;
    QPushButton* _cancelButton = nullptr;
    QPushButton* _applyButton = nullptr;

    // Sub-page instances (order matches Windows PreferenceDlg)
    GeneralSubDlg* _generalPage = nullptr;
    ToolbarSubDlg* _toolbarPage = nullptr;
    TabbarSubDlg* _tabbarPage = nullptr;
    EditingSubDlg* _editingPage = nullptr;
    Editing2SubDlg* _editing2Page = nullptr;
    DarkModeSubDlg* _darkModePage = nullptr;
    MarginsBorderEdgeSubDlg* _marginsBorderEdgePage = nullptr;
    NewDocumentSubDlg* _newDocumentPage = nullptr;
    DefaultDirectorySubDlg* _defaultDirectoryPage = nullptr;
    RecentFilesHistorySubDlg* _recentFilesHistoryPage = nullptr;
    FileAssocSubDlg* _fileAssocPage = nullptr;
    LanguageSubDlg* _languagePage = nullptr;
    IndentationSubDlg* _indentationPage = nullptr;
    HighlightingSubDlg* _highlightingPage = nullptr;
    PrintSubDlg* _printPage = nullptr;
    SearchingSubDlg* _searchingPage = nullptr;
    BackupSubDlg* _backupPage = nullptr;
    AutoCompletionSubDlg* _autoCompletionPage = nullptr;
    MultiInstanceSubDlg* _multiInstancePage = nullptr;
    DelimiterSubDlg* _delimiterPage = nullptr;
    PerformanceSubDlg* _performancePage = nullptr;
    CloudLinkSubDlg* _cloudLinkPage = nullptr;
    SearchEngineSubDlg* _searchEnginePage = nullptr;
    MISCSubDlg* _miscPage = nullptr;

    // Category list
    QVector<CategoryInfo> _categories;

    // Settings interface
    PlatformLayer::ISettings* _settings = nullptr;
};

} // namespace QtControls
