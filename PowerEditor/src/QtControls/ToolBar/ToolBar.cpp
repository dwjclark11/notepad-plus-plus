// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ToolBar.h"
#include "../../menuCmdID.h"
#include "../../Parameters.h"
#include <QToolButton>
#include <QStyle>
#include <QMenu>
#include <QMainWindow>
#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QImage>
#include <QDebug>
#include <filesystem>
#include <cstring>

// Icon list indices matching Windows ImageListSet enum
enum ToolbarIconList
{
	HLIST_DEFAULT,
	HLIST_DISABLE,
	HLIST_DEFAULT2,
	HLIST_DISABLE2,
	HLIST_DEFAULT_DM,
	HLIST_DISABLE_DM,
	HLIST_DEFAULT_DM2,
	HLIST_DISABLE_DM2
};

struct ToolbarIconIdUnit
{
	const char* _id;
	bool _hasDisabledIcon;
};

static const ToolbarIconIdUnit toolbarIconIDs[] = {
	{ "new", false },
	{ "open", false },
	{ "save", true },
	{ "save-all", true },
	{ "close", false },
	{ "close-all", false },
	{ "print", false },
	{ "cut", true },
	{ "copy", true },
	{ "paste", true },
	{ "undo", true },
	{ "redo", true },
	{ "find", false },
	{ "replace", false },
	{ "zoom-in", false },
	{ "zoom-out", false },
	{ "sync-vertical", false },
	{ "sync-horizontal", false },
	{ "word-wrap", false },
	{ "all-chars", false },
	{ "indent-guide", false },
	{ "udl-dlg", false },
	{ "doc-map", false },
	{ "doc-list", false },
	{ "function-list", false },
	{ "folder-as-workspace", false },
	{ "monitoring", true },
	{ "record", true },
	{ "stop-record", true },
	{ "playback", true },
	{ "playback-multiple", true },
	{ "save-macro", true }
};

