// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

// Qt/Linux implementation of NppParameters class
// This file provides the Linux/Qt-specific implementation of the Parameters
// functionality that was originally Windows-specific in Parameters.cpp

#include "Parameters.h"

#ifndef _WIN32

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <exception>
#include <locale>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <SciLexer.h>
#include <Scintilla.h>

#include <iostream>

#include "Common.h"
#include "ContextMenu.h"
#include "Notepad_plus_Window.h"
#include "Notepad_plus_msgs.h"
#include "NppConstants.h"
#include "NppDarkMode.h"
#include "NppXml.h"
#include "ScintillaEditView.h"
#include "TabBar.h"
#include "ToolBar.h"
#include "UserDefineDialog.h"
#include "WordStyleDlg.h"
#include "menuCmdID.h"
#include "resource.h"
#include "Shortcut.h"

// VK constants are now included via Common.h -> LinuxTypes.h

// Qt includes for settings
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// Platform abstraction
#include "Platform/FileSystem.h"
#include "Platform/Settings.h"

// pathAppend is defined in Common.h for Linux
extern std::wstring pathAppend(std::wstring& strDest, const std::wstring& str2append);

static constexpr const wchar_t localConfFile[] = L"doLocalConf.xml";
static constexpr const wchar_t notepadStyleFile[] = L"asNotepad.xml";

static constexpr int NB_MAX_FINDHISTORY_FIND = 30;
static constexpr int NB_MAX_FINDHISTORY_REPLACE = 30;
static constexpr int NB_MAX_FINDHISTORY_PATH = 30;
static constexpr int NB_MAX_FINDHISTORY_FILTER = 20;

// Session backup extension
static constexpr const wchar_t SESSION_BACKUP_EXT[] = L".inCaseOfCorruption.bak";

using namespace std;

namespace // anonymous namespace
{

struct WinMenuKeyDefinition
{
    int vKey = 0;
    int functionId = 0;
    bool isCtrl = false;
    bool isAlt = false;
    bool isShift = false;
    const wchar_t* specialName = nullptr;
};

// Simplified key definitions for Linux - full definitions would be in a separate file
static constexpr WinMenuKeyDefinition winKeyDefs[]
{
    { 'N', IDM_FILE_NEW, true, false, false, nullptr },
    { 'O', IDM_FILE_OPEN, true, false, false, nullptr },
    { 'S', IDM_FILE_SAVE, true, false, false, nullptr },
    { 'S', IDM_FILE_SAVEAS, true, true, false, nullptr },
    { 'W', IDM_FILE_CLOSE, true, false, false, nullptr },
    { 'F', IDM_SEARCH_FIND, true, false, false, nullptr },
    { 'H', IDM_SEARCH_REPLACE, true, false, false, nullptr },
    { 'G', IDM_SEARCH_GOTOLINE, true, false, false, nullptr },
    { 'B', IDM_SEARCH_GOTOMATCHINGBRACE, true, false, false, nullptr },
    { 'Z', IDM_EDIT_UNDO, true, false, false, nullptr },
    { 'Y', IDM_EDIT_REDO, true, false, false, nullptr },
    { 'X', IDM_EDIT_CUT, true, false, false, nullptr },
    { 'C', IDM_EDIT_COPY, true, false, false, nullptr },
    { 'V', IDM_EDIT_PASTE, true, false, false, nullptr },
    { 'A', IDM_EDIT_SELECTALL, true, false, false, nullptr },
    { 'F', IDM_SEARCH_FINDINFILES, true, false, true, nullptr },
    { VK_F3, IDM_SEARCH_FINDNEXT, false, false, false, nullptr },
    { VK_F3, IDM_SEARCH_FINDPREV, false, false, true, nullptr },
    { VK_F5, IDM_EXECUTE, false, false, false, nullptr },
    { VK_F11, IDM_VIEW_FULLSCREENTOGGLE, false, false, false, nullptr },
    { VK_F12, IDM_VIEW_POSTIT, false, false, false, nullptr },
    { VK_NULL, 0, false, false, false, nullptr }
};

struct ScintillaKeyDefinition
{
    const wchar_t* name = nullptr;
    int functionId = 0;
    bool isCtrl = false;
    bool isAlt = false;
    bool isShift = false;
    int vKey = 0;
    int redirFunctionId = 0;
};

static constexpr ScintillaKeyDefinition scintKeyDefs[]
{
    {L"SCI_SELECTALL", SCI_SELECTALL, true, false, false, 'A', 0},
    {L"SCI_CLEAR", SCI_CLEAR, false, false, false, VK_DELETE, 0},
    {L"SCI_UNDO", SCI_UNDO, true, false, false, 'Z', 0},
    {L"SCI_REDO", SCI_REDO, true, false, false, 'Y', 0},
    {L"SCI_NEWLINE", SCI_NEWLINE, false, false, false, VK_RETURN, 0},
    {L"SCI_TAB", SCI_TAB, false, false, false, VK_TAB, 0},
    {L"SCI_BACKTAB", SCI_BACKTAB, false, false, true, VK_TAB, 0},
    {L"SCI_ZOOMIN", SCI_ZOOMIN, true, false, false, VK_ADD, 0},
    {L"SCI_ZOOMOUT", SCI_ZOOMOUT, true, false, false, VK_SUBTRACT, 0},
    {L"SCI_SETZOOM", SCI_SETZOOM, true, false, false, VK_DIVIDE, 0},
    {L"SCI_LINEDOWN", SCI_LINEDOWN, false, false, false, VK_DOWN, 0},
    {L"SCI_LINEUP", SCI_LINEUP, false, false, false, VK_UP, 0},
    {L"SCI_CHARLEFT", SCI_CHARLEFT, false, false, false, VK_LEFT, 0},
    {L"SCI_CHARRIGHT", SCI_CHARRIGHT, false, false, false, VK_RIGHT, 0},
    {L"SCI_WORDLEFT", SCI_WORDLEFT, true, false, false, VK_LEFT, 0},
    {L"SCI_WORDRIGHT", SCI_WORDRIGHT, true, false, false, VK_RIGHT, 0},
    {L"SCI_HOME", SCI_HOME, false, false, false, VK_HOME, 0},
    {L"SCI_LINEEND", SCI_LINEEND, false, false, false, VK_END, 0},
    {L"SCI_DOCUMENTSTART", SCI_DOCUMENTSTART, true, false, false, VK_HOME, 0},
    {L"SCI_DOCUMENTEND", SCI_DOCUMENTEND, true, false, false, VK_END, 0},
    {L"SCI_PAGEUP", SCI_PAGEUP, false, false, false, VK_PRIOR, 0},
    {L"SCI_PAGEDOWN", SCI_PAGEDOWN, false, false, false, VK_NEXT, 0},
    {L"SCI_DELETEBACK", SCI_DELETEBACK, false, false, false, VK_BACK, 0},
    {L"SCI_CANCEL", SCI_CANCEL, false, false, false, VK_ESCAPE, 0},
    {nullptr, 0, false, false, false, 0, 0}
};

int strVal(const wchar_t* str, int base)
{
    if (!str) return -1;
    if (!str[0]) return 0;

    wchar_t* finStr;
    int result = wcstol(str, &finStr, base);
    if (*finStr != '\0')
        return -1;
    return result;
}

int decStrVal(const wchar_t* str)
{
    return strVal(str, 10);
}

int hexStrVal(const wchar_t* str)
{
    return strVal(str, 16);
}

int getKwClassFromName(const wchar_t* str)
{
    if (!str) return -1;
    if (!lstrcmp(L"instre1", str)) return LANG_INDEX_INSTR;
    if (!lstrcmp(L"instre2", str)) return LANG_INDEX_INSTR2;
    if (!lstrcmp(L"type1", str)) return LANG_INDEX_TYPE;
    if (!lstrcmp(L"type2", str)) return LANG_INDEX_TYPE2;
    if (!lstrcmp(L"type3", str)) return LANG_INDEX_TYPE3;
    if (!lstrcmp(L"type4", str)) return LANG_INDEX_TYPE4;
    if (!lstrcmp(L"type5", str)) return LANG_INDEX_TYPE5;
    if (!lstrcmp(L"type6", str)) return LANG_INDEX_TYPE6;
    if (!lstrcmp(L"type7", str)) return LANG_INDEX_TYPE7;
    if (!lstrcmp(L"substyle1", str)) return LANG_INDEX_SUBSTYLE1;
    if (!lstrcmp(L"substyle2", str)) return LANG_INDEX_SUBSTYLE2;
    if (!lstrcmp(L"substyle3", str)) return LANG_INDEX_SUBSTYLE3;
    if (!lstrcmp(L"substyle4", str)) return LANG_INDEX_SUBSTYLE4;
    if (!lstrcmp(L"substyle5", str)) return LANG_INDEX_SUBSTYLE5;
    if (!lstrcmp(L"substyle6", str)) return LANG_INDEX_SUBSTYLE6;
    if (!lstrcmp(L"substyle7", str)) return LANG_INDEX_SUBSTYLE7;
    if (!lstrcmp(L"substyle8", str)) return LANG_INDEX_SUBSTYLE8;

    if ((str[1] == '\0') && (str[0] >= '0') && (str[0] <= '8'))
        return str[0] - '0';

    return -1;
}

// Helper to convert wstring to QString
QString wstringToQString(const std::wstring& wstr)
{
    return QString::fromWCharArray(wstr.c_str());
}

// Helper to convert QString to wstring
std::wstring qstringToWstring(const QString& qstr)
{
    return qstr.toStdWString();
}

// Get XDG config directory
std::wstring getXdgConfigDir()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configPath.isEmpty()) {
        // Fallback to home directory/.config
        const char* home = getenv("HOME");
        if (home) {
            configPath = QString::fromUtf8(home) + "/.config";
        } else {
            // Last resort fallback
            configPath = "/tmp";
        }
    }
    configPath += "/notepad-plus-plus";
    return qstringToWstring(configPath);
}

// Get application data directory (for system-wide config)
std::wstring getAppDataDir()
{
    // On Linux, use /usr/share/notepad-plus-plus or /usr/local/share/notepad-plus-plus
    // For now, return a sensible default
    return L"/usr/share/notepad-plus-plus";
}

} // anonymous namespace

static bool getBoolAttribute(const NppXml::Element& elem, const char* name)
{
    const char* str = NppXml::attribute(elem, name);
    if (str)
        return std::strcmp(str, "yes") == 0;
    return false;
}

static void setBoolAttribute(NppXml::Element& elem, const char* name, bool isYes)
{
    NppXml::setAttribute(elem, name, isYes ? "yes" : "no");
}

void cutString(const wchar_t* str2cut, vector<std::wstring>& patternVect)
{
    if (str2cut == nullptr) return;

    const wchar_t* pBegin = str2cut;
    const wchar_t* pEnd = pBegin;

    static const auto& loc = std::locale::classic();

    while (*pEnd != '\0')
    {
        if (std::isspace(*pEnd, loc))
        {
            if (pBegin != pEnd)
                patternVect.emplace_back(pBegin, pEnd);
            pBegin = pEnd + 1;
        }
        ++pEnd;
    }

    if (pBegin != pEnd)
        patternVect.emplace_back(pBegin, pEnd);
}

void cutStringBy(const wchar_t* str2cut, vector<std::wstring>& patternVect, char byChar, bool allowEmptyStr)
{
    if (str2cut == nullptr) return;

    const wchar_t* pBegin = str2cut;
    const wchar_t* pEnd = pBegin;

    while (*pEnd != '\0')
    {
        if (*pEnd == byChar)
        {
            if (allowEmptyStr)
                patternVect.emplace_back(pBegin, pEnd);
            else if (pBegin != pEnd)
                patternVect.emplace_back(pBegin, pEnd);
            pBegin = pEnd + 1;
        }
        ++pEnd;
    }
    if (allowEmptyStr)
        patternVect.emplace_back(pBegin, pEnd);
    else if (pBegin != pEnd)
        patternVect.emplace_back(pBegin, pEnd);
}

