// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "Notepad_plus_Window.h"

#include "../../menuCmdID.h"
#include "../../resource.h"
#include "../../Notepad_plus.h"
#include "../../Parameters.h"
#include "../../localization.h"
#include "../../MISC/PluginsManager/Notepad_plus_msgs.h"

// Dialog includes
#include "../FindReplace/FindReplaceDlg.h"
#include "../GoToLine/GoToLineDlg.h"
#include "../Preference/preferenceDlg.h"
#include "../WordStyleDlg/WordStyleDlg.h"
#include "../ShortcutMapper/ShortcutMapper.h"
#include "../RunDlg/RunDlg.h"
#include "../RunMacroDlg/RunMacroDlg.h"
#include "../AboutDlg/AboutDlg.h"
#include "../AboutDlg/CmdLineArgsDlg.h"
#include "../AboutDlg/DebugInfoDlg.h"
#include "../UserDefineDialog/UserDefineDialog.h"
#include "../ScintillaComponent/ScintillaEditView.h"
#include "../ShortcutManager/ShortcutManager.h"
#include "../../WinControls/PluginsAdmin/pluginsAdminRes.h"
#include "../../EncodingMapper.h"
#include "../../QtCore/Buffer.h"
#include "../DocTabView/DocTabView.h"
#include "../../ScintillaComponent/AutoCompletion.h"
#include "../../ScintillaComponent/xmlMatchedTagsHighlighter.h"
#include "ScintillaEditBase.h"

#include <QApplication>
#include <QStyle>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <QSettings>
#include <QScreen>

#include <algorithm>
#include <iostream>
#include <memory>
#include <QActionGroup>
#include <QProcess>
#include <QDesktopServices>
#include <QDebug>
#include <QDateTime>
#include <QLocale>
#include <QClipboard>

// Plugin message dispatcher (defined in NppPluginMessages.cpp, global namespace)
extern void nppPluginMessageDispatcher_register(Notepad_plus* pNpp);
extern void nppPluginMessageDispatcher_unregister();

