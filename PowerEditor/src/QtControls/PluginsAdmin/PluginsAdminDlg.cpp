// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "PluginsAdminDlg.h"
#include "../../MISC/PluginsManager/PluginsManager.h"
#include "../../WinControls/PluginsAdmin/pluginsAdminRes.h"
#include "../../Parameters.h"
#include "../../resource.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QCryptographicHash>
#include <QtGui/QResizeEvent>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <filesystem>

namespace QtControls {

PluginsAdminDlg::PluginsAdminDlg(QWidget* parent)
    : StaticDialog(parent)
    , _availableList(this)
    , _updateList(this)
    , _installedList(this)
    , _incompatibleList(this)
{
    setWindowTitle(tr("Plugins Admin"));
    resize(700, 500);

    // Initialize status
    _nppCurrentStatus = new NppCurrentStatus();
}

PluginsAdminDlg::~PluginsAdminDlg()
{
    delete _nppCurrentStatus;
}

void PluginsAdminDlg::create(int dialogID, bool isRTL, bool msgDestParent)
{
    Q_UNUSED(dialogID)
    Q_UNUSED(msgDestParent)

    setupUI();
    connectSignals();
    setupDialog(isRTL);
    _isCreated = true;
}

void PluginsAdminDlg::doDialog(bool isRTL)
{
    if (!isCreated()) {
        create(IDD_PLUGINSADMIN_DLG, isRTL);
    }
    display();
    raise();
    activateWindow();
}

void PluginsAdminDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Search section
    auto* searchLayout = new QHBoxLayout();
    _searchLabel = new QLabel(tr("Search:"), this);
    searchLayout->addWidget(_searchLabel);

    _searchEdit = new QLineEdit(this);
    _searchEdit->setPlaceholderText(tr("Search for plugins..."));
    searchLayout->addWidget(_searchEdit, 1);

    _searchNextButton = new QPushButton(tr("Find Next"), this);
    searchLayout->addWidget(_searchNextButton);

    mainLayout->addLayout(searchLayout);

    // Tab widget
    _tabWidget = new QTabWidget(this);
    mainLayout->addWidget(_tabWidget, 1);

    createTabs();

    // Version info section
    auto* versionLayout = new QHBoxLayout();
    _versionLabel = new QLabel(tr("Plugin List Version:"), this);
    versionLayout->addWidget(_versionLabel);

    _versionNumberLabel = new QLabel("-", this);
    versionLayout->addWidget(_versionNumberLabel);

    versionLayout->addStretch();

    _repoLinkLabel = new QLabel(tr("<a href=\"https://github.com/notepad-plus-plus/nppPluginList\">Plugin List Repository</a>"), this);
    _repoLinkLabel->setOpenExternalLinks(true);
    versionLayout->addWidget(_repoLinkLabel);

    mainLayout->addLayout(versionLayout);

    // Button section
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _installButton = new QPushButton(tr("Install"), this);
    buttonLayout->addWidget(_installButton);

    _updateButton = new QPushButton(tr("Update"), this);
    buttonLayout->addWidget(_updateButton);

    _removeButton = new QPushButton(tr("Remove"), this);
    buttonLayout->addWidget(_removeButton);

    buttonLayout->addSpacing(20);

    _closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(_closeButton);

    mainLayout->addLayout(buttonLayout);

    updateButtonStates();
}