std::wstring LocalizationSwitcher::getLangFromXmlFileName(const wchar_t* fn) const
{
    // Simple implementation - extract language from filename like "english.xml"
    std::wstring filename(fn);
    size_t dotPos = filename.find_last_of(L'.');
    if (dotPos != std::wstring::npos)
    {
        std::wstring langName = filename.substr(0, dotPos);
        // Convert to proper case
        if (!langName.empty())
        {
            // First letter uppercase, rest lowercase
            langName[0] = std::towupper(langName[0]);
            for (size_t i = 1; i < langName.length(); ++i)
                langName[i] = std::towlower(langName[i]);
        }
        return langName;
    }
    return std::wstring();
}

std::wstring LocalizationSwitcher::getXmlFilePathFromLangName(const wchar_t* langName) const
{
    for (size_t i = 0, len = _localizationList.size(); i < len; ++i)
    {
        if (lstrcmp(langName, _localizationList[i].first.c_str()) == 0)
            return _localizationList[i].second;
    }
    return std::wstring();
}

bool LocalizationSwitcher::addLanguageFromXml(const std::wstring& xmlFullPath)
{
    const wchar_t* fn = xmlFullPath.c_str();
    // Find last path separator
    const wchar_t* lastSep = wcsrchr(fn, L'/');
    if (!lastSep)
        lastSep = wcsrchr(fn, L'\\');
    if (lastSep)
        fn = lastSep + 1;

    std::wstring foundLang = getLangFromXmlFileName(fn);
    if (!foundLang.empty())
    {
        _localizationList.push_back(pair<wstring, wstring>(foundLang, xmlFullPath));
        return true;
    }
    return false;
}

bool LocalizationSwitcher::switchToLang(const wchar_t* lang2switch) const
{
    wstring langPath = getXmlFilePathFromLangName(lang2switch);
    if (langPath.empty())
        return false;

    // On Linux, copy the language file to nativeLang.xml location
    std::wstring nativeLangPath = _nativeLangPath;
    if (nativeLangPath.empty())
    {
        nativeLangPath = getXdgConfigDir();
        nativeLangPath += L"/nativeLang.xml";
    }

    // Use QFile for copy
    return QFile::copy(wstringToQString(langPath), wstringToQString(nativeLangPath));
}

std::wstring ThemeSwitcher::getThemeFromXmlFileName(const wchar_t* xmlFullPath) const
{
    if (!xmlFullPath || !xmlFullPath[0])
        return std::wstring();

    std::wstring path(xmlFullPath);
    // Find last path separator
    size_t lastSep = path.find_last_of(L"/\\");
    std::wstring filename = (lastSep != std::wstring::npos) ? path.substr(lastSep + 1) : path;

    // Remove extension
    size_t dotPos = filename.find_last_of(L'.');
    if (dotPos != std::wstring::npos)
        filename = filename.substr(0, dotPos);

    return filename;
}

int DynamicMenu::getTopLevelItemNumber() const
{
    int nb = 0;
    std::wstring previousFolderName;
    for (const MenuItemUnit& i : _menuItems)
    {
        if (i._parentFolderName.empty())
        {
            ++nb;
        }
        else
        {
            if (previousFolderName.empty())
            {
                ++nb;
                previousFolderName = i._parentFolderName;
            }
            else if (previousFolderName != i._parentFolderName)
            {
                ++nb;
                previousFolderName = i._parentFolderName;
            }
        }
    }
    return nb;
}

bool DynamicMenu::attach(HMENU hMenu, unsigned int posBase, int lastCmd, const std::wstring& lastCmdLabel)
{
    if (!hMenu) return false;

    _hMenu = hMenu;
    _posBase = posBase;
    _lastCmd = lastCmd;
    _lastCmdLabel = lastCmdLabel;

    return createMenu();
}

bool DynamicMenu::clearMenu() const
{
    // On Linux/Qt, menu handling is different - this is a stub
    return true;
}

bool DynamicMenu::createMenu() const
{
    // On Linux/Qt, menu handling is different - this is a stub
    return true;
}

winVer NppParameters::getWindowsVersion()
{
    // On Linux, return unknown - version info doesn't apply
    _platForm = Platform::PF_X64; // Assume 64-bit for modern Linux
    return WV_UNKNOWN;
}

NppParameters::NppParameters()
{
    std::cout << "[NppParameters::NppParameters] Starting constructor..." << std::endl << std::flush;

    // Get "windows" version (always unknown on Linux)
    _winVersion = getWindowsVersion();

    // Get current system code page (use UTF-8 on Linux)
    _currentSystemCodepage = CP_UTF8;

    // Prepare for default path - use executable location
    // Get executable path using Qt - but handle case where QApplication doesn't exist yet
    QString appPath;
    if (QCoreApplication::instance()) {
        appPath = QCoreApplication::applicationFilePath();
    } else {
        // Fallback: use current working directory
        appPath = QDir::currentPath() + "/notepad-plus-plus";
    }

    std::wstring wAppPath = qstringToWstring(appPath);

    // Remove filename to get directory
    int lastSepPos = appPath.lastIndexOf('/');
    if (lastSepPos > 0) {
        QString dirPath = appPath.left(lastSepPos);
        _nppPath = qstringToWstring(dirPath);
    } else {
        _nppPath = L"/usr/bin";  // Fallback
    }

    // Initialize current directory to startup directory
    QString currentDir = QDir::currentPath();
    _currentDirectory = qstringToWstring(currentDir);

    _appdataNppDir.clear();
    std::wcout << L"[NppParameters::NppParameters] _nppPath: '" << _nppPath << L"' (length: " << _nppPath.length() << L")" << std::endl;
    std::wstring notepadStylePath = _nppPath;
    std::wcout << L"[NppParameters::NppParameters] notepadStylePath: '" << notepadStylePath << L"' (length: " << notepadStylePath.length() << L")" << std::endl;
    std::wcout << L"[NppParameters::NppParameters] notepadStylePath created, about to append..." << std::endl;
    std::wstring notepadFile = notepadStyleFile;  // Convert array to wstring explicitly
    std::wcout << L"[NppParameters::NppParameters] notepadFile: " << notepadFile << std::endl;
    pathAppend(notepadStylePath, notepadFile);

    _asNotepadStyle = doesFileExist(notepadStylePath.c_str());

    // Load initial accelerator key definitions
    initMenuKeys();
    initScintillaKeys();
}

NppParameters::~NppParameters()
{
    for (auto it = _pXmlExternalLexerDoc.begin(), end = _pXmlExternalLexerDoc.end(); it != end; ++it)
        delete (*it);

    _pXmlExternalLexerDoc.clear();
}

bool NppParameters::reloadStylers(const wchar_t* stylePath)
{
    delete _pXmlUserStylerDoc;

    const wchar_t* stylePathToLoad = stylePath != nullptr ? stylePath : _stylerPath.c_str();
    _pXmlUserStylerDoc = new TiXmlDocument(stylePathToLoad);

    bool loadOkay = _pXmlUserStylerDoc->LoadFile();
    if (!loadOkay)
    {
        delete _pXmlUserStylerDoc;
        _pXmlUserStylerDoc = nullptr;
        return false;
    }
    _lexerStylerVect.clear();
    _widgetStyleArray.clear();

    getUserStylersFromXmlTree();

    // Reload plugin styles.
    for (size_t i = 0; i < getExternalLexerDoc()->size(); ++i)
    {
        TiXmlDocument* externalLexerDoc = getExternalLexerDoc()->at(i);
        TiXmlNode* root = externalLexerDoc->FirstChild(L"NotepadPlus");
        if (root)
            feedStylerArray(root);
    }
    return true;
}

bool NppParameters::reloadLang()
{
    // Use user path
    std::wstring nativeLangPath(_localizationSwitcher._nativeLangPath);

    // If "nativeLang.xml" does not exist, use npp path
    if (!doesFileExist(nativeLangPath.c_str()))
    {
        nativeLangPath = _nppPath;
        pathAppend(nativeLangPath, std::wstring(L"nativeLang.xml"));
        if (!doesFileExist(nativeLangPath.c_str()))
            return false;
    }

    delete _pXmlNativeLangDoc;

    _pXmlNativeLangDoc = new NppXml::NewDocument();
    const bool loadOkay = NppXml::loadFileNativeLang(_pXmlNativeLangDoc, nativeLangPath.c_str());
    if (!loadOkay)
    {
        delete _pXmlNativeLangDoc;
        _pXmlNativeLangDoc = nullptr;
        return false;
    }
    return loadOkay;
}

std::wstring NppParameters::getSpecialFolderLocation(int folderKind)
{
    // On Linux, map Windows CSIDL constants to XDG directories
    QString path;
    switch (folderKind)
    {
        case 0x001a: // CSIDL_APPDATA
            path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
            break;
        case 0x001c: // CSIDL_PROGRAM_FILES
            path = QString::fromWCharArray(L"/usr/share");
            break;
        default:
            path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            break;
    }
    return qstringToWstring(path);
}

std::wstring NppParameters::getSettingsFolder()
{
    if (_isLocal)
        return _nppPath;

    std::wstring settingsFolderPath = getXdgConfigDir();

    // Ensure directory exists
    if (!settingsFolderPath.empty()) {
        QDir dir;
        dir.mkpath(wstringToQString(settingsFolderPath));
    }

    return settingsFolderPath;
}