namespace QtControls {

// Maps a Windows codepage number to a display name for the status bar
static QString charsetEncodingName(int codepage)
{
    switch (codepage) {
        case 1250: return "Windows-1250";
        case 1251: return "Windows-1251";
        case 1252: return "Windows-1252";
        case 1253: return "Windows-1253";
        case 1254: return "Windows-1254";
        case 1255: return "Windows-1255";
        case 1256: return "Windows-1256";
        case 1257: return "Windows-1257";
        case 1258: return "Windows-1258";
        case 28591: return "ISO 8859-1";
        case 28592: return "ISO 8859-2";
        case 28593: return "ISO 8859-3";
        case 28594: return "ISO 8859-4";
        case 28595: return "ISO 8859-5";
        case 28596: return "ISO 8859-6";
        case 28597: return "ISO 8859-7";
        case 28598: return "ISO 8859-8";
        case 28599: return "ISO 8859-9";
        case 28603: return "ISO 8859-13";
        case 28604: return "ISO 8859-14";
        case 28605: return "ISO 8859-15";
        case 437: return "OEM 437";
        case 720: return "OEM 720";
        case 737: return "OEM 737";
        case 775: return "OEM 775";
        case 850: return "OEM 850";
        case 852: return "OEM 852";
        case 855: return "OEM 855";
        case 857: return "OEM 857";
        case 858: return "OEM 858";
        case 860: return "OEM 860";
        case 861: return "OEM 861";
        case 862: return "OEM 862";
        case 863: return "OEM 863";
        case 865: return "OEM 865";
        case 866: return "OEM 866";
        case 869: return "OEM 869";
        case 950: return "Big5";
        case 936: return "GB2312";
        case 932: return "Shift-JIS";
        case 949: return "Windows-949";
        case 51949: return "EUC-KR";
        case 874: return "TIS-620";
        case 10007: return "Mac Cyrillic";
        case 21866: return "KOI8-U";
        case 20866: return "KOI8-R";
        default: return QString("CP %1").arg(codepage);
    }
}

namespace MainWindow {

// ============================================================================
// Constructor / Destructor
// ============================================================================

MainWindow::MainWindow()
    : QMainWindow(nullptr)
{
    // Set window properties
    setWindowTitle("Notepad++");
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    destroy();
}

// ============================================================================
// Initialization
// ============================================================================

bool MainWindow::init(Notepad_plus* pNotepad_plus)
{
    if (!pNotepad_plus) {
        return false;
    }

    _pNotepad_plus = pNotepad_plus;

    // Initialize shortcut manager first (before UI setup)
    _shortcutManager = ShortcutManager::getInstance();
    _shortcutManager->setParent(this);

    // Set up command callback for shortcut manager
    _shortcutManager->setCommandCallback([this](int commandId) {
        // Route commands to Notepad_plus core
        if (_pNotepad_plus) {
            // Use the core's command handling via WM_COMMAND-like mechanism
            // This allows shortcuts to trigger the same handlers as menu items
            switch (commandId) {
                // File commands
                case IDM_FILE_NEW: onFileNew(); break;
                case IDM_FILE_OPEN: onFileOpen(); break;
                case IDM_FILE_SAVE: onFileSave(); break;
                case IDM_FILE_SAVEAS: onFileSaveAs(); break;
                case IDM_FILE_SAVEALL: onFileSaveAll(); break;
                case IDM_FILE_CLOSE: onFileClose(); break;
                case IDM_FILE_CLOSEALL: onFileCloseAll(); break;
                case IDM_FILE_CLOSEALL_BUT_CURRENT: onFileCloseAllButCurrent(); break;
                case IDM_FILE_CLOSEALL_BUT_PINNED: onFileCloseAllButPinned(); break;
                case IDM_FILE_CLOSEALL_TOLEFT: onFileCloseAllToLeft(); break;
                case IDM_FILE_CLOSEALL_TORIGHT: onFileCloseAllToRight(); break;
                case IDM_FILE_CLOSEALL_UNCHANGED: onFileCloseAllUnchanged(); break;
                case IDM_FILE_PRINT: onFilePrint(); break;
                case IDM_FILE_PRINTNOW: onFilePrintNow(); break;
                case IDM_FILE_EXIT: onFileExit(); break;

                // Edit commands
                case IDM_EDIT_UNDO: onEditUndo(); break;
                case IDM_EDIT_REDO: onEditRedo(); break;
                case IDM_EDIT_CUT: onEditCut(); break;
                case IDM_EDIT_COPY: onEditCopy(); break;
                case IDM_EDIT_PASTE: onEditPaste(); break;
                case IDM_EDIT_DELETE: onEditDelete(); break;
                case IDM_EDIT_SELECTALL: onEditSelectAll(); break;
                case IDM_EDIT_TOGGLEREADONLY: onEditToggleReadOnly(); break;
                case IDM_EDIT_INSERT_DATETIME_SHORT: onEditInsertDateTimeShort(); break;
                case IDM_EDIT_INSERT_DATETIME_LONG: onEditInsertDateTimeLong(); break;
                case IDM_EDIT_INSERT_DATETIME_CUSTOMIZED: onEditInsertDateTimeCustomized(); break;

                // Search commands
                case IDM_SEARCH_FIND: onSearchFind(); break;
                case IDM_SEARCH_REPLACE: onSearchReplace(); break;
                case IDM_SEARCH_FINDNEXT: onSearchFindNext(); break;
                case IDM_SEARCH_FINDPREV: onSearchFindPrev(); break;
                case IDM_SEARCH_GOTOLINE: onSearchGoToLine(); break;

                // View commands
                case IDM_VIEW_FULLSCREENTOGGLE: onViewFullScreen(); break;
                case IDM_VIEW_POSTIT: onViewPostIt(); break;
                case IDM_VIEW_ALWAYSONTOP: onViewAlwaysOnTop(); break;
                case IDM_VIEW_WRAP: onViewWordWrap(); break;
                case IDM_VIEW_TAB_SPACE: onViewShowWhiteSpace(); break;
                case IDM_VIEW_EOL: onViewShowEOL(); break;
                case IDM_VIEW_INDENT_GUIDE: onViewShowIndentGuide(); break;
                case IDM_VIEW_FUNC_LIST: onViewFunctionList(); break;
                case IDM_VIEW_PROJECT_PANEL_1: onViewProjectPanel(); break;
                case IDM_VIEW_DOC_MAP: onViewDocumentMap(); break;
                case IDM_VIEW_FILEBROWSER: onViewFileBrowser(); break;
                case IDM_EDIT_CLIPBOARDHISTORY_PANEL: onViewClipboardHistory(); break;

                // Tab color commands
                case IDM_VIEW_TAB_COLOUR_NONE: onViewTabColour(-1); break;
                case IDM_VIEW_TAB_COLOUR_1: onViewTabColour(0); break;
                case IDM_VIEW_TAB_COLOUR_2: onViewTabColour(1); break;
                case IDM_VIEW_TAB_COLOUR_3: onViewTabColour(2); break;
                case IDM_VIEW_TAB_COLOUR_4: onViewTabColour(3); break;
                case IDM_VIEW_TAB_COLOUR_5: onViewTabColour(4); break;

                // Text direction commands
                case IDM_EDIT_RTL: onEditTextDirection(true); break;
                case IDM_EDIT_LTR: onEditTextDirection(false); break;

                // Hide lines
                case IDM_VIEW_HIDELINES: onViewHideLines(); break;

                // Change history navigation
                case IDM_SEARCH_CHANGED_NEXT: onSearchChangedNext(); break;
                case IDM_SEARCH_CHANGED_PREV: onSearchChangedPrev(); break;
                case IDM_SEARCH_CLEAR_CHANGE_HISTORY: onSearchClearChangeHistory(); break;

                // Macro commands
                case IDM_MACRO_STARTRECORDINGMACRO: onMacroStartRecording(); break;
                case IDM_MACRO_STOPRECORDINGMACRO: onMacroStopRecording(); break;
                case IDM_MACRO_PLAYBACKRECORDEDMACRO: onMacroPlayback(); break;
                case IDM_MACRO_RUNMULTIMACRODLG: onMacroRunMultiple(); break;

                // Run commands
                case IDM_EXECUTE: onRunRun(); break;

                // Default: try to handle via core if possible
                default:
                    // For unhandled commands, we could emit a signal or log
                    qDebug() << "Unhandled command ID:" << commandId;
                    break;
            }
        }
    });

    // Setup UI components
    std::cout << "[MainWindow::init] About to setupUI..." << std::endl;
    setupUI();
    std::cout << "[MainWindow::init] setupUI done. About to connectSignals..." << std::endl;
    connectSignals();
    std::cout << "[MainWindow::init] connectSignals done. About to createDockWindows..." << std::endl;
    createDockWindows();
    std::cout << "[MainWindow::init] createDockWindows done. About to loadSettings..." << std::endl;
    loadSettings();
    std::cout << "[MainWindow::init] loadSettings done. About to updateMenuState..." << std::endl;

    // Initialize menu state
    std::cout << "[MainWindow::init] About to call updateMenuState..." << std::endl;
    updateMenuState();
    std::cout << "[MainWindow::init] updateMenuState done." << std::endl;

    // Initialize toolbar state
    std::cout << "[MainWindow::init] About to call updateToolBarState..." << std::endl;
    updateToolBarState();
    std::cout << "[MainWindow::init] updateToolBarState done." << std::endl;

    // Initialize status bar
    std::cout << "[MainWindow::init] About to call updateStatusBar..." << std::endl;
    updateStatusBar();
    std::cout << "[MainWindow::init] updateStatusBar done." << std::endl;

    // Apply shortcuts from NppParameters to registered actions
    std::cout << "[MainWindow::init] About to call applyShortcuts..." << std::endl;
    _shortcutManager->applyShortcuts();
    std::cout << "[MainWindow::init] applyShortcuts done." << std::endl;

    // Connect shortcut manager signals
    std::cout << "[MainWindow::init] About to connect shortcut signals..." << std::endl;
    connect(_shortcutManager, &ShortcutManager::shortcutsReloaded,
            this, &MainWindow::refreshShortcuts);
    std::cout << "[MainWindow::init] Shortcut signals connected." << std::endl;

    // Initialize plugin manager
    std::cout << "[MainWindow::init] About to call initPlugins..." << std::endl;
    initPlugins();
    std::cout << "[MainWindow::init] initPlugins done." << std::endl;

    // Notify plugins that Notepad++ is ready
    {
        SCNotification scnN{};
        scnN.nmhdr.code = NPPN_READY;
        scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(this);
        scnN.nmhdr.idFrom = 0;
        _pNotepad_plus->getPluginsManager().notify(&scnN);
    }

    // Connect toolbar icon set change to plugin notification
    if (_mainToolBar)
    {
        connect(_mainToolBar, &ToolBar::iconSetChanged, this, [this](int state)
        {
            SCNotification scnN{};
            scnN.nmhdr.code = NPPN_TOOLBARICONSETCHANGED;
            scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(this);
            scnN.nmhdr.idFrom = static_cast<uptr_t>(state);
            _pNotepad_plus->getPluginsManager().notify(&scnN);
        });
    }

    // Set up external lexer buffer callback on edit views
    auto externalLexerCb = [](BufferID bufID, void* userData)
    {
        auto* self = static_cast<MainWindow*>(userData);
        SCNotification scnN{};
        scnN.nmhdr.code = NPPN_EXTERNALLEXERBUFFER;
        scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(self);
        scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(bufID);
        self->_pNotepad_plus->getPluginsManager().notify(&scnN);
    };
    if (getMainEditView())
        getMainEditView()->setExternalLexerBufferCallback(externalLexerCb, this);
    if (getSubEditView())
        getSubEditView()->setExternalLexerBufferCallback(externalLexerCb, this);

    std::cout << "[MainWindow::init] Initialization complete!" << std::endl;
    return true;
}

void MainWindow::setupUI()
{
    // Validate Notepad_plus pointer
    if (!_pNotepad_plus) {
        std::cerr << "[MainWindow::setupUI] ERROR: _pNotepad_plus is null!" << std::endl;
        return;
    }

    // Create central widget with splitter for editors
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create editor splitter
    _editorSplitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(_editorSplitter);

    std::cout << "[MainWindow::setupUI] Initializing main edit view..." << std::endl;

    // Initialize main edit view - check for null
    ScintillaEditView* mainEditView = _pNotepad_plus->getMainEditView();
    if (!mainEditView) {
        std::cerr << "[MainWindow::setupUI] ERROR: Main edit view is null!" << std::endl;
        return;
    }
    mainEditView->init(_editorSplitter);
    std::cout << "[MainWindow::setupUI] Main edit view widget: " << _pNotepad_plus->getMainEditView()->getWidget() << std::endl;

    // Create container for main view (tab bar + editor)
    auto* mainContainer = new QWidget(_editorSplitter);
    auto* mainVLayout = new QVBoxLayout(mainContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    // Initialize main doc tab and add to container
    _mainDocTab = _pNotepad_plus->getMainDocTab();
    if (!_mainDocTab) {
        std::cerr << "[MainWindow::setupUI] ERROR: Main doc tab is null!" << std::endl;
        return;
    }
    _mainDocTab->init(mainContainer, mainEditView);
    mainVLayout->addWidget(_mainDocTab->getWidget());

    // Add main editor to container
    QWidget* mainEditWidget = mainEditView->getWidget();
    if (!mainEditWidget) {
        std::cerr << "[MainWindow::setupUI] ERROR: Main edit widget is null!" << std::endl;
        return;
    }
    std::cout << "[MainWindow::setupUI] Adding main edit widget to layout: " << mainEditWidget << std::endl;
    mainVLayout->addWidget(mainEditWidget, 1);

    // Initialize sub edit view
    std::cout << "[MainWindow::setupUI] Initializing sub edit view..." << std::endl;
    ScintillaEditView* subEditView = _pNotepad_plus->getSubEditView();
    if (!subEditView) {
        std::cerr << "[MainWindow::setupUI] ERROR: Sub edit view is null!" << std::endl;
        return;
    }
    subEditView->init(_editorSplitter);
    auto* subContainer = new QWidget(_editorSplitter);
    auto* subVLayout = new QVBoxLayout(subContainer);
    subVLayout->setContentsMargins(0, 0, 0, 0);
    subVLayout->setSpacing(0);
    _subDocTab = _pNotepad_plus->getSubDocTab();
    if (!_subDocTab) {
        std::cerr << "[MainWindow::setupUI] ERROR: Sub doc tab is null!" << std::endl;
        return;
    }
    _subDocTab->init(subContainer, subEditView);
    subVLayout->addWidget(_subDocTab->getWidget());
    QWidget* subEditWidget = subEditView->getWidget();
    if (!subEditWidget) {
        std::cerr << "[MainWindow::setupUI] ERROR: Sub edit widget is null!" << std::endl;
        return;
    }
    subVLayout->addWidget(subEditWidget, 1);

    // Add containers to splitter and hide sub view
    _editorSplitter->addWidget(mainContainer);
    _editorSplitter->addWidget(subContainer);
    subContainer->hide();  // Only show main view by default

    // Set initial splitter sizes (80% main, 20% sub - but sub is hidden)
    QList<int> sizes;
    sizes << 800 << 200;
    _editorSplitter->setSizes(sizes);

    // Ensure main container and edit widget are visible
    mainContainer->show();
    mainEditWidget->show();

    // Also ensure central widget and splitter are visible
    centralWidget->show();
    _editorSplitter->show();

    // Force layout update to ensure proper sizing
    centralWidget->updateGeometry();
    _editorSplitter->updateGeometry();
    mainContainer->updateGeometry();

    std::cout << "[MainWindow::setupUI] Forced layout update" << std::endl;

    std::cout << "[MainWindow::setupUI] Layout setup complete." << std::endl;
    std::cout << "[MainWindow::setupUI] centralWidget visible: " << centralWidget->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] _editorSplitter visible: " << _editorSplitter->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainContainer visible after show(): " << mainContainer->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget visible after show(): " << mainEditWidget->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainContainer visible: " << mainContainer->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget visible: " << mainEditWidget->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget size: " << mainEditWidget->width() << "x" << mainEditWidget->height() << std::endl;
    std::cout << "[MainWindow::setupUI] _editorSplitter sizes: " << _editorSplitter->sizes()[0] << ", " << _editorSplitter->sizes()[1] << std::endl;

    std::cout << "[MainWindow::setupUI] About to init menu bar..." << std::endl;
    // Initialize menu bar
    initMenuBar();
    std::cout << "[MainWindow::setupUI] Menu bar done." << std::endl;

    // Apply localization to menus
    NppParameters& nppParam = NppParameters::getInstance();
    NativeLangSpeaker* pNativeLangSpeaker = nppParam.getNativeLangSpeaker();
    if (pNativeLangSpeaker)
    {
        pNativeLangSpeaker->changeMenuLangQt(menuBar());
    }
    std::cout << "[MainWindow::setupUI] Menu localization done." << std::endl;

    // Add more debugging for widget hierarchy
    std::cout << "[MainWindow::setupUI] mainContainer parent: " << mainContainer->parentWidget() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget parent: " << mainEditWidget->parentWidget() << std::endl;
    std::cout << "[MainWindow::setupUI] _editorSplitter parent: " << _editorSplitter->parentWidget() << std::endl;

    std::cout << "[MainWindow::setupUI] About to init tool bar..." << std::endl;
    // Initialize toolbar
    initToolBar();
    std::cout << "[MainWindow::setupUI] Tool bar done." << std::endl;

    std::cout << "[MainWindow::setupUI] About to init status bar..." << std::endl;
    // Initialize status bar
    initStatusBar();
    std::cout << "[MainWindow::setupUI] Status bar done." << std::endl;

    std::cout << "[MainWindow::setupUI] About to create docking manager..." << std::endl;
    // Create docking manager
    _dockingManager = new DockingManager();
    std::cout << "[MainWindow::setupUI] Docking manager created." << std::endl;
    std::cout << "[MainWindow::setupUI] About to init docking manager..." << std::endl;
    _dockingManager->init(this);
    std::cout << "[MainWindow::setupUI] Docking manager init done." << std::endl;

    // Create update timer - DISABLED to prevent crash
    // _updateTimer = new QTimer(this);
    // connect(_updateTimer, &QTimer::timeout, this, [this]() {
    //     updateStatusBar();
    //     updateMenuState();
    // });
    // _updateTimer->start(500); // Update every 500ms
}

void MainWindow::connectSignals()
{
    // Connect tab close signals from DocTabView to MainWindow slots
    if (_mainDocTab) {
        connect(_mainDocTab, &DocTabView::tabCloseRequested,
                this, &MainWindow::onMainTabCloseRequested);
        // Connect tab change signal to activate buffer
        connect(_mainDocTab, &DocTabView::currentChanged,
                this, &MainWindow::onMainTabChanged);
    }
    if (_subDocTab) {
        connect(_subDocTab, &DocTabView::tabCloseRequested,
                this, &MainWindow::onSubTabCloseRequested);
        // Connect tab change signal to activate buffer
        connect(_subDocTab, &DocTabView::currentChanged,
                this, &MainWindow::onSubTabChanged);
    }

    // Connect Scintilla charAdded signal to auto-completion engine
    if (_pNotepad_plus) {
        ScintillaEditView* mainEditView = _pNotepad_plus->getMainEditView();
        ScintillaEditView* subEditView = _pNotepad_plus->getSubEditView();

        if (mainEditView && mainEditView->getWidget()) {
            auto* mainSciWidget = qobject_cast<ScintillaEditBase*>(mainEditView->getWidget());
            if (mainSciWidget) {
                connect(mainSciWidget, &ScintillaEditBase::charAdded, this, [this](int ch) {
                    if (!_pNotepad_plus) return;
                    _pNotepad_plus->maintainIndentation(static_cast<wchar_t>(ch));
                    AutoCompletion* autoC = _pNotepad_plus->getAutoCompleteMain();
                    if (autoC) {
                        autoC->update(ch);
                    }
                });
            }
        }

        if (subEditView && subEditView->getWidget()) {
            auto* subSciWidget = qobject_cast<ScintillaEditBase*>(subEditView->getWidget());
            if (subSciWidget) {
                connect(subSciWidget, &ScintillaEditBase::charAdded, this, [this](int ch) {
                    if (!_pNotepad_plus) return;
                    _pNotepad_plus->maintainIndentation(static_cast<wchar_t>(ch));
                    AutoCompletion* autoC = _pNotepad_plus->getAutoCompleteSub();
                    if (autoC) {
                        autoC->update(ch);
                    }
                });
            }
        }
    }
}

void MainWindow::createDockWindows()
{
    // Create Function List panel
    _functionListPanel = new FunctionListPanel(this);
    _functionListPanel->init(nullptr); // Will be set properly when core is ready
    _dockingManager->addPanel("functionList", _functionListPanel->getWidget(),
                               DockingManager::DockArea::Left, tr("Function List"));

    // Create Project Panel
    _projectPanel = new ProjectPanel(this);
    _projectPanel->init(nullptr);
    _dockingManager->addPanel("projectPanel", _projectPanel->getWidget(),
                               DockingManager::DockArea::Left, tr("Project"));

    // Create Document Map panel
    _documentMap = new DocumentMap(this);
    if (_pNotepad_plus)
    {
        _documentMap->init(_pNotepad_plus->getEditViewPtr());
    }
    else
    {
        _documentMap->init(nullptr);
    }
    _dockingManager->addPanel("documentMap", _documentMap->getWidget(),
                               DockingManager::DockArea::Right, tr("Document Map"));

    // Create Clipboard History panel
    _clipboardHistoryPanel = new ClipboardHistoryPanel(this);
    _clipboardHistoryPanel->init(nullptr);
    _dockingManager->addPanel("clipboardHistory", _clipboardHistoryPanel->getWidget(),
                               DockingManager::DockArea::Right, tr("Clipboard History"));

    // Create File Browser panel
    _fileBrowser = new FileBrowser(this);
    _fileBrowser->init(nullptr);
    _dockingManager->addPanel("fileBrowser", _fileBrowser->getWidget(),
                               DockingManager::DockArea::Left, tr("Folder as Workspace"));

    // Connect file browser's open request to file opening
    connect(_fileBrowser, &FileBrowser::fileOpenRequested, this, [this](const QString& filePath) {
        if (_pNotepad_plus) {
            _pNotepad_plus->doOpen(filePath.toStdWString());
        }
    });

    // Connect Scintilla painted() signals to Document Map scroll sync
    if (_documentMap && _pNotepad_plus)
    {
        ScintillaEditView* mainEditView = _pNotepad_plus->getMainEditView();
        ScintillaEditView* subEditView = _pNotepad_plus->getSubEditView();

        if (mainEditView && mainEditView->getWidget())
        {
            auto* mainSci = qobject_cast<ScintillaEditBase*>(mainEditView->getWidget());
            if (mainSci)
            {
                connect(mainSci, &ScintillaEditBase::painted,
                        _documentMap, &DocumentMap::onMainEditorScrolled);
            }
        }

        if (subEditView && subEditView->getWidget())
        {
            auto* subSci = qobject_cast<ScintillaEditBase*>(subEditView->getWidget());
            if (subSci)
            {
                connect(subSci, &ScintillaEditBase::painted,
                        _documentMap, &DocumentMap::onMainEditorScrolled);
            }
        }
    }

    // Connect Scintilla updateUi signals for sync scroll and XML tag matching
    if (_pNotepad_plus)
    {
        ScintillaEditView* mainEditView = _pNotepad_plus->getMainEditView();
        ScintillaEditView* subEditView = _pNotepad_plus->getSubEditView();

        auto connectUpdateUi = [this](ScintillaEditView* editView) {
            if (!editView || !editView->getWidget())
                return;
            auto* sciWidget = qobject_cast<ScintillaEditBase*>(editView->getWidget());
            if (!sciWidget)
                return;

            connect(sciWidget, &ScintillaEditBase::updateUi, this,
                [this, editView](Scintilla::Update updated) {
                    if (!_pNotepad_plus)
                        return;

                    // Synchronized scrolling
                    int updateFlags = static_cast<int>(updated);
                    if ((updateFlags & SC_UPDATE_V_SCROLL) || (updateFlags & SC_UPDATE_H_SCROLL))
                    {
                        if (_pNotepad_plus->getSyncInfo().doSync())
                        {
                            _pNotepad_plus->doSynScroll(editView);
                        }
                    }

                    // XML tag matching on cursor movement
                    if (updateFlags & SC_UPDATE_SELECTION)
                    {
                        const NppGUI& nppGui = NppParameters::getInstance().getNppGUI();
                        if (nppGui._enableTagsMatchHilite)
                        {
                            ScintillaEditView* activeView = _pNotepad_plus->getCurrentEditView();
                            if (activeView == editView)
                            {
                                XmlMatchedTagsHighlighter xmlTagMatchHiliter(activeView);
                                xmlTagMatchHiliter.tagMatch(nppGui._enableTagAttrsHilite);
                            }
                        }
                    }
                });
        };

        connectUpdateUi(mainEditView);
        connectUpdateUi(subEditView);
    }

    // Initially hide all panels
    _dockingManager->hidePanel("functionList");
    _dockingManager->hidePanel("projectPanel");
    _dockingManager->hidePanel("documentMap");
    _dockingManager->hidePanel("clipboardHistory");
    _dockingManager->hidePanel("fileBrowser");
}

// ============================================================================
// Window Interface
// ============================================================================

void MainWindow::destroy()
{
    // Save settings before destroying
    saveSettings();

    // Clean up panels
    if (_functionListPanel) {
        delete _functionListPanel;
        _functionListPanel = nullptr;
    }

    if (_projectPanel) {
        delete _projectPanel;
        _projectPanel = nullptr;
    }

    if (_documentMap) {
        delete _documentMap;
        _documentMap = nullptr;
    }

    if (_clipboardHistoryPanel) {
        delete _clipboardHistoryPanel;
        _clipboardHistoryPanel = nullptr;
    }

    if (_fileBrowser) {
        delete _fileBrowser;
        _fileBrowser = nullptr;
    }

    if (_dockingManager) {
        _dockingManager->destroy();
        delete _dockingManager;
        _dockingManager = nullptr;
    }

    if (_mainToolBar) {
        _mainToolBar->destroy();
        delete _mainToolBar;
        _mainToolBar = nullptr;
    }

    if (_statusBar) {
        _statusBar->destroy();
        delete _statusBar;
        _statusBar = nullptr;
    }

    if (_tabBar) {
        _tabBar->destroy();
        delete _tabBar;
        _tabBar = nullptr;
    }
}

void MainWindow::display(bool toShow)
{
    if (toShow) {
        show();
        raise();
        activateWindow();
    } else {
        hide();
    }
}

void MainWindow::reSizeTo(QRect& rc)
{
    setGeometry(rc);
}

// ============================================================================
// Menu Operations
// ============================================================================

void MainWindow::initMenuBar()
{
    _menuBar = new QMenuBar(this);
    setMenuBar(_menuBar);

    createFileMenu();
    createEditMenu();
    createSearchMenu();
    createViewMenu();
    createEncodingMenu();
    createLanguageMenu();
    createSettingsMenu();
    createMacroMenu();
    createRunMenu();
    createWindowMenu();
    createHelpMenu();
    createPluginsMenu();

    // Register all menu actions with shortcut manager
    registerMenuActionsWithShortcutManager();
}

void MainWindow::createFileMenu()
{
    _fileMenu = _menuBar->addMenu(tr("&File"));

    // New
    auto* newAction = _fileMenu->addAction(tr("&New"), this, &MainWindow::onFileNew);
    newAction->setShortcut(QKeySequence::New);

    // Open
    auto* openAction = _fileMenu->addAction(tr("&Open..."), this, &MainWindow::onFileOpen);
    openAction->setShortcut(QKeySequence::Open);

    _fileMenu->addSeparator();

    // Save
    auto* saveAction = _fileMenu->addAction(tr("&Save"), this, &MainWindow::onFileSave);
    saveAction->setShortcut(QKeySequence::Save);

    // Save As
    auto* saveAsAction = _fileMenu->addAction(tr("Save &As..."), this, &MainWindow::onFileSaveAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);

    // Save All
    auto* saveAllAction = _fileMenu->addAction(tr("Save A&ll"), this, &MainWindow::onFileSaveAll);

    _fileMenu->addSeparator();

    // Close
    auto* closeAction = _fileMenu->addAction(tr("&Close"), this, &MainWindow::onFileClose);
    closeAction->setShortcut(QKeySequence::Close);

    // Close All
    _fileMenu->addAction(tr("Clos&e All"), this, &MainWindow::onFileCloseAll);

    // Close Multiple Tabs submenu
    auto* closeMultipleMenu = _fileMenu->addMenu(tr("Close Multiple Tabs"));
    auto* closeAllButCurrentAction = closeMultipleMenu->addAction(tr("Close All But Active Document"), this, &MainWindow::onFileCloseAllButCurrent);
    closeAllButCurrentAction->setProperty("commandId", IDM_FILE_CLOSEALL_BUT_CURRENT);
    auto* closeAllButPinnedAction = closeMultipleMenu->addAction(tr("Close All But Pinned Documents"), this, &MainWindow::onFileCloseAllButPinned);
    closeAllButPinnedAction->setProperty("commandId", IDM_FILE_CLOSEALL_BUT_PINNED);
    auto* closeAllToLeftAction = closeMultipleMenu->addAction(tr("Close All to the Left"), this, &MainWindow::onFileCloseAllToLeft);
    closeAllToLeftAction->setProperty("commandId", IDM_FILE_CLOSEALL_TOLEFT);
    auto* closeAllToRightAction = closeMultipleMenu->addAction(tr("Close All to the Right"), this, &MainWindow::onFileCloseAllToRight);
    closeAllToRightAction->setProperty("commandId", IDM_FILE_CLOSEALL_TORIGHT);
    auto* closeAllUnchangedAction = closeMultipleMenu->addAction(tr("Close All Unchanged"), this, &MainWindow::onFileCloseAllUnchanged);
    closeAllUnchangedAction->setProperty("commandId", IDM_FILE_CLOSEALL_UNCHANGED);

    _fileMenu->addSeparator();

    // Recent files submenu
    _recentFilesMenu = _fileMenu->addMenu(tr("Recent Files"));
    connect(_recentFilesMenu, &QMenu::aboutToShow, this, &MainWindow::onRecentFilesMenuAboutToShow);

    _fileMenu->addSeparator();

    // Open in Default Viewer
    auto* defaultViewerAction = _fileMenu->addAction(tr("Open in Default Viewer"), this, &MainWindow::onFileOpenInDefaultViewer);
    defaultViewerAction->setProperty("commandId", IDM_FILE_OPEN_DEFAULT_VIEWER);

    _fileMenu->addSeparator();

    // Print
    auto* printAction = _fileMenu->addAction(tr("&Print..."), this, &MainWindow::onFilePrint);
    printAction->setShortcut(QKeySequence::Print);
    printAction->setProperty("commandId", IDM_FILE_PRINT);

    // Print Now
    auto* printNowAction = _fileMenu->addAction(tr("Print No&w"), this, &MainWindow::onFilePrintNow);
    printNowAction->setProperty("commandId", IDM_FILE_PRINTNOW);

    _fileMenu->addSeparator();

    // Exit
    auto* exitAction = _fileMenu->addAction(tr("E&xit"), this, &MainWindow::onFileExit);
    exitAction->setShortcut(QKeySequence::Quit);
}

void MainWindow::createEditMenu()
{
    _editMenu = _menuBar->addMenu(tr("&Edit"));

    // Undo
    auto* undoAction = _editMenu->addAction(tr("&Undo"), this, &MainWindow::onEditUndo);
    undoAction->setShortcut(QKeySequence::Undo);

    // Redo
    auto* redoAction = _editMenu->addAction(tr("&Redo"), this, &MainWindow::onEditRedo);
    redoAction->setShortcut(QKeySequence::Redo);

    _editMenu->addSeparator();

    // Cut
    auto* cutAction = _editMenu->addAction(tr("Cu&t"), this, &MainWindow::onEditCut);
    cutAction->setShortcut(QKeySequence::Cut);

    // Copy
    auto* copyAction = _editMenu->addAction(tr("&Copy"), this, &MainWindow::onEditCopy);
    copyAction->setShortcut(QKeySequence::Copy);

    // Paste
    auto* pasteAction = _editMenu->addAction(tr("&Paste"), this, &MainWindow::onEditPaste);
    pasteAction->setShortcut(QKeySequence::Paste);

    // Delete
    auto* deleteAction = _editMenu->addAction(tr("&Delete"), this, &MainWindow::onEditDelete);
    deleteAction->setShortcut(QKeySequence::Delete);

    _editMenu->addSeparator();

    // Clipboard Special submenu
    auto* clipSpecialMenu = _editMenu->addMenu(tr("Clipboard"));
    auto* copyBinaryAction = clipSpecialMenu->addAction(tr("Copy Binary Content"), this, &MainWindow::onEditCopyBinary);
    copyBinaryAction->setProperty("commandId", IDM_EDIT_COPY_BINARY);
    auto* cutBinaryAction = clipSpecialMenu->addAction(tr("Cut Binary Content"), this, &MainWindow::onEditCutBinary);
    cutBinaryAction->setProperty("commandId", IDM_EDIT_CUT_BINARY);
    auto* pasteBinaryAction = clipSpecialMenu->addAction(tr("Paste Binary Content"), this, &MainWindow::onEditPasteBinary);
    pasteBinaryAction->setProperty("commandId", IDM_EDIT_PASTE_BINARY);
    clipSpecialMenu->addSeparator();
    auto* pasteHtmlAction = clipSpecialMenu->addAction(tr("Paste HTML Content"), this, &MainWindow::onEditPasteAsHtml);
    pasteHtmlAction->setProperty("commandId", IDM_EDIT_PASTE_AS_HTML);
    auto* pasteRtfAction = clipSpecialMenu->addAction(tr("Paste RTF Content"), this, &MainWindow::onEditPasteAsRtf);
    pasteRtfAction->setProperty("commandId", IDM_EDIT_PASTE_AS_RTF);

    _editMenu->addSeparator();

    // Select All
    auto* selectAllAction = _editMenu->addAction(tr("Select &All"), this, &MainWindow::onEditSelectAll);
    selectAllAction->setShortcut(QKeySequence::SelectAll);

    _editMenu->addSeparator();

    // Insert submenu
    auto* insertMenu = _editMenu->addMenu(tr("Insert"));
    auto* dateTimeShortAction = insertMenu->addAction(tr("Date and Time - Short"), this, &MainWindow::onEditInsertDateTimeShort);
    dateTimeShortAction->setProperty("commandId", IDM_EDIT_INSERT_DATETIME_SHORT);
    auto* dateTimeLongAction = insertMenu->addAction(tr("Date and Time - Long"), this, &MainWindow::onEditInsertDateTimeLong);
    dateTimeLongAction->setProperty("commandId", IDM_EDIT_INSERT_DATETIME_LONG);
    auto* dateTimeCustomAction = insertMenu->addAction(tr("Date and Time - Customized"), this, &MainWindow::onEditInsertDateTimeCustomized);
    dateTimeCustomAction->setProperty("commandId", IDM_EDIT_INSERT_DATETIME_CUSTOMIZED);
    insertMenu->addSeparator();
    insertMenu->addAction(tr("Full File Path"), this, &MainWindow::onEditInsertFullPath);
    insertMenu->addAction(tr("File Name"), this, &MainWindow::onEditInsertFileName);
    insertMenu->addAction(tr("Current Directory"), this, &MainWindow::onEditInsertDirPath);

    // Copy to Clipboard submenu
    auto* copyToMenu = _editMenu->addMenu(tr("Copy to Clipboard"));
    copyToMenu->addAction(tr("Current Full File Path"), this, &MainWindow::onEditCopyFullPath);
    copyToMenu->addAction(tr("Current File Name"), this, &MainWindow::onEditCopyFileName);
    copyToMenu->addAction(tr("Current Directory Path"), this, &MainWindow::onEditCopyDirPath);

    // Indent submenu
    auto* indentMenu = _editMenu->addMenu(tr("Indent"));
    indentMenu->addAction(tr("Increase Line Indent"), this, &MainWindow::onEditIncreaseIndent);
    indentMenu->addAction(tr("Decrease Line Indent"), this, &MainWindow::onEditDecreaseIndent);

    // Convert submenu
    auto* convertMenu = _editMenu->addMenu(tr("Convert Case to"));
    convertMenu->addAction(tr("Uppercase"), this, &MainWindow::onEditUpperCase);
    convertMenu->addAction(tr("Lowercase"), this, &MainWindow::onEditLowerCase);
    convertMenu->addAction(tr("Title Case"), this, &MainWindow::onEditTitleCase);

    _editMenu->addSeparator();

    // Search on Internet
    auto* searchInternetAction = _editMenu->addAction(tr("Search on Internet"), this, &MainWindow::onEditSearchOnInternet);
    searchInternetAction->setProperty("commandId", IDM_EDIT_SEARCHONINTERNET);

    _editMenu->addSeparator();

    // Read-Only toggle
    auto* readOnlyAction = _editMenu->addAction(tr("Set Read-Only"), this, &MainWindow::onEditToggleReadOnly);
    readOnlyAction->setCheckable(true);
    readOnlyAction->setProperty("commandId", IDM_EDIT_TOGGLEREADONLY);
}

void MainWindow::createSearchMenu()
{
    _searchMenu = _menuBar->addMenu(tr("&Search"));

    // Find
    auto* findAction = _searchMenu->addAction(tr("&Find..."), this, &MainWindow::onSearchFind);
    findAction->setShortcut(QKeySequence::Find);

    // Find Next
    auto* findNextAction = _searchMenu->addAction(tr("Find &Next"), this, &MainWindow::onSearchFindNext);
    findNextAction->setShortcut(QKeySequence::FindNext);

    // Find Previous
    auto* findPrevAction = _searchMenu->addAction(tr("Find &Previous"), this, &MainWindow::onSearchFindPrev);
    findPrevAction->setShortcut(QKeySequence::FindPrevious);

    _searchMenu->addSeparator();

    // Replace
    auto* replaceAction = _searchMenu->addAction(tr("&Replace..."), this, &MainWindow::onSearchReplace);
    replaceAction->setShortcut(QKeySequence::Replace);

    _searchMenu->addSeparator();

    // Go To
    auto* gotoAction = _searchMenu->addAction(tr("&Go To..."), this, &MainWindow::onSearchGoToLine);
    gotoAction->setShortcut(QKeySequence("Ctrl+G"));

    _searchMenu->addSeparator();

    // Bookmarks submenu
    auto* bookmarkMenu = _searchMenu->addMenu(tr("Bookmark"));
    bookmarkMenu->addAction(tr("Toggle Bookmark"));
    bookmarkMenu->addAction(tr("Next Bookmark"));
    bookmarkMenu->addAction(tr("Previous Bookmark"));
    bookmarkMenu->addAction(tr("Clear All Bookmarks"));

    // Find in Files
    _searchMenu->addSeparator();
    _searchMenu->addAction(tr("Find in Files..."));
}

void MainWindow::createViewMenu()
{
    _viewMenu = _menuBar->addMenu(tr("&View"));

    // View Mode submenu
    auto* viewModeMenu = _viewMenu->addMenu(tr("View Mode"));
    auto* fullScreenAction = viewModeMenu->addAction(tr("&Full Screen"), this, &MainWindow::onViewFullScreen);
    fullScreenAction->setShortcut(QKeySequence::FullScreen);

    viewModeMenu->addAction(tr("&Post-it"), this, &MainWindow::onViewPostIt);
    viewModeMenu->addAction(tr("Distraction &Free Mode"), this, &MainWindow::onViewDistractionFreeMode);

    _viewMenu->addSeparator();

    // Always on Top
    _alwaysOnTopAction = _viewMenu->addAction(tr("Always on &Top"), this, &MainWindow::onViewAlwaysOnTop);
    _alwaysOnTopAction->setCheckable(true);

    _viewMenu->addSeparator();

    // Word Wrap
    _wordWrapAction = _viewMenu->addAction(tr("Word &Wrap"), this, &MainWindow::onViewWordWrap);
    _wordWrapAction->setCheckable(true);

    // Show Symbols submenu
    auto* symbolsMenu = _viewMenu->addMenu(tr("Show Symbol"));
    _showWhiteSpaceAction = symbolsMenu->addAction(tr("Show White Space and TAB"), this, &MainWindow::onViewShowWhiteSpace);
    _showWhiteSpaceAction->setCheckable(true);

    _showEOLAction = symbolsMenu->addAction(tr("Show End of Line"), this, &MainWindow::onViewShowEOL);
    _showEOLAction->setCheckable(true);

    _showIndentGuideAction = symbolsMenu->addAction(tr("Show Indent Guide"), this, &MainWindow::onViewShowIndentGuide);
    _showIndentGuideAction->setCheckable(true);

    _viewMenu->addSeparator();

    // Zoom submenu
    auto* zoomMenu = _viewMenu->addMenu(tr("Zoom"));
    auto* zoomInAction = zoomMenu->addAction(tr("Zoom &In"), this, [this]() {
        if (_pNotepad_plus) {
            ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
            if (view) view->execute(SCI_ZOOMIN);
        }
    });
    zoomInAction->setShortcut(QKeySequence::ZoomIn);

    auto* zoomOutAction = zoomMenu->addAction(tr("Zoom &Out"), this, [this]() {
        if (_pNotepad_plus) {
            ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
            if (view) view->execute(SCI_ZOOMOUT);
        }
    });
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);