void PluginsAdminDlg::createTabs()
{
    // Available tab
    _availableTab = new QWidget();
    auto* availLayout = new QVBoxLayout(_availableTab);
    availLayout->setContentsMargins(4, 4, 4, 4);
    availLayout->addWidget(&_availableList);
    _tabWidget->addTab(_availableTab, tr("Available"));

    // Updates tab
    _updatesTab = new QWidget();
    auto* updateLayout = new QVBoxLayout(_updatesTab);
    updateLayout->setContentsMargins(4, 4, 4, 4);
    updateLayout->addWidget(&_updateList);
    _tabWidget->addTab(_updatesTab, tr("Updates"));

    // Installed tab
    _installedTab = new QWidget();
    auto* installedLayout = new QVBoxLayout(_installedTab);
    installedLayout->setContentsMargins(4, 4, 4, 4);
    installedLayout->addWidget(&_installedList);
    _tabWidget->addTab(_installedTab, tr("Installed"));

    // Incompatible tab
    _incompatibleTab = new QWidget();
    auto* incompatibleLayout = new QVBoxLayout(_incompatibleTab);
    incompatibleLayout->setContentsMargins(4, 4, 4, 4);
    incompatibleLayout->addWidget(&_incompatibleList);
    _tabWidget->addTab(_incompatibleTab, tr("Incompatible"));
}