bool NppParameters::load()
{
    L_END = L_EXTERNAL;
    bool isAllLoaded = true;

    _isx64 = sizeof(void*) == 8;

    // Make localConf.xml path
    std::wstring localConfPath(_nppPath);
    pathAppend(localConfPath, localConfFile);

    // Test if doLocalConf.xml exists
    _isLocal = doesFileExist(localConfPath.c_str());

    // On Linux, we don't have the UAC/program files restriction
    // but we respect the local config file if present

    _pluginRootDir = _nppPath;
    pathAppend(_pluginRootDir, L"plugins");

    //
    // the 3rd priority: general default configuration
    //
    std::wstring nppPluginRootParent;
    if (_isLocal)
    {
        _userPath = nppPluginRootParent = _nppPath;
        _userPluginConfDir = _pluginRootDir;
        pathAppend(_userPluginConfDir, L"Config");
    }
    else
    {
        _userPath = getXdgConfigDir();

        // Ensure directory exists
        if (!_userPath.empty()) {
            QDir dir;
            dir.mkpath(wstringToQString(_userPath));
        }

        _appdataNppDir = _userPluginConfDir = _userPath;

        pathAppend(_userPluginConfDir, L"plugins");
        if (!_userPluginConfDir.empty()) {
            QDir dir;
            dir.mkpath(wstringToQString(_userPluginConfDir));
        }

        pathAppend(_userPluginConfDir, L"Config");
        if (!_userPluginConfDir.empty()) {
            QDir dir;
            dir.mkpath(wstringToQString(_userPluginConfDir));
        }
    }

    _pluginConfDir = _pluginRootDir;
    pathAppend(_pluginConfDir, L"Config");

    // Create directories if they don't exist
    if (!nppPluginRootParent.empty()) {
        QDir dir;
        dir.mkpath(wstringToQString(nppPluginRootParent));
    }
    if (!_pluginRootDir.empty()) {
        QDir dir;
        dir.mkpath(wstringToQString(_pluginRootDir));
    }

    _sessionPath = _userPath;

    // Detection cloud settings (simplified for Linux)
    _isCloud = false;

    //
    // the 1st priority: custom settings dir via command line argument
    //
    if (!_cmdSettingsDir.empty())
    {
        if (!doesDirectoryExist(_cmdSettingsDir.c_str()))
        {
            // Invalid directory - will be handled by caller
        }
        else
        {
            _userPath = _cmdSettingsDir;
            _sessionPath = _userPath;
        }
    }

    //--------------------------//
    // langs.xml : for per-user //
    //--------------------------//
    std::wstring langs_xml_path(_userPath);
    pathAppend(langs_xml_path, L"langs.xml");

    std::wstring modelLangsPath(_nppPath);
    pathAppend(modelLangsPath, L"langs.model.xml");

    bool doRecover = false;
    if (!doesFileExist(langs_xml_path.c_str()))
    {
        doRecover = true;
    }
    else
    {
        // Check if file is empty
        QFileInfo info(wstringToQString(langs_xml_path));
        if (info.size() == 0)
            doRecover = true;
    }

    if (doRecover && doesFileExist(modelLangsPath.c_str()))
    {
        QFile::copy(wstringToQString(modelLangsPath), wstringToQString(langs_xml_path));
    }

    _pXmlDoc = new TiXmlDocument(langs_xml_path);

    bool loadOkay = _pXmlDoc->LoadFile();
    if (!loadOkay)
    {
        delete _pXmlDoc;
        _pXmlDoc = nullptr;
        isAllLoaded = false;
    }
    else
    {
        getLangKeywordsFromXmlTree();
    }

    //---------------------------//
    // config.xml : for per-user //
    //---------------------------//
    std::wstring configPath(_userPath);
    pathAppend(configPath, L"config.xml");

    std::wstring srcConfigPath(_nppPath);
    pathAppend(srcConfigPath, L"config.model.xml");

    if (!doesFileExist(configPath.c_str()) && doesFileExist(srcConfigPath.c_str()))
    {
        QFile::copy(wstringToQString(srcConfigPath), wstringToQString(configPath));
    }

    _pXmlUserDoc = new TiXmlDocument(configPath.c_str());
    loadOkay = _pXmlUserDoc->LoadFile();

    if (!loadOkay)
    {
        TiXmlDeclaration* decl = new TiXmlDeclaration(L"1.0", L"UTF-8", L"");
        _pXmlUserDoc->LinkEndChild(decl);
    }
    else
    {
        getUserParametersFromXmlTree();
    }

    //----------------------------//
    // stylers.xml : for per-user //
    //----------------------------//

    _stylerPath = _userPath;
    pathAppend(_stylerPath, L"stylers.xml");

    if (!doesFileExist(_stylerPath.c_str()))
    {
        std::wstring srcStylersPath(_nppPath);
        pathAppend(srcStylersPath, L"stylers.model.xml");
        if (doesFileExist(srcStylersPath.c_str()))
        {
            QFile::copy(wstringToQString(srcStylersPath), wstringToQString(_stylerPath));
        }
    }

    if (_nppGUI._themeName.empty() || (!doesFileExist(_nppGUI._themeName.c_str())))
        _nppGUI._themeName.assign(_stylerPath);

    _pXmlUserStylerDoc = new TiXmlDocument(_nppGUI._themeName.c_str());

    loadOkay = _pXmlUserStylerDoc->LoadFile();
    if (!loadOkay)
    {
        delete _pXmlUserStylerDoc;
        _pXmlUserStylerDoc = nullptr;
        isAllLoaded = false;
    }
    else
    {
        getUserStylersFromXmlTree();
    }

    _themeSwitcher._stylesXmlPath = _stylerPath;
    _themeSwitcher.addDefaultThemeFromXml(_stylerPath);

    //-----------------------------------//
    // userDefineLang.xml : for per-user //
    //-----------------------------------//
    _userDefineLangsFolderPath = _userDefineLangPath = _userPath;
    pathAppend(_userDefineLangPath, L"userDefineLang.xml");
    pathAppend(_userDefineLangsFolderPath, L"userDefineLangs");

    _pXmlUserLangDoc = new TiXmlDocument(_userDefineLangPath.c_str());
    loadOkay = _pXmlUserLangDoc->LoadFile();
    if (!loadOkay)
    {
        delete _pXmlUserLangDoc;
        _pXmlUserLangDoc = nullptr;
        isAllLoaded = false;
    }
    else
    {
        auto r = addUserDefineLangsFromXmlTree(_pXmlUserLangDoc);
        if (r.second - r.first > 0)
            _pXmlUserLangsDoc.push_back(UdlXmlFileState(_pXmlUserLangDoc, false, true, r));
    }

    // Initialize localization
    std::wstring nativeLangPath = _userPath;
    pathAppend(nativeLangPath, L"nativeLang.xml");
    _localizationSwitcher._nativeLangPath = nativeLangPath;

    // Load shortcuts
    _shortcutsPath = _userPath;
    pathAppend(_shortcutsPath, L"shortcuts.xml");

    _pXmlShortcutDoc = new NppXml::NewDocument();
    loadOkay = NppXml::loadFileShortcut(_pXmlShortcutDoc, _shortcutsPath.c_str());
    if (loadOkay)
    {
        getShortcutsFromXmlTree();
        getMacrosFromXmlTree();
        getUserCmdsFromXmlTree();
        getPluginCmdsFromXmlTree();
        getScintKeysFromXmlTree();
    }
    else
    {
        delete _pXmlShortcutDoc;
        _pXmlShortcutDoc = nullptr;
    }

    // Load context menu
    _contextMenuPath = _nppPath;
    pathAppend(_contextMenuPath, L"contextMenu.xml");

    //----------------------------//
    // session.xml : for per-user //
    //----------------------------//
    pathAppend(_sessionPath, L"session.xml");

    // Don't load session.xml if not required in order to speed up!!
    const NppGUI& nppGUI = getNppGUI();
    if (nppGUI._rememberLastSession)
    {
        NppXml::Document pXmlSessionDoc = new NppXml::NewDocument();
        bool loadOkay = NppXml::loadFile(pXmlSessionDoc, _sessionPath.c_str());
        if (loadOkay)
        {
            loadOkay = getSessionFromXmlTree(pXmlSessionDoc, _session);
        }

        if (!loadOkay)
        {
            std::wstring sessionInCaseOfCorruption_bak = _sessionPath;
            sessionInCaseOfCorruption_bak += SESSION_BACKUP_EXT;
            if (doesFileExist(sessionInCaseOfCorruption_bak.c_str()))
            {
                bool bFileSwapOk = false;
                if (doesFileExist(_sessionPath.c_str()))
                {
                    // an invalid session.xml file exists - try to replace it with backup
                    bFileSwapOk = QFile::remove(wstringToQString(_sessionPath));
                    if (bFileSwapOk)
                    {
                        bFileSwapOk = QFile::copy(wstringToQString(sessionInCaseOfCorruption_bak), wstringToQString(_sessionPath));
                    }
                }
                else
                {
                    // no session.xml file - copy backup to session.xml
                    bFileSwapOk = QFile::copy(wstringToQString(sessionInCaseOfCorruption_bak), wstringToQString(_sessionPath));
                }

                if (bFileSwapOk)
                {
                    NppXml::Document pXmlSessionBackupDoc = new NppXml::NewDocument();
                    loadOkay = NppXml::loadFile(pXmlSessionBackupDoc, _sessionPath.c_str());
                    if (loadOkay)
                        loadOkay = getSessionFromXmlTree(pXmlSessionBackupDoc, _session);

                    delete pXmlSessionBackupDoc;
                }

                if (!loadOkay)
                    isAllLoaded = false;
            }
            else
            {
                // no backup file - this is OK for first run
                // Don't mark as failed, just continue with empty session
            }
        }

        delete pXmlSessionDoc;
    }

    return isAllLoaded;
}

// Static methods implementation
LangType NppParameters::getLangIDFromStr(const wchar_t* langName)
{
    if (!langName || !langName[0])
        return L_TEXT;

    int lang = static_cast<int32_t>(L_TEXT);
    for (; lang < L_EXTERNAL; ++lang)
    {
        const wchar_t* name = ScintillaEditView::_langNameInfoArray[lang]._langName;
        if (lstrcmp(name, langName) == 0)
        {
            return static_cast<LangType>(lang);
        }
    }

    // Cannot find language, check if it's an external one
    LangType l = static_cast<LangType>(lang);
    if (l == L_EXTERNAL)
    {
        int id = NppParameters::getInstance().getExternalLangIndexFromName(langName);
        if (id != -1) return static_cast<LangType>(id + L_EXTERNAL);
    }

    return L_TEXT;
}

std::wstring NppParameters::getLocPathFromStr(const std::wstring& localizationCode)
{
    // Map language codes to XML filenames
    // English variants
    if (localizationCode == L"en" || localizationCode == L"en-au" || localizationCode == L"en-bz" ||
        localizationCode == L"en-ca" || localizationCode == L"en-cb" || localizationCode == L"en-gb" ||
        localizationCode == L"en-ie" || localizationCode == L"en-jm" || localizationCode == L"en-nz" ||
        localizationCode == L"en-ph" || localizationCode == L"en-tt" || localizationCode == L"en-us" ||
        localizationCode == L"en-za" || localizationCode == L"en-zw")
        return L"english.xml";

    // A
    if (localizationCode == L"af") return L"afrikaans.xml";
    if (localizationCode == L"sq") return L"albanian.xml";
    if (localizationCode == L"ar") return L"arabic.xml";
    if (localizationCode == L"an") return L"aragonese.xml";
    if (localizationCode == L"az") return L"azerbaijani.xml";

    // B
    if (localizationCode == L"eu") return L"basque.xml";
    if (localizationCode == L"be") return L"belarusian.xml";
    if (localizationCode == L"bn") return L"bengali.xml";
    if (localizationCode == L"bs") return L"bosnian.xml";
    if (localizationCode == L"pt-br") return L"brazilian_portuguese.xml";
    if (localizationCode == L"br-fr") return L"breton.xml";
    if (localizationCode == L"bg") return L"bulgarian.xml";

    // C
    if (localizationCode == L"ca") return L"catalan.xml";
    if (localizationCode == L"zh-tw" || localizationCode == L"zh-hk" || localizationCode == L"zh-sg")
        return L"chinese.xml";
    if (localizationCode == L"zh" || localizationCode == L"zh-cn")
        return L"chineseSimplified.xml";
    if (localizationCode == L"hr") return L"croatian.xml";
    if (localizationCode == L"cs") return L"czech.xml";

    // D
    if (localizationCode == L"da") return L"danish.xml";
    if (localizationCode == L"nl") return L"dutch.xml";

    // E
    if (localizationCode == L"eo") return L"esperanto.xml";
    if (localizationCode == L"et") return L"estonian.xml";

    // F
    if (localizationCode == L"fo") return L"faroese.xml";
    if (localizationCode == L"fa") return L"farsi.xml";
    if (localizationCode == L"fi") return L"finnish.xml";
    if (localizationCode == L"fr") return L"french.xml";
    if (localizationCode == L"fy") return L"friulian.xml";

    // G
    if (localizationCode == L"gl") return L"galician.xml";
    if (localizationCode == L"ka") return L"georgian.xml";
    if (localizationCode == L"de") return L"german.xml";
    if (localizationCode == L"el") return L"greek.xml";
    if (localizationCode == L"gu") return L"gujarati.xml";

    // H
    if (localizationCode == L"he") return L"hebrew.xml";
    if (localizationCode == L"hi") return L"hindi.xml";
    if (localizationCode == L"hu") return L"hungarian.xml";

    // I
    if (localizationCode == L"is") return L"icelandic.xml";
    if (localizationCode == L"id" || localizationCode == L"in") return L"indonesian.xml";
    if (localizationCode == L"it") return L"italian.xml";

    // J
    if (localizationCode == L"ja") return L"japanese.xml";

    // K
    if (localizationCode == L"kn") return L"kannada.xml";
    if (localizationCode == L"kk") return L"kazakh.xml";
    if (localizationCode == L"ko") return L"korean.xml";
    if (localizationCode == L"ku") return L"kurdish.xml";
    if (localizationCode == L"ky") return L"kyrgyz.xml";

    // L
    if (localizationCode == L"lv") return L"latvian.xml";
    if (localizationCode == L"lt") return L"lithuanian.xml";
    if (localizationCode == L"lb") return L"luxembourgish.xml";

    // M
    if (localizationCode == L"mk") return L"macedonian.xml";
    if (localizationCode == L"ms") return L"malay.xml";
    if (localizationCode == L"mr") return L"marathi.xml";
    if (localizationCode == L"mn") return L"mongolian.xml";

    // N
    if (localizationCode == L"no" || localizationCode == L"nb") return L"norwegian.xml";
    if (localizationCode == L"nn") return L"nynorsk.xml";

    // O
    if (localizationCode == L"oc") return L"occitan.xml";

    // P
    if (localizationCode == L"pl") return L"polish.xml";
    if (localizationCode == L"pt") return L"portuguese.xml";
    if (localizationCode == L"pa") return L"punjabi.xml";

    // R
    if (localizationCode == L"ro") return L"romanian.xml";
    if (localizationCode == L"ru") return L"russian.xml";

    // S
    if (localizationCode == L"sa") return L"sanskrit.xml";
    if (localizationCode == L"sr") return L"serbian.xml";
    if (localizationCode == L"sr-cyrl") return L"serbianCyrillic.xml";
    if (localizationCode == L"si") return L"sinhala.xml";
    if (localizationCode == L"sk") return L"slovak.xml";
    if (localizationCode == L"sl") return L"slovenian.xml";
    if (localizationCode == L"es" || localizationCode == L"es-es") return L"spanish.xml";
    if (localizationCode == L"es-ar" || localizationCode == L"es-bo" || localizationCode == L"es-cl" ||
        localizationCode == L"es-co" || localizationCode == L"es-cr" || localizationCode == L"es-do" ||
        localizationCode == L"es-ec" || localizationCode == L"es-gt" || localizationCode == L"es-hn" ||
        localizationCode == L"es-mx" || localizationCode == L"es-ni" || localizationCode == L"es-pa" ||
        localizationCode == L"es-pe" || localizationCode == L"es-pr" || localizationCode == L"es-py" ||
        localizationCode == L"es-sv" || localizationCode == L"es-uy" || localizationCode == L"es-ve")
        return L"spanish_ar.xml";
    if (localizationCode == L"sv") return L"swedish.xml";

    // T
    if (localizationCode == L"tl") return L"tagalog.xml";
    if (localizationCode == L"tg") return L"tajik.xml";
    if (localizationCode == L"ta") return L"tamil.xml";
    if (localizationCode == L"tt") return L"tatar.xml";
    if (localizationCode == L"te") return L"telugu.xml";
    if (localizationCode == L"th") return L"thai.xml";
    if (localizationCode == L"tr") return L"turkish.xml";

    // U
    if (localizationCode == L"ug") return L"uyghur.xml";
    if (localizationCode == L"uk") return L"ukrainian.xml";
    if (localizationCode == L"uz") return L"uzbek.xml";

    // V
    if (localizationCode == L"vi") return L"vietnamese.xml";

    // W
    if (localizationCode == L"cy") return L"welsh.xml";

    // Z
    if (localizationCode == L"zu") return L"zulu.xml";

    // Default to English if no match
    return L"english.xml";
}