    zoomMenu->addAction(tr("Restore Default Zoom"), this, [this]() {
        if (_pNotepad_plus) {
            ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
            if (view) view->execute(SCI_SETZOOM, 0);
        }
    });

    _viewMenu->addSeparator();

    // Panels submenu
    auto* panelsMenu = _viewMenu->addMenu(tr("Panel"));
    auto* functionListAction = panelsMenu->addAction(tr("Function &List"), this, &MainWindow::onViewFunctionList);
    functionListAction->setCheckable(true);
    auto* projectPanelAction = panelsMenu->addAction(tr("&Project Panel"), this, &MainWindow::onViewProjectPanel);
    projectPanelAction->setCheckable(true);
    auto* documentMapAction = panelsMenu->addAction(tr("&Document Map"), this, &MainWindow::onViewDocumentMap);
    documentMapAction->setCheckable(true);
    auto* clipboardHistoryAction = panelsMenu->addAction(tr("&Clipboard History"), this, &MainWindow::onViewClipboardHistory);
    clipboardHistoryAction->setCheckable(true);
    auto* fileBrowserAction = panelsMenu->addAction(tr("Folder as &Workspace"), this, &MainWindow::onViewFileBrowser);
    fileBrowserAction->setCheckable(true);

    // Tab Color submenu
    _viewMenu->addSeparator();
    auto* tabColorMenu = _viewMenu->addMenu(tr("Tab Color"));
    tabColorMenu->addAction(tr("Apply Color 1"), this, [this]() { onViewTabColour(0); });
    tabColorMenu->addAction(tr("Apply Color 2"), this, [this]() { onViewTabColour(1); });
    tabColorMenu->addAction(tr("Apply Color 3"), this, [this]() { onViewTabColour(2); });
    tabColorMenu->addAction(tr("Apply Color 4"), this, [this]() { onViewTabColour(3); });
    tabColorMenu->addAction(tr("Apply Color 5"), this, [this]() { onViewTabColour(4); });
    tabColorMenu->addSeparator();
    tabColorMenu->addAction(tr("Remove Color"), this, [this]() { onViewTabColour(-1); });

    // Text Direction
    _viewMenu->addSeparator();
    _viewMenu->addAction(tr("Text Direction RTL"), this, [this]() { onEditTextDirection(true); });
    _viewMenu->addAction(tr("Text Direction LTR"), this, [this]() { onEditTextDirection(false); });

    // Monitoring (tail -f)
    _viewMenu->addSeparator();
    _monitoringAction = _viewMenu->addAction(tr("Monitoring (tail -f)"), this, &MainWindow::onViewMonitoring);
    _monitoringAction->setCheckable(true);
    _monitoringAction->setChecked(false);

    // Tab Bar
    _viewMenu->addSeparator();
    auto* tabBarAction = _viewMenu->addAction(tr("Tab Bar"));
    tabBarAction->setCheckable(true);
    tabBarAction->setChecked(true);