namespace QtControls {

// ============================================================================
// ToolBar implementation
// ============================================================================

ToolBar::ToolBar()
    : _state(TB_SMALL)
    , _nbButtons(0)
    , _nbDynButtons(0)
    , _nbTotalButtons(0)
    , _nbCurrentButtons(0)
    , _pRebar(nullptr)
    , _dpi(96)
{
}

ToolBar::~ToolBar()
{
    destroy();
}

void ToolBar::initTheme(NppXml::Document toolIconsDocRoot)
{
    _toolIcons = NppXml::firstChildElement(toolIconsDocRoot, "NotepadPlus");
    if (_toolIcons)
    {
        _toolIcons = NppXml::firstChildElement(_toolIcons, "ToolBarIcons");
        if (_toolIcons)
        {
            namespace fs = std::filesystem;
            fs::path iconFolderDir = NppParameters::getInstance().getUserPath();
            iconFolderDir /= L"toolbarIcons";

            const char* folderName = NppXml::attribute(_toolIcons, "icoFolderName");
            if (folderName && folderName[0] != '\0')
                iconFolderDir /= folderName;
            else
                iconFolderDir /= "default";

            size_t i = 0;
            std::string disabledSuffix = "_disabled";
            std::string ext = ".ico";
            for (const ToolbarIconIdUnit& icoUnit : toolbarIconIDs)
            {
                fs::path locator = iconFolderDir;
                locator /= icoUnit._id;
                locator.replace_extension(ext);
                if (fs::exists(locator))
                {
                    QString loc = QString::fromStdString(locator.string());
                    _customIconVect.push_back(iconLocator(HLIST_DEFAULT, i, loc));
                    _customIconVect.push_back(iconLocator(HLIST_DEFAULT2, i, loc));
                    _customIconVect.push_back(iconLocator(HLIST_DEFAULT_DM, i, loc));
                    _customIconVect.push_back(iconLocator(HLIST_DEFAULT_DM2, i, loc));
                }

                if (icoUnit._hasDisabledIcon)
                {
                    fs::path locatorDis = iconFolderDir;
                    locatorDis /= icoUnit._id;
                    locatorDis += disabledSuffix;
                    locatorDis.replace_extension(ext);
                    if (fs::exists(locatorDis))
                    {
                        QString loc = QString::fromStdString(locatorDis.string());
                        _customIconVect.push_back(iconLocator(HLIST_DISABLE, i, loc));
                        _customIconVect.push_back(iconLocator(HLIST_DISABLE2, i, loc));
                        _customIconVect.push_back(iconLocator(HLIST_DISABLE_DM, i, loc));
                        _customIconVect.push_back(iconLocator(HLIST_DISABLE_DM2, i, loc));
                    }
                }
                ++i;
            }
        }
    }
}

void ToolBar::initHideButtonsConf(NppXml::Document toolButtonsDocRoot, const ToolBarButtonUnit* buttonUnitArray, int arraySize)
{
    NppXml::Element toolButtons = NppXml::firstChildElement(toolButtonsDocRoot, "NotepadPlus");
    if (toolButtons)
    {
        toolButtons = NppXml::firstChildElement(toolButtons, "ToolbarButtons");
        if (toolButtons)
        {
            // Standard toolbar buttons
            NppXml::Element standardToolButtons = NppXml::firstChildElement(toolButtons, "Standard");
            if (standardToolButtons)
            {
                _toolbarStdButtonsConfArray = std::make_unique<bool[]>(arraySize);

                const char* isHideAll = NppXml::attribute(standardToolButtons, "hideAll");
                if (isHideAll && (std::strcmp(isHideAll, "yes") == 0))
                {
                    for (int i = 0; i < arraySize; ++i)
                        _toolbarStdButtonsConfArray[i] = false;
                }
                else
                {
                    for (int i = 0; i < arraySize; ++i)
                        _toolbarStdButtonsConfArray[i] = true;

                    for (NppXml::Element childNode = NppXml::firstChildElement(standardToolButtons, "Button");
                        childNode;
                        childNode = NppXml::nextSiblingElement(childNode, "Button"))
                    {
                        int cmdID = NppXml::intAttribute(childNode, "id", -1);
                        int index = NppXml::intAttribute(childNode, "index", -1);
                        const char* isHide = NppXml::attribute(childNode, "hide");

                        if (cmdID > -1 && index > -1 && isHide && (std::strcmp(isHide, "yes") == 0))
                        {
                            if (index < arraySize && buttonUnitArray[index]._cmdID == cmdID)
                                _toolbarStdButtonsConfArray[index] = false;
                        }
                    }
                }
            }

            // Plugin toolbar buttons
            NppXml::Element pluginToolButtons = NppXml::firstChildElement(toolButtons, "Plugin");
            if (pluginToolButtons)
            {
                const char* isHideAll = NppXml::attribute(pluginToolButtons, "hideAll");
                if (isHideAll && (std::strcmp(isHideAll, "yes") == 0))
                {
                    _toolbarPluginButtonsConf._isHideAll = true;
                    return;
                }

                for (NppXml::Element childNode = NppXml::firstChildElement(pluginToolButtons, "Button");
                    childNode;
                    childNode = NppXml::nextSiblingElement(childNode, "Button"))
                {
                    bool doShow = true;
                    const char* isHide = NppXml::attribute(childNode, "hide");
                    doShow = !isHide || (std::strcmp(isHide, "yes") != 0);
                    _toolbarPluginButtonsConf._showPluginButtonsArray.push_back(doShow);
                }
            }
        }
    }
}

bool ToolBar::init(QWidget* parent, toolBarStatusType type,
                   const ToolBarButtonUnit* buttonUnitArray, int arraySize)
{
    if (!parent) return false;

    _parent = parent;
    _state = type;
    _dpi = 96; // Default DPI

    // Create the toolbar widget
    _widget = new QToolBar(parent);
    _widget->setObjectName("MainToolBar");
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return false;

    // Configure toolbar appearance
    toolbar->setMovable(true);
    toolbar->setFloatable(true);
    toolbar->setIconSize(QSize(16, 16));

    // Connect actionTriggered once - fires for any action click on this toolbar
    connect(toolbar, &QToolBar::actionTriggered, this, [this](QAction* action) {
        if (action)
        {
            int cmdID = action->data().toInt();
            if (cmdID != 0)
            {
                emit commandTriggered(cmdID);
            }
        }
    });

    // Setup icon lists
    setupIcons(type);

    // Create button array
    _nbButtons = static_cast<size_t>(arraySize);
    _nbDynButtons = _vDynBtnReg.size();
    _nbTotalButtons = _nbButtons + (_nbDynButtons > 0 ? _nbDynButtons + 1 : 0);

    _pTBB.clear();
    if (buttonUnitArray && arraySize > 0) {
        for (int i = 0; i < arraySize; ++i) {
            auto unit = std::make_unique<ToolBarButtonUnit>();
            *unit = buttonUnitArray[i];
            _pTBB.push_back(std::move(unit));
        }
    }

    // Fill toolbar with buttons
    reset(true);

    return true;
}

void ToolBar::destroy()
{
    if (_pRebar) {
        _pRebar->removeBand(_rbBand.wID);
        _pRebar = nullptr;
    }

    _actions.clear();
    _cmdToAction.clear();
    _pTBB.clear();
    _toolbarStdButtonsConfArray.reset();

    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

void ToolBar::enable(int cmdID, bool doEnable) const
{
    auto it = _cmdToAction.find(cmdID);
    if (it != _cmdToAction.end() && it->second) {
        it->second->setEnabled(doEnable);
    }
}

int ToolBar::getWidth() const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return 0;
    return toolbar->width();
}

int ToolBar::getHeight() const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return 0;
    return toolbar->height();
}