void NppParameters::destroyInstance()
{
    // Clean up the singleton instance if needed
    // In this implementation, the instance is managed by getInstancePointer
}

// ============================================================================
// XML Config Parsing Implementations
// ============================================================================

void NppParameters::getLangKeywordsFromXmlTree()
{
    if (!_pXmlDoc)
        return;

    TiXmlNode* root = _pXmlDoc->FirstChild(L"NotepadPlus");
    if (!root)
        return;

    feedKeyWordsParameters(root);
}

void NppParameters::feedKeyWordsParameters(TiXmlNode* node)
{
    TiXmlNode* langRoot = node->FirstChildElement(L"Languages");
    if (!langRoot)
        return;

    for (TiXmlNode* langNode = langRoot->FirstChildElement(L"Language");
        langNode;
        langNode = langNode->NextSibling(L"Language"))
    {
        if (_nbLang < NB_LANG)
        {
            TiXmlElement* element = langNode->ToElement();
            const wchar_t* name = element->Attribute(L"name");
            if (name)
            {
                const wchar_t* ext = element->Attribute(L"ext");
                const wchar_t* commentLine = element->Attribute(L"commentLine");
                const wchar_t* commentStart = element->Attribute(L"commentStart");
                const wchar_t* commentEnd = element->Attribute(L"commentEnd");

                const std::string cmtLine = commentLine ? wstring2string(commentLine, CP_UTF8) : "";
                const std::string cmtStart = commentStart ? wstring2string(commentStart, CP_UTF8) : "";
                const std::string cmtEnd = commentEnd ? wstring2string(commentEnd, CP_UTF8) : "";

                int tabSettings = -1;
                const wchar_t* tsVal = element->Attribute(L"tabSettings", &tabSettings);
                const wchar_t* buVal = element->Attribute(L"backspaceUnindent");

                _langList[_nbLang] = std::make_unique<Lang>(getLangIDFromStr(name), name, ext ? ext : L"",
                    cmtLine.c_str(), cmtStart.c_str(), cmtEnd.c_str(),
                    tsVal ? tabSettings : -1, buVal && (std::wcscmp(buVal, L"yes") == 0));

                for (TiXmlNode* kwNode = langNode->FirstChildElement(L"Keywords");
                    kwNode;
                    kwNode = kwNode->NextSibling(L"Keywords"))
                {
                    const wchar_t* indexName = (kwNode->ToElement())->Attribute(L"name");
                    TiXmlNode* kwVal = kwNode->FirstChild();
                    const wchar_t* keyWords = L"";
                    if ((indexName) && (kwVal))
                        keyWords = kwVal->Value();

                    int i = getKwClassFromName(indexName);
                    if (i >= 0 && i <= KEYWORDSET_MAX)
                        _langList[_nbLang]->setWords(keyWords, i);
                }
                ++_nbLang;
            }
        }
    }
}

bool NppParameters::getUserParametersFromXmlTree()
{
    if (!_pXmlUserDoc)
        return false;

    TiXmlNode* root = _pXmlUserDoc->FirstChild(L"NotepadPlus");
    if (!root)
        return false;

    feedGUIParameters(root);
    feedFileListParameters(root);

    // Erase and recreate History root
    TiXmlNode* node = root->FirstChildElement(L"History");
    if (node)
        root->RemoveChild(node);

    TiXmlElement HistoryNode(L"History");
    root->InsertEndChild(HistoryNode);

    feedFindHistoryParameters(root);
    feedProjectPanelsParameters(root);
    feedFileBrowserParameters(root);
    feedColumnEditorParameters(root);

    return true;
}

void NppParameters::feedGUIParameters(TiXmlNode* node)
{
    TiXmlNode* GUIRoot = node->FirstChildElement(L"GUIConfigs");
    if (!GUIRoot)
        return;

    for (TiXmlNode* childNode = GUIRoot->FirstChildElement(L"GUIConfig");
        childNode;
        childNode = childNode->NextSibling(L"GUIConfig"))
    {
        TiXmlElement* element = childNode->ToElement();
        const wchar_t* nm = element->Attribute(L"name");
        if (!nm)
            continue;

        auto parseYesNoBoolAttribute = [&element](const wchar_t* attrName, bool defaultValue = false) -> bool {
            const wchar_t* val = element->Attribute(attrName);
            if (val)
            {
                if (!lstrcmp(val, L"yes"))
                    return true;
                else if (!lstrcmp(val, L"no"))
                    return false;
            }
            return defaultValue;
        };

        if (!lstrcmp(nm, L"ToolBar"))
        {
            TiXmlNode* textNode = childNode->FirstChild();
            if (textNode)
            {
                const wchar_t* val = textNode->Value();
                if (val)
                {
                    if (!lstrcmp(val, L"small"))
                        _nppGUI._tbIconInfo._tbIconSet = TB_SMALL;
                    else if (!lstrcmp(val, L"large"))
                        _nppGUI._tbIconInfo._tbIconSet = TB_LARGE;
                    else if (!lstrcmp(val, L"small2"))
                        _nppGUI._tbIconInfo._tbIconSet = TB_SMALL2;
                    else if (!lstrcmp(val, L"large2"))
                        _nppGUI._tbIconInfo._tbIconSet = TB_LARGE2;
                    else
                        _nppGUI._tbIconInfo._tbIconSet = TB_STANDARD;
                }
            }
        }
        else if (!lstrcmp(nm, L"StatusBar"))
        {
            TiXmlNode* textNode = childNode->FirstChild();
            if (textNode)
            {
                const wchar_t* val = textNode->Value();
                if (val)
                {
                    if (!lstrcmp(val, L"hide"))
                        _nppGUI._statusBarShow = false;
                    else
                        _nppGUI._statusBarShow = true;
                }
            }
        }
        else if (!lstrcmp(nm, L"TabBar"))
        {
            _nppGUI._tabStatus = parseYesNoBoolAttribute(L"dragAndDrop") ? TAB_DRAWTOPBAR : 0;
            if (parseYesNoBoolAttribute(L"drawTopBar"))
                _nppGUI._tabStatus |= TAB_DRAWTOPBAR;
            if (parseYesNoBoolAttribute(L"drawInactiveTab"))
                _nppGUI._tabStatus |= TAB_DRAWINACTIVETAB;
            if (parseYesNoBoolAttribute(L"reduce"))
                _nppGUI._tabStatus |= TAB_REDUCE;
            if (parseYesNoBoolAttribute(L"closeButton"))
                _nppGUI._tabStatus |= TAB_CLOSEBUTTON;
            if (parseYesNoBoolAttribute(L"doubleClick2Close"))
                _nppGUI._tabStatus |= TAB_DBCLK2CLOSE;
            if (parseYesNoBoolAttribute(L"vertical"))
                _nppGUI._tabStatus |= TAB_VERTICAL;
            if (parseYesNoBoolAttribute(L"multiLine"))
                _nppGUI._tabStatus |= TAB_MULTILINE;
            if (parseYesNoBoolAttribute(L"hide"))
                _nppGUI._tabStatus |= TAB_HIDE;
            if (parseYesNoBoolAttribute(L"quitOnEmpty"))
                _nppGUI._tabStatus |= TAB_QUITONEMPTY;
            if (parseYesNoBoolAttribute(L"iconSetNumber"))
                _nppGUI._tabStatus |= TAB_ALTICONS;
        }
        else if (!lstrcmp(nm, L"RememberLastSession"))
        {
            TiXmlNode* textNode = childNode->FirstChild();
            if (textNode)
            {
                const wchar_t* val = textNode->Value();
                if (val)
                    _nppGUI._rememberLastSession = (!lstrcmp(val, L"yes"));
            }
        }
        else if (!lstrcmp(nm, L"NewDocDefaultSettings"))
        {
            int format = -1;
            element->Attribute(L"format", &format);
            if (format >= 0 && format <= 3)
                _nppGUI._newDocDefaultSettings._format = static_cast<EolType>(format);

            int encoding = -1;
            element->Attribute(L"encoding", &encoding);
            if (encoding >= 0)
                _nppGUI._newDocDefaultSettings._unicodeMode = static_cast<UniMode>(encoding);

            int lang = -1;
            element->Attribute(L"lang", &lang);
            if (lang >= 0)
                _nppGUI._newDocDefaultSettings._lang = static_cast<LangType>(lang);
        }
        else if (!lstrcmp(nm, L"langsExcluded"))
        {
            int excludedInt = 0;
            element->Attribute(L"gr0", &excludedInt);
            // Store as bitmask - simplified
        }
        else if (!lstrcmp(nm, L"MaitainIndent"))
        {
            TiXmlNode* textNode = childNode->FirstChild();
            if (textNode)
            {
                const wchar_t* val = textNode->Value();
                if (val)
                    _nppGUI._maintainIndent = (!lstrcmp(val, L"yes")) ? autoIndent_advanced : autoIndent_none;
            }
        }
        else if (!lstrcmp(nm, L"noUpdate"))
        {
            TiXmlNode* textNode = childNode->FirstChild();
            if (textNode)
            {
                const wchar_t* val = textNode->Value();
                if (val)
                    _nppGUI._autoUpdateOpt._doAutoUpdate = (lstrcmp(val, L"yes") != 0) ? NppGUI::autoupdate_on_startup : NppGUI::autoupdate_disabled;
            }
        }
    }
}