    // Status Bar
    auto* statusBarAction = _viewMenu->addAction(tr("Status Bar"));
    statusBarAction->setCheckable(true);
    statusBarAction->setChecked(true);
}

void MainWindow::createEncodingMenu()
{
    _encodingMenu = _menuBar->addMenu(tr("&Encoding"));

    // Action group for exclusive checkmarks
    _encodingActionGroup = new QActionGroup(this);
    _encodingActionGroup->setExclusive(true);

    // Helper lambda to add a checkable encoding action
    auto addEncodingAction = [this](QMenu* menu, const QString& text, int cmdId) -> QAction*
    {
        QAction* action = menu->addAction(text);
        action->setCheckable(true);
        action->setData(cmdId);
        _encodingActionGroup->addAction(action);
        if (cmdId >= IDM_FORMAT_ENCODE && cmdId <= IDM_FORMAT_ENCODE_END) {
            _charsetActions[cmdId] = action;
            connect(action, &QAction::triggered, this, [this, cmdId]() {
                onCharsetSelected(cmdId);
            });
        }
        return action;
    };

    // Basic encodings
    _ansiAction = addEncodingAction(_encodingMenu, tr("Encode in &ANSI"), IDM_FORMAT_ANSI);
    connect(_ansiAction, &QAction::triggered, this, &MainWindow::onEncodingANSI);

    _encodingMenu->addSeparator();

    _utf8Action = addEncodingAction(_encodingMenu, tr("Encode in &UTF-8"), IDM_FORMAT_UTF_8);
    connect(_utf8Action, &QAction::triggered, this, &MainWindow::onEncodingUTF8);

    _utf8BomAction = addEncodingAction(_encodingMenu, tr("Encode in UTF-8-&BOM"), IDM_FORMAT_AS_UTF_8);
    connect(_utf8BomAction, &QAction::triggered, this, &MainWindow::onEncodingUTF8BOM);

    _encodingMenu->addSeparator();

    _utf16beAction = addEncodingAction(_encodingMenu, tr("Encode in &UTF-16 BE BOM"), IDM_FORMAT_UTF_16BE);
    connect(_utf16beAction, &QAction::triggered, this, &MainWindow::onEncodingUTF16BE);

    _utf16leAction = addEncodingAction(_encodingMenu, tr("Encode in UTF-16 &LE BOM"), IDM_FORMAT_UTF_16LE);
    connect(_utf16leAction, &QAction::triggered, this, &MainWindow::onEncodingUTF16LE);

    _encodingMenu->addSeparator();

    // ========================================================================
    // Character Sets submenu
    // ========================================================================
    QMenu* charsetMenu = _encodingMenu->addMenu(tr("Character Sets"));

    // --- Arabic ---
    QMenu* arabicMenu = charsetMenu->addMenu(tr("Arabic"));
    addEncodingAction(arabicMenu, tr("ISO 8859-6"), IDM_FORMAT_ISO_8859_6);
    addEncodingAction(arabicMenu, tr("OEM 720"), IDM_FORMAT_DOS_720);
    addEncodingAction(arabicMenu, tr("Windows-1256"), IDM_FORMAT_WIN_1256);

    // --- Baltic ---
    QMenu* balticMenu = charsetMenu->addMenu(tr("Baltic"));
    addEncodingAction(balticMenu, tr("ISO 8859-4"), IDM_FORMAT_ISO_8859_4);
    addEncodingAction(balticMenu, tr("ISO 8859-13"), IDM_FORMAT_ISO_8859_13);
    addEncodingAction(balticMenu, tr("OEM 775"), IDM_FORMAT_DOS_775);
    addEncodingAction(balticMenu, tr("Windows-1257"), IDM_FORMAT_WIN_1257);

    // --- Celtic ---
    QMenu* celticMenu = charsetMenu->addMenu(tr("Celtic"));
    addEncodingAction(celticMenu, tr("ISO 8859-14"), IDM_FORMAT_ISO_8859_14);

    // --- Central European ---
    QMenu* centralEuMenu = charsetMenu->addMenu(tr("Central European"));
    addEncodingAction(centralEuMenu, tr("ISO 8859-2"), IDM_FORMAT_ISO_8859_2);
    addEncodingAction(centralEuMenu, tr("OEM 852"), IDM_FORMAT_DOS_852);
    addEncodingAction(centralEuMenu, tr("Windows-1250"), IDM_FORMAT_WIN_1250);

    // --- Chinese ---
    QMenu* chineseMenu = charsetMenu->addMenu(tr("Chinese"));
    addEncodingAction(chineseMenu, tr("Big5"), IDM_FORMAT_BIG5);
    addEncodingAction(chineseMenu, tr("GB2312"), IDM_FORMAT_GB2312);

    // --- Cyrillic ---
    QMenu* cyrillicMenu = charsetMenu->addMenu(tr("Cyrillic"));
    addEncodingAction(cyrillicMenu, tr("ISO 8859-5"), IDM_FORMAT_ISO_8859_5);
    addEncodingAction(cyrillicMenu, tr("KOI8-R"), IDM_FORMAT_KOI8R_CYRILLIC);
    addEncodingAction(cyrillicMenu, tr("KOI8-U"), IDM_FORMAT_KOI8U_CYRILLIC);
    addEncodingAction(cyrillicMenu, tr("Mac Cyrillic"), IDM_FORMAT_MAC_CYRILLIC);
    addEncodingAction(cyrillicMenu, tr("OEM 855"), IDM_FORMAT_DOS_855);
    addEncodingAction(cyrillicMenu, tr("OEM 866"), IDM_FORMAT_DOS_866);
    addEncodingAction(cyrillicMenu, tr("Windows-1251"), IDM_FORMAT_WIN_1251);

    // --- Greek ---
    QMenu* greekMenu = charsetMenu->addMenu(tr("Greek"));
    addEncodingAction(greekMenu, tr("ISO 8859-7"), IDM_FORMAT_ISO_8859_7);
    addEncodingAction(greekMenu, tr("OEM 737"), IDM_FORMAT_DOS_737);
    addEncodingAction(greekMenu, tr("OEM 869"), IDM_FORMAT_DOS_869);
    addEncodingAction(greekMenu, tr("Windows-1253"), IDM_FORMAT_WIN_1253);

    // --- Hebrew ---
    QMenu* hebrewMenu = charsetMenu->addMenu(tr("Hebrew"));
    addEncodingAction(hebrewMenu, tr("ISO 8859-8"), IDM_FORMAT_ISO_8859_8);
    addEncodingAction(hebrewMenu, tr("OEM 862"), IDM_FORMAT_DOS_862);
    addEncodingAction(hebrewMenu, tr("Windows-1255"), IDM_FORMAT_WIN_1255);

    // --- Japanese ---
    QMenu* japaneseMenu = charsetMenu->addMenu(tr("Japanese"));
    addEncodingAction(japaneseMenu, tr("Shift-JIS"), IDM_FORMAT_SHIFT_JIS);

    // --- Korean ---
    QMenu* koreanMenu = charsetMenu->addMenu(tr("Korean"));
    addEncodingAction(koreanMenu, tr("EUC-KR"), IDM_FORMAT_EUC_KR);
    addEncodingAction(koreanMenu, tr("Windows-949"), IDM_FORMAT_KOREAN_WIN);

    // --- North European ---
    QMenu* northEuMenu = charsetMenu->addMenu(tr("North European"));
    addEncodingAction(northEuMenu, tr("OEM 861 (Icelandic)"), IDM_FORMAT_DOS_861);
    addEncodingAction(northEuMenu, tr("OEM 865 (Nordic)"), IDM_FORMAT_DOS_865);

    // --- Thai ---
    QMenu* thaiMenu = charsetMenu->addMenu(tr("Thai"));
    addEncodingAction(thaiMenu, tr("TIS-620"), IDM_FORMAT_TIS_620);

    // --- Turkish ---
    QMenu* turkishMenu = charsetMenu->addMenu(tr("Turkish"));
    addEncodingAction(turkishMenu, tr("ISO 8859-3"), IDM_FORMAT_ISO_8859_3);
    addEncodingAction(turkishMenu, tr("ISO 8859-9"), IDM_FORMAT_ISO_8859_9);
    addEncodingAction(turkishMenu, tr("OEM 857"), IDM_FORMAT_DOS_857);
    addEncodingAction(turkishMenu, tr("Windows-1254"), IDM_FORMAT_WIN_1254);

    // --- Vietnamese ---
    QMenu* vietnameseMenu = charsetMenu->addMenu(tr("Vietnamese"));
    addEncodingAction(vietnameseMenu, tr("Windows-1258"), IDM_FORMAT_WIN_1258);

    // --- Western European ---
    QMenu* westernEuMenu = charsetMenu->addMenu(tr("Western European"));
    addEncodingAction(westernEuMenu, tr("ISO 8859-1"), IDM_FORMAT_ISO_8859_1);
    addEncodingAction(westernEuMenu, tr("ISO 8859-15"), IDM_FORMAT_ISO_8859_15);
    addEncodingAction(westernEuMenu, tr("OEM 437 (US)"), IDM_FORMAT_DOS_437);
    addEncodingAction(westernEuMenu, tr("OEM 850 (Western European)"), IDM_FORMAT_DOS_850);
    addEncodingAction(westernEuMenu, tr("OEM 858 (Multilingual Latin I + Euro)"), IDM_FORMAT_DOS_858);
    addEncodingAction(westernEuMenu, tr("OEM 860 (Portuguese)"), IDM_FORMAT_DOS_860);
    addEncodingAction(westernEuMenu, tr("OEM 863 (French Canadian)"), IDM_FORMAT_DOS_863);
    addEncodingAction(westernEuMenu, tr("Windows-1252"), IDM_FORMAT_WIN_1252);

    // Default: check UTF-8
    _utf8Action->setChecked(true);
}

void MainWindow::createLanguageMenu()
{
    _languageMenu = _menuBar->addMenu(tr("&Language"));

    // Common languages
    QStringList languages = {
        "Normal Text",
        "C",
        "C++",
        "C#",
        "Java",
        "Python",
        "JavaScript",
        "HTML",
        "CSS",
        "XML",
        "JSON",
        "SQL",
        "PHP",
        "Ruby",
        "Go",
        "Rust",
        "TypeScript",
        "Shell",
        "PowerShell",
        "Batch",
        "Makefile",
        "CMake",
        "Markdown",
        "YAML",
        "Lua",
        "Perl",
        "R",
        "Swift",
        "Kotlin",
        "Scala",
        "Groovy",
        "VB",
        "VBScript",
        "ActionScript",
        "CoffeeScript",
        "Dart",
        "Elixir",
        "Erlang",
        "Fortran",
        "Haskell",
        "Julia",
        "Lisp",
        "MATLAB",
        "Objective-C",
        "Pascal",
        "Raku",
        "Tcl",
        "Verilog",
        "VHDL"
    };

    auto* langGroup = new QActionGroup(this);
    langGroup->setExclusive(true);

    for (const QString& lang : languages) {
        auto* action = _languageMenu->addAction(lang, this, [this]() {
            onLanguageSelected(qobject_cast<QAction*>(sender()));
        });
        action->setCheckable(true);
        langGroup->addAction(action);
    }

    _languageMenu->addSeparator();
    _languageMenu->addAction(tr("Define your language..."), this, &MainWindow::onLanguageDefineUserLang);
    _languageMenu->addAction(tr("User-Defined"));
}

void MainWindow::createSettingsMenu()
{
    _settingsMenu = _menuBar->addMenu(tr("&Settings"));

    // Preferences
    _settingsMenu->addAction(tr("&Preferences..."), this, &MainWindow::onSettingsPreferences);

    _settingsMenu->addSeparator();

    // Style Configurator
    _settingsMenu->addAction(tr("Style &Configurator..."), this, &MainWindow::onSettingsStyleConfigurator);

    // Shortcut Mapper
    _settingsMenu->addAction(tr("Shortcut &Mapper..."), this, &MainWindow::onSettingsShortcutMapper);

    _settingsMenu->addSeparator();

    // Import submenu
    auto* importMenu = _settingsMenu->addMenu(tr("Import"));
    importMenu->addAction(tr("Import plugin(s)..."));
    importMenu->addAction(tr("Import style theme(s)..."));

    // Edit Popup ContextMenu
    _settingsMenu->addAction(tr("Edit Popup ContextMenu"));

    _settingsMenu->addSeparator();

    // Plugin Manager
    _settingsMenu->addAction(tr("Plugins &Admin..."), this, &MainWindow::onSettingsPluginManager);
}

void MainWindow::createPluginsMenu()
{
    // Plugins menu is created dynamically after plugins are loaded
    // This will be populated by initPlugins()
    _pluginsMenu = nullptr;
}

void MainWindow::createMacroMenu()
{
    _macroMenu = _menuBar->addMenu(tr("&Macro"));

    // Start Recording
    _macroMenu->addAction(tr("Start &Recording"), this, &MainWindow::onMacroStartRecording);

    // Stop Recording
    _macroMenu->addAction(tr("S&top Recording"), this, &MainWindow::onMacroStopRecording);

    _macroMenu->addSeparator();

    // Playback
    auto* playbackAction = _macroMenu->addAction(tr("&Playback"), this, &MainWindow::onMacroPlayback);
    playbackAction->setShortcut(QKeySequence("Shift+Ctrl+P"));

    // Run a Macro Multiple Times
    _macroMenu->addAction(tr("Run a Macro Multiple &Times..."), this, &MainWindow::onMacroRunMultiple);

    _macroMenu->addSeparator();

    // Save Current Recorded Macro
    _macroMenu->addAction(tr("Save Current Recorded Macro..."));
}

void MainWindow::createRunMenu()
{
    _runMenu = _menuBar->addMenu(tr("&Run"));

    // Run...
    auto* runAction = _runMenu->addAction(tr("&Run..."), this, &MainWindow::onRunRun);
    runAction->setShortcut(QKeySequence("F5"));

    _runMenu->addSeparator();

    // Launch in browser actions
    _runMenu->addAction(tr("Launch in &Chrome"), this, &MainWindow::onRunLaunchInBrowser);
    _runMenu->addAction(tr("Launch in &Firefox"));
    _runMenu->addAction(tr("Launch in &IE"));
    _runMenu->addAction(tr("Launch in &Safari"));

    _runMenu->addSeparator();

    // Get PHP help
    _runMenu->addAction(tr("Get PHP help"));

    // Wikipedia Search
    _runMenu->addAction(tr("Wikipedia Search"));

    _runMenu->addSeparator();

    // Modify Shortcut / Delete Command
    _runMenu->addAction(tr("Modify Shortcut / Delete Command..."));
}

void MainWindow::createWindowMenu()
{
    _windowMenu = _menuBar->addMenu(tr("&Window"));

    // New Instance
    _windowMenu->addAction(tr("New &Instance"), this, &MainWindow::onWindowNewInstance);

    _windowMenu->addSeparator();

    // Move to Other View
    _windowMenu->addAction(tr("&Move to Other View"), this, &MainWindow::onWindowMoveToOtherView);

    // Clone to Other View
    _windowMenu->addAction(tr("&Clone to Other View"), this, &MainWindow::onWindowCloneToOtherView);

    _windowMenu->addSeparator();

    // Window list
    _windowMenu->addAction(tr("Window List"), this, &MainWindow::onWindowList);
}

void MainWindow::createHelpMenu()
{
    _helpMenu = _menuBar->addMenu(tr("&Help"));

    // About Notepad++
    _helpMenu->addAction(tr("&About Notepad++"), this, &MainWindow::onHelpAbout);

    _helpMenu->addSeparator();

    // Command Line Arguments
    _helpMenu->addAction(tr("Command Line Arguments..."), this, &MainWindow::onHelpCmdLineArgs);

    // Debug Info
    _helpMenu->addAction(tr("Debug Info..."), this, &MainWindow::onHelpDebugInfo);
}

void MainWindow::updateMenuState()
{
    // Update menu items based on current document state
    if (!_pNotepad_plus) {
        return;
    }

    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();

    if (!view || !buffer) {
        return;
    }

    // Update Edit menu actions based on document state
    bool canUndo = view->execute(SCI_CANUNDO) != 0;
    bool canRedo = view->execute(SCI_CANREDO) != 0;
    bool hasSelection = view->hasSelection();
    bool isReadOnly = buffer->isReadOnly();

    // Find and update Edit menu actions
    if (_editMenu) {
        for (QAction* action : _editMenu->actions()) {
            QString text = action->text();
            if (text.contains("Undo")) {
                action->setEnabled(canUndo && !isReadOnly);
            } else if (text.contains("Redo")) {
                action->setEnabled(canRedo && !isReadOnly);
            } else if (text.contains("Cut")) {
                action->setEnabled(hasSelection && !isReadOnly);
            } else if (text.contains("Copy")) {
                action->setEnabled(hasSelection);
            } else if (text.contains("Paste")) {
                action->setEnabled(view->execute(SCI_CANPASTE) != 0 && !isReadOnly);
            } else if (text.contains("Delete")) {
                action->setEnabled(!isReadOnly);
            }
        }
    }

    // Update View menu check states
    if (_wordWrapAction) {
        _wordWrapAction->setChecked(view->isWrap());
    }
    if (_showWhiteSpaceAction) {
        _showWhiteSpaceAction->setChecked(view->isShownSpaceAndTab());
    }
    if (_showEOLAction) {
        _showEOLAction->setChecked(view->isShownEol());
    }
    if (_showIndentGuideAction) {
        _showIndentGuideAction->setChecked(view->isShownIndentGuide());
    }
    if (_monitoringAction) {
        _monitoringAction->setChecked(buffer->isMonitoringOn());
    }

    // Update panel visibility check states in View menu
    // This is done by finding the Panel submenu and updating actions
    if (_viewMenu) {
        for (QAction* action : _viewMenu->actions()) {
            if (action->menu() && action->text() == tr("Panel")) {
                QMenu* panelsMenu = action->menu();
                for (QAction* panelAction : panelsMenu->actions()) {
                    QString panelText = panelAction->text();
                    if (panelText.contains("Function")) {
                        panelAction->setChecked(isPanelVisible("functionList"));
                    } else if (panelText.contains("Project")) {
                        panelAction->setChecked(isPanelVisible("projectPanel"));
                    } else if (panelText.contains("Document Map")) {
                        panelAction->setChecked(isPanelVisible("documentMap"));
                    } else if (panelText.contains("Clipboard")) {
                        panelAction->setChecked(isPanelVisible("clipboardHistory"));
                    } else if (panelText.contains("Workspace") || panelText.contains("Folder")) {
                        panelAction->setChecked(isPanelVisible("fileBrowser"));
                    }
                }
                break;
            }
        }
    }

    // Update Language menu - check the current language
    if (_languageMenu && buffer) {
        LangType currentLang = buffer->getLangType();
        for (QAction* action : _languageMenu->actions()) {
            if (action->isCheckable()) {
                QString langName = action->text();
                LangType actionLang = L_TEXT;

                // Map language name to LangType
                if (langName == "Normal Text") actionLang = L_TEXT;
                else if (langName == "C") actionLang = L_C;
                else if (langName == "C++") actionLang = L_CPP;
                else if (langName == "C#") actionLang = L_CS;
                else if (langName == "Java") actionLang = L_JAVA;
                else if (langName == "Python") actionLang = L_PYTHON;
                else if (langName == "JavaScript") actionLang = L_JAVASCRIPT;
                else if (langName == "HTML") actionLang = L_HTML;
                else if (langName == "CSS") actionLang = L_CSS;
                else if (langName == "XML") actionLang = L_XML;
                else if (langName == "JSON") actionLang = L_JSON;
                else if (langName == "SQL") actionLang = L_SQL;
                else if (langName == "PHP") actionLang = L_PHP;
                else if (langName == "Ruby") actionLang = L_RUBY;
                else if (langName == "Go") actionLang = L_GOLANG;
                else if (langName == "Rust") actionLang = L_RUST;
                else if (langName == "TypeScript") actionLang = L_TYPESCRIPT;
                else if (langName == "Shell" || langName == "Bash") actionLang = L_BASH;
                else if (langName == "PowerShell") actionLang = L_POWERSHELL;
                else if (langName == "Batch") actionLang = L_BATCH;
                else if (langName == "Makefile") actionLang = L_MAKEFILE;
                else if (langName == "CMake") actionLang = L_CMAKE;
                else if (langName == "YAML") actionLang = L_YAML;
                else if (langName == "Lua") actionLang = L_LUA;
                else if (langName == "Perl") actionLang = L_PERL;
                else if (langName == "R") actionLang = L_R;
                else if (langName == "Swift") actionLang = L_SWIFT;
                else if (langName == "VB") actionLang = L_VB;
                else if (langName == "ActionScript") actionLang = L_FLASH;
                else if (langName == "CoffeeScript") actionLang = L_COFFEESCRIPT;
                else if (langName == "Erlang") actionLang = L_ERLANG;
                else if (langName == "Fortran") actionLang = L_FORTRAN;
                else if (langName == "Haskell") actionLang = L_HASKELL;
                else if (langName == "Lisp") actionLang = L_LISP;
                else if (langName == "MATLAB") actionLang = L_MATLAB;
                else if (langName == "Objective-C") actionLang = L_OBJC;
                else if (langName == "Pascal") actionLang = L_PASCAL;
                else if (langName == "Raku") actionLang = L_RAKU;
                else if (langName == "Tcl") actionLang = L_TCL;
                else if (langName == "Verilog") actionLang = L_VERILOG;
                else if (langName == "VHDL") actionLang = L_VHDL;

                action->setChecked(currentLang == actionLang);
            }
        }
    }

    // Update Encoding menu check state
    updateEncodingMenu();
}

// ============================================================================
// Toolbar Operations
// ============================================================================

void MainWindow::initToolBar()
{
    _mainToolBar = new ToolBar();

    // Define toolbar buttons - ToolBarButtonUnit has 10 int fields:
    // _cmdID, _defaultIcon, _grayIcon, _defaultIcon2, _grayIcon2,
    // _defaultDarkModeIcon, _grayDarkModeIcon, _defaultDarkModeIcon2, _grayDarkModeIcon2, _stdIcon
    static const ToolBarButtonUnit toolBarButtons[] = {
        // File operations
        { IDM_FILE_NEW, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_FILE_OPEN, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_FILE_SAVE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_FILE_SAVEALL, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // Edit operations
        { IDM_EDIT_CUT, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_EDIT_COPY, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_EDIT_PASTE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        { IDM_EDIT_UNDO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_EDIT_REDO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // Search operations
        { IDM_SEARCH_FIND, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_SEARCH_REPLACE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // View operations
        { IDM_VIEW_ZOOMIN, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_VIEW_ZOOMOUT, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // Macro operations
        { IDM_MACRO_STARTRECORDINGMACRO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_MACRO_STOPRECORDINGMACRO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_MACRO_PLAYBACKRECORDEDMACRO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    };

    _mainToolBar->init(this, TB_STANDARD, toolBarButtons,
                       sizeof(toolBarButtons) / sizeof(ToolBarButtonUnit));

    // Connect toolbar button clicks to command dispatch
    connect(_mainToolBar, &ToolBar::commandTriggered, this, [this](int cmdID) {
        if (_shortcutManager) {
            _shortcutManager->executeCommand(cmdID);
        }
    });

    // Add toolbar to main window
    addToolBar(_mainToolBar->getToolBar());
}

void MainWindow::updateToolBarState()
{
    // Update toolbar button states based on current document state
    if (!_mainToolBar || !_pNotepad_plus) {
        return;
    }

    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();

    if (!view || !buffer) {
        return;
    }

    // Update toolbar button states
    bool canUndo = view->execute(SCI_CANUNDO) != 0;
    bool canRedo = view->execute(SCI_CANREDO) != 0;
    bool hasSelection = view->hasSelection();
    bool isReadOnly = buffer->isReadOnly();

    // Enable/disable toolbar buttons based on state
    // Note: Toolbar button enabling/disabling would be implemented
    // in the ToolBar class. This is a placeholder for that logic.
    (void)canUndo;
    (void)canRedo;
    (void)hasSelection;
    (void)isReadOnly;
}

// ============================================================================
// Status Bar Operations
// ============================================================================

void MainWindow::initStatusBar()
{
    _statusBar = new StatusBar();

    // Define status bar parts
    int parts[] = { 200, 150, 150, 100, 100, 100, -1 }; // -1 for last part (stretch)
    _statusBar->init(this, 7);
    _statusBar->setParts(7, parts);

    // Set initial text
    _statusBar->setText(tr("Ready"), 0);
    _statusBar->setText(tr("Windows (CRLF)"), 1);
    _statusBar->setText(tr("UTF-8"), 2);
    _statusBar->setText(tr("Normal text file"), 3);
    _statusBar->setText(tr("Ln 1, Col 1"), 4);
    _statusBar->setText(tr("Sel 0 | 0"), 5);
    _statusBar->setText(tr("100%"), 6);

    setStatusBar(_statusBar->getStatusBar());
}

void MainWindow::updateStatusBar()
{
    if (!_statusBar) {
        return;
    }

    // Update status bar information from the current editor
    ScintillaEditView* view = nullptr;
    Buffer* buffer = nullptr;

    if (_pNotepad_plus) {
        view = _pNotepad_plus->getCurrentEditView();
        buffer = _pNotepad_plus->getCurrentBuffer();
    }

    if (view && buffer) {
        // Get cursor position
        intptr_t pos = view->execute(SCI_GETCURRENTPOS);
        intptr_t line = view->execute(SCI_LINEFROMPOSITION, pos);
        intptr_t col = view->execute(SCI_GETCOLUMN, pos);

        // Line and column (1-based)
        _statusBar->setText(tr("Ln %1, Col %2").arg(line + 1).arg(col + 1), 4);

        // Selection info
        intptr_t selStart = view->execute(SCI_GETSELECTIONSTART);
        intptr_t selEnd = view->execute(SCI_GETSELECTIONEND);
        intptr_t selLength = selEnd - selStart;
        intptr_t selLines = (selLength > 0) ?
            (view->execute(SCI_LINEFROMPOSITION, selEnd) - view->execute(SCI_LINEFROMPOSITION, selStart) + 1) : 0;

        if (selLength > 0) {
            _statusBar->setText(tr("Sel %1 | %2").arg(selLength).arg(selLines), 5);
        } else {
            _statusBar->setText(tr("Sel 0 | 0"), 5);
        }

        // Document type / Language
        LangType langType = buffer->getLangType();
        const wchar_t* langNameC = NppParameters::getInstance().getLangExtFromLangType(langType);
        QString langName = (langNameC && langNameC[0]) ? QString::fromStdWString(langNameC) : tr("Normal text file");
        _statusBar->setText(langName, 3);

        // Encoding
        QString encodingStr;
        int charsetEncoding = buffer->getEncoding();
        if (charsetEncoding != -1) {
            // A charset encoding is active - show the charset name
            encodingStr = charsetEncodingName(charsetEncoding);
        } else {
            UniMode uniMode = buffer->getUnicodeMode();
            switch (uniMode) {
                case uniUTF8: encodingStr = "UTF-8 BOM"; break;
                case uniUTF8_NoBOM: encodingStr = "UTF-8"; break;
                case uni16BE: encodingStr = "UTF-16 BE BOM"; break;
                case uni16LE: encodingStr = "UTF-16 LE BOM"; break;
                case uni16BE_NoBOM: encodingStr = "UTF-16 BE"; break;
                case uni16LE_NoBOM: encodingStr = "UTF-16 LE"; break;
                case uni7Bit: encodingStr = "UTF-8"; break;
                default: encodingStr = "ANSI"; break;
            }
        }
        _statusBar->setText(encodingStr, 2);

        // EOL format
        auto eol = buffer->getEolFormat();
        QString eolStr;
        switch (eol) {
            case Buffer::eolWindows: eolStr = "Windows (CRLF)"; break;
            case Buffer::eolMac: eolStr = "Macintosh (CR)"; break;
            case Buffer::eolUnix: eolStr = "Unix (LF)"; break;
            default: eolStr = "Windows (CRLF)"; break;
        }
        _statusBar->setText(eolStr, 1);

        // Zoom level
        int zoom = static_cast<int>(view->execute(SCI_GETZOOM));
        int zoomPercent = 100 + (zoom * 10);  // Each zoom step is 10%
        _statusBar->setText(tr("%1%").arg(zoomPercent), 6);

        // Update document modification indicator in title
        updateTitle();
    } else {
        // Default values when no document is open
        _statusBar->setText(tr("Ln 1, Col 1"), 4);
        _statusBar->setText(tr("Sel 0 | 0"), 5);
        _statusBar->setText(tr("100%"), 6);
    }
}

// ============================================================================
// Panel Management
// ============================================================================

void MainWindow::showPanel(const QString& panelName, bool show)
{
    if (!_dockingManager) {
        return;
    }

    if (show) {
        _dockingManager->showPanel(panelName);
    } else {
        _dockingManager->hidePanel(panelName);
    }
}

bool MainWindow::isPanelVisible(const QString& panelName)
{
    if (!_dockingManager) {
        return false;
    }

    return _dockingManager->isPanelVisible(panelName);
}

// ============================================================================
// Document Management
// ============================================================================

void MainWindow::addTab(const QString& title, const QString& filePath)
{
    (void)filePath;

    if (_tabBar) {
        _tabBar->insertAtEnd(title);
    }
}

void MainWindow::closeTab(int index)
{
    if (_tabBar) {
        _tabBar->deleteItemAt(static_cast<size_t>(index));
    }
}

void MainWindow::switchTab(int index)
{
    if (_tabWidget) {
        _tabWidget->setCurrentIndex(index);
    }

    if (_tabBar) {
        _tabBar->activateAt(index);
    }
}

// ============================================================================
// Settings
// ============================================================================

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    // Restore geometry
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        // Default size
        resize(1200, 800);
        move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
    }

    // Restore window state
    QByteArray state = settings.value("windowState").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // Restore docking layout
    if (_dockingManager) {
        QByteArray dockState = settings.value("dockLayout").toByteArray();
        if (!dockState.isEmpty()) {
            _dockingManager->restoreLayout(dockState);
        }
    }

    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    // Save geometry
    settings.setValue("geometry", saveGeometry());

    // Save window state
    settings.setValue("windowState", saveState());

    // Save docking layout
    if (_dockingManager) {
        settings.setValue("dockLayout", _dockingManager->saveLayout());
    }

    settings.endGroup();
}

// ============================================================================
// Window State
// ============================================================================

void MainWindow::saveWindowState()
{
    _normalWindowState = saveState();
    _normalGeometry = saveGeometry();
}

void MainWindow::restoreWindowState()
{
    if (!_normalWindowState.isEmpty()) {
        restoreState(_normalWindowState);
    }
    if (!_normalGeometry.isEmpty()) {
        restoreGeometry(_normalGeometry);
    }
}

void MainWindow::toggleFullScreen()
{
    if (_isFullScreen) {
        setFullScreen(false);
    } else {
        setFullScreen(true);
    }
}

void MainWindow::setFullScreen(bool fullScreen)
{
    if (fullScreen == _isFullScreen) {
        return;
    }

    if (fullScreen) {
        saveWindowState();
        menuBar()->hide();
        statusBar()->hide();
        showFullScreen();
        _isFullScreen = true;
    } else {
        showNormal();
        restoreWindowState();
        menuBar()->show();
        statusBar()->show();
        _isFullScreen = false;
    }
}

void MainWindow::togglePostItMode()
{
    if (_isPostItMode) {
        // Restore normal mode
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
        showNormal();
        restoreWindowState();
        _isPostItMode = false;
    } else {
        // Enter post-it mode
        saveWindowState();
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        show();
        _isPostItMode = true;
    }
}

void MainWindow::toggleDistractionFreeMode()
{
    // Toggle distraction free mode
    // This hides all UI elements except the editor
    _isDistractionFree = !_isDistractionFree;

    if (_isDistractionFree) {
        menuBar()->hide();
        statusBar()->hide();
        if (_mainToolBar) {
            _mainToolBar->display(false);
        }
        // Hide all panels
        if (_dockingManager) {
            _dockingManager->hideAllPanels();
        }
    } else {
        menuBar()->show();
        statusBar()->show();
        if (_mainToolBar) {
            _mainToolBar->display(true);
        }
    }
}

void MainWindow::setAlwaysOnTop(bool alwaysOnTop)
{
    Qt::WindowFlags flags = windowFlags();
    if (alwaysOnTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();
}

bool MainWindow::isAlwaysOnTop() const
{
    return windowFlags() & Qt::WindowStaysOnTopHint;
}

// ============================================================================
// Tray Icon
// ============================================================================

bool MainWindow::isTrayIconSupported() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool MainWindow::shouldMinimizeToTray() const
{
    if (!isTrayIconSupported()) {
        return false;
    }

    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();
    int trayAction = nppGUI._isMinimizedToTray;

    return (trayAction == sta_minimize || trayAction == sta_minimize_close);
}

bool MainWindow::shouldCloseToTray() const
{
    if (!isTrayIconSupported()) {
        return false;
    }

    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();
    int trayAction = nppGUI._isMinimizedToTray;

    return (trayAction == sta_close || trayAction == sta_minimize_close);
}

void MainWindow::createTrayIconMenu()
{
    if (!_trayIconMenu) {
        _trayIconMenu = new QMenu(this);

        _trayIconShowAction = new QAction(tr("Show Notepad++"), this);
        connect(_trayIconShowAction, &QAction::triggered,
                this, &MainWindow::onTrayIconShowTriggered);
        _trayIconMenu->addAction(_trayIconShowAction);

        _trayIconMenu->addSeparator();

        _trayIconExitAction = new QAction(tr("Exit"), this);
        connect(_trayIconExitAction, &QAction::triggered,
                this, &MainWindow::onTrayIconExitTriggered);
        _trayIconMenu->addAction(_trayIconExitAction);

        if (_trayIcon) {
            _trayIcon->setContextMenu(_trayIconMenu);
        }
    }
}

void MainWindow::minimizeToTray()
{
    if (!isTrayIconSupported()) {
        // Tray not supported, just minimize normally
        showMinimized();
        return;
    }

    if (!_trayIcon) {
        _trayIcon = new QSystemTrayIcon(this);

        // Try to load icon from theme or use application icon
        QIcon trayIcon = QIcon::fromTheme("notepad++");
        if (trayIcon.isNull()) {
            // Use the application window icon
            trayIcon = windowIcon();
        }
        if (trayIcon.isNull()) {
            // Fallback to a standard icon
            trayIcon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
        }

        _trayIcon->setIcon(trayIcon);
        _trayIcon->setToolTip("Notepad++");

        connect(_trayIcon, &QSystemTrayIcon::activated,
                this, &MainWindow::onTrayIconActivated);

        createTrayIconMenu();
    }

    _isMinimizedToTray = true;
    _trayIcon->show();
    hide();
}

void MainWindow::restoreFromTray()
{
    _isMinimizedToTray = false;

    show();
    raise();
    activateWindow();

    if (_trayIcon) {
        _trayIcon->hide();
    }
}

void MainWindow::onTrayIconShowTriggered()
{
    restoreFromTray();
}

void MainWindow::onTrayIconExitTriggered()
{
    // Properly close the application
    _isMinimizedToTray = false;
    QApplication::quit();
}

// ============================================================================
// Event Handlers
// ============================================================================

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Check if we should minimize to tray instead of closing
    if (shouldCloseToTray() && !_isMinimizedToTray) {
        minimizeToTray();
        event->ignore();
        return;
    }

    // Check for unsaved documents - prompts user to save/discard/cancel
    if (_pNotepad_plus)
    {
        PluginsManager& pluginsManager = _pNotepad_plus->getPluginsManager();

        // Notify plugins that shutdown is about to begin
        SCNotification scnN{};
        scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(this);
        scnN.nmhdr.idFrom = 0;
        scnN.nmhdr.code = NPPN_BEFORESHUTDOWN;
        pluginsManager.notify(&scnN);

        bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();
        if (!_pNotepad_plus->fileCloseAll(false, isSnapshotMode))
        {
            // Notify plugins that shutdown was cancelled
            scnN.nmhdr.code = NPPN_CANCELSHUTDOWN;
            pluginsManager.notify(&scnN);

            event->ignore();
            return;
        }

        // Notify plugins of final shutdown
        scnN.nmhdr.code = NPPN_SHUTDOWN;
        pluginsManager.notify(&scnN);

        // Unregister the plugin message dispatcher
        ::nppPluginMessageDispatcher_unregister();
    }

    saveSettings();
    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    // Save geometry on resize
    if (!_isFullScreen && !_isPostItMode) {
        // Debounce the save
        QTimer::singleShot(500, this, &MainWindow::saveSettings);
    }
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);

    // Save geometry on move
    if (!_isFullScreen && !_isPostItMode) {
        // Debounce the save
        QTimer::singleShot(500, this, &MainWindow::saveSettings);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);

    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() & Qt::WindowMinimized) {
            // Window was minimized - check if we should minimize to tray
            if (shouldMinimizeToTray() && !_isMinimizedToTray) {
                // Use a single-shot timer to allow the minimize animation to complete
                QTimer::singleShot(0, this, [this]() {
                    minimizeToTray();
                });
            }
        }
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    // Force update of tab bars to ensure tabs are visible after window is shown
    // This fixes the issue where the initial tab doesn't appear on fresh startup
    static bool firstShow = true;
    if (firstShow && _mainDocTab) {
        firstShow = false;

        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(_mainDocTab->getWidget());
        if (tabWidget) {
            // The tab bar may have zero height if created before the window was shown
            // Force it to recalculate by toggling the tab position
            QTabWidget::TabPosition originalPos = tabWidget->tabPosition();
            tabWidget->setTabPosition(QTabWidget::North);
            tabWidget->setTabPosition(originalPos);

            // Ensure tab bar is visible
            QTabBar* tabBar = tabWidget->tabBar();
            if (tabBar) {
                tabBar->show();
                tabBar->updateGeometry();
            }

            // Force layout recalculation of the entire splitter hierarchy
            if (_editorSplitter) {
                _editorSplitter->updateGeometry();
                QList<int> sizes = _editorSplitter->sizes();
                if (sizes.size() >= 2 && sizes[0] == 0) {
                    sizes[0] = 800;
                    sizes[1] = 200;
                    _editorSplitter->setSizes(sizes);
                }
            }

            std::cout << "[MainWindow::showEvent] Forced tab widget update, tab count: "
                      << tabWidget->count() << ", tabBar visible: "
                      << (tabBar ? tabBar->isVisible() : false) << std::endl;
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls() && _pNotepad_plus) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            QString filePath = url.toLocalFile();
            if (!filePath.isEmpty()) {
                _pNotepad_plus->doOpen(filePath.toStdWString());
            }
        }
        event->acceptProposedAction();
    }
}

// ============================================================================
// Slot Implementations - File Menu
// ============================================================================

void MainWindow::onFileNew()
{
    std::cout << "[MainWindow::onFileNew] Called, _pNotepad_plus=" << _pNotepad_plus << std::endl;
    if (_pNotepad_plus) {
        _pNotepad_plus->fileNew();
    } else {
        std::cerr << "[MainWindow::onFileNew] ERROR: _pNotepad_plus is null!" << std::endl;
    }
}

void MainWindow::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), QString(),
        tr("All Files (*);;Text Files (*.txt)"));

    if (!fileName.isEmpty() && _pNotepad_plus) {
        _pNotepad_plus->doOpen(fileName.toStdWString());
    }
}