void PluginsAdminDlg::connectSignals()
{
    connect(_tabWidget, &QTabWidget::currentChanged, this, &PluginsAdminDlg::onTabChanged);
    connect(_searchEdit, &QLineEdit::textChanged, this, &PluginsAdminDlg::onSearchTextChanged);
    connect(_searchNextButton, &QPushButton::clicked, this, &PluginsAdminDlg::onSearchNextClicked);
    connect(_installButton, &QPushButton::clicked, this, &PluginsAdminDlg::onInstallClicked);
    connect(_updateButton, &QPushButton::clicked, this, &PluginsAdminDlg::onUpdateClicked);
    connect(_removeButton, &QPushButton::clicked, this, &PluginsAdminDlg::onRemoveClicked);
    connect(_closeButton, &QPushButton::clicked, this, &PluginsAdminDlg::onCloseClicked);

    // Connect list signals
    connect(&_availableList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
    connect(&_availableList, &PluginViewList::itemDoubleClicked,
            this, &PluginsAdminDlg::onItemDoubleClicked);
    connect(&_updateList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
    connect(&_installedList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
    connect(&_incompatibleList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
}

void PluginsAdminDlg::onTabChanged(int index)
{
    Q_UNUSED(index)
    updateButtonStates();

    // Apply current search filter to the new tab's list
    QString searchText = _searchEdit->text();
    if (!searchText.isEmpty()) {
        onSearchTextChanged(searchText);
    }
}

void PluginsAdminDlg::onSearchTextChanged(const QString& text)
{
    _lastSearchText = text;

    PluginViewList* currentList = getCurrentList();
    if (currentList) {
        currentList->filterItems(text, _searchInNames, _searchInDescs);
    }
}

void PluginsAdminDlg::onSearchNextClicked()
{
    searchInPlugins(true);
}

void PluginsAdminDlg::onInstallClicked()
{
    installPlugins();
}

void PluginsAdminDlg::onUpdateClicked()
{
    updatePlugins();
}

void PluginsAdminDlg::onRemoveClicked()
{
    removePlugins();
}

void PluginsAdminDlg::onCloseClicked()
{
    hide();
}

void PluginsAdminDlg::onItemSelectionChanged(int index)
{
    Q_UNUSED(index)
    updateButtonStates();
}

void PluginsAdminDlg::onItemDoubleClicked(int index)
{
    Q_UNUSED(index)
    // Show plugin details or toggle selection
}

void PluginsAdminDlg::updateButtonStates()
{
    int currentTab = _tabWidget->currentIndex();

    // Enable/disable buttons based on current tab
    _installButton->setEnabled(currentTab == AVAILABLE_LIST);
    _updateButton->setEnabled(currentTab == UPDATES_LIST);
    _removeButton->setEnabled(currentTab == INSTALLED_LIST);

    // Check if items are selected/checked for additional button state logic
    PluginViewList* currentList = getCurrentList();
    if (currentList) {
        auto checked = currentList->getCheckedIndexes();
        bool hasChecked = !checked.empty();

        switch (currentTab) {
            case AVAILABLE_LIST:
                _installButton->setEnabled(hasChecked);
                break;
            case UPDATES_LIST:
                _updateButton->setEnabled(hasChecked);
                break;
            case INSTALLED_LIST:
                _removeButton->setEnabled(hasChecked);
                break;
            default:
                break;
        }
    }
}

PluginViewList* PluginsAdminDlg::getCurrentList()
{
    switch (_tabWidget->currentIndex()) {
        case AVAILABLE_LIST:
            return &_availableList;
        case UPDATES_LIST:
            return &_updateList;
        case INSTALLED_LIST:
            return &_installedList;
        case INCOMPATIBLE_LIST:
            return &_incompatibleList;
        default:
            return nullptr;
    }
}

const PluginViewList* PluginsAdminDlg::getCurrentList() const
{
    switch (_tabWidget->currentIndex()) {
        case AVAILABLE_LIST:
            return &_availableList;
        case UPDATES_LIST:
            return &_updateList;
        case INSTALLED_LIST:
            return &_installedList;
        case INCOMPATIBLE_LIST:
            return &_incompatibleList;
        default:
            return nullptr;
    }
}

bool PluginsAdminDlg::initFromJson()
{
    // Determine plugin list path
    if (_pluginListFullPath.empty())
    {
        NppParameters& nppParam = NppParameters::getInstance();
        std::wstring confDir = nppParam.getUserPluginConfDir();
        _pluginListFullPath = confDir + L"/pl/nppPluginList.json";
    }

    QFile file(QString::fromStdWString(_pluginListFullPath));
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    QJsonObject root = doc.object();

    // Read version
    if (root.contains("version"))
        _pluginListVersion = root["version"].toString().toStdWString();

    // Parse plugin array
    QJsonArray plugins = root["npp-plugins"].toArray();
    for (const auto& pluginVal : plugins)
    {
        QJsonObject obj = pluginVal.toObject();

        auto* pi = new PluginUpdateInfo();

        pi->_folderName = obj["folder-name"].toString().toStdWString();
        pi->_displayName = obj["display-name"].toString().toStdWString();
        pi->_author = obj["author"].toString().toStdWString();
        pi->_description = obj["description"].toString().toStdWString();
        pi->_id = obj["id"].toString().toStdWString();
        pi->_repository = obj["repository"].toString().toStdWString();
        pi->_homepage = obj["homepage"].toString().toStdWString();

        if (obj.contains("version"))
        {
            std::wstring verStr = obj["version"].toString().toStdWString();
            pi->_version = Version(verStr);
        }

        if (obj.contains("npp-compatible-versions"))
        {
            QString compatStr = obj["npp-compatible-versions"].toString();
            QStringList parts = compatStr.split('-');
            if (parts.size() == 2)
            {
                pi->_nppCompatibleVersions.first = Version(parts[0].toStdWString());
                pi->_nppCompatibleVersions.second = Version(parts[1].toStdWString());
            }
        }

        _availableList.pushBack(pi);
    }

    // Update version label
    if (_versionNumberLabel && !_pluginListVersion.empty())
        _versionNumberLabel->setText(QString::fromStdWString(_pluginListVersion));

    return true;
}

void PluginsAdminDlg::switchDialog(int indexToSwitch)
{
    if (indexToSwitch >= 0 && indexToSwitch < _tabWidget->count()) {
        _tabWidget->setCurrentIndex(indexToSwitch);
    }
}

void PluginsAdminDlg::setPluginsManager(PluginsManager* pluginsManager)
{
    _pPluginsManager = pluginsManager;
}

bool PluginsAdminDlg::updateList()
{
    // Clear existing lists
    _availableList.clear();
    _updateList.clear();
    _installedList.clear();
    _incompatibleList.clear();

    // Load from JSON
    if (!initFromJson()) {
        return false;
    }

    // Load installed plugins
    loadFromPluginInfos();

    // Check for updates
    checkUpdates();

    // Build incompatible list
    initIncompatiblePluginList();

    return true;
}

void PluginsAdminDlg::setAdminMode(bool isAdm)
{
    if (_nppCurrentStatus) {
        _nppCurrentStatus->_isAdminMode = isAdm;
    }
}

bool PluginsAdminDlg::installPlugins()
{
    auto checked = _availableList.getCheckedPlugins();
    if (checked.empty()) {
        QMessageBox::information(this, tr("No Plugins Selected"),
                                 tr("Please select plugins to install."));
        return false;
    }

    return exitToInstallRemovePlugins(pa_install, checked);
}

bool PluginsAdminDlg::updatePlugins()
{
    auto checked = _updateList.getCheckedPlugins();
    if (checked.empty()) {
        QMessageBox::information(this, tr("No Plugins Selected"),
                                 tr("Please select plugins to update."));
        return false;
    }

    return exitToInstallRemovePlugins(pa_update, checked);
}

bool PluginsAdminDlg::removePlugins()
{
    auto checked = _installedList.getCheckedPlugins();
    if (checked.empty()) {
        QMessageBox::information(this, tr("No Plugins Selected"),
                                 tr("Please select plugins to remove."));
        return false;
    }

    // Confirm removal
    auto reply = QMessageBox::question(this, tr("Confirm Removal"),
                                       tr("Are you sure you want to remove the selected plugins?"),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return false;
    }

    return exitToInstallRemovePlugins(pa_remove, checked);
}

void PluginsAdminDlg::changeTabName(LIST_TYPE index, const wchar_t* name2change)
{
    if (!name2change) return;

    QString name = QString::fromWCharArray(name2change);
    int tabIndex = -1;

    switch (index) {
        case AVAILABLE_LIST:
            tabIndex = 0;
            break;
        case UPDATES_LIST:
            tabIndex = 1;
            break;
        case INSTALLED_LIST:
            tabIndex = 2;
            break;
        case INCOMPATIBLE_LIST:
            tabIndex = 3;
            break;
        default:
            return;
    }

    if (tabIndex >= 0) {
        _tabWidget->setTabText(tabIndex, name);
    }
}

void PluginsAdminDlg::changeColumnName(COLUMN_TYPE index, const wchar_t* name2change)
{
    _availableList.changeColumnName(index, name2change);
    _updateList.changeColumnName(index, name2change);
    _installedList.changeColumnName(index, name2change);
    _incompatibleList.changeColumnName(index, name2change);
}

std::wstring PluginsAdminDlg::getPluginListVerStr() const
{
    return _pluginListVersion;
}

void PluginsAdminDlg::collectNppCurrentStatusInfos()
{
    if (!_nppCurrentStatus)
        return;

    NppParameters& nppParam = NppParameters::getInstance();

    // On Linux, there is no admin mode or Program Files concept
    _nppCurrentStatus->_isAdminMode = (geteuid() == 0);
    _nppCurrentStatus->_isInProgramFiles = false;
    _nppCurrentStatus->_isAppDataPluginsAllowed = true;
    _nppCurrentStatus->_nppInstallPath = nppParam.getNppPath();
    _nppCurrentStatus->_appdataPath = nppParam.getAppDataNppDir();
}

bool PluginsAdminDlg::searchInPlugins(bool isNextMode)
{
    Q_UNUSED(isNextMode)

    QString searchText = _searchEdit->text();
    if (searchText.isEmpty()) {
        return false;
    }

    PluginViewList* currentList = getCurrentList();
    if (!currentList) {
        return false;
    }

    // Search in names first, then descriptions
    long result = searchInNamesFromCurrentSel(*currentList, searchText.toStdWString(), isNextMode);
    if (result == -1 && _searchInDescs) {
        result = searchInDescsFromCurrentSel(*currentList, searchText.toStdWString(), isNextMode);
    }

    return result != -1;
}

bool PluginsAdminDlg::isFoundInListFromIndex(const PluginViewList& inWhichList, int index,
                                              const std::wstring& str2search, bool inWhichPart) const
{
    PluginUpdateInfo* pi = inWhichList.getPluginInfoFromUiIndex(static_cast<size_t>(index));
    if (!pi) return false;

    QString search = QString::fromStdWString(str2search).toLower();

    if (inWhichPart) {  // Search in names
        QString name = QString::fromStdWString(pi->_displayName).toLower();
        return name.contains(search);
    } else {  // Search in descriptions
        QString desc = QString::fromStdWString(pi->_description).toLower();
        return desc.contains(search);
    }
}

long PluginsAdminDlg::searchFromCurrentSel(const PluginViewList& inWhichList,
                                            const std::wstring& str2search,
                                            bool inWhichPart, bool isNextMode) const
{
    Q_UNUSED(isNextMode)

    long startIndex = inWhichList.getSelectedIndex();
    if (startIndex < 0) startIndex = 0;

    size_t count = inWhichList.nbItem();
    for (size_t i = 0; i < count; ++i) {
        long index = static_cast<long>((startIndex + i + 1) % count);
        if (isFoundInListFromIndex(inWhichList, index, str2search, inWhichPart)) {
            const_cast<PluginViewList&>(inWhichList).setSelection(index);
            return index;
        }
    }

    return -1;
}

long PluginsAdminDlg::searchInNamesFromCurrentSel(const PluginViewList& inWhichList,
                                                   const std::wstring& str2search,
                                                   bool isNextMode) const
{
    return searchFromCurrentSel(inWhichList, str2search, true, isNextMode);
}

long PluginsAdminDlg::searchInDescsFromCurrentSel(const PluginViewList& inWhichList,
                                                   const std::wstring& str2search,
                                                   bool isNextMode) const
{
    return searchFromCurrentSel(inWhichList, str2search, false, isNextMode);
}

bool PluginsAdminDlg::initAvailablePluginsViewFromList()
{
    // The available list is populated during initFromJson().
    // This method ensures the UI is refreshed after all data loading.
    return _availableList.nbItem() > 0;
}

bool PluginsAdminDlg::initIncompatiblePluginList()
{
    _incompatibleList.clear();

    // Get the current Notepad++ version for compatibility checks
    Version nppVer(VERSION_INTERNAL_VALUE);

    // Scan plugin directory for .so files that failed to load
    NppParameters& nppParam = NppParameters::getInstance();
    std::wstring pluginDir = nppParam.getPluginRootDir();

    std::filesystem::path pluginPath(
        std::string(pluginDir.begin(), pluginDir.end()));

    if (!std::filesystem::exists(pluginPath))
        return true;

    for (const auto& entry : std::filesystem::directory_iterator(pluginPath))
    {
        if (!entry.is_directory())
            continue;

        // Look for .so files in each plugin subdirectory
        for (const auto& fileEntry : std::filesystem::directory_iterator(entry.path()))
        {
            if (fileEntry.path().extension() != ".so")
                continue;

            std::wstring fileName(fileEntry.path().filename().wstring());
            std::wstring fullPath(fileEntry.path().wstring());

            // Check if this plugin is already loaded
            if (_pPluginsManager && _pPluginsManager->isPluginLoaded(fileName.c_str()))
                continue;

            // This .so exists but is not loaded -- it may be incompatible
            auto* pui = new PluginUpdateInfo(fullPath, fileName);

            // Check against available list for version compatibility info
            std::wstring folderName = entry.path().filename().wstring();
            int availIdx = -1;
            PluginUpdateInfo* availInfo = _availableList.findPluginInfoFromFolderName(
                folderName, availIdx);

            if (availInfo)
            {
                // Check if plugin is compatible with current Notepad++ version
                if (!availInfo->_nppCompatibleVersions.first.empty() &&
                    !nppVer.isCompatibleTo(availInfo->_nppCompatibleVersions.first,
                                           availInfo->_nppCompatibleVersions.second))
                {
                    pui->_displayName = availInfo->_displayName;
                    pui->_description = availInfo->_description;
                    pui->_author = availInfo->_author;
                    _incompatibleList.pushBack(pui);
                    continue;
                }
            }

            // Plugin not loaded but no incompatibility info -- still add
            _incompatibleList.pushBack(pui);
        }
    }

    return true;
}

bool PluginsAdminDlg::loadFromPluginInfos()
{
    if (!_pPluginsManager)
        return false;

    _installedList.clear();

    // Iterate through loaded plugins and cross-reference with available list
    for (const auto& dll : _pPluginsManager->getLoadedDlls())
    {
        // Extract folder name (plugin name without .so extension)
        std::wstring folderName = dll._displayName;

        int listIndex = -1;
        PluginUpdateInfo* foundInfo = _availableList.findPluginInfoFromFolderName(folderName, listIndex);

        if (!foundInfo)
        {
            // Plugin not in the available list -- create entry from file info
            auto* pui = new PluginUpdateInfo(dll._fullFilePath, dll._fileName);
            _installedList.pushBack(pui);
        }
        else
        {
            // Plugin found in available list -- merge info
            auto* pui = new PluginUpdateInfo(*foundInfo);
            pui->_fullFilePath = dll._fullFilePath;
            pui->_version.setVersionFrom(dll._fullFilePath);
            _installedList.pushBack(pui);

            // Hide from available list since it is already installed
            _availableList.hideFromListIndex(listIndex);

            // If installed version is older, add to update list
            if (pui->_version < foundInfo->_version)
            {
                auto* pui2 = new PluginUpdateInfo(*foundInfo);
                _updateList.pushBack(pui2);
            }
        }
    }

    return true;
}

bool PluginsAdminDlg::checkUpdates()
{
    // Updates are populated during loadFromPluginInfos() when an installed
    // plugin version is older than the available version. This method handles
    // any additional cross-referencing with incompatible plugins.
    for (size_t j = 0, nb = _incompatibleList.nbItem(); j < nb; ++j)
    {
        const PluginUpdateInfo* incompatPlugin = _incompatibleList.getPluginInfoFromUiIndex(j);
        if (!incompatPlugin)
            continue;

        int listIndex = -1;
        PluginUpdateInfo* foundAvail = _availableList.findPluginInfoFromFolderName(
            incompatPlugin->_folderName, listIndex);

        if (foundAvail && foundAvail->_version > incompatPlugin->_version)
        {
            _availableList.hideFromListIndex(listIndex);
            auto* pui = new PluginUpdateInfo(*foundAvail);
            _updateList.pushBack(pui);
        }
    }

    return true;
}

bool PluginsAdminDlg::exitToInstallRemovePlugins(Operation op, const std::vector<PluginUpdateInfo*>& puis)
{
    NppParameters& nppParam = NppParameters::getInstance();
    std::wstring pluginRootDir = nppParam.getPluginRootDir();
    QString pluginDir = QString::fromStdWString(pluginRootDir);

    // Ensure plugin directory exists
    QDir().mkpath(pluginDir);

    int successCount = 0;
    int failCount = 0;

    QProgressDialog progress(this);
    progress.setWindowTitle(tr("Plugin Admin"));
    progress.setMinimum(0);
    progress.setMaximum(static_cast<int>(puis.size()));
    progress.setModal(true);

    for (int idx = 0; idx < static_cast<int>(puis.size()); ++idx)
    {
        const PluginUpdateInfo* pi = puis[idx];

        if (op == pa_remove)
        {
            // Remove: delete the plugin's folder from the plugins directory
            progress.setLabelText(tr("Removing %1...").arg(QString::fromStdWString(pi->_displayName)));
            progress.setValue(idx);

            if (progress.wasCanceled())
                break;

            std::wstring folderName = pi->_folderName;
            if (folderName.empty())
            {
                // Derive folder name from display name (remove .so extension)
                auto lastDot = pi->_displayName.find_last_of(L'.');
                folderName = (lastDot != std::wstring::npos)
                    ? pi->_displayName.substr(0, lastDot)
                    : pi->_displayName;
            }

            QString targetDir = pluginDir + "/" + QString::fromStdWString(folderName);
            QDir dir(targetDir);
            if (dir.exists())
            {
                if (dir.removeRecursively())
                    ++successCount;
                else
                    ++failCount;
            }
            else
            {
                ++failCount;
            }
        }
        else if (op == pa_install || op == pa_update)
        {
            // Install/Update: download from repository and extract
            progress.setLabelText(
                tr("%1 %2...")
                    .arg(op == pa_install ? tr("Installing") : tr("Updating"))
                    .arg(QString::fromStdWString(pi->_displayName)));
            progress.setValue(idx);

            if (progress.wasCanceled())
                break;

            if (pi->_repository.empty())
            {
                ++failCount;
                continue;
            }

            // Build the download URL
            // Repository format: https://github.com/user/repo/releases/download/vX.Y/plugin.zip
            QString repoUrl = QString::fromStdWString(pi->_repository);
            QString pluginId = QString::fromStdWString(pi->_id);

            // Download the plugin archive
            QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString archivePath = tmpDir + "/npp_plugin_" + pluginId + ".zip";

            // Use curl for downloading (widely available on Linux)
            QProcess downloadProc;
            downloadProc.start("curl", {"-fsSL", "-o", archivePath, repoUrl});
            downloadProc.waitForFinished(60000);

            if (downloadProc.exitCode() != 0)
            {
                ++failCount;
                continue;
            }

            // Verify the downloaded file exists
            if (!QFile::exists(archivePath))
            {
                ++failCount;
                continue;
            }

            // For update: clean old files first
            std::wstring folderName = pi->_folderName;
            if (folderName.empty())
            {
                auto lastDot = pi->_displayName.find_last_of(L'.');
                folderName = (lastDot != std::wstring::npos)
                    ? pi->_displayName.substr(0, lastDot)
                    : pi->_displayName;
            }

            QString targetDir = pluginDir + "/" + QString::fromStdWString(folderName);

            if (op == pa_update)
            {
                QDir dir(targetDir);
                if (dir.exists())
                    dir.removeRecursively();
            }

            QDir().mkpath(targetDir);

            // Extract the archive using unzip
            QProcess extractProc;
            extractProc.start("unzip", {"-o", archivePath, "-d", targetDir});
            extractProc.waitForFinished(30000);

            if (extractProc.exitCode() == 0)
                ++successCount;
            else
                ++failCount;

            // Clean up the temp archive
            QFile::remove(archivePath);
        }
    }

    progress.setValue(static_cast<int>(puis.size()));

    // Show results summary
    QString message;
    if (failCount == 0)
    {
        message = tr("All operations completed successfully (%1 plugins).\n"
                      "Please restart Notepad++ for changes to take effect.")
                      .arg(successCount);
        QMessageBox::information(this, tr("Plugin Admin"), message);
    }
    else
    {
        message = tr("Operations completed: %1 succeeded, %2 failed.\n"
                      "Please restart Notepad++ for changes to take effect.")
                      .arg(successCount).arg(failCount);
        QMessageBox::warning(this, tr("Plugin Admin"), message);
    }

    // Refresh the lists
    updateList();

    return failCount == 0;
}

bool PluginsAdminDlg::run_dlgProc(QEvent* event)
{
    Q_UNUSED(event)
    return true;
}

void PluginsAdminDlg::resizeEvent(QResizeEvent* event)
{
    StaticDialog::resizeEvent(event);

    // Adjust column widths on resize
    if (_availableList.getTableWidget()) {
        _availableList.getTableWidget()->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    }
}

} // namespace QtControls