void NppParameters::feedFileListParameters(TiXmlNode* node)
{
    TiXmlNode* histNode = node->FirstChildElement(L"History");
    if (!histNode)
        return;

    int nbMaxFile = 10;
    const wchar_t* nbMaxStr = (histNode->ToElement())->Attribute(L"nbMaxFile", &nbMaxFile);
    if (nbMaxStr)
        _nbMaxRecentFile = nbMaxFile;

    for (TiXmlNode* childNode = histNode->FirstChildElement(L"File");
        childNode;
        childNode = childNode->NextSibling(L"File"))
    {
        const wchar_t* filePath = (childNode->ToElement())->Attribute(L"filename");
        if (filePath && _nbRecentFile < NB_MAX_LRF_FILE)
        {
            _LRFileList[_nbRecentFile] = std::make_unique<std::wstring>(filePath);
            ++_nbRecentFile;
        }
    }
}

void NppParameters::feedFindHistoryParameters(TiXmlNode* node)
{
    TiXmlNode* findHistoryRoot = node->FirstChildElement(L"FindHistory");
    if (!findHistoryRoot)
        return;

    TiXmlElement* element = findHistoryRoot->ToElement();
    if (element)
    {
        int nbMaxFindHistoryFind = NB_MAX_FINDHISTORY_FIND;
        element->Attribute(L"nbMaxFindHistoryFind", &nbMaxFindHistoryFind);
        _findHistory._nbMaxFindHistoryFind = nbMaxFindHistoryFind;

        int nbMaxFindHistoryReplace = NB_MAX_FINDHISTORY_REPLACE;
        element->Attribute(L"nbMaxFindHistoryReplace", &nbMaxFindHistoryReplace);
        _findHistory._nbMaxFindHistoryReplace = nbMaxFindHistoryReplace;

        int nbMaxFindHistoryPath = NB_MAX_FINDHISTORY_PATH;
        element->Attribute(L"nbMaxFindHistoryPath", &nbMaxFindHistoryPath);
        _findHistory._nbMaxFindHistoryPath = nbMaxFindHistoryPath;

        int nbMaxFindHistoryFilter = NB_MAX_FINDHISTORY_FILTER;
        element->Attribute(L"nbMaxFindHistoryFilter", &nbMaxFindHistoryFilter);
        _findHistory._nbMaxFindHistoryFilter = nbMaxFindHistoryFilter;
    }

    for (TiXmlNode* childNode = findHistoryRoot->FirstChildElement(L"Path");
        childNode;
        childNode = childNode->NextSibling(L"Path"))
    {
        const wchar_t* filePath = (childNode->ToElement())->Attribute(L"name");
        if (filePath)
            _findHistory._findHistoryPaths.push_back(std::wstring(filePath));
    }

    for (TiXmlNode* childNode = findHistoryRoot->FirstChildElement(L"Filter");
        childNode;
        childNode = childNode->NextSibling(L"Filter"))
    {
        const wchar_t* filter = (childNode->ToElement())->Attribute(L"name");
        if (filter)
            _findHistory._findHistoryFilters.push_back(std::wstring(filter));
    }

    for (TiXmlNode* childNode = findHistoryRoot->FirstChildElement(L"Find");
        childNode;
        childNode = childNode->NextSibling(L"Find"))
    {
        const wchar_t* find = (childNode->ToElement())->Attribute(L"name");
        if (find)
            _findHistory._findHistoryFinds.push_back(std::wstring(find));
    }

    for (TiXmlNode* childNode = findHistoryRoot->FirstChildElement(L"Replace");
        childNode;
        childNode = childNode->NextSibling(L"Replace"))
    {
        const wchar_t* replace = (childNode->ToElement())->Attribute(L"name");
        if (replace)
            _findHistory._findHistoryReplaces.push_back(std::wstring(replace));
    }
}

void NppParameters::feedProjectPanelsParameters(TiXmlNode* node)
{
    TiXmlNode* projPanelRoot = node->FirstChildElement(L"ProjectPanels");
    if (!projPanelRoot)
        return;

    for (int i = 0; i < 3; ++i)
    {
        wchar_t projPanelName[32];
        swprintf(projPanelName, 32, L"ProjectPanel%d", i + 1);
        TiXmlNode* projPanelNode = projPanelRoot->FirstChildElement(projPanelName);
        if (projPanelNode)
        {
            const wchar_t* filePath = (projPanelNode->ToElement())->Attribute(L"filePath");
            if (filePath)
                _workSpaceFilePaths[i] = filePath;
        }
    }
}

void NppParameters::feedFileBrowserParameters(TiXmlNode* node)
{
    TiXmlNode* fileBrowserRoot = node->FirstChildElement(L"FileBrowser");
    if (!fileBrowserRoot)
        return;

    for (TiXmlNode* childNode = fileBrowserRoot->FirstChildElement(L"root");
        childNode;
        childNode = childNode->NextSibling(L"root"))
    {
        const wchar_t* rootPath = (childNode->ToElement())->Attribute(L"foldername");
        if (rootPath)
            _fileBrowserRoot.push_back(std::wstring(rootPath));
    }
}

void NppParameters::feedColumnEditorParameters(TiXmlNode* node)
{
    TiXmlNode* colEdRoot = node->FirstChildElement(L"ColumnEditor");
    if (!colEdRoot)
        return;

    // Parse column editor settings if needed
    (void)colEdRoot;
}

bool NppParameters::getUserStylersFromXmlTree()
{
    if (!_pXmlUserStylerDoc)
        return false;

    TiXmlNode* root = _pXmlUserStylerDoc->FirstChild(L"NotepadPlus");
    if (!root)
        return false;

    return feedStylerArray(root);
}

bool NppParameters::feedStylerArray(TiXmlNode* node)
{
    TiXmlNode* lexerStylesRoot = node->FirstChildElement(L"LexerStyles");
    if (!lexerStylesRoot)
        return false;

    for (TiXmlNode* childNode = lexerStylesRoot->FirstChildElement(L"LexerType");
        childNode;
        childNode = childNode->NextSibling(L"LexerType"))
    {
        TiXmlElement* element = childNode->ToElement();
        const wchar_t* lexerName = element->Attribute(L"name");
        const wchar_t* lexerDesc = element->Attribute(L"desc");
        const wchar_t* lexerExcluded = element->Attribute(L"excluded");
        if (!lexerName)
            continue;

        _lexerStylerVect.addLexerStyler(lexerName, lexerDesc, lexerExcluded, childNode);
    }

    // Parse global styles
    TiXmlNode* globalStylesRoot = node->FirstChildElement(L"GlobalStyles");
    if (globalStylesRoot)
    {
        for (TiXmlNode* styleNode = globalStylesRoot->FirstChildElement(L"WidgetStyle");
            styleNode;
            styleNode = styleNode->NextSibling(L"WidgetStyle"))
        {
            TiXmlElement* styleElement = styleNode->ToElement();
            int styleID = -1;
            styleElement->Attribute(L"styleID", &styleID);
            _widgetStyleArray.addStyler(styleID, styleNode);
        }
    }

    return true;
}

std::pair<unsigned char, unsigned char> NppParameters::addUserDefineLangsFromXmlTree(TiXmlDocument* tixmldoc)
{
    if (!tixmldoc)
        return std::make_pair(static_cast<unsigned char>(0), static_cast<unsigned char>(0));

    TiXmlNode* root = tixmldoc->FirstChild(L"NotepadPlus");
    if (!root)
        return std::make_pair(static_cast<unsigned char>(0), static_cast<unsigned char>(0));

    return feedUserLang(root);
}

// ============================================================================
// Shortcuts XML Parsing
// ============================================================================

bool NppParameters::getShortcuts(const NppXml::Element& element, Shortcut& sc, std::string* folderName)
{
    if (!element)
        return false;

    const char* name = NppXml::attribute(element, "name", "");

    const bool isCtrl = getBoolAttribute(element, "Ctrl");
    const bool isAlt = getBoolAttribute(element, "Alt");
    const bool isShift = getBoolAttribute(element, "Shift");

    const int key = NppXml::intAttribute(element, "Key", -1);
    if (key == -1)
        return false;

    if (folderName)
        *folderName = NppXml::attribute(element, "FolderName", "");

    sc = Shortcut(name, isCtrl, isAlt, isShift, static_cast<unsigned char>(key));
    return true;
}

bool NppParameters::getInternalCommandShortcuts(const NppXml::Element& element, CommandShortcut& cs, std::string* folderName)
{
    if (!element)
        return false;

    Shortcut sc;
    if (!getShortcuts(element, sc, folderName))
        return false;

    // Update the base Shortcut portion of the CommandShortcut
    cs.setName(sc.getName());
    cs.setKeyCombo(sc.getKeyCombo());
    return true;
}

void NppParameters::getActions(const NppXml::Element& element, Macro& macro)
{
    for (NppXml::Element actionNode = NppXml::firstChildElement(element, "Action");
        actionNode;
        actionNode = NppXml::nextSiblingElement(actionNode, "Action"))
    {
        int type = NppXml::intAttribute(actionNode, "type", 0);
        int msg = NppXml::intAttribute(actionNode, "message", 0);
        int wParam = NppXml::intAttribute(actionNode, "wParam", 0);
        int lParam = NppXml::intAttribute(actionNode, "lParam", 0);
        const char* sParam = NppXml::attribute(actionNode, "sParam", "");

        macro.push_back(recordedMacroStep(msg, wParam, lParam, sParam, type));
    }
}

void NppParameters::feedShortcut(const NppXml::Element& element)
{
    NppXml::Element shortcutsRoot = NppXml::firstChildElement(element, "InternalCommands");
    if (!shortcutsRoot)
        return;

    for (NppXml::Element childNode = NppXml::firstChildElement(shortcutsRoot, "Shortcut");
        childNode;
        childNode = NppXml::nextSiblingElement(childNode, "Shortcut"))
    {
        const int id = NppXml::intAttribute(childNode, "id", -1);
        if (id > 0)
        {
            size_t len = _shortcuts.size();
            for (size_t i = 0; i < len; ++i)
            {
                if (_shortcuts[i].getID() == static_cast<unsigned long>(id))
                {
                    if (getInternalCommandShortcuts(childNode, _shortcuts[i]))
                        addUserModifiedIndex(i);
                    break;
                }
            }
        }
    }
}

void NppParameters::feedMacros(const NppXml::Element& element)
{
    NppXml::Element macrosRoot = NppXml::firstChildElement(element, "Macros");
    if (!macrosRoot)
        return;

    for (NppXml::Element childNode = NppXml::firstChildElement(macrosRoot, "Macro");
        childNode;
        childNode = NppXml::nextSiblingElement(childNode, "Macro"))
    {
        Shortcut sc;
        std::string fdnm;
        if (getShortcuts(childNode, sc, &fdnm))
        {
            Macro macro;
            getActions(childNode, macro);
            const auto cmdID = ID_MACRO + static_cast<int>(_macros.size());
            _macros.push_back(MacroShortcut(sc, macro, cmdID));
            _macroMenuItems.push_back(MenuItemUnit(cmdID, string2wstring(sc.getName(), CP_UTF8), string2wstring(fdnm, CP_UTF8)));
        }
    }
}

void NppParameters::feedUserCmds(const NppXml::Element& element)
{
    NppXml::Element userCmdsRoot = NppXml::firstChildElement(element, "UserDefinedCommands");
    if (!userCmdsRoot)
        return;

    for (NppXml::Element childNode = NppXml::firstChildElement(userCmdsRoot, "Command");
        childNode;
        childNode = NppXml::nextSiblingElement(childNode, "Command"))
    {
        Shortcut sc;
        std::string fdnm;
        if (getShortcuts(childNode, sc, &fdnm))
        {
            NppXml::Node aNode = NppXml::firstChild(childNode);
            if (aNode)
            {
                const char* cmdStr = NppXml::value(aNode);
                if (cmdStr)
                {
                    const auto cmdID = ID_USER_CMD + static_cast<int>(_userCommands.size());
                    _userCommands.push_back(UserCommand(sc, cmdStr, cmdID));
                    _runMenuItems.push_back(MenuItemUnit(cmdID, string2wstring(sc.getName(), CP_UTF8), string2wstring(fdnm, CP_UTF8)));
                }
            }
        }
    }
}