void MainWindow::onFileSave()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSave();
    }
}

void MainWindow::onFileSaveAs()
{
    // Delegate to Notepad_plus which handles the save dialog and operation
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAs(BUFFER_INVALID, false);
    }
}

void MainWindow::onFileSaveAll()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAll();
    }
}

void MainWindow::onFileClose()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileClose();
    }
}

void MainWindow::onFileCloseAll()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAll(true, false);
    }
}

void MainWindow::onFileCloseAllButCurrent()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->fileCloseAllButCurrent();
    }
}

void MainWindow::onFileCloseAllButPinned()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->fileCloseAllButPinned();
    }
}

void MainWindow::onFileCloseAllToLeft()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->fileCloseAllToLeft();
    }
}

void MainWindow::onFileCloseAllToRight()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->fileCloseAllToRight();
    }
}

void MainWindow::onFileCloseAllUnchanged()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->fileCloseAllUnchanged();
    }
}

void MainWindow::onFilePrint()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->filePrint(true);
    }
}

void MainWindow::onFilePrintNow()
{
    if (_pNotepad_plus)
    {
        _pNotepad_plus->filePrint(false);
    }
}

void MainWindow::onFileExit()
{
    close();
}

void MainWindow::onRecentFilesMenuAboutToShow()
{
    _recentFilesMenu->clear();

    if (!_pNotepad_plus)
    {
        _recentFilesMenu->addAction(tr("(Empty)"))->setEnabled(false);
        return;
    }

    LastRecentFileList& lrf = _pNotepad_plus->getLastRecentFileList();
    int count = lrf.getSize();

    if (count == 0)
    {
        _recentFilesMenu->addAction(tr("(Empty)"))->setEnabled(false);
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        std::wstring& filePath = lrf.getIndex(i);
        QString label = QString::fromStdWString(filePath);
        QAction* action = _recentFilesMenu->addAction(label);
        action->setData(label);
        connect(action, &QAction::triggered, this, &MainWindow::onRecentFileTriggered);
    }

    _recentFilesMenu->addSeparator();
    _recentFilesMenu->addAction(tr("Clear Recent File List"), this, &MainWindow::onClearRecentFiles);
}