void ToolBar::reduce()
{
    _state = TB_SMALL;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::enlarge()
{
    _state = TB_LARGE;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::reduceToSet2()
{
    _state = TB_SMALL2;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::enlargeToSet2()
{
    _state = TB_LARGE2;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::setToBmpIcons()
{
    _state = TB_STANDARD;
    reset(true);
    redraw();
}

bool ToolBar::getCheckState(int ID2Check) const
{
    auto it = _cmdToAction.find(ID2Check);
    if (it != _cmdToAction.end() && it->second) {
        return it->second->isChecked();
    }
    return false;
}

void ToolBar::setCheck(int ID2Check, bool willBeChecked) const
{
    auto it = _cmdToAction.find(ID2Check);
    if (it != _cmdToAction.end() && it->second) {
        it->second->setChecked(willBeChecked);
    }
}

bool ToolBar::change2CustomIconsIfAny()
{
    if (!_toolIcons) return false;

    for (size_t i = 0, len = _customIconVect.size(); i < len; ++i)
        changeIcons(_customIconVect[i]._listIndex, _customIconVect[i]._iconIndex, _customIconVect[i]._iconLocation.toStdWString().c_str());
    return true;
}

bool ToolBar::changeIcons(size_t whichLst, size_t iconIndex, const wchar_t* iconLocation) const
{
    if (!iconLocation) return false;

    QString path = QString::fromWCharArray(iconLocation);
    QIcon icon(path);
    if (icon.isNull()) return false;

    // Find the action at the given icon index
    if (iconIndex >= _pTBB.size()) return false;
    int cmdID = _pTBB[iconIndex]->_cmdID;
    if (cmdID == 0) return false;

    auto it = _cmdToAction.find(cmdID);
    if (it == _cmdToAction.end() || !it->second) return false;

    // For disabled icon lists, add the pixmap as a disabled mode icon
    if (whichLst == HLIST_DISABLE || whichLst == HLIST_DISABLE2 ||
        whichLst == HLIST_DISABLE_DM || whichLst == HLIST_DISABLE_DM2)
    {
        QIcon currentIcon = it->second->icon();
        QPixmap pm = QPixmap(path);
        if (!pm.isNull())
        {
            currentIcon.addPixmap(pm, QIcon::Disabled, QIcon::Off);
            it->second->setIcon(currentIcon);
        }
    }
    else
    {
        it->second->setIcon(icon);
    }

    return true;
}

void ToolBar::registerDynBtn(unsigned int message, void* iconHandles, void* absentIco)
{
    // Register dynamic button (for plugins)
    // Note: Only possible before init!
    if (_widget || message == 0) return;

    DynamicCmdIcoBmp dynBtn;
    dynBtn._message = static_cast<int>(message);

    // On Linux, iconHandles is expected to be a QIcon* or null
    QIcon* iconPtr = static_cast<QIcon*>(iconHandles);
    if (iconPtr && !iconPtr->isNull())
    {
        dynBtn._icon = *iconPtr;
        dynBtn._iconDarkMode = *iconPtr;
    }
    else
    {
        // Use absent icon as fallback
        QIcon* absentPtr = static_cast<QIcon*>(absentIco);
        if (absentPtr && !absentPtr->isNull())
        {
            dynBtn._icon = *absentPtr;
            dynBtn._iconDarkMode = *absentPtr;
        }
        else
        {
            dynBtn._icon = QIcon::fromTheme("application-x-addon");
        }
    }

    _vDynBtnReg.push_back(dynBtn);
    _nbDynButtons = _vDynBtnReg.size();
}

void ToolBar::registerDynBtnDM(unsigned int message, void* iconHandles)
{
    // Register dynamic button with dark mode support
    if (_widget || message == 0) return;

    DynamicCmdIcoBmp dynBtn;
    dynBtn._message = static_cast<int>(message);

    // On Linux, iconHandles is expected to be a QIcon* or null
    QIcon* iconPtr = static_cast<QIcon*>(iconHandles);
    if (iconPtr && !iconPtr->isNull())
    {
        dynBtn._icon = *iconPtr;
        dynBtn._iconDarkMode = *iconPtr;
    }

    _vDynBtnReg.push_back(dynBtn);
    _nbDynButtons = _vDynBtnReg.size();
}

void ToolBar::doPopup(QPoint chevPoint)
{
    // Show popup menu for overflow buttons
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    QMenu menu;
    bool hasVisible = false;

    for (QAction* action : toolbar->actions()) {
        // In Qt, we can't easily detect "hidden" buttons due to overflow
        // So we show all actions in the popup as a fallback
        if (action && !action->isSeparator()) {
            menu.addAction(action);
            hasVisible = true;
        }
    }

    if (hasVisible) {
        menu.exec(chevPoint);
    }
}

void ToolBar::addToRebar(ReBar* rebar)
{
    if (!rebar || _pRebar) return;

    _pRebar = rebar;

    // Initialize band info
    _rbBand = ReBarBandInfo();
    _rbBand.wID = REBAR_BAR_TOOLBAR;
    _rbBand.hwndChild = _widget;
    _rbBand.cxMinChild = 0;
    _rbBand.cyMinChild = getHeight();
    _rbBand.cyMaxChild = getHeight();
    _rbBand.cxIdeal = getWidth();
    _rbBand.cx = getWidth();

    _pRebar->addBand(&_rbBand, true);
}

void ToolBar::resizeIconsDpi(int dpi)
{
    _dpi = dpi;

    int iconSize = 16;
    if (_state == TB_LARGE || _state == TB_LARGE2) {
        iconSize = 32;
    }

    // Scale by DPI
    iconSize = iconSize * dpi / 96;

    QToolBar* toolbar = getToolBar();
    if (toolbar) {
        toolbar->setIconSize(QSize(iconSize, iconSize));
    }
}

void ToolBar::reset(bool create)
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    if (create) {
        // Clear existing actions
        toolbar->clear();
        _actions.clear();
        _cmdToAction.clear();

        // Add standard buttons
        for (size_t i = 0; i < _nbButtons && i < _pTBB.size(); ++i) {
            const auto& unit = _pTBB[i];
            if (!unit) continue;

            if (unit->_cmdID == 0) {
                // Separator
                toolbar->addSeparator();
            } else {
                // Button
                QAction* action = new QAction(toolbar);
                action->setData(unit->_cmdID);

                // Set icon and tooltip
                QIcon icon = getIconForCommand(unit->_cmdID);
                if (!icon.isNull())
                    action->setIcon(icon);
                QString tooltip = getTooltipForCommand(unit->_cmdID);
                if (!tooltip.isEmpty())
                    action->setToolTip(tooltip);

                toolbar->addAction(action);
                _actions.push_back(action);
                _cmdToAction[unit->_cmdID] = action;
            }
        }

        // Add separator and dynamic buttons if any
        if (_nbDynButtons > 0) {
            toolbar->addSeparator();

            for (const auto& dynBtn : _vDynBtnReg) {
                QAction* action = new QAction(toolbar);
                action->setData(dynBtn._message);
                action->setIcon(dynBtn._icon);
                action->setText(dynBtn._tooltip);

                toolbar->addAction(action);
                _actions.push_back(action);
                _cmdToAction[dynBtn._message] = action;
            }
        }

        _nbCurrentButtons = _nbTotalButtons;
    }

    // Update band info if attached to rebar
    if (_pRebar) {
        _rbBand.hwndChild = _widget;
        _rbBand.cxMinChild = 0;
        _rbBand.cyMinChild = getHeight();
        _rbBand.cyMaxChild = getHeight();
        _rbBand.cxIdeal = getWidth();
        _rbBand.cx = getWidth();

        _pRebar->reNew(REBAR_BAR_TOOLBAR, &_rbBand);
    }
}

void ToolBar::setState(toolBarStatusType state)
{
    _state = state;
    emit iconSetChanged(static_cast<int>(_state));
}

void ToolBar::setDefaultImageList()
{
    updateButtonImages();
}

void ToolBar::setDisableImageList()
{
    applyDisabledIcons();
}

void ToolBar::setDefaultImageList2()
{
    updateButtonImages();
}

void ToolBar::setDisableImageList2()
{
    applyDisabledIcons();
}

void ToolBar::setDefaultImageListDM()
{
    updateButtonImages();
}

void ToolBar::setDisableImageListDM()
{
    applyDisabledIcons();
}

void ToolBar::setDefaultImageListDM2()
{
    updateButtonImages();
}

void ToolBar::setDisableImageListDM2()
{
    applyDisabledIcons();
}

QPixmap ToolBar::generateDisabledPixmap(const QPixmap& src)
{
    if (src.isNull()) return src;

    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y)
    {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x)
        {
            QRgb pixel = line[x];
            int alpha = qAlpha(pixel);
            int gray = qGray(pixel);
            // Reduce opacity to 40% and convert to grayscale
            line[x] = qRgba(gray, gray, gray, alpha * 40 / 100);
        }
    }
    return QPixmap::fromImage(img);
}

void ToolBar::applyDisabledIcons()
{
    for (auto& [cmdID, action] : _cmdToAction)
    {
        if (!action) continue;
        QIcon icon = action->icon();
        if (icon.isNull()) continue;

        // Check if the icon already has a disabled pixmap set
        QPixmap disabledPm = icon.pixmap(16, 16, QIcon::Disabled, QIcon::Off);
        QPixmap normalPm = icon.pixmap(16, 16, QIcon::Normal, QIcon::Off);

        // If disabled and normal look identical, generate a grayed version
        if (disabledPm.toImage() == normalPm.toImage())
        {
            QPixmap grayPm = generateDisabledPixmap(normalPm);
            icon.addPixmap(grayPm, QIcon::Disabled, QIcon::Off);
            action->setIcon(icon);
        }
    }
}

QIcon ToolBar::getIconForCommand(int cmdID) const
{
    // Use freedesktop icon theme names (works with KDE/GNOME themes)
    // Falls back to QStyle standard icons where no theme icon exists
    switch (cmdID)
    {
        case IDM_FILE_NEW:      return QIcon::fromTheme("document-new");
        case IDM_FILE_OPEN:     return QIcon::fromTheme("document-open");
        case IDM_FILE_SAVE:     return QIcon::fromTheme("document-save");
        case IDM_FILE_SAVEALL:  return QIcon::fromTheme("document-save-all",
                                    QIcon::fromTheme("document-save"));
        case IDM_FILE_CLOSE:    return QIcon::fromTheme("document-close");
        case IDM_FILE_CLOSEALL: return QIcon::fromTheme("document-close-all",
                                    QIcon::fromTheme("document-close"));
        case IDM_FILE_PRINT:    return QIcon::fromTheme("document-print");

        case IDM_EDIT_CUT:      return QIcon::fromTheme("edit-cut");
        case IDM_EDIT_COPY:     return QIcon::fromTheme("edit-copy");
        case IDM_EDIT_PASTE:    return QIcon::fromTheme("edit-paste");
        case IDM_EDIT_UNDO:     return QIcon::fromTheme("edit-undo");
        case IDM_EDIT_REDO:     return QIcon::fromTheme("edit-redo");
        case IDM_EDIT_SELECTALL: return QIcon::fromTheme("edit-select-all");

        case IDM_SEARCH_FIND:       return QIcon::fromTheme("edit-find");
        case IDM_SEARCH_REPLACE:    return QIcon::fromTheme("edit-find-replace");
        case IDM_SEARCH_FINDINFILES: return QIcon::fromTheme("folder-open",
                                        QIcon::fromTheme("edit-find"));

        case IDM_VIEW_ZOOMIN:   return QIcon::fromTheme("zoom-in");
        case IDM_VIEW_ZOOMOUT:  return QIcon::fromTheme("zoom-out");
        case IDM_VIEW_ZOOMRESTORE: return QIcon::fromTheme("zoom-original");

        case IDM_MACRO_STARTRECORDINGMACRO:   return QIcon::fromTheme("media-record");
        case IDM_MACRO_STOPRECORDINGMACRO:    return QIcon::fromTheme("media-playback-stop");
        case IDM_MACRO_PLAYBACKRECORDEDMACRO: return QIcon::fromTheme("media-playback-start");

        default: return QIcon();
    }
}

QString ToolBar::getTooltipForCommand(int cmdID) const
{
    switch (cmdID)
    {
        case IDM_FILE_NEW:      return tr("New File");
        case IDM_FILE_OPEN:     return tr("Open File");
        case IDM_FILE_SAVE:     return tr("Save");
        case IDM_FILE_SAVEALL:  return tr("Save All");
        case IDM_FILE_CLOSE:    return tr("Close");
        case IDM_FILE_CLOSEALL: return tr("Close All");
        case IDM_FILE_PRINT:    return tr("Print");

        case IDM_EDIT_CUT:      return tr("Cut");
        case IDM_EDIT_COPY:     return tr("Copy");
        case IDM_EDIT_PASTE:    return tr("Paste");
        case IDM_EDIT_UNDO:     return tr("Undo");
        case IDM_EDIT_REDO:     return tr("Redo");
        case IDM_EDIT_SELECTALL: return tr("Select All");

        case IDM_SEARCH_FIND:       return tr("Find");
        case IDM_SEARCH_REPLACE:    return tr("Replace");
        case IDM_SEARCH_FINDINFILES: return tr("Find in Files");

        case IDM_VIEW_ZOOMIN:   return tr("Zoom In");
        case IDM_VIEW_ZOOMOUT:  return tr("Zoom Out");
        case IDM_VIEW_ZOOMRESTORE: return tr("Restore Default Zoom");

        case IDM_MACRO_STARTRECORDINGMACRO:   return tr("Start Recording");
        case IDM_MACRO_STOPRECORDINGMACRO:    return tr("Stop Recording");
        case IDM_MACRO_PLAYBACKRECORDEDMACRO: return tr("Playback");

        default: return QString();
    }
}

void ToolBar::setupIcons(toolBarStatusType type)
{
    int iconSize = 16;
    if (type == TB_LARGE || type == TB_LARGE2)
        iconSize = 32;

    QToolBar* toolbar = getToolBar();
    if (toolbar)
        toolbar->setIconSize(QSize(iconSize, iconSize));
}

void ToolBar::fillToolbar()
{
    // Default toolbar filling - called by reset
}

void ToolBar::updateButtonImages()
{
    // Update action icons for all buttons
    for (size_t i = 0; i < _nbButtons && i < _pTBB.size(); ++i)
    {
        const auto& unit = _pTBB[i];
        if (!unit || unit->_cmdID == 0) continue;

        auto it = _cmdToAction.find(unit->_cmdID);
        if (it != _cmdToAction.end() && it->second)
        {
            QIcon icon = getIconForCommand(unit->_cmdID);
            if (!icon.isNull())
                it->second->setIcon(icon);
        }
    }
}

// ============================================================================
// ReBar implementation
// ============================================================================

ReBar::ReBar()
{
    _usedIDs.clear();
}

ReBar::~ReBar()
{
    destroy();
}

void ReBar::destroy()
{
    _bands.clear();
    _bandWidgets.clear();
    _usedIDs.clear();

    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

void ReBar::init(QWidget* parent)
{
    _parent = parent;

    // In Qt, ReBar functionality is handled by QMainWindow's toolbar areas
    // We create a container widget to hold the toolbars
    _widget = new QWidget(parent);
    _widget->setObjectName("ReBarContainer");
}

bool ReBar::addBand(ReBarBandInfo* rBand, bool useID)
{
    if (!rBand) return false;

    // Assign ID if needed
    if (useID) {
        if (isIDTaken(rBand->wID)) {
            return false;
        }
    } else {
        rBand->wID = getNewID();
    }

    // Store band info
    _bands[rBand->wID] = *rBand;

    // If band has a child widget, track it
    if (rBand->hwndChild) {
        _bandWidgets[rBand->wID] = rBand->hwndChild;
    }

    return true;
}

void ReBar::reNew(int id, ReBarBandInfo* rBand)
{
    if (!rBand) return;

    auto it = _bands.find(id);
    if (it != _bands.end()) {
        it->second = *rBand;

        if (rBand->hwndChild) {
            _bandWidgets[id] = rBand->hwndChild;
        }
    }
}

void ReBar::removeBand(int id)
{
    _bands.erase(id);
    _bandWidgets.erase(id);

    if (id >= REBAR_BAR_EXTERNAL) {
        releaseID(id);
    }
}

void ReBar::setIDVisible(int id, bool show)
{
    auto it = _bandWidgets.find(id);
    if (it != _bandWidgets.end() && it->second) {
        it->second->setVisible(show);
    }

    auto bandIt = _bands.find(id);
    if (bandIt != _bands.end()) {
        if (show) {
            bandIt->second.fStyle &= ~0x08; // Clear hidden flag (RBBS_HIDDEN = 0x08)
        } else {
            bandIt->second.fStyle |= 0x08;  // Set hidden flag
        }
    }
}

bool ReBar::getIDVisible(int id)
{
    auto bandIt = _bands.find(id);
    if (bandIt != _bands.end()) {
        return (bandIt->second.fStyle & 0x08) == 0; // Check RBBS_HIDDEN
    }
    return false;
}

void ReBar::setGrayBackground(int id)
{
    auto it = _bandWidgets.find(id);
    if (it != _bandWidgets.end() && it->second)
    {
        QPalette pal = it->second->palette();
        pal.setColor(QPalette::Window, QColor(0xe0, 0xe0, 0xe0));
        it->second->setAutoFillBackground(true);
        it->second->setPalette(pal);
    }
}

int ReBar::getNewID()
{
    int idToUse = REBAR_BAR_EXTERNAL;

    for (int usedID : _usedIDs) {
        if (usedID < idToUse) {
            continue;
        } else if (usedID == idToUse) {
            ++idToUse;
        } else {
            break; // Found gap
        }
    }

    _usedIDs.push_back(idToUse);
    return idToUse;
}

void ReBar::releaseID(int id)
{
    auto it = std::find(_usedIDs.begin(), _usedIDs.end(), id);
    if (it != _usedIDs.end()) {
        _usedIDs.erase(it);
    }
}

bool ReBar::isIDTaken(int id)
{
    return std::find(_usedIDs.begin(), _usedIDs.end(), id) != _usedIDs.end();
}

} // namespace QtControls