void NppParameters::feedPluginCustomizedCmds(const NppXml::Element& element)
{
    NppXml::Element pluginCmdsRoot = NppXml::firstChildElement(element, "PluginCommands");
    if (!pluginCmdsRoot)
        return;

    for (NppXml::Element childNode = NppXml::firstChildElement(pluginCmdsRoot, "PluginCommand");
        childNode;
        childNode = NppXml::nextSiblingElement(childNode, "PluginCommand"))
    {
        const char* moduleName = NppXml::attribute(childNode, "moduleName");
        if (!moduleName)
            continue;

        const int internalID = NppXml::intAttribute(childNode, "internalID", -1);
        if (internalID == -1)
            continue;

        size_t len = _pluginCommands.size();
        for (size_t i = 0; i < len; ++i)
        {
            PluginCmdShortcut& pscOrig = _pluginCommands[i];
            if (!strncasecmp(pscOrig.getModuleName(), moduleName, std::strlen(moduleName)) &&
                pscOrig.getInternalID() == internalID)
            {
                getShortcuts(childNode, _pluginCommands[i]);
                addPluginModifiedIndex(i);
                break;
            }
        }
    }
}

void NppParameters::feedScintKeys(const NppXml::Element& element)
{
    NppXml::Element scintKeysRoot = NppXml::firstChildElement(element, "ScintillaKeys");
    if (!scintKeysRoot)
        return;

    for (NppXml::Element childNode = NppXml::firstChildElement(scintKeysRoot, "ScintKey");
        childNode;
        childNode = NppXml::nextSiblingElement(childNode, "ScintKey"))
    {
        const int scintKey = NppXml::intAttribute(childNode, "ScintID", -1);
        if (scintKey == -1)
            continue;

        const int menuID = NppXml::intAttribute(childNode, "menuCmdID", -1);
        if (menuID == -1)
            continue;

        size_t len = _scintillaKeyCommands.size();
        for (int i = 0; i < static_cast<int>(len); ++i)
        {
            ScintillaKeyMap& skmOrig = _scintillaKeyCommands[i];
            if (skmOrig.getScintillaKeyID() == static_cast<unsigned long>(scintKey) &&
                skmOrig.getMenuCmdID() == menuID)
            {
                _scintillaKeyCommands[i].clearDups();
                getShortcuts(childNode, _scintillaKeyCommands[i]);
                _scintillaKeyCommands[i].setKeyComboByIndex(0, _scintillaKeyCommands[i].getKeyCombo());
                addScintillaModifiedIndex(i);

                KeyCombo kc;
                for (NppXml::Element nextNode = NppXml::firstChildElement(childNode, "NextKey");
                    nextNode;
                    nextNode = NppXml::nextSiblingElement(nextNode, "NextKey"))
                {
                    const char* str = NppXml::attribute(nextNode, "Ctrl");
                    if (!str) continue;
                    kc._isCtrl = (strcmp("yes", str) == 0);

                    str = NppXml::attribute(nextNode, "Alt");
                    if (!str) continue;
                    kc._isAlt = (strcmp("yes", str) == 0);

                    str = NppXml::attribute(nextNode, "Shift");
                    if (!str) continue;
                    kc._isShift = (strcmp("yes", str) == 0);

                    const int nextKey = NppXml::intAttribute(nextNode, "Key", -1);
                    if (nextKey >= 0)
                    {
                        kc._key = static_cast<unsigned char>(nextKey);
                        _scintillaKeyCommands[i].addKeyCombo(kc);
                    }
                }
                break;
            }
        }
    }
}

bool NppParameters::getShortcutsFromXmlTree()
{
    if (!_pXmlShortcutDoc)
        return false;

    NppXml::Element root = NppXml::firstChildElement(_pXmlShortcutDoc, "NotepadPlus");
    if (!root)
        return false;

    feedShortcut(root);
    return true;
}

bool NppParameters::getMacrosFromXmlTree()
{
    if (!_pXmlShortcutDoc)
        return false;

    NppXml::Element root = NppXml::firstChildElement(_pXmlShortcutDoc, "NotepadPlus");
    if (!root)
        return false;

    feedMacros(root);
    return true;
}

bool NppParameters::getUserCmdsFromXmlTree()
{
    if (!_pXmlShortcutDoc)
        return false;

    NppXml::Element root = NppXml::firstChildElement(_pXmlShortcutDoc, "NotepadPlus");
    if (!root)
        return false;

    feedUserCmds(root);
    return true;
}

bool NppParameters::getPluginCmdsFromXmlTree()
{
    if (!_pXmlShortcutDoc)
        return false;

    NppXml::Element root = NppXml::firstChildElement(_pXmlShortcutDoc, "NotepadPlus");
    if (!root)
        return false;

    feedPluginCustomizedCmds(root);
    return true;
}

bool NppParameters::getScintKeysFromXmlTree()
{
    if (!_pXmlShortcutDoc)
        return false;

    NppXml::Element root = NppXml::firstChildElement(_pXmlShortcutDoc, "NotepadPlus");
    if (!root)
        return false;

    feedScintKeys(root);
    return true;
}

void NppParameters::initMenuKeys()
{
    int previousFuncID = 0;
    for (const auto& keyDef : winKeyDefs)
    {
        if (keyDef.vKey == 0 && keyDef.functionId == 0)
            break;

        Shortcut sc((keyDef.specialName ? wstring2string(keyDef.specialName, CP_UTF8).c_str() : ""),
            keyDef.isCtrl, keyDef.isAlt, keyDef.isShift, static_cast<unsigned char>(keyDef.vKey));
        _shortcuts.push_back(CommandShortcut(sc, keyDef.functionId, previousFuncID == keyDef.functionId));
        previousFuncID = keyDef.functionId;
    }
}

void NppParameters::initScintillaKeys()
{
    int prevIndex = -1;
    int prevID = -1;
    for (const auto& keyDef : scintKeyDefs)
    {
        if (keyDef.name == nullptr)
            break;

        if (keyDef.functionId == prevID)
        {
            KeyCombo kc;
            kc._isCtrl = keyDef.isCtrl;
            kc._isAlt = keyDef.isAlt;
            kc._isShift = keyDef.isShift;
            kc._key = static_cast<unsigned char>(keyDef.vKey);
            _scintillaKeyCommands[prevIndex].addKeyCombo(kc);
        }
        else
        {
            Shortcut s(wstring2string(keyDef.name, CP_UTF8).c_str(), keyDef.isCtrl, keyDef.isAlt, keyDef.isShift, static_cast<unsigned char>(keyDef.vKey));
            ScintillaKeyMap sm(s, keyDef.functionId, keyDef.redirFunctionId);
            _scintillaKeyCommands.push_back(sm);
            ++prevIndex;
        }
        prevID = keyDef.functionId;
    }
}

// Getter/setter implementations

LangType NppParameters::getLangFromExt(const wchar_t* ext)
{
    if (!ext || !ext[0])
        return L_TEXT;

    // Check user defined languages first
    // Then check built-in languages
    for (int i = 0; i < _nbLang; ++i)
    {
        if (_langList[i])
        {
            const wchar_t* defList = _langList[i]->getDefaultExtList();
            if (defList && defList[0])
            {
                // Simple extension matching
                std::wstring extList(defList);
                std::wstring searchExt(ext);
                // Add dot if not present
                if (searchExt[0] != L'.')
                    searchExt = L"." + searchExt;

                // Check if extension is in list
                size_t pos = extList.find(searchExt);
                if (pos != std::wstring::npos)
                    return _langList[i]->getLangID();
            }
        }
    }

    return L_TEXT;
}

void NppParameters::setWorkingDir(const wchar_t* newPath)
{
    if (newPath && newPath[0])
    {
        _currentDirectory = newPath;
    }
}

void NppParameters::setWorkSpaceFilePath(int i, const wchar_t* wsFile)
{
    if (i >= 0 && i < 3 && wsFile)
    {
        _workSpaceFilePaths[i] = wsFile;
    }
}

int NppParameters::getExternalLangIndexFromName(const wchar_t* externalLangName) const
{
    if (!externalLangName || !externalLangName[0])
        return -1;

    // Convert wchar_t* to std::string for comparison
    std::wstring wname(externalLangName);
    std::string name(wname.begin(), wname.end());

    for (int i = 0; i < _nbExternalLang; ++i)
    {
        if (_externalLangArray[i] && _externalLangArray[i]->_name == name)
            return i;
    }
    return -1;
}

const UserLangContainer* NppParameters::getULCFromName(const wchar_t* userLangName) const
{
    if (!userLangName || !userLangName[0])
        return nullptr;

    for (int i = 0; i < _nbUserLang; ++i)
    {
        if (_userLangArray[i] && lstrcmp(_userLangArray[i]->getName(), userLangName) == 0)
            return _userLangArray[i].get();
    }
    return nullptr;
}

const wchar_t* NppParameters::getUserDefinedLangNameFromExt(wchar_t* ext, wchar_t* fullName) const
{
    // Check user defined languages for matching extension
    for (int i = 0; i < _nbUserLang; ++i)
    {
        if (_userLangArray[i])
        {
            const wchar_t* udlExt = _userLangArray[i]->getExtention();
            if (udlExt && udlExt[0])
            {
                // Check if extension matches
                if (ext && lstrcmp(ext, udlExt) == 0)
                    return _userLangArray[i]->getName();
            }
        }
    }
    return nullptr;
}

int NppParameters::addUserLangToEnd(const UserLangContainer* userLang, const wchar_t* newName)
{
    if (_nbUserLang >= NB_MAX_USER_LANG || !userLang || !newName)
        return -1;

    _userLangArray[_nbUserLang] = std::make_unique<UserLangContainer>(*userLang);
    // Would set name here in full implementation
    return _nbUserLang++;
}

void NppParameters::removeUserLang(size_t index)
{
    if (index >= static_cast<size_t>(_nbUserLang))
        return;

    _userLangArray[index].reset();
    // Shift remaining elements
    for (int i = index; i < _nbUserLang - 1; ++i)
    {
        _userLangArray[i] = std::move(_userLangArray[i + 1]);
    }
    _nbUserLang--;
}

bool NppParameters::isExistingExternalLangName(const char* newName) const
{
    if (!newName || !newName[0])
        return false;

    std::string name(newName);
    for (int i = 0; i < _nbExternalLang; ++i)
    {
        if (_externalLangArray[i] && _externalLangArray[i]->_name == name)
            return true;
    }
    return false;
}

int NppParameters::addExternalLangToEnd(std::unique_ptr<ExternalLangContainer> externalLang)
{
    if (_nbExternalLang >= NB_MAX_EXTERNAL_LANG || !externalLang)
        return -1;

    _externalLangArray[_nbExternalLang] = std::move(externalLang);
    return _nbExternalLang++;
}

void NppParameters::getExternalLexerFromXmlTree(TiXmlDocument* externalLexerDoc)
{
    // Stub - would parse external lexer XML
    (void)externalLexerDoc;
}

// Write methods - stubs for saving settings

bool NppParameters::writeRecentFileHistorySettings(int nbMaxFile) const
{
    (void)nbMaxFile;
    return true;
}

bool NppParameters::writeHistory(const wchar_t* fullpath)
{
    (void)fullpath;
    return true;
}

bool NppParameters::writeProjectPanelsSettings() const
{
    return true;
}

bool NppParameters::writeColumnEditorSettings() const
{
    return true;
}

bool NppParameters::writeFileBrowserSettings(const std::vector<std::wstring>& rootPath,
                                              const std::wstring& latestSelectedItemPath) const
{
    (void)rootPath;
    (void)latestSelectedItemPath;
    return true;
}

bool NppParameters::writeScintillaParams()
{
    return true;
}

void NppParameters::createXmlTreeFromGUIParams()
{
    // Stub - would create XML tree from GUI parameters
}

std::wstring NppParameters::writeStyles(LexerStylerArray& lexersStylers, StyleArray& globalStylers)
{
    (void)lexersStylers;
    (void)globalStylers;
    return L"";
}

bool NppParameters::insertTabInfo(const wchar_t* langName, int tabInfo, bool backspaceUnindent)
{
    (void)langName;
    (void)tabInfo;
    (void)backspaceUnindent;
    return true;
}

void NppParameters::writeDefaultUDL()
{
}