void MainWindow::onRecentFileTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action || !_pNotepad_plus)
        return;

    QString filePath = action->data().toString();
    if (!filePath.isEmpty())
    {
        _pNotepad_plus->doOpen(filePath.toStdWString());
    }
}

void MainWindow::onClearRecentFiles()
{
    if (!_pNotepad_plus)
        return;

    LastRecentFileList& lrf = _pNotepad_plus->getLastRecentFileList();
    lrf.clear();
}

// ============================================================================
// Slot Implementations - Edit Menu
// ============================================================================

void MainWindow::onEditUndo()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_UNDO);
        }
    }
}

void MainWindow::onEditRedo()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_REDO);
        }
    }
}

void MainWindow::onEditCut()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_CUT);
        }
    }
}

void MainWindow::onEditCopy()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_COPY);
        }
    }
}

void MainWindow::onEditPaste()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_PASTE);
        }
    }
}

void MainWindow::onEditDelete()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_CLEAR);
        }
    }
}

void MainWindow::onEditSelectAll()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_SELECTALL);
        }
    }
}

void MainWindow::onEditInsertDateTime()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            // Get current date and time
            QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(dateTime.toUtf8().constData()));
        }
    }
}

void MainWindow::onEditInsertDateTimeShort()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    QDateTime now = QDateTime::currentDateTime();
    QLocale locale;
    QString dateStr = locale.toString(now.date(), QLocale::ShortFormat);
    QString timeStr = locale.toString(now.time(), QLocale::ShortFormat);

    QString dateTimeStr;
    const NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();
    if (nppGUI._dateTimeReverseDefaultOrder)
        dateTimeStr = dateStr + " " + timeStr;
    else
        dateTimeStr = timeStr + " " + dateStr;

    view->execute(SCI_BEGINUNDOACTION);
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(""));
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(dateTimeStr.toUtf8().constData()));
    view->execute(SCI_ENDUNDOACTION);
}

void MainWindow::onEditInsertDateTimeLong()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    QDateTime now = QDateTime::currentDateTime();
    QLocale locale;
    QString dateStr = locale.toString(now.date(), QLocale::LongFormat);
    QString timeStr = locale.toString(now.time(), QLocale::ShortFormat);

    QString dateTimeStr;
    const NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();
    if (nppGUI._dateTimeReverseDefaultOrder)
        dateTimeStr = dateStr + " " + timeStr;
    else
        dateTimeStr = timeStr + " " + dateStr;

    view->execute(SCI_BEGINUNDOACTION);
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(""));
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(dateTimeStr.toUtf8().constData()));
    view->execute(SCI_ENDUNDOACTION);
}

void MainWindow::onEditInsertDateTimeCustomized()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    QDateTime now = QDateTime::currentDateTime();
    const NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();

    // Convert Windows-style format to Qt format
    QString format = QString::fromStdWString(nppGUI._dateTimeFormat);
    // Convert Windows 'tt' (AM/PM) to Qt 'AP'
    format.replace("tt", "AP");
    format.replace("t", "A");

    QString dateTimeStr = now.toString(format);

    view->execute(SCI_BEGINUNDOACTION);
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(""));
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(dateTimeStr.toUtf8().constData()));
    view->execute(SCI_ENDUNDOACTION);
}

void MainWindow::onEditToggleReadOnly()
{
    if (!_pNotepad_plus) return;
    Buffer* buf = _pNotepad_plus->getCurrentBuffer();
    if (!buf) return;

    bool newReadOnly = !buf->isUserReadOnly();
    buf->setUserReadOnly(newReadOnly);

    // Update Scintilla read-only state
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (view)
    {
        view->execute(SCI_SETREADONLY, newReadOnly ? 1 : 0);
    }
}

void MainWindow::onEditInsertFullPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (buffer && view) {
            std::wstring path = buffer->getFullPathName();
            QString qPath = QString::fromStdWString(path);
            view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(qPath.toUtf8().constData()));
        }
    }
}

void MainWindow::onEditInsertFileName()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (buffer && view) {
            std::wstring fileName = buffer->getFileName();
            QString qFileName = QString::fromStdWString(fileName);
            view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(qFileName.toUtf8().constData()));
        }
    }
}

void MainWindow::onEditInsertDirPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (buffer && view) {
            std::wstring path = buffer->getFullPathName();
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                QString qDir = QString::fromStdWString(dir);
                view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(qDir.toUtf8().constData()));
            }
        }
    }
}

void MainWindow::onEditCopyFullPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        if (buffer) {
            std::wstring path = buffer->getFullPathName();
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString::fromStdWString(path));
        }
    }
}

void MainWindow::onEditCopyFileName()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        if (buffer) {
            std::wstring fileName = buffer->getFileName();
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString::fromStdWString(fileName));
        }
    }
}

void MainWindow::onEditCopyDirPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        if (buffer) {
            std::wstring path = buffer->getFullPathName();
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                QClipboard* clipboard = QApplication::clipboard();
                clipboard->setText(QString::fromStdWString(dir));
            }
        }
    }
}

void MainWindow::onEditIncreaseIndent()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_TAB);
        }
    }
}

void MainWindow::onEditDecreaseIndent()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_BACKTAB);
        }
    }
}

void MainWindow::onEditUpperCase()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->convertSelectedTextToUpperCase();
        }
    }
}

void MainWindow::onEditLowerCase()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->convertSelectedTextToLowerCase();
        }
    }
}

void MainWindow::onEditTitleCase()
{
    // Title case conversion - convert first letter of each word to uppercase
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            // Get selected text
            QByteArray selectedText;
            size_t selStart = view->execute(SCI_GETSELECTIONSTART);
            size_t selEnd = view->execute(SCI_GETSELECTIONEND);
            if (selEnd > selStart) {
                size_t len = selEnd - selStart;
                selectedText.resize(static_cast<int>(len) + 1);
                view->execute(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(selectedText.data()));
                QString text = QString::fromUtf8(selectedText);

                // Convert to title case
                bool newWord = true;
                for (int i = 0; i < text.length(); ++i) {
                    if (text[i].isLetter()) {
                        if (newWord) {
                            text[i] = text[i].toUpper();
                            newWord = false;
                        } else {
                            text[i] = text[i].toLower();
                        }
                    } else {
                        newWord = true;
                    }
                }

                // Replace selection
                view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(text.toUtf8().constData()));
            }
        }
    }
}

void MainWindow::onEditCopyBinary()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    // SCI_GETSELTEXT returns length including null terminator
    size_t bufLen = view->execute(SCI_GETSELTEXT, 0, 0);
    if (bufLen <= 1) return;

    size_t dataLen = bufLen - 1;
    auto pBinText = std::make_unique<char[]>(bufLen);
    view->execute(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(pBinText.get()));

    QMimeData* mimeData = new QMimeData();
    mimeData->setText(QString::fromUtf8(pBinText.get()));
    QByteArray rawData(pBinText.get(), static_cast<int>(dataLen));
    mimeData->setData("application/x-npp-binary-data", rawData);
    QByteArray lenData;
    lenData.setNum(static_cast<qulonglong>(dataLen));
    mimeData->setData("application/x-npp-binary-length", lenData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void MainWindow::onEditCutBinary()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    onEditCopyBinary();
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(""));
}

void MainWindow::onEditPasteBinary()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    if (!mimeData) return;

    if (mimeData->hasFormat("application/x-npp-binary-data"))
    {
        QByteArray rawData = mimeData->data("application/x-npp-binary-data");
        view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(""));
        view->execute(SCI_ADDTEXT, rawData.size(), reinterpret_cast<sptr_t>(rawData.constData()));
    }
    else if (mimeData->hasText())
    {
        QByteArray text = mimeData->text().toUtf8();
        view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(text.constData()));
    }
}

void MainWindow::onEditPasteAsHtml()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !mimeData->hasFormat("text/html")) return;

    QByteArray htmlData = mimeData->data("text/html");
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(htmlData.constData()));
}

void MainWindow::onEditPasteAsRtf()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !mimeData->hasFormat("text/rtf")) return;

    QByteArray rtfData = mimeData->data("text/rtf");
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(rtfData.constData()));
}

void MainWindow::onEditSearchOnInternet()
{
    if (!_pNotepad_plus) return;
    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    if (view->execute(SCI_GETSELECTIONS) != 1) return;

    size_t textLen = view->execute(SCI_GETSELTEXT, 0, 0);
    if (textLen <= 1) return;

    auto selText = std::make_unique<char[]>(textLen);
    view->execute(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(selText.get()));
    QString selectedText = QString::fromUtf8(selText.get());

    const NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();
    QString url;

    if (nppGUI._searchEngineChoice == NppGUI::se_custom)
    {
        url = QString::fromStdWString(nppGUI._searchEngineCustom).trimmed();
        if (url.isEmpty() || (!url.startsWith("http://") && !url.startsWith("https://")))
        {
            url = "https://www.google.com/search?q=$(CURRENT_WORD)";
        }
    }
    else if (nppGUI._searchEngineChoice == NppGUI::se_duckDuckGo || nppGUI._searchEngineChoice == NppGUI::se_bing)
    {
        url = "https://duckduckgo.com/?q=$(CURRENT_WORD)";
    }
    else if (nppGUI._searchEngineChoice == NppGUI::se_google)
    {
        url = "https://www.google.com/search?q=$(CURRENT_WORD)";
    }
    else if (nppGUI._searchEngineChoice == NppGUI::se_yahoo)
    {
        url = "https://search.yahoo.com/search?q=$(CURRENT_WORD)";
    }
    else if (nppGUI._searchEngineChoice == NppGUI::se_stackoverflow)
    {
        url = "https://stackoverflow.com/search?q=$(CURRENT_WORD)";
    }

    QString encodedText = QUrl::toPercentEncoding(selectedText);
    url.replace("$(CURRENT_WORD)", encodedText);

    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::onFileOpenInDefaultViewer()
{
    if (!_pNotepad_plus) return;
    Buffer* buf = _pNotepad_plus->getCurrentBuffer();
    if (!buf) return;

    std::wstring path = buf->getFullPathName();
    if (!path.empty())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(path)));
    }
}

// ============================================================================
// Slot Implementations - Search Menu
// ============================================================================

void MainWindow::onSearchFind()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(FIND_DLG);
    }
}

void MainWindow::onSearchReplace()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(REPLACE_DLG);
    }
}

void MainWindow::onSearchFindNext()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->findNext(1); // 1 = forward direction
    }
}

void MainWindow::onSearchFindPrev()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->findNext(-1); // -1 = backward direction
    }
}

void MainWindow::onSearchGoToLine()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showGoToLineDlg();
    }
}

// ============================================================================
// Slot Implementations - View Menu
// ============================================================================

void MainWindow::onViewFullScreen()
{
    toggleFullScreen();
}

void MainWindow::onViewPostIt()
{
    togglePostItMode();
}

void MainWindow::onViewDistractionFreeMode()
{
    toggleDistractionFreeMode();
}

void MainWindow::onViewAlwaysOnTop()
{
    setAlwaysOnTop(!isAlwaysOnTop());
    if (_alwaysOnTopAction)
    {
        _alwaysOnTopAction->setChecked(isAlwaysOnTop());
    }
}

void MainWindow::onViewTabColour(int colorId)
{
    if (!_pNotepad_plus) return;

    DocTabView* docTab = _pNotepad_plus->getCurrentDocTab();
    if (!docTab) return;

    int currentIndex = docTab->getCurrentTabIndex();
    BufferID bufferId = docTab->getBufferByIndex(currentIndex);
    if (bufferId)
    {
        docTab->setIndividualTabColour(bufferId, colorId);
    }
}

void MainWindow::onEditTextDirection(bool isRTL)
{
    if (!_pNotepad_plus) return;

    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    if (!view) return;

    if (view->isTextDirectionRTL() == isRTL)
        return;

    view->changeTextDirection(isRTL);

    // Wrap then unwrap to fix display of mirrored characters
    bool isWrapped = view->isWrap();
    view->wrap(!isWrapped);
    view->wrap(isWrapped);
}

void MainWindow::onViewWordWrap()
{
    if (!_pNotepad_plus) return;
    bool enabled = _wordWrapAction ? _wordWrapAction->isChecked() : false;
    _pNotepad_plus->wrapAllEditors(enabled);
    updateMenuState();
}

void MainWindow::onViewShowWhiteSpace()
{
    if (!_pNotepad_plus) return;
    bool enabled = _showWhiteSpaceAction ? _showWhiteSpaceAction->isChecked() : false;
    _pNotepad_plus->showWhiteSpace(enabled);
    updateMenuState();
}

void MainWindow::onViewShowEOL()
{
    if (!_pNotepad_plus) return;
    bool enabled = _showEOLAction ? _showEOLAction->isChecked() : false;
    _pNotepad_plus->showEOL(enabled);
    updateMenuState();
}

void MainWindow::onViewShowIndentGuide()
{
    if (!_pNotepad_plus) return;
    bool enabled = _showIndentGuideAction ? _showIndentGuideAction->isChecked() : false;
    _pNotepad_plus->showIndentGuide(enabled);
    updateMenuState();
}

void MainWindow::onViewFunctionList()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleFunctionList();
        updateMenuState();
    }
}

void MainWindow::onViewProjectPanel()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleProjectPanel(0);
        updateMenuState();
    }
}

void MainWindow::onViewDocumentMap()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleDocumentMap();
        updateMenuState();
    }
}

void MainWindow::onViewClipboardHistory()
{
    if (isPanelVisible("clipboardHistory")) {
        showPanel("clipboardHistory", false);
    } else {
        showPanel("clipboardHistory", true);
    }
    updateMenuState();
}

void MainWindow::onViewFileBrowser()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleFileBrowser();
        updateMenuState();
    }
}

void MainWindow::onViewMonitoring()
{
    if (!_pNotepad_plus)
        return;

    Buffer* buf = _pNotepad_plus->getCurrentBuffer();
    if (!buf)
        return;

    bool isMonitoring = buf->isMonitoringOn();
    _pNotepad_plus->monitoringStartOrStopAndUpdateUI(buf, !isMonitoring);

    // Update the menu check state
    if (_monitoringAction)
        _monitoringAction->setChecked(!isMonitoring);

    updateStatusBar();
}

void MainWindow::onViewHideLines()
{
    if (!_pNotepad_plus)
        return;

    ScintillaEditView* pEditView = _pNotepad_plus->getCurrentEditView();
    if (pEditView)
        pEditView->hideLines();
}

void MainWindow::onSearchChangedNext()
{
    if (_pNotepad_plus)
        _pNotepad_plus->changedHistoryGoTo(IDM_SEARCH_CHANGED_NEXT);
}

void MainWindow::onSearchChangedPrev()
{
    if (_pNotepad_plus)
        _pNotepad_plus->changedHistoryGoTo(IDM_SEARCH_CHANGED_PREV);
}

void MainWindow::onSearchClearChangeHistory()
{
    if (_pNotepad_plus)
        _pNotepad_plus->clearChangesHistory(_pNotepad_plus->currentView());
}

// ============================================================================
// Slot Implementations - Encoding Menu
// ============================================================================

void MainWindow::onEncodingANSI()
{
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            buf->setEncoding(-1);
            buf->setUnicodeMode(uni8Bit);
            updateStatusBar();
            updateEncodingMenu();
        }
    }
}

void MainWindow::onEncodingUTF8()
{
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            buf->setEncoding(-1);
            buf->setUnicodeMode(uniUTF8_NoBOM);
            updateStatusBar();
            updateEncodingMenu();
        }
    }
}

void MainWindow::onEncodingUTF8BOM()
{
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            buf->setEncoding(-1);
            buf->setUnicodeMode(uniUTF8);
            updateStatusBar();
            updateEncodingMenu();
        }
    }
}

