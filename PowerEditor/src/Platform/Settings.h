// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

namespace PlatformLayer {

// Forward declarations
struct SessionInfo;
struct FilePosition;

// ============================================================================
// ISettings Interface
// ============================================================================
class ISettings {
public:
    virtual ~ISettings() = default;

    // Singleton accessor
    static ISettings& getInstance();

    // Test injection support
    static void setTestInstance(ISettings* instance) { _testInstance = instance; }
    static void resetTestInstance() { _testInstance = nullptr; }

    // ------------------------------------------------------------------------
    // Initialization and Paths
    // ------------------------------------------------------------------------
    virtual bool init() = 0;
    virtual std::wstring getConfigPath() const = 0;
    virtual std::wstring getSettingsDir() const = 0;
    virtual std::wstring getUserPluginsDir() const = 0;

    // ------------------------------------------------------------------------
    // Basic Settings (Registry/INI style)
    // ------------------------------------------------------------------------
    virtual bool writeInt(const std::wstring& section, const std::wstring& key, int value) = 0;
    virtual bool writeString(const std::wstring& section, const std::wstring& key, const std::wstring& value) = 0;
    virtual bool writeBool(const std::wstring& section, const std::wstring& key, bool value) = 0;
    virtual bool writeBinary(const std::wstring& section, const std::wstring& key, const uint8_t* data, size_t size) = 0;

    virtual int readInt(const std::wstring& section, const std::wstring& key, int defaultValue = 0) = 0;
    virtual std::wstring readString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue = L"") = 0;
    virtual bool readBool(const std::wstring& section, const std::wstring& key, bool defaultValue = false) = 0;
    virtual std::vector<uint8_t> readBinary(const std::wstring& section, const std::wstring& key) = 0;

    // ------------------------------------------------------------------------
    // XML Configuration (config.xml)
    // ------------------------------------------------------------------------
    virtual bool saveConfig() = 0;
    virtual bool loadConfig() = 0;

    // XML value access
    virtual bool setXmlValue(const std::wstring& path, const std::wstring& value) = 0;
    virtual bool setXmlValueInt(const std::wstring& path, int value) = 0;
    virtual bool setXmlValueBool(const std::wstring& path, bool value) = 0;

    virtual std::wstring getXmlValue(const std::wstring& path, const std::wstring& defaultValue = L"") = 0;
    virtual int getXmlValueInt(const std::wstring& path, int defaultValue = 0) = 0;
    virtual bool getXmlValueBool(const std::wstring& path, bool defaultValue = false) = 0;

    // ------------------------------------------------------------------------
    // Session Management (session.xml)
    // ------------------------------------------------------------------------
    virtual bool saveSession(const SessionInfo& session) = 0;
    virtual bool loadSession(SessionInfo& session) = 0;

    // ------------------------------------------------------------------------
    // Recent Files History
    // ------------------------------------------------------------------------
    virtual void addToRecentFiles(const std::wstring& filePath) = 0;
    virtual std::vector<std::wstring> getRecentFiles() = 0;
    virtual void clearRecentFiles() = 0;

    // ------------------------------------------------------------------------
    // File Associations (Windows only, no-op on Linux)
    // ------------------------------------------------------------------------
    virtual bool registerFileAssociation(const std::wstring& extension, const std::wstring& description) = 0;
    virtual bool unregisterFileAssociation(const std::wstring& extension) = 0;
    virtual bool isFileAssociated(const std::wstring& extension) = 0;

    // ------------------------------------------------------------------------
    // Plugin Settings
    // ------------------------------------------------------------------------
    virtual bool writePluginSetting(const std::wstring& pluginName, const std::wstring& key, const std::wstring& value) = 0;
    virtual std::wstring readPluginSetting(const std::wstring& pluginName, const std::wstring& key, const std::wstring& defaultValue = L"") = 0;

private:
    static ISettings* _testInstance;
};

// ============================================================================
// Data Structures
// ============================================================================

struct FilePosition {
    intptr_t firstVisibleLine = 0;
    intptr_t startPos = 0;
    intptr_t endPos = 0;
    intptr_t xOffset = 0;
    intptr_t selMode = 0;
    intptr_t scrollWidth = 1;
    intptr_t offset = 0;
    intptr_t wrapCount = 0;
};

struct SessionFileInfo {
    std::wstring fileName;
    std::wstring langName;
    std::vector<size_t> marks;
    std::vector<size_t> foldStates;
    int encoding = -1;
    bool isUserReadOnly = false;
    bool isMonitoring = false;
    int individualTabColour = -1;
    bool isRTL = false;
    bool isPinned = false;
    bool isUntitledTabRenamed = false;
    FilePosition position;
    std::wstring backupFilePath;
    uint64_t originalFileLastModifTimestamp = 0;

    SessionFileInfo() = default;
    explicit SessionFileInfo(const std::wstring& fn) : fileName(fn) {}
};

struct SessionInfo {
    std::vector<SessionFileInfo> files;
    std::vector<std::wstring> folderWorkspaces;
    intptr_t activeIndex = 0;
    intptr_t activeLeaf = 0;
    std::wstring sessionName;
};

// ============================================================================
// Utility Functions
// ============================================================================
namespace SettingsUtils {

// Path utilities
std::wstring getConfigFilePath(const std::wstring& filename = L"config.xml");
std::wstring getSessionFilePath(const std::wstring& filename = L"session.xml");

// Default settings creation
void createDefaultConfig(const std::wstring& path);
void createDefaultSession(const std::wstring& path);

} // namespace SettingsUtils

} // namespace PlatformLayer