void NppParameters::writeNonDefaultUDL()
{
}

void NppParameters::writeNeed2SaveUDL()
{
}

void NppParameters::writeShortcuts()
{
}

void NppParameters::writeSession(const Session& session, const wchar_t* fileName)
{
    const wchar_t* sessionPathName = fileName ? fileName : _sessionPath.c_str();

    // Make sure session file is not read-only
    removeReadOnlyFlagFromFileAttributes(sessionPathName);

    // Backup session file before overwriting it
    bool doesBackupCopyExist = false;
    if (doesFileExist(sessionPathName))
    {
        std::wstring backupPathName = sessionPathName;
        backupPathName += SESSION_BACKUP_EXT;

        // Make sure backup file is not read-only, if it exists
        removeReadOnlyFlagFromFileAttributes(backupPathName.c_str());

        // Copy current session file to backup
        doesBackupCopyExist = QFile::copy(wstringToQString(sessionPathName), wstringToQString(backupPathName));
    }

    // Prepare for writing
    NppXml::Document pXmlSessionDoc = new NppXml::NewDocument();
    NppXml::createNewDeclaration(pXmlSessionDoc);
    NppXml::Element root = NppXml::createChildElement(pXmlSessionDoc, "NotepadPlus");

    if (root)
    {
        NppXml::Element sessionNode = NppXml::createChildElement(root, "Session");
        NppXml::setUInt64Attribute(sessionNode, "activeView", session._activeView);

        struct ViewElem {
            NppXml::Element viewNode;
            const std::vector<sessionFileInfo>* viewFiles;
            size_t activeIndex;
        };

        static constexpr int nbElem = 2;
        ViewElem viewElems[nbElem]{
            ViewElem{.viewNode = NppXml::createChildElement(sessionNode, "mainView"), .viewFiles = &session._mainViewFiles, .activeIndex = session._activeMainIndex},
            ViewElem{.viewNode = NppXml::createChildElement(sessionNode, "subView"), .viewFiles = &session._subViewFiles, .activeIndex = session._activeSubIndex}
        };

        for (size_t k = 0; k < nbElem; ++k)
        {
            NppXml::setUInt64Attribute(viewElems[k].viewNode, "activeIndex", viewElems[k].activeIndex);
            const std::vector<sessionFileInfo>& viewSessionFiles = *(viewElems[k].viewFiles);

            for (const auto& vsFile : viewSessionFiles)
            {
                NppXml::Element fileNameNode = NppXml::createChildElement(viewElems[k].viewNode, "File");

                NppXml::setInt64Attribute(fileNameNode, "firstVisibleLine", vsFile._firstVisibleLine);
                NppXml::setInt64Attribute(fileNameNode, "xOffset", vsFile._xOffset);
                NppXml::setInt64Attribute(fileNameNode, "scrollWidth", vsFile._scrollWidth);
                NppXml::setInt64Attribute(fileNameNode, "startPos", vsFile._startPos);
                NppXml::setInt64Attribute(fileNameNode, "endPos", vsFile._endPos);
                NppXml::setInt64Attribute(fileNameNode, "selMode", vsFile._selMode);
                NppXml::setInt64Attribute(fileNameNode, "offset", vsFile._offset);
                NppXml::setInt64Attribute(fileNameNode, "wrapCount", vsFile._wrapCount);
                NppXml::setAttribute(fileNameNode, "lang", wstring2string(vsFile._langName, CP_UTF8).c_str());
                NppXml::setAttribute(fileNameNode, "encoding", vsFile._encoding);
                setBoolAttribute(fileNameNode, "userReadOnly", (vsFile._isUserReadOnly && !vsFile._isMonitoring));
                NppXml::setAttribute(fileNameNode, "filename", wstring2string(vsFile._fileName, CP_UTF8).c_str());
                NppXml::setAttribute(fileNameNode, "backupFilePath", wstring2string(vsFile._backupFilePath, CP_UTF8).c_str());
                NppXml::setAttribute(fileNameNode, "originalFileLastModifTimestamp", vsFile._originalFileLastModifTimestamp.dwLowDateTime);
                NppXml::setAttribute(fileNameNode, "originalFileLastModifTimestampHigh", vsFile._originalFileLastModifTimestamp.dwHighDateTime);
                NppXml::setAttribute(fileNameNode, "tabColourId", vsFile._individualTabColour);
                setBoolAttribute(fileNameNode, "RTL", vsFile._isRTL);
                setBoolAttribute(fileNameNode, "tabPinned", vsFile._isPinned);

                // Save this info only when it's an untitled entry
                if (vsFile._isUntitledTabRenamed)
                    NppXml::setAttribute(fileNameNode, "untitleTabRenamed", "yes");

                // docMap
                NppXml::setInt64Attribute(fileNameNode, "mapFirstVisibleDisplayLine", vsFile._mapPos._firstVisibleDisplayLine);
                NppXml::setInt64Attribute(fileNameNode, "mapFirstVisibleDocLine", vsFile._mapPos._firstVisibleDocLine);
                NppXml::setInt64Attribute(fileNameNode, "mapLastVisibleDocLine", vsFile._mapPos._lastVisibleDocLine);
                NppXml::setInt64Attribute(fileNameNode, "mapNbLine", vsFile._mapPos._nbLine);
                NppXml::setInt64Attribute(fileNameNode, "mapHigherPos", vsFile._mapPos._higherPos);
                NppXml::setInt64Attribute(fileNameNode, "mapWidth", vsFile._mapPos._width);
                NppXml::setInt64Attribute(fileNameNode, "mapHeight", vsFile._mapPos._height);
                NppXml::setInt64Attribute(fileNameNode, "mapKByteInDoc", vsFile._mapPos._KByteInDoc);
                NppXml::setInt64Attribute(fileNameNode, "mapWrapIndentMode", vsFile._mapPos._wrapIndentMode);
                setBoolAttribute(fileNameNode, "mapIsWrap", vsFile._mapPos._isWrap);

                for (const auto& markLine : vsFile._marks)
                {
                    NppXml::Element markNode = NppXml::createChildElement(fileNameNode, "Mark");
                    NppXml::setUInt64Attribute(markNode, "line", markLine);
                }

                for (const auto& foldLine : vsFile._foldStates)
                {
                    NppXml::Element foldNode = NppXml::createChildElement(fileNameNode, "Fold");
                    NppXml::setUInt64Attribute(foldNode, "line", foldLine);
                }
            }
        }

        if (session._includeFileBrowser)
        {
            // Node structure and naming corresponds to config.xml
            NppXml::Element fileBrowserRootNode = NppXml::createChildElement(sessionNode, "FileBrowser");
            NppXml::setAttribute(fileBrowserRootNode, "latestSelectedItem", wstring2string(session._fileBrowserSelectedItem, CP_UTF8).c_str());
            for (const auto& fbRoot : session._fileBrowserRoots)
            {
                NppXml::Element fileNameNode = NppXml::createChildElement(fileBrowserRootNode, "root");
                NppXml::setAttribute(fileNameNode, "foldername", wstring2string(fbRoot, CP_UTF8).c_str());
            }
        }
    }

    //
    // Write the session file
    //
    bool sessionSaveOK = NppXml::saveFile(pXmlSessionDoc, sessionPathName);

    //
    // Double checking: prevent written session file corrupted while writing
    //
    if (sessionSaveOK)
    {
        NppXml::Document pXmlSessionCheck = new NppXml::NewDocument();
        sessionSaveOK = NppXml::loadFile(pXmlSessionCheck, sessionPathName);
        if (sessionSaveOK)
        {
            Session sessionCheck;
            sessionSaveOK = getSessionFromXmlTree(pXmlSessionCheck, sessionCheck);
        }
        delete pXmlSessionCheck;
    }

    delete pXmlSessionDoc;
}

bool NppParameters::writeFindHistory()
{
    return true;
}

// Session loading
bool NppParameters::loadSession(Session& session, const wchar_t* sessionFileName, const bool bSuppressErrorMsg)
{
    (void)session;
    (void)sessionFileName;
    (void)bSuppressErrorMsg;
    return false;
}

// Context menu
bool NppParameters::getContextMenuFromXmlTree(HMENU mainMenuHandle, HMENU pluginsMenu, bool isEditCM)
{
    (void)mainMenuHandle;
    (void)pluginsMenu;
    (void)isEditCM;
    return true;
}

bool NppParameters::reloadContextMenuFromXmlTree(HMENU mainMenuHandle, HMENU pluginsMenu)
{
    (void)mainMenuHandle;
    (void)pluginsMenu;
    return true;
}

// Transparency - no-op on Linux (handled by Qt/window manager)
void NppParameters::SetTransparent(HWND hwnd, int percent)
{
    (void)hwnd;
    (void)percent;
}

void NppParameters::removeTransparent(HWND hwnd)
{
    (void)hwnd;
}

// Version strings
std::wstring NppParameters::getWinVersionStr() const
{
    return L"Linux";
}

std::wstring NppParameters::getWinVerBitStr() const
{
    return _isx64 ? L"64-bit" : L"32-bit";
}

// Cloud settings
bool NppParameters::writeSettingsFilesOnCloudForThe1stTime(const std::wstring& cloudSettingsPath)
{
    (void)cloudSettingsPath;
    return true;
}

void NppParameters::setCloudChoice(const wchar_t* pathChoice)
{
    (void)pathChoice;
}

void NppParameters::removeCloudChoice()
{
}

bool NppParameters::isCloudPathChanged() const
{
    return false;
}

// UDL management
void NppParameters::setUdlXmlDirtyFromIndex(size_t i)
{
    (void)i;
}

void NppParameters::setUdlXmlDirtyFromXmlDoc(const TiXmlDocument* xmlDoc)
{
    (void)xmlDoc;
}

void NppParameters::removeIndexFromXmlUdls(size_t i)
{
    (void)i;
}

// Tab colors
void NppParameters::initTabCustomColors()
{
}

void NppParameters::setIndividualTabColor(COLORREF colour2Set, int colourIndex, bool isDarkMode)
{
    (void)colour2Set;
    (void)colourIndex;
    (void)isDarkMode;
}

COLORREF NppParameters::getIndividualTabColor(int colourIndex, bool isDarkMode, bool saturated)
{
    (void)colourIndex;
    (void)isDarkMode;
    (void)saturated;
    return RGB(0, 0, 0);
}

void NppParameters::initFindDlgStatusMsgCustomColors()
{
}

void NppParameters::setFindDlgStatusMsgIndexColor(COLORREF colour2Set, int colourIndex)
{
    (void)colour2Set;
    (void)colourIndex;
}

COLORREF NppParameters::getFindDlgStatusMsgColor(int colourIndex)
{
    (void)colourIndex;
    return RGB(0, 0, 0);
}

// Font list
void NppParameters::setFontList(HWND hWnd)
{
    (void)hWnd;
    // Would enumerate system fonts and populate _fontlist
}

bool NppParameters::isInFontList(const std::wstring& fontName2Search) const
{
    for (const auto& font : _fontlist)
    {
        if (font == fontName2Search)
            return true;
    }
    return false;
}

COLORREF NppParameters::getCurLineHilitingColour()
{
    return _currentDefaultBgColor;
}

void NppParameters::setCurLineHilitingColour(COLORREF colour2Set)
{
    _currentDefaultBgColor = colour2Set;
}

// Import/Export UDL
bool NppParameters::importUDLFromFile(const std::wstring& sourceFile)
{
    (void)sourceFile;
    return false;
}

bool NppParameters::exportUDLToFile(size_t langIndex2export, const std::wstring& fileName2save)
{
    (void)langIndex2export;
    (void)fileName2save;
    return false;
}

// Command ID mapping
int NppParameters::langTypeToCommandID(LangType lt) const
{
    // Map language type to menu command ID
    if (lt >= L_TEXT && lt < L_EXTERNAL)
    {
        return IDM_LANG_C + static_cast<int>(lt);
    }
    return 0;
}