void MainWindow::onEncodingUTF16BE()
{
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            buf->setEncoding(-1);
            buf->setUnicodeMode(uni16BE);
            updateStatusBar();
            updateEncodingMenu();
        }
    }
}

void MainWindow::onEncodingUTF16LE()
{
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            buf->setEncoding(-1);
            buf->setUnicodeMode(uni16LE);
            updateStatusBar();
            updateEncodingMenu();
        }
    }
}

void MainWindow::onCharsetSelected(int cmdId)
{
    if (!_pNotepad_plus)
        return;

    Buffer* buf = _pNotepad_plus->getCurrentBuffer();
    if (!buf)
        return;

    // Get codepage from the command ID using EncodingMapper
    int index = cmdId - IDM_FORMAT_ENCODE;
    const EncodingMapper& em = EncodingMapper::getInstance();
    int codepage = em.getEncodingFromIndex(index);
    if (codepage == -1)
        return;

    // Warn about unsaved changes
    if (buf->isDirty()) {
        int answer = QMessageBox::question(
            this,
            tr("Save Current Modification"),
            tr("You should save the current modification.\n"
               "All the saved modifications cannot be undone.\n\n"
               "Continue?"),
            QMessageBox::Yes | QMessageBox::No);
        if (answer != QMessageBox::Yes)
            return;
    }

    // Set the encoding and reload
    buf->setEncoding(codepage);
    buf->setUnicodeMode(uniUTF8_NoBOM);

    // Reload the file with the new encoding
    MainFileManager.reloadBuffer(buf->getID());

    updateStatusBar();
    updateEncodingMenu();
}

void MainWindow::updateEncodingMenu()
{
    if (!_pNotepad_plus)
        return;

    Buffer* buf = _pNotepad_plus->getCurrentBuffer();
    if (!buf)
        return;

    // Uncheck all first
    if (_encodingActionGroup) {
        QAction* checked = _encodingActionGroup->checkedAction();
        if (checked) {
            // QActionGroup with exclusive mode handles this, but we need to
            // programmatically set the right one
            checked->setChecked(false);
        }
    }

    int encoding = buf->getEncoding();
    if (encoding != -1) {
        // A charset encoding is active - find the matching action
        const EncodingMapper& em = EncodingMapper::getInstance();
        int index = em.getIndexFromEncoding(encoding);
        if (index >= 0) {
            int cmdId = IDM_FORMAT_ENCODE + index;
            auto it = _charsetActions.find(cmdId);
            if (it != _charsetActions.end()) {
                it.value()->setChecked(true);
                return;
            }
        }
    }

    // No charset encoding - check based on UniMode
    UniMode mode = buf->getUnicodeMode();
    switch (mode) {
        case uni8Bit:
            if (_ansiAction) _ansiAction->setChecked(true);
            break;
        case uniUTF8_NoBOM:
        case uni7Bit:
            if (_utf8Action) _utf8Action->setChecked(true);
            break;
        case uniUTF8:
            if (_utf8BomAction) _utf8BomAction->setChecked(true);
            break;
        case uni16BE:
        case uni16BE_NoBOM:
            if (_utf16beAction) _utf16beAction->setChecked(true);
            break;
        case uni16LE:
        case uni16LE_NoBOM:
            if (_utf16leAction) _utf16leAction->setChecked(true);
            break;
        default:
            if (_utf8Action) _utf8Action->setChecked(true);
            break;
    }
}

// ============================================================================
// Slot Implementations - Language Menu
// ============================================================================

void MainWindow::onLanguageSelected(QAction* action)
{
    if (!action || !_pNotepad_plus) {
        return;
    }

    QString langName = action->text();
    // Convert language name to LangType and set it
    LangType langType = L_TEXT;
    if (langName == "C") langType = L_C;
    else if (langName == "C++") langType = L_CPP;
    else if (langName == "C#") langType = L_CS;
    else if (langName == "Java") langType = L_JAVA;
    else if (langName == "Python") langType = L_PYTHON;
    else if (langName == "JavaScript") langType = L_JAVASCRIPT;
    else if (langName == "HTML") langType = L_HTML;
    else if (langName == "CSS") langType = L_CSS;
    else if (langName == "XML") langType = L_XML;
    else if (langName == "JSON") langType = L_JSON;
    else if (langName == "SQL") langType = L_SQL;
    else if (langName == "PHP") langType = L_PHP;
    else if (langName == "Ruby") langType = L_RUBY;
    else if (langName == "Go") langType = L_GOLANG;
    else if (langName == "Rust") langType = L_RUST;
    else if (langName == "TypeScript") langType = L_TYPESCRIPT;
    else if (langName == "Shell" || langName == "Bash") langType = L_BASH;
    else if (langName == "PowerShell") langType = L_POWERSHELL;
    else if (langName == "Batch") langType = L_BATCH;
    else if (langName == "Makefile") langType = L_MAKEFILE;
    else if (langName == "CMake") langType = L_CMAKE;
    else if (langName == "Markdown") langType = L_TEXT;  // No markdown lexer yet
    else if (langName == "YAML") langType = L_YAML;
    else if (langName == "Lua") langType = L_LUA;
    else if (langName == "Perl") langType = L_PERL;
    else if (langName == "R") langType = L_R;
    else if (langName == "Swift") langType = L_SWIFT;
    else if (langName == "Kotlin") langType = L_TEXT;  // No kotlin lexer yet
    else if (langName == "Scala") langType = L_TEXT;  // No scala lexer yet
    else if (langName == "Groovy") langType = L_TEXT;  // No groovy lexer yet
    else if (langName == "VB") langType = L_VB;
    else if (langName == "VBScript") langType = L_VB;
    else if (langName == "ActionScript") langType = L_FLASH;
    else if (langName == "CoffeeScript") langType = L_COFFEESCRIPT;
    else if (langName == "Dart") langType = L_TEXT;  // No dart lexer yet
    else if (langName == "Elixir") langType = L_TEXT;  // No elixir lexer yet
    else if (langName == "Erlang") langType = L_ERLANG;
    else if (langName == "Fortran") langType = L_FORTRAN;
    else if (langName == "Haskell") langType = L_HASKELL;
    else if (langName == "Julia") langType = L_TEXT;  // No julia lexer yet
    else if (langName == "Lisp") langType = L_LISP;
    else if (langName == "MATLAB") langType = L_MATLAB;
    else if (langName == "Objective-C") langType = L_OBJC;
    else if (langName == "Pascal") langType = L_PASCAL;
    else if (langName == "Raku") langType = L_RAKU;
    else if (langName == "Tcl") langType = L_TCL;
    else if (langName == "Verilog") langType = L_VERILOG;
    else if (langName == "VHDL") langType = L_VHDL;

    // Set the language through the core
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (buffer) {
        buffer->setLangType(langType);
        // Update status bar to reflect language change
        updateStatusBar();
    }
}

void MainWindow::onLanguageDefineUserLang()
{
    if (!_userDefineDialog) {
        _userDefineDialog = new QtControls::UserDefineDialog(this);
        // Initialize with the main edit view
        ScintillaEditView* mainView = getMainEditView();
        ScintillaEditView** ppEditView = &mainView;
        _userDefineDialog->init(ppEditView);
    }
    _userDefineDialog->doDialog();
}

// ============================================================================
// Slot Implementations - Settings Menu
// ============================================================================

void MainWindow::onSettingsPreferences()
{
    if (!_preferenceDlg)
    {
        _preferenceDlg = new QtControls::PreferenceDlg(this);

        // Connect dark mode change to plugin notification
        auto* darkModePage = _preferenceDlg->findChild<QtControls::DarkModeSubDlg*>();
        if (darkModePage)
        {
            connect(darkModePage, &QtControls::DarkModeSubDlg::darkModeChanged,
                this, [this](bool /*enabled*/)
                {
                    SCNotification scnN{};
                    scnN.nmhdr.code = NPPN_DARKMODECHANGED;
                    scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(this);
                    scnN.nmhdr.idFrom = 0;
                    _pNotepad_plus->getPluginsManager().notify(&scnN);
                });
        }
    }
    _preferenceDlg->doDialog();
}

void MainWindow::onSettingsStyleConfigurator()
{
    if (!_wordStyleDlg) {
        _wordStyleDlg = new QtControls::WordStyleDlg(this);
        _wordStyleDlg->init();
    }
    _wordStyleDlg->doDialog();

    // Notify plugins that word styles have been updated
    if (_pNotepad_plus)
    {
        SCNotification scnN{};
        scnN.nmhdr.code = NPPN_WORDSTYLESUPDATED;
        scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(this);
        scnN.nmhdr.idFrom = 0;
        _pNotepad_plus->getPluginsManager().notify(&scnN);
    }
}

void MainWindow::onSettingsShortcutMapper()
{
    if (!_shortcutMapper)
    {
        _shortcutMapper = new QtControls::ShortcutMapper::ShortcutMapper(this);

        // Connect shortcut remapped signal to plugin notification
        connect(_shortcutMapper, &QtControls::ShortcutMapper::ShortcutMapper::shortcutRemapped,
            this, [this](int cmdID, const KeyCombo& newKey)
            {
                SCNotification scnN{};
                scnN.nmhdr.code = NPPN_SHORTCUTREMAPPED;
                scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(const_cast<KeyCombo*>(&newKey));
                scnN.nmhdr.idFrom = static_cast<uptr_t>(cmdID);
                _pNotepad_plus->getPluginsManager().notify(&scnN);
            });
    }
    _shortcutMapper->doDialog();
}

void MainWindow::onSettingsPluginManager()
{
    if (!_pluginsAdminDlg) {
        _pluginsAdminDlg = new PluginsAdminDlg(this);
        _pluginsAdminDlg->create(IDD_PLUGINSADMIN_DLG, false);
        _pluginsAdminDlg->setPluginsManager(&_pNotepad_plus->getPluginsManager());
    }
    _pluginsAdminDlg->doDialog(false);
}

// ============================================================================
// Plugin Management
// ============================================================================

void MainWindow::initPlugins()
{
    if (!_pNotepad_plus) {
        return;
    }

    // Initialize NppData for plugins
    // Note: Scintilla handles are void* - plugins use these as identifiers
    NppData nppData;
    nppData._nppHandle = reinterpret_cast<HWND>(this);
    // Use the edit view pointers as handles - plugins treat these as opaque handles
    ScintillaEditView* mainView = _pNotepad_plus->getMainEditView();
    ScintillaEditView* subView = _pNotepad_plus->getSubEditView();
    nppData._scintillaMainHandle = reinterpret_cast<HWND>(mainView ? mainView->getHSelf() : nullptr);
    nppData._scintillaSecondHandle = reinterpret_cast<HWND>(subView ? subView->getHSelf() : nullptr);

    // Initialize plugin manager
    _pNotepad_plus->getPluginsManager().init(nppData);

    // Register the plugin message dispatcher so plugins can call SendMessage()
    ::nppPluginMessageDispatcher_register(_pNotepad_plus);

    // Load plugins from the plugins directory
    NppParameters& nppParam = NppParameters::getInstance();
    std::wstring pluginDir = nppParam.getPluginRootDir();

    // Load plugins (without plugin admin list for now)
    _pNotepad_plus->getPluginsManager().loadPlugins(pluginDir.c_str(), nullptr, nullptr);

    // Initialize plugin menu - this registers commands but doesn't create Qt menus
    _pNotepad_plus->getPluginsManager().initMenu(nullptr, false);

    // Notify plugins that toolbar can be modified
    SCNotification scnN{};
    scnN.nmhdr.code = NPPN_TBMODIFICATION;
    scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(this);
    scnN.nmhdr.idFrom = 0;
    _pNotepad_plus->getPluginsManager().notify(&scnN);

    // Create Qt plugins menu if plugins were loaded
    if (_pNotepad_plus->getPluginsManager().hasPlugins()) {
        populatePluginsMenu();
    }
}

void MainWindow::populatePluginsMenu()
{
    // Create the Plugins menu before the Window menu
    if (!_menuBar) {
        return;
    }

    // Find the Window menu position to insert before it
    int windowMenuIndex = -1;
    for (int i = 0; i < _menuBar->actions().size(); ++i) {
        if (_menuBar->actions()[i]->menu() == _windowMenu) {
            windowMenuIndex = i;
            break;
        }
    }

    // Create Plugins menu
    _pluginsMenu = new QMenu(tr("&Plugins"), this);

    // Add "Plugins Admin..." at the top
    _pluginsMenu->addAction(tr("Plugins Admin..."), this, &MainWindow::onSettingsPluginManager);
    _pluginsMenu->addSeparator();

    // Add plugin commands
    size_t pluginCount = _pNotepad_plus->getPluginsManager().getPluginCount();
    for (size_t i = 0; i < pluginCount; ++i) {
        const PluginInfo* pluginInfo = _pNotepad_plus->getPluginsManager().getPluginInfo(i);
        if (!pluginInfo) continue;

        // Create submenu for this plugin
        QString pluginName = QString::fromStdWString(pluginInfo->_funcName);
        QMenu* pluginSubMenu = _pluginsMenu->addMenu(pluginName);

        // Add plugin commands
        for (int j = 0; j < pluginInfo->_nbFuncItem; ++j) {
            const FuncItem& funcItem = pluginInfo->_funcItems[j];

            if (funcItem._pFunc == nullptr) {
                // Separator
                pluginSubMenu->addSeparator();
            } else {
                QString itemName = QString::fromWCharArray(funcItem._itemName);
                QAction* action = pluginSubMenu->addAction(itemName, this, &MainWindow::onPluginCommandTriggered);
                // Store command ID in the action's data
                action->setData(funcItem._cmdID);

                // Set shortcut if provided
                if (funcItem._pShKey) {
                    QKeySequence shortcut;
                    QString seq;
                    if (funcItem._pShKey->_isCtrl) seq += "Ctrl+";
                    if (funcItem._pShKey->_isAlt) seq += "Alt+";
                    if (funcItem._pShKey->_isShift) seq += "Shift+";
                    seq += QKeySequence(funcItem._pShKey->_key).toString();
                    action->setShortcut(QKeySequence(seq));
                }

                // Set checkable state
                if (funcItem._init2Check) {
                    action->setCheckable(true);
                    action->setChecked(true);
                }
            }
        }
    }

    // Insert the Plugins menu before the Window menu
    if (windowMenuIndex >= 0) {
        _menuBar->insertMenu(_menuBar->actions()[windowMenuIndex], _pluginsMenu);
    } else {
        _menuBar->addMenu(_pluginsMenu);
    }
}

void MainWindow::onPluginCommandTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    int cmdID = action->data().toInt();
    if (cmdID <= 0) return;

    // Find and execute the plugin command
    // The command ID is ID_PLUGINS_CMD + index into _pluginsCommands
    int commandIndex = cmdID - ID_PLUGINS_CMD;
    if (commandIndex >= 0) {
        _pNotepad_plus->getPluginsManager().runPluginCommand(static_cast<size_t>(commandIndex));
    }
}

// ============================================================================
// Slot Implementations - Macro Menu
// ============================================================================

void MainWindow::onMacroStartRecording()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->startMacroRecording();
    }
}

void MainWindow::onMacroStopRecording()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->stopMacroRecording();
    }
}

void MainWindow::onMacroPlayback()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->macroPlayback();
    }
}

void MainWindow::onMacroRunMultiple()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showRunMacroDlg();
    }
}

// ============================================================================
// Slot Implementations - Run Menu
// ============================================================================

void MainWindow::onRunRun()
{
    if (_pNotepad_plus) { _pNotepad_plus->showRunDlg(); }
}

void MainWindow::onRunLaunchInBrowser()
{
    if (!_pNotepad_plus) return;
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (!buffer) return;
    std::wstring filePath = buffer->getFullPathName();
    if (filePath.empty()) return;
    QString url = QString::fromStdWString(filePath);
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = QUrl::fromLocalFile(url).toString();
    }
    QDesktopServices::openUrl(QUrl(url));
}

// ============================================================================
// Slot Implementations - Window Menu
// ============================================================================

void MainWindow::onWindowNewInstance()
{
    // Launch a new Notepad++ instance
    QString appPath = QApplication::applicationFilePath();
    QProcess::startDetached(appPath, QStringList());
}

void MainWindow::onWindowSplit()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->docGotoAnotherEditView(TransferMove);
    }
}

void MainWindow::onWindowMoveToOtherView()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->docGotoAnotherEditView(TransferMove);
    }
}

void MainWindow::onWindowCloneToOtherView()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->docGotoAnotherEditView(TransferClone);
    }
}

void MainWindow::onWindowList()
{
    if (!_pNotepad_plus || !_mainDocTab)
        return;

    if (!_windowsDlg)
    {
        _windowsDlg = new WindowsDlg(this);
    }
    _windowsDlg->init(_pNotepad_plus, _mainDocTab);
    _windowsDlg->doDialog();
}

// ============================================================================
// Slot Implementations - Help Menu
// ============================================================================

void MainWindow::onHelpAbout()
{
    if (!_aboutDlg) {
        _aboutDlg = new QtControls::AboutDlg(this);
    }
    _aboutDlg->doDialog();
}

void MainWindow::onHelpCmdLineArgs()
{
    // Create and show command line arguments dialog
    if (!_cmdLineArgsDlg) {
        _cmdLineArgsDlg = new QtControls::CmdLineArgsDlg(this);
    }
    _cmdLineArgsDlg->doDialog();
}

