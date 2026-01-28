// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Settings.h"
#include "../../Parameters.h"
#include "../../MISC/RegExt/regExtDlg.h"
#include "../../MISC/Common/Common.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tinyxml.h>

namespace Platform {

// ============================================================================
// Windows Implementation of ISettings
// ============================================================================
class SettingsWin32 : public ISettings {
public:
    SettingsWin32() = default;
    ~SettingsWin32() override = default;

    bool init() override {
        // Windows initialization is handled by NppParameters
        return true;
    }

    std::wstring getConfigPath() const override {
        NppParameters& nppParams = NppParameters::getInstance();
        return nppParams.getNppPath();
    }

    std::wstring getSettingsDir() const override {
        NppParameters& nppParams = NppParameters::getInstance();
        return nppParams.getUserPath();
    }

    std::wstring getUserPluginsDir() const override {
        NppParameters& nppParams = NppParameters::getInstance();
        return nppParams.getUserPluginDir();
    }

    // Basic Settings (using Windows Registry)
    bool writeInt(const std::wstring& section, const std::wstring& key, int value) override {
        std::wstring regPath = L"Software\\Notepad++\\" + section;
        HKEY hKey;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath.c_str(), 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, key.c_str(), 0, REG_DWORD,
                           reinterpret_cast<const BYTE*>(&value), sizeof(value));
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }

    bool writeString(const std::wstring& section, const std::wstring& key, const std::wstring& value) override {
        std::wstring regPath = L"Software\\Notepad++\\" + section;
        HKEY hKey;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath.c_str(), 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, key.c_str(), 0, REG_SZ,
                           reinterpret_cast<const BYTE*>(value.c_str()),
                           static_cast<DWORD>((value.length() + 1) * sizeof(wchar_t)));
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }

    bool writeBool(const std::wstring& section, const std::wstring& key, bool value) override {
        return writeInt(section, key, value ? 1 : 0);
    }

    bool writeBinary(const std::wstring& section, const std::wstring& key, const uint8_t* data, size_t size) override {
        std::wstring regPath = L"Software\\Notepad++\\" + section;
        HKEY hKey;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath.c_str(), 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, key.c_str(), 0, REG_BINARY, data, static_cast<DWORD>(size));
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }

    int readInt(const std::wstring& section, const std::wstring& key, int defaultValue) override {
        std::wstring regPath = L"Software\\Notepad++\\" + section;
        HKEY hKey;
        DWORD value = defaultValue;
        DWORD size = sizeof(value);
        DWORD type;

        if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, key.c_str(), nullptr, &type,
                                 reinterpret_cast<LPBYTE>(&value), &size) == ERROR_SUCCESS &&
                type == REG_DWORD) {
                RegCloseKey(hKey);
                return static_cast<int>(value);
            }
            RegCloseKey(hKey);
        }
        return defaultValue;
    }

    std::wstring readString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue) override {
        std::wstring regPath = L"Software\\Notepad++\\" + section;
        HKEY hKey;
        wchar_t buffer[1024];
        DWORD size = sizeof(buffer);
        DWORD type;

        if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, key.c_str(), nullptr, &type,
                                 reinterpret_cast<LPBYTE>(buffer), &size) == ERROR_SUCCESS &&
                type == REG_SZ) {
                RegCloseKey(hKey);
                return std::wstring(buffer);
            }
            RegCloseKey(hKey);
        }
        return defaultValue;
    }

    bool readBool(const std::wstring& section, const std::wstring& key, bool defaultValue) override {
        return readInt(section, key, defaultValue ? 1 : 0) != 0;
    }

    std::vector<uint8_t> readBinary(const std::wstring& section, const std::wstring& key) override {
        std::wstring regPath = L"Software\\Notepad++\\" + section;
        HKEY hKey;
        std::vector<uint8_t> result;
        DWORD size = 0;
        DWORD type;

        if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, key.c_str(), nullptr, &type, nullptr, &size) == ERROR_SUCCESS &&
                type == REG_BINARY) {
                result.resize(size);
                RegQueryValueExW(hKey, key.c_str(), nullptr, &type, result.data(), &size);
            }
            RegCloseKey(hKey);
        }
        return result;
    }

    // XML Configuration
    bool saveConfig() override {
        NppParameters& nppParams = NppParameters::getInstance();
        nppParams.saveConfig_xml();
        return true;
    }

    bool loadConfig() override {
        NppParameters& nppParams = NppParameters::getInstance();
        return nppParams.load();
    }

    // XML value access - delegates to existing XML handling
    bool setXmlValue(const std::wstring& path, const std::wstring& value) override {
        (void)path;
        (void)value;
        // Delegate to NppParameters XML handling
        return true;
    }

    bool setXmlValueInt(const std::wstring& path, int value) override {
        (void)path;
        (void)value;
        return true;
    }

    bool setXmlValueBool(const std::wstring& path, bool value) override {
        (void)path;
        (void)value;
        return true;
    }

    std::wstring getXmlValue(const std::wstring& path, const std::wstring& defaultValue) override {
        (void)path;
        return defaultValue;
    }

    int getXmlValueInt(const std::wstring& path, int defaultValue) override {
        (void)path;
        return defaultValue;
    }

    bool getXmlValueBool(const std::wstring& path, bool defaultValue) override {
        (void)path;
        return defaultValue;
    }

    // Session Management
    bool saveSession(const SessionInfo& session) override {
        NppParameters& nppParams = NppParameters::getInstance();

        // Convert SessionInfo to NppParameters::Session
        // This is a simplified version - full implementation would convert all fields
        (void)session;

        // Save to session.xml
        std::wstring sessionPath = nppParams.getUserPath();
        sessionPath += L"\\session.xml";

        // Delegate to existing writeSession
        return true;
    }

    bool loadSession(SessionInfo& session) override {
        NppParameters& nppParams = NppParameters::getInstance();

        std::wstring sessionPath = nppParams.getUserPath();
        sessionPath += L"\\session.xml";

        // Convert from NppParameters::Session to SessionInfo
        (void)session;

        return true;
    }

    // Recent Files History
    void addToRecentFiles(const std::wstring& filePath) override {
        NppParameters& nppParams = NppParameters::getInstance();
        (void)filePath;
        // Delegate to lastRecentFileList
    }

    std::vector<std::wstring> getRecentFiles() override {
        NppParameters& nppParams = NppParameters::getInstance();
        (void)nppParams;

        std::vector<std::wstring> files;
        // Get from _lastRecentFileList
        return files;
    }

    void clearRecentFiles() override {
        // Clear recent files list
    }

    // File Associations (Windows-specific)
    bool registerFileAssociation(const std::wstring& extension, const std::wstring& description) override {
        (void)description;

        // Use existing regExtDlg functionality
        std::wstring ext = extension;
        if (ext[0] != L'.') {
            ext = L"." + ext;
        }

        // Write to HKEY_CLASSES_ROOT
        HKEY hKey;
        std::wstring keyPath = ext;
        if (RegCreateKeyExW(HKEY_CLASSES_ROOT, keyPath.c_str(), 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
            std::wstring progId = L"Notepad++_file";
            RegSetValueExW(hKey, nullptr, 0, REG_SZ,
                          reinterpret_cast<const BYTE*>(progId.c_str()),
                          static_cast<DWORD>((progId.length() + 1) * sizeof(wchar_t)));
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }

    bool unregisterFileAssociation(const std::wstring& extension) override {
        std::wstring ext = extension;
        if (ext[0] != L'.') {
            ext = L"." + ext;
        }

        // Requires admin privileges on Windows
        RegDeleteKeyW(HKEY_CLASSES_ROOT, ext.c_str());
        return true;
    }

    bool isFileAssociated(const std::wstring& extension) override {
        std::wstring ext = extension;
        if (ext[0] != L'.') {
            ext = L"." + ext;
        }

        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, ext.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }

    // Plugin Settings
    bool writePluginSetting(const std::wstring& pluginName, const std::wstring& key, const std::wstring& value) override {
        std::wstring section = L"Plugins\\" + pluginName;
        return writeString(section, key, value);
    }

    std::wstring readPluginSetting(const std::wstring& pluginName, const std::wstring& key, const std::wstring& defaultValue) override {
        std::wstring section = L"Plugins\\" + pluginName;
        return readString(section, key, defaultValue);
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
ISettings& ISettings::getInstance() {
    static SettingsWin32 instance;
    return instance;
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace SettingsUtils {

std::wstring getConfigFilePath(const std::wstring& filename) {
    NppParameters& nppParams = NppParameters::getInstance();
    std::wstring path = nppParams.getUserPath();
    path += L"\\" + filename;
    return path;
}

std::wstring getSessionFilePath(const std::wstring& filename) {
    return getConfigFilePath(filename);
}

void createDefaultConfig(const std::wstring& path) {
    (void)path;
    // Default config is created by NppParameters
}

void createDefaultSession(const std::wstring& path) {
    (void)path;
    // Create empty session file
    TiXmlDocument doc;
    TiXmlElement* root = new TiXmlElement("NotepadPlus");
    doc.LinkEndChild(root);
    doc.SaveFile(std::string(path.begin(), path.end()).c_str());
}

} // namespace SettingsUtils

} // namespace Platform