// XML helper
TiXmlNode* NppParameters::getChildElementByAttribute(TiXmlNode* pere, const wchar_t* childName,
                                                      const wchar_t* attributeName, const wchar_t* attributeVal) const
{
    if (!pere)
        return nullptr;

    for (TiXmlNode* child = pere->FirstChild(childName); child; child = child->NextSibling(childName))
    {
        TiXmlElement* element = child->ToElement();
        if (element)
        {
            const wchar_t* attrVal = element->Attribute(attributeName);
            if (attrVal && lstrcmp(attrVal, attributeVal) == 0)
                return child;
        }
    }
    return nullptr;
}

// Static session loading
bool NppParameters::getSessionFromXmlTree(const NppXml::Document& pSessionDoc, Session& session)
{
    if (!pSessionDoc)
        return false;

    NppXml::Element root = NppXml::firstChildElement(pSessionDoc, "NotepadPlus");
    if (!root)
        return false;

    NppXml::Element sessionRoot = NppXml::firstChildElement(root, "Session");
    if (!sessionRoot)
        return false;

    const int index = NppXml::intAttribute(sessionRoot, "activeView", -1);
    if (index >= 0)
    {
        session._activeView = index;
    }

    static constexpr size_t nbView = 2;
    NppXml::Element viewRoots[nbView]{
        NppXml::firstChildElement(sessionRoot, "mainView"),
        NppXml::firstChildElement(sessionRoot, "subView")
    };

    for (size_t k = 0; k < nbView; ++k)
    {
        if (viewRoots[k])
        {
            const int index2 = NppXml::intAttribute(viewRoots[k], "activeIndex", -1);
            if (index2 >= 0)
            {
                if (k == 0)
                    session._activeMainIndex = index2;
                else // k == 1
                    session._activeSubIndex = index2;
            }
            for (NppXml::Element childNode = NppXml::firstChildElement(viewRoots[k], "File");
                childNode;
                childNode = NppXml::nextSiblingElement(childNode, "File"))
            {
                const char* fileName = NppXml::attribute(childNode, "filename");
                if (fileName)
                {
                    Position position{
                        ._firstVisibleLine = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "firstVisibleLine", 0)),
                        ._startPos = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "startPos", 0)),
                        ._endPos = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "endPos", 0)),
                        ._xOffset = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "xOffset", 0)),
                        ._selMode = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "selMode", 0)),
                        ._scrollWidth = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "scrollWidth", 1)),
                        ._offset = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "offset", 0)),
                        ._wrapCount = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "wrapCount", 0))
                    };

                    MapPosition mapPosition{
                        ._firstVisibleDisplayLine = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapFirstVisibleDisplayLine", -1)),
                        ._firstVisibleDocLine = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapFirstVisibleDocLine", -1)),
                        ._lastVisibleDocLine = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapLastVisibleDocLine", -1)),
                        ._nbLine = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapNbLine", -1)),
                        ._higherPos = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapHigherPos", -1)),
                        ._width = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapWidth", -1)),
                        ._height = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapHeight", -1)),
                        ._wrapIndentMode = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapWrapIndentMode", -1)),
                        ._KByteInDoc = static_cast<intptr_t>(NppXml::int64Attribute(childNode, "mapKByteInDoc", MapPosition::getMaxPeekLenInKB())),
                        ._isWrap = getBoolAttribute(childNode, "mapIsWrap")
                    };

                    const char* langName = NppXml::attribute(childNode, "lang");

                    std::wstring wstrFileName = string2wstring(fileName, CP_UTF8);
                    std::wstring wstrLangName = langName ? string2wstring(langName, CP_UTF8) : L"";

                    const char* backupFilePathAttr = NppXml::attribute(childNode, "backupFilePath");
                    std::wstring backupFilePath;
                    if (backupFilePathAttr)
                    {
                        backupFilePath = string2wstring(backupFilePathAttr, CP_UTF8);
                        // Validate backup file path - ensure it's in the user's backup directory
                        std::wstring currentBackupFilePath = NppParameters::getInstance().getUserPath();
                        pathAppend(currentBackupFilePath, L"backup");
                        if (backupFilePath.find(currentBackupFilePath) != 0)
                        {
                            // Reconstruct backup file path to use correct location
                            size_t lastSlash = backupFilePath.find_last_of(L"/\\");
                            if (lastSlash != std::wstring::npos)
                            {
                                std::wstring fileNameOnly = backupFilePath.substr(lastSlash + 1);
                                backupFilePath = currentBackupFilePath;
                                pathAppend(backupFilePath, fileNameOnly);
                            }
                        }
                    }

                    FILETIME fileModifiedTimestamp{
                        .dwLowDateTime = static_cast<DWORD>(NppXml::uint64Attribute(childNode, "originalFileLastModifTimestamp", 0)),
                        .dwHighDateTime = static_cast<DWORD>(NppXml::uint64Attribute(childNode, "originalFileLastModifTimestampHigh", 0))
                    };

                    const int encoding = NppXml::intAttribute(childNode, "encoding", -1);

                    const bool isUserReadOnly = getBoolAttribute(childNode, "userReadOnly");
                    const bool isPinned = getBoolAttribute(childNode, "tabPinned");
                    const bool isUntitleTabRenamed = getBoolAttribute(childNode, "untitleTabRenamed");

                    sessionFileInfo sfi(wstrFileName.c_str(), wstrLangName.c_str(), encoding,
                        isUserReadOnly, isPinned, isUntitleTabRenamed,
                        position, backupFilePath.empty() ? nullptr : backupFilePath.c_str(),
                        fileModifiedTimestamp, mapPosition);

                    sfi._individualTabColour = NppXml::intAttribute(childNode, "tabColourId", -1);
                    sfi._isRTL = getBoolAttribute(childNode, "RTL");

                    for (NppXml::Element markNode = NppXml::firstChildElement(childNode, "Mark");
                        markNode;
                        markNode = NppXml::nextSiblingElement(markNode, "Mark"))
                    {
                        const auto lineNumber = static_cast<intptr_t>(NppXml::int64Attribute(markNode, "line", -1));
                        if (lineNumber > -1)
                        {
                            sfi._marks.push_back(static_cast<size_t>(lineNumber));
                        }
                    }

                    for (NppXml::Element foldNode = NppXml::firstChildElement(childNode, "Fold");
                        foldNode;
                        foldNode = NppXml::nextSiblingElement(foldNode, "Fold"))
                    {
                        const auto lineNumber = static_cast<intptr_t>(NppXml::int64Attribute(foldNode, "line", -1));
                        if (lineNumber > -1)
                        {
                            sfi._foldStates.push_back(static_cast<size_t>(lineNumber));
                        }
                    }
                    if (k == 0)
                        session._mainViewFiles.push_back(sfi);
                    else // k == 1
                        session._subViewFiles.push_back(sfi);
                }
            }
        }
    }

    // Node structure and naming corresponds to config.xml
    NppXml::Element fileBrowserRoot = NppXml::firstChildElement(sessionRoot, "FileBrowser");
    if (fileBrowserRoot)
    {
        const char* selectedItemPath = NppXml::attribute(fileBrowserRoot, "latestSelectedItem");
        if (selectedItemPath)
        {
            session._fileBrowserSelectedItem = string2wstring(selectedItemPath, CP_UTF8);
        }

        for (NppXml::Element childNode = NppXml::firstChildElement(fileBrowserRoot, "root");
            childNode;
            childNode = NppXml::nextSiblingElement(childNode, "root"))
        {
            const char* fileName = NppXml::attribute(childNode, "foldername");
            if (fileName)
            {
                session._fileBrowserRoots.push_back(string2wstring(fileName, CP_UTF8));
            }
        }
    }

    return true;
}

// Shortcut modification tracking
void NppParameters::addUserModifiedIndex(size_t index)
{
    (void)index;
}

void NppParameters::addPluginModifiedIndex(size_t index)
{
    (void)index;
}

void NppParameters::addScintillaModifiedIndex(int index)
{
    (void)index;
}

// Save config
void NppParameters::saveConfig_xml()
{
    // Would save all settings to config.xml
}

// GUP params (updater)
void NppParameters::buildGupParams(std::wstring& params) const
{
    (void)params;
}

// Language name info
LanguageNameInfo NppParameters::getLangNameInfoFromNameID(const std::wstring& langNameID)
{
    LanguageNameInfo info;
    (void)langNameID;
    return info;
}

// Date implementation
Date::Date(const wchar_t* dateStr)
{
    // Parse date string in format YYYYMMDD
    if (dateStr && wcslen(dateStr) >= 8)
    {
        _year = std::stoul(std::wstring(dateStr, 4));
        _month = std::stoul(std::wstring(dateStr + 4, 2));
        _day = std::stoul(std::wstring(dateStr + 6, 2));
    }
}

Date::Date(int nbDaysFromNow)
{
    time_t now = time(nullptr);
    now += nbDaysFromNow * 24 * 60 * 60;
    tm* localTime = localtime(&now);
    if (localTime)
    {
        _year = localTime->tm_year + 1900;
        _month = localTime->tm_mon + 1;
        _day = localTime->tm_mday;
    }
}

void Date::now()
{
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    if (localTime)
    {
        _year = localTime->tm_year + 1900;
        _month = localTime->tm_mon + 1;
        _day = localTime->tm_mday;
    }
}

// EolType conversion
EolType convertIntToFormatType(int value, EolType defvalue)
{
    switch (value)
    {
        case 0: return EolType::windows;
        case 1: return EolType::macos;
        case 2: return EolType::unix;
        case 3: return EolType::osdefault;
        default: return defvalue;
    }
}

// ============================================================================
// Style/Lexer XML parsing helpers (Linux implementations)
// ============================================================================

void LexerStylerArray::addLexerStyler(const wchar_t* lexerName, const wchar_t* lexerDesc, const wchar_t* lexerUserExt, TiXmlNode* lexerNode)
{
    _lexerStylerVect.emplace_back();
    LexerStyler& ls = _lexerStylerVect.back();
    ls.setLexerName(lexerName);
    if (lexerDesc)
        ls.setLexerDesc(lexerDesc);
    if (lexerUserExt)
        ls.setLexerUserExt(lexerUserExt);

    for (TiXmlNode* childNode = lexerNode->FirstChildElement(L"WordsStyle");
        childNode;
        childNode = childNode->NextSibling(L"WordsStyle"))
    {
        TiXmlElement* element = childNode->ToElement();
        const wchar_t* styleIDStr = element->Attribute(L"styleID");
        if (styleIDStr)
        {
            int styleID = decStrVal(styleIDStr);
            if (styleID != -1)
                ls.addStyler(styleID, childNode);
        }
    }
}

void StyleArray::addStyler(int styleID, TiXmlNode* styleNode)
{
    _styleVect.emplace_back();
    Style& s = _styleVect.back();
    s._styleID = styleID;

    if (styleNode)
    {
        TiXmlElement* element = styleNode->ToElement();

        const wchar_t* str = element->Attribute(L"name");
        if (str)
            s._styleDesc = str;

        str = element->Attribute(L"fgColor");
        if (str)
        {
            int result = hexStrVal(str);
            s._fgColor = static_cast<COLORREF>(result & 0x00FFFFFF);
        }

        str = element->Attribute(L"bgColor");
        if (str)
        {
            int result = hexStrVal(str);
            s._bgColor = static_cast<COLORREF>(result & 0x00FFFFFF);
        }

        str = element->Attribute(L"colorStyle");
        if (str)
            s._colorStyle = decStrVal(str);

        str = element->Attribute(L"fontName");
        if (str)
        {
            s._fontName = str;
            s._isFontEnabled = true;
        }

        str = element->Attribute(L"fontStyle");
        if (str)
            s._fontStyle = decStrVal(str);

        str = element->Attribute(L"fontSize");
        if (str)
            s._fontSize = decStrVal(str);

        TiXmlNode* v = styleNode->FirstChild();
        if (v)
            s._keywords = v->Value();
    }
}

std::pair<unsigned char, unsigned char> NppParameters::feedUserLang(TiXmlNode* node)
{
    // User-defined languages parsing - stub for now
    // Returns (0, 0) indicating no user languages were loaded
    (void)node;
    return std::make_pair(static_cast<unsigned char>(0), static_cast<unsigned char>(0));
}

#endif // !_WIN32