void MainWindow::onHelpDebugInfo()
{
    // Create and show debug info dialog
    if (!_debugInfoDlg) {
        _debugInfoDlg = new QtControls::DebugInfoDlg(this);
    }
    _debugInfoDlg->doDialog();
}

// ============================================================================
// Slot Implementations - Tab Bar
// ============================================================================

void MainWindow::onTabChanged(int index)
{
    switchTab(index);
}

void MainWindow::onTabCloseRequested(int index)
{
    closeTab(index);
}

void MainWindow::onMainTabCloseRequested(int index)
{
    if (!_pNotepad_plus || !_mainDocTab) {
        return;
    }

    // Get the buffer ID for the tab being closed
    QtControls::BufferID bufferId = _mainDocTab->getBufferByIndex(static_cast<size_t>(index));
    if (bufferId != QtControls::BUFFER_INVALID) {
        // Activate the buffer first (required for proper closing)
        _mainDocTab->activateBuffer(bufferId);
        // Call fileClose with the specific buffer ID and view
        _pNotepad_plus->fileClose(bufferId, MAIN_VIEW);
    }
}

void MainWindow::onSubTabCloseRequested(int index)
{
    if (!_pNotepad_plus || !_subDocTab) {
        return;
    }

    // Get the buffer ID for the tab being closed
    QtControls::BufferID bufferId = _subDocTab->getBufferByIndex(static_cast<size_t>(index));
    if (bufferId != QtControls::BUFFER_INVALID) {
        // Activate the buffer first (required for proper closing)
        _subDocTab->activateBuffer(bufferId);
        // Call fileClose with the specific buffer ID and view
        _pNotepad_plus->fileClose(bufferId, SUB_VIEW);
    }
}

void MainWindow::onMainTabChanged(int index)
{
    std::cout << "[MainWindow::onMainTabChanged] ENTER - index=" << index << std::endl;

    if (!_pNotepad_plus || !_mainDocTab) {
        std::cerr << "[MainWindow::onMainTabChanged] ERROR: _pNotepad_plus or _mainDocTab is null" << std::endl;
        return;
    }

    // Get the buffer ID for the tab being activated
    QtControls::BufferID bufferId = _mainDocTab->getBufferByIndex(static_cast<size_t>(index));
    std::cout << "[MainWindow::onMainTabChanged] Got bufferId=" << bufferId << " for index=" << index << std::endl;

    if (bufferId != QtControls::BUFFER_INVALID) {
        std::cout << "[MainWindow::onMainTabChanged] Calling switchToFile..." << std::endl;
        // Switch to the file (this handles view switching and buffer activation)
        _pNotepad_plus->switchToFile(bufferId);
        std::cout << "[MainWindow::onMainTabChanged] switchToFile completed" << std::endl;
        // Update UI state after buffer switch
        updateMenuState();
        updateToolBarState();
        updateStatusBar();
    } else {
        std::cerr << "[MainWindow::onMainTabChanged] WARNING: BUFFER_INVALID for index=" << index << std::endl;
    }
    std::cout << "[MainWindow::onMainTabChanged] EXIT" << std::endl;
}

void MainWindow::onSubTabChanged(int index)
{
    std::cout << "[MainWindow::onSubTabChanged] ENTER - index=" << index << std::endl;

    if (!_pNotepad_plus || !_subDocTab) {
        std::cerr << "[MainWindow::onSubTabChanged] ERROR: _pNotepad_plus or _subDocTab is null" << std::endl;
        return;
    }

    // Get the buffer ID for the tab being activated
    QtControls::BufferID bufferId = _subDocTab->getBufferByIndex(static_cast<size_t>(index));
    std::cout << "[MainWindow::onSubTabChanged] Got bufferId=" << bufferId << " for index=" << index << std::endl;

    if (bufferId != QtControls::BUFFER_INVALID) {
        std::cout << "[MainWindow::onSubTabChanged] Calling switchToFile..." << std::endl;
        // Switch to the file (this handles view switching and buffer activation)
        _pNotepad_plus->switchToFile(bufferId);
        std::cout << "[MainWindow::onSubTabChanged] switchToFile completed" << std::endl;
        // Update UI state after buffer switch
        updateMenuState();
        updateToolBarState();
        updateStatusBar();
    } else {
        std::cerr << "[MainWindow::onSubTabChanged] WARNING: BUFFER_INVALID for index=" << index << std::endl;
    }
    std::cout << "[MainWindow::onSubTabChanged] EXIT" << std::endl;
}

// ============================================================================
// Slot Implementations - Panel
// ============================================================================

void MainWindow::onPanelVisibilityChanged(bool visible)
{
    // Update menu check states based on panel visibility
    (void)visible;
}

// ============================================================================
// Slot Implementations - Tray Icon
// ============================================================================

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
            // Double-click restores the window
            restoreFromTray();
            break;

        case QSystemTrayIcon::Trigger:
            // Single click also restores for better UX
            restoreFromTray();
            break;

        case QSystemTrayIcon::Context:
            // Right-click shows context menu (handled automatically by setContextMenu)
            break;

        default:
            break;
    }
}

// ============================================================================
// Getters
// ============================================================================

ScintillaEditView* MainWindow::getMainEditView() const
{
    if (_pNotepad_plus) {
        return _pNotepad_plus->getMainEditView();
    }
    return nullptr;
}

ScintillaEditView* MainWindow::getSubEditView() const
{
    if (_pNotepad_plus) {
        return _pNotepad_plus->getSubEditView();
    }
    return nullptr;
}

void MainWindow::updateTitle()
{
    if (!_pNotepad_plus) {
        setWindowTitle("Notepad++");
        return;
    }

    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (!buffer) {
        setWindowTitle("Notepad++");
        return;
    }

    QString title;

    // File name - use QString directly to avoid null pointer issues
    title = buffer->getFileNameQString();

    // Add modification indicator
    if (buffer->isDirty()) {
        title = "*" + title;
    }

    // Add read-only indicator
    if (buffer->isReadOnly()) {
        title = title + " [Read Only]";
    }

    // Add application name
    title = title + " - Notepad++";

    // Add admin indicator if running as admin (Linux doesn't typically have this concept)
    // but we could add it if needed

    setWindowTitle(title);
}

void MainWindow::updateDocumentState()
{
    // Update document-related UI state
    updateMenuState();
    updateToolBarState();
    updateStatusBar();
    updateTitle();
}

void MainWindow::refreshShortcuts()
{
    if (_shortcutManager) {
        _shortcutManager->applyShortcuts();
    }
}

// ============================================================================
// Shortcut Management
// ============================================================================

void MainWindow::registerMenuActionsWithShortcutManager()
{
    if (!_shortcutManager) {
        return;
    }

    // File menu
    if (_fileMenu) {
        for (QAction* action : _fileMenu->actions()) {
            if (action->menu()) continue; // Skip submenus for now
            QString text = action->text();
            if (text.contains("New") && !text.contains("Restore")) {
                action->setProperty("commandId", IDM_FILE_NEW);
                _shortcutManager->registerAction(IDM_FILE_NEW, action, QString("File"));
            } else if (text.contains("Open...")) {
                action->setProperty("commandId", IDM_FILE_OPEN);
                _shortcutManager->registerAction(IDM_FILE_OPEN, action, QString("File"));
            } else if (text.contains("Save") && !text.contains("As") && !text.contains("All")) {
                action->setProperty("commandId", IDM_FILE_SAVE);
                _shortcutManager->registerAction(IDM_FILE_SAVE, action, QString("File"));
            } else if (text.contains("Save As...")) {
                action->setProperty("commandId", IDM_FILE_SAVEAS);
                _shortcutManager->registerAction(IDM_FILE_SAVEAS, action, QString("File"));
            } else if (text.contains("Save All")) {
                action->setProperty("commandId", IDM_FILE_SAVEALL);
                _shortcutManager->registerAction(IDM_FILE_SAVEALL, action, QString("File"));
            } else if (text.contains("Close") && !text.contains("All")) {
                action->setProperty("commandId", IDM_FILE_CLOSE);
                _shortcutManager->registerAction(IDM_FILE_CLOSE, action, QString("File"));
            } else if (text.contains("Close All")) {
                action->setProperty("commandId", IDM_FILE_CLOSEALL);
                _shortcutManager->registerAction(IDM_FILE_CLOSEALL, action, QString("File"));
            } else if (text.contains("Exit")) {
                action->setProperty("commandId", IDM_FILE_EXIT);
                _shortcutManager->registerAction(IDM_FILE_EXIT, action, QString("File"));
            }
        }
    }

    // Edit menu
    if (_editMenu) {
        for (QAction* action : _editMenu->actions()) {
            if (action->menu()) continue;
            QString text = action->text();
            if (text.contains("Undo")) {
                action->setProperty("commandId", IDM_EDIT_UNDO);
                _shortcutManager->registerAction(IDM_EDIT_UNDO, action, QString("Edit"));
            } else if (text.contains("Redo")) {
                action->setProperty("commandId", IDM_EDIT_REDO);
                _shortcutManager->registerAction(IDM_EDIT_REDO, action, QString("Edit"));
            } else if (text.contains("Cut")) {
                action->setProperty("commandId", IDM_EDIT_CUT);
                _shortcutManager->registerAction(IDM_EDIT_CUT, action, QString("Edit"));
            } else if (text.contains("Copy")) {
                action->setProperty("commandId", IDM_EDIT_COPY);
                _shortcutManager->registerAction(IDM_EDIT_COPY, action, QString("Edit"));
            } else if (text.contains("Paste")) {
                action->setProperty("commandId", IDM_EDIT_PASTE);
                _shortcutManager->registerAction(IDM_EDIT_PASTE, action, QString("Edit"));
            } else if (text.contains("Delete")) {
                action->setProperty("commandId", IDM_EDIT_DELETE);
                _shortcutManager->registerAction(IDM_EDIT_DELETE, action, QString("Edit"));
            } else if (text.contains("Select All")) {
                action->setProperty("commandId", IDM_EDIT_SELECTALL);
                _shortcutManager->registerAction(IDM_EDIT_SELECTALL, action, QString("Edit"));
            }
        }
    }

    // Search menu
    if (_searchMenu) {
        for (QAction* action : _searchMenu->actions()) {
            if (action->menu()) continue;
            QString text = action->text();
            if (text.contains("Find...")) {
                action->setProperty("commandId", IDM_SEARCH_FIND);
                _shortcutManager->registerAction(IDM_SEARCH_FIND, action, QString("Search"));
            } else if (text.contains("Find Next")) {
                action->setProperty("commandId", IDM_SEARCH_FINDNEXT);
                _shortcutManager->registerAction(IDM_SEARCH_FINDNEXT, action, QString("Search"));
            } else if (text.contains("Find Previous")) {
                action->setProperty("commandId", IDM_SEARCH_FINDPREV);
                _shortcutManager->registerAction(IDM_SEARCH_FINDPREV, action, QString("Search"));
            } else if (text.contains("Replace...")) {
                action->setProperty("commandId", IDM_SEARCH_REPLACE);
                _shortcutManager->registerAction(IDM_SEARCH_REPLACE, action, QString("Search"));
            } else if (text.contains("Go To...")) {
                action->setProperty("commandId", IDM_SEARCH_GOTOLINE);
                _shortcutManager->registerAction(IDM_SEARCH_GOTOLINE, action, QString("Search"));
            }
        }
    }

    // View menu - register view mode actions
    if (_viewMenu) {
        for (QAction* action : _viewMenu->actions()) {
            if (action->menu()) {
                // Handle submenus
                QMenu* subMenu = action->menu();
                QString subMenuText = action->text();
                if (subMenuText == tr("View Mode")) {
                    for (QAction* viewAction : subMenu->actions()) {
                        QString viewText = viewAction->text();
                        if (viewText.contains("Full Screen")) {
                            viewAction->setProperty("commandId", IDM_VIEW_FULLSCREENTOGGLE);
                            _shortcutManager->registerAction(IDM_VIEW_FULLSCREENTOGGLE, viewAction, QString("View"));
                        } else if (viewText.contains("Post-it")) {
                            viewAction->setProperty("commandId", IDM_VIEW_POSTIT);
                            _shortcutManager->registerAction(IDM_VIEW_POSTIT, viewAction, QString("View"));
                        } else if (viewText.contains("Distraction")) {
                            viewAction->setProperty("commandId", IDM_VIEW_DISTRACTIONFREE);
                            _shortcutManager->registerAction(IDM_VIEW_DISTRACTIONFREE, viewAction, QString("View"));
                        }
                    }
                } else if (subMenuText == tr("Show Symbol")) {
                    for (QAction* symbolAction : subMenu->actions()) {
                        QString symbolText = symbolAction->text();
                        if (symbolText.contains("White Space")) {
                            symbolAction->setProperty("commandId", IDM_VIEW_TAB_SPACE);
                            _shortcutManager->registerAction(IDM_VIEW_TAB_SPACE, symbolAction, QString("View"));
                        } else if (symbolText.contains("End of Line")) {
                            symbolAction->setProperty("commandId", IDM_VIEW_EOL);
                            _shortcutManager->registerAction(IDM_VIEW_EOL, symbolAction, QString("View"));
                        } else if (symbolText.contains("Indent Guide")) {
                            symbolAction->setProperty("commandId", IDM_VIEW_INDENT_GUIDE);
                            _shortcutManager->registerAction(IDM_VIEW_INDENT_GUIDE, symbolAction, QString("View"));
                        }
                    }
                } else if (subMenuText == tr("Panel")) {
                    for (QAction* panelAction : subMenu->actions()) {
                        QString panelText = panelAction->text();
                        if (panelText.contains("Function")) {
                            panelAction->setProperty("commandId", IDM_VIEW_FUNC_LIST);
                            _shortcutManager->registerAction(IDM_VIEW_FUNC_LIST, panelAction, QString("View"));
                        } else if (panelText.contains("Project")) {
                            panelAction->setProperty("commandId", IDM_VIEW_PROJECT_PANEL_1);
                            _shortcutManager->registerAction(IDM_VIEW_PROJECT_PANEL_1, panelAction, QString("View"));
                        } else if (panelText.contains("Document Map")) {
                            panelAction->setProperty("commandId", IDM_VIEW_DOC_MAP);
                            _shortcutManager->registerAction(IDM_VIEW_DOC_MAP, panelAction, QString("View"));
                        } else if (panelText.contains("Clipboard")) {
                            panelAction->setProperty("commandId", IDM_EDIT_CLIPBOARDHISTORY_PANEL);
                            _shortcutManager->registerAction(IDM_EDIT_CLIPBOARDHISTORY_PANEL, panelAction, QString("View"));
                        } else if (panelText.contains("Workspace") || panelText.contains("Folder")) {
                            panelAction->setProperty("commandId", IDM_VIEW_FILEBROWSER);
                            _shortcutManager->registerAction(IDM_VIEW_FILEBROWSER, panelAction, QString("View"));
                        }
                    }
                }
            } else {
                QString text = action->text();
                if (text.contains("Always on Top")) {
                    action->setProperty("commandId", IDM_VIEW_ALWAYSONTOP);
                    _shortcutManager->registerAction(IDM_VIEW_ALWAYSONTOP, action, QString("View"));
                } else if (text.contains("Word Wrap")) {
                    action->setProperty("commandId", IDM_VIEW_WRAP);
                    _shortcutManager->registerAction(IDM_VIEW_WRAP, action, QString("View"));
                }
            }
        }
    }

    // Macro menu
    if (_macroMenu) {
        for (QAction* action : _macroMenu->actions()) {
            QString text = action->text();
            if (text.contains("Start Recording")) {
                action->setProperty("commandId", IDM_MACRO_STARTRECORDINGMACRO);
                _shortcutManager->registerAction(IDM_MACRO_STARTRECORDINGMACRO, action, QString("Macro"));
            } else if (text.contains("Stop Recording")) {
                action->setProperty("commandId", IDM_MACRO_STOPRECORDINGMACRO);
                _shortcutManager->registerAction(IDM_MACRO_STOPRECORDINGMACRO, action, QString("Macro"));
            } else if (text.contains("Playback")) {
                action->setProperty("commandId", IDM_MACRO_PLAYBACKRECORDEDMACRO);
                _shortcutManager->registerAction(IDM_MACRO_PLAYBACKRECORDEDMACRO, action, QString("Macro"));
            } else if (text.contains("Run a Macro Multiple")) {
                action->setProperty("commandId", IDM_MACRO_RUNMULTIMACRODLG);
                _shortcutManager->registerAction(IDM_MACRO_RUNMULTIMACRODLG, action, QString("Macro"));
            } else if (text.contains("Save Current Recorded Macro")) {
                action->setProperty("commandId", IDM_MACRO_SAVECURRENTMACRO);
                _shortcutManager->registerAction(IDM_MACRO_SAVECURRENTMACRO, action, QString("Macro"));
            }
        }
    }

    // Run menu
    if (_runMenu) {
        for (QAction* action : _runMenu->actions()) {
            QString text = action->text();
            if (text.contains("Run...")) {
                action->setProperty("commandId", IDM_EXECUTE);
                _shortcutManager->registerAction(IDM_EXECUTE, action, QString("Run"));
            }
        }
    }

    // Apply shortcuts from NppParameters
    _shortcutManager->applyShortcuts();
}

} // namespace MainWindow

} // namespace QtControls
