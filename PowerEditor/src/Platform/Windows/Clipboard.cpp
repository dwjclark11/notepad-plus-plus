// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Clipboard.h"
#include "../../MISC/Common/Common.h"
#include <windows.h>
#include <shlobj.h>

// Custom format names
#define CF_HTML L"HTML Format"
#define CF_RTF L"Rich Text Format"
#define CF_NPPTEXTLEN L"Notepad++ Binary Length"

namespace Platform {

// ============================================================================
// ClipboardData implementation
// ============================================================================
ClipboardData::ClipboardData(const std::string& text) {
    format = ClipboardFormat::Text;
    data.assign(text.begin(), text.end());
    data.push_back(0);  // null terminator
}

ClipboardData::ClipboardData(const std::wstring& text) {
    format = ClipboardFormat::UnicodeText;
    size_t bytes = text.length() * sizeof(wchar_t);
    data.resize(bytes + sizeof(wchar_t));
    memcpy(data.data(), text.c_str(), bytes);
    data[bytes] = 0;
    data[bytes + 1] = 0;
}

ClipboardData::ClipboardData(const std::vector<uint8_t>& binaryData) {
    format = ClipboardFormat::Binary;
    data = binaryData;
    isBinary = true;
}

std::string ClipboardData::toString() const {
    if (data.empty()) return std::string();

    // Check if null terminated
    if (data.back() == 0) {
        return std::string(reinterpret_cast<const char*>(data.data()));
    }

    return std::string(reinterpret_cast<const char*>(data.data()), data.size());
}

std::wstring ClipboardData::toWString() const {
    if (data.empty()) return std::wstring();

    // Check if it's a wchar_t array
    if (data.size() >= sizeof(wchar_t) && data.size() % sizeof(wchar_t) == 0) {
        size_t len = data.size() / sizeof(wchar_t);
        // Check if null terminated
        if (data.back() == 0) {
            return std::wstring(reinterpret_cast<const wchar_t*>(data.data()));
        }
        return std::wstring(reinterpret_cast<const wchar_t*>(data.data()), len);
    }

    // Convert from UTF-8
    std::string str = toString();
    return string2wstring(str, CP_UTF8);
}

// ============================================================================
// Windows Implementation of IClipboard
// ============================================================================
class ClipboardWin32 : public IClipboard {
public:
    ClipboardWin32() : _isOpen(false), _sequenceNumber(0) {
        _htmlFormatId = 0;
        _rtfFormatId = 0;
        _nppTextLenFormatId = 0;
        _monitoring = false;
    }

    ~ClipboardWin32() override {
        if (_isOpen) {
            close();
        }
        stopMonitoring();
    }

    bool hasData() override {
        return IsClipboardFormatAvailable(CF_UNICODETEXT) ||
               IsClipboardFormatAvailable(CF_TEXT) ||
               IsClipboardFormatAvailable(CF_HTML) ||
               IsClipboardFormatAvailable(CF_RTF);
    }

    bool hasFormat(ClipboardFormat format) override {
        switch (format) {
            case ClipboardFormat::Text:
                return IsClipboardFormatAvailable(CF_TEXT) != FALSE;
            case ClipboardFormat::UnicodeText:
                return IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
            case ClipboardFormat::Html:
                return IsClipboardFormatAvailable(_htmlFormatId != 0 ? _htmlFormatId : CF_HTML) != FALSE;
            case ClipboardFormat::Rtf:
                return IsClipboardFormatAvailable(_rtfFormatId != 0 ? _rtfFormatId : CF_RTF) != FALSE;
            case ClipboardFormat::Binary:
                // Binary is indicated by presence of CF_NPPTEXTLEN with CF_TEXT
                return IsClipboardFormatAvailable(_nppTextLenFormatId != 0 ? _nppTextLenFormatId :
                       RegisterClipboardFormatW(CF_NPPTEXTLEN)) != FALSE;
            default:
                return false;
        }
    }

    bool hasCustomFormat(const std::string& formatName) override {
        UINT format = RegisterClipboardFormatA(formatName.c_str());
        return IsClipboardFormatAvailable(format) != FALSE;
    }

    std::wstring getText() override {
        if (!IsClipboardFormatAvailable(CF_UNICODETEXT) && !IsClipboardFormatAvailable(CF_TEXT)) {
            return std::wstring();
        }

        if (!open()) {
            return std::wstring();
        }

        std::wstring result;

        // Try Unicode first
        if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData) {
                wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
                if (pszText) {
                    result = pszText;
                    GlobalUnlock(hData);
                }
            }
        }
        // Fall back to ANSI text
        else if (IsClipboardFormatAvailable(CF_TEXT)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* pszText = static_cast<char*>(GlobalLock(hData));
                if (pszText) {
                    result = string2wstring(pszText, CP_ACP);
                    GlobalUnlock(hData);
                }
            }
        }

        close();
        return result;
    }

    bool setText(const std::wstring& text) override {
        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        size_t len = text.length() + 1;
        HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar_t));
        if (!hData) {
            close();
            return false;
        }

        wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
        if (!pszText) {
            GlobalFree(hData);
            close();
            return false;
        }

        memcpy(pszText, text.c_str(), len * sizeof(wchar_t));
        GlobalUnlock(hData);

        if (!SetClipboardData(CF_UNICODETEXT, hData)) {
            GlobalFree(hData);
            close();
            return false;
        }

        close();
        return true;
    }

    bool setTextWithLength(const std::wstring& text, size_t originalLength) override {
        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        // Set Unicode text
        size_t len = text.length() + 1;
        HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar_t));
        if (hData) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText) {
                memcpy(pszText, text.c_str(), len * sizeof(wchar_t));
                GlobalUnlock(hData);
                SetClipboardData(CF_UNICODETEXT, hData);
            } else {
                GlobalFree(hData);
            }
        }

        // Set ANSI text (for binary preservation)
        std::string ansiText = wstring2string(text, CP_ACP);
        HGLOBAL hDataAnsi = GlobalAlloc(GMEM_MOVEABLE, ansiText.length() + 1);
        if (hDataAnsi) {
            char* pszText = static_cast<char*>(GlobalLock(hDataAnsi));
            if (pszText) {
                memcpy(pszText, ansiText.c_str(), ansiText.length() + 1);
                GlobalUnlock(hDataAnsi);
                SetClipboardData(CF_TEXT, hDataAnsi);
            } else {
                GlobalFree(hDataAnsi);
            }
        }

        // Set the binary length marker
        UINT cf_nppTextLen = RegisterClipboardFormatW(CF_NPPTEXTLEN);
        HGLOBAL hDataLen = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG));
        if (hDataLen) {
            ULONG* pLen = static_cast<ULONG*>(GlobalLock(hDataLen));
            if (pLen) {
                *pLen = static_cast<ULONG>(originalLength);
                GlobalUnlock(hDataLen);
                SetClipboardData(cf_nppTextLen, hDataLen);
            } else {
                GlobalFree(hDataLen);
            }
        }

        close();
        return true;
    }

    ClipboardData getData() override {
        ClipboardData result;

        if (hasFormat(ClipboardFormat::UnicodeText)) {
            result.format = ClipboardFormat::UnicodeText;
            std::wstring text = getText();
            result.data.resize(text.length() * sizeof(wchar_t));
            memcpy(result.data.data(), text.c_str(), text.length() * sizeof(wchar_t));
        } else if (hasFormat(ClipboardFormat::Text)) {
            result.format = ClipboardFormat::Text;
            std::wstring wtext = getText();
            std::string text = wstring2string(wtext, CP_UTF8);
            result.data.assign(text.begin(), text.end());
        }

        return result;
    }

    ClipboardData getData(ClipboardFormat format) override {
        if (!open()) {
            return ClipboardData();
        }

        ClipboardData result;
        result.format = format;

        UINT formatId = 0;
        switch (format) {
            case ClipboardFormat::Text:
                formatId = CF_TEXT;
                break;
            case ClipboardFormat::UnicodeText:
                formatId = CF_UNICODETEXT;
                break;
            case ClipboardFormat::Html:
                formatId = _htmlFormatId != 0 ? _htmlFormatId : RegisterClipboardFormatW(CF_HTML);
                break;
            case ClipboardFormat::Rtf:
                formatId = _rtfFormatId != 0 ? _rtfFormatId : RegisterClipboardFormatW(CF_RTF);
                break;
            default:
                close();
                return ClipboardData();
        }

        if (IsClipboardFormatAvailable(formatId)) {
            HANDLE hData = GetClipboardData(formatId);
            if (hData) {
                SIZE_T size = GlobalSize(hData);
                void* pData = GlobalLock(hData);
                if (pData && size > 0) {
                    result.data.resize(size);
                    memcpy(result.data.data(), pData, size);
                    GlobalUnlock(hData);
                }
            }
        }

        close();
        return result;
    }

    bool setData(const ClipboardData& data) override {
        if (data.format == ClipboardFormat::UnicodeText || data.format == ClipboardFormat::Text) {
            return setText(data.toWString());
        }

        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        UINT formatId = CF_TEXT;
        switch (data.format) {
            case ClipboardFormat::UnicodeText:
                formatId = CF_UNICODETEXT;
                break;
            case ClipboardFormat::Html:
                formatId = _htmlFormatId != 0 ? _htmlFormatId : RegisterClipboardFormatW(CF_HTML);
                break;
            case ClipboardFormat::Rtf:
                formatId = _rtfFormatId != 0 ? _rtfFormatId : RegisterClipboardFormatW(CF_RTF);
                break;
            default:
                formatId = CF_TEXT;
        }

        HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, data.data.size());
        if (!hData) {
            close();
            return false;
        }

        void* pDest = GlobalLock(hData);
        if (!pDest) {
            GlobalFree(hData);
            close();
            return false;
        }

        memcpy(pDest, data.data.data(), data.data.size());
        GlobalUnlock(hData);

        if (!SetClipboardData(formatId, hData)) {
            GlobalFree(hData);
            close();
            return false;
        }

        close();
        return true;
    }

    bool setData(const std::vector<ClipboardData>& dataItems) override {
        if (dataItems.empty()) {
            return false;
        }

        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        bool success = true;

        for (const auto& item : dataItems) {
            UINT formatId = CF_TEXT;
            switch (item.format) {
                case ClipboardFormat::Text:
                    formatId = CF_TEXT;
                    break;
                case ClipboardFormat::UnicodeText:
                    formatId = CF_UNICODETEXT;
                    break;
                case ClipboardFormat::Html:
                    formatId = _htmlFormatId != 0 ? _htmlFormatId : RegisterClipboardFormatW(CF_HTML);
                    break;
                case ClipboardFormat::Rtf:
                    formatId = _rtfFormatId != 0 ? _rtfFormatId : RegisterClipboardFormatW(CF_RTF);
                    break;
                default:
                    continue;
            }

            HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, item.data.size());
            if (!hData) {
                success = false;
                continue;
            }

            void* pDest = GlobalLock(hData);
            if (!pDest) {
                GlobalFree(hData);
                success = false;
                continue;
            }

            memcpy(pDest, item.data.data(), item.data.size());
            GlobalUnlock(hData);

            if (!SetClipboardData(formatId, hData)) {
                GlobalFree(hData);
                success = false;
            }
        }

        close();
        return success;
    }

    std::string getHtml() override {
        ClipboardData data = getData(ClipboardFormat::Html);
        return data.toString();
    }

    bool setHtml(const std::string& html, const std::string& text) override {
        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        // Set plain text as well
        std::wstring wtext = string2wstring(text, CP_UTF8);
        size_t len = wtext.length() + 1;
        HGLOBAL hTextData = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar_t));
        if (hTextData) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hTextData));
            if (pszText) {
                memcpy(pszText, wtext.c_str(), len * sizeof(wchar_t));
                GlobalUnlock(hTextData);
                SetClipboardData(CF_UNICODETEXT, hTextData);
            }
        }

        // Set HTML format
        UINT cf_html = _htmlFormatId != 0 ? _htmlFormatId : RegisterClipboardFormatW(CF_HTML);
        HGLOBAL hHtmlData = GlobalAlloc(GMEM_MOVEABLE, html.length() + 1);
        if (hHtmlData) {
            char* pHtml = static_cast<char*>(GlobalLock(hHtmlData));
            if (pHtml) {
                memcpy(pHtml, html.c_str(), html.length() + 1);
                GlobalUnlock(hHtmlData);
                SetClipboardData(cf_html, hHtmlData);
            }
        }

        close();
        return true;
    }

    std::string getRtf() override {
        ClipboardData data = getData(ClipboardFormat::Rtf);
        return data.toString();
    }

    bool setRtf(const std::string& rtf, const std::string& text) override {
        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        // Set plain text
        std::wstring wtext = string2wstring(text, CP_UTF8);
        size_t len = wtext.length() + 1;
        HGLOBAL hTextData = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar_t));
        if (hTextData) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hTextData));
            if (pszText) {
                memcpy(pszText, wtext.c_str(), len * sizeof(wchar_t));
                GlobalUnlock(hTextData);
                SetClipboardData(CF_UNICODETEXT, hTextData);
            }
        }

        // Set RTF format
        UINT cf_rtf = _rtfFormatId != 0 ? _rtfFormatId : RegisterClipboardFormatW(CF_RTF);
        HGLOBAL hRtfData = GlobalAlloc(GMEM_MOVEABLE, rtf.length() + 1);
        if (hRtfData) {
            char* pRtf = static_cast<char*>(GlobalLock(hRtfData));
            if (pRtf) {
                memcpy(pRtf, rtf.c_str(), rtf.length() + 1);
                GlobalUnlock(hRtfData);
                SetClipboardData(cf_rtf, hRtfData);
            }
        }

        close();
        return true;
    }

    uint32_t registerFormat(const std::string& formatName) override {
        return static_cast<uint32_t>(RegisterClipboardFormatA(formatName.c_str()));
    }

    std::vector<uint8_t> getCustomData(const std::string& formatName) override {
        UINT format = RegisterClipboardFormatA(formatName.c_str());

        if (!open()) {
            return std::vector<uint8_t>();
        }

        std::vector<uint8_t> result;

        if (IsClipboardFormatAvailable(format)) {
            HANDLE hData = GetClipboardData(format);
            if (hData) {
                SIZE_T size = GlobalSize(hData);
                void* pData = GlobalLock(hData);
                if (pData && size > 0) {
                    result.resize(size);
                    memcpy(result.data(), pData, size);
                    GlobalUnlock(hData);
                }
            }
        }

        close();
        return result;
    }

    bool setCustomData(const std::string& formatName, const std::vector<uint8_t>& data) override {
        UINT format = RegisterClipboardFormatA(formatName.c_str());

        if (!open()) {
            return false;
        }

        if (!EmptyClipboard()) {
            close();
            return false;
        }

        HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, data.size());
        if (!hData) {
            close();
            return false;
        }

        void* pDest = GlobalLock(hData);
        if (!pDest) {
            GlobalFree(hData);
            close();
            return false;
        }

        memcpy(pDest, data.data(), data.size());
        GlobalUnlock(hData);

        if (!SetClipboardData(format, hData)) {
            GlobalFree(hData);
            close();
            return false;
        }

        close();
        return true;
    }

    void startMonitoring() override {
        _monitoring = true;
        _lastSequenceNumber = GetClipboardSequenceNumber();
    }

    void stopMonitoring() override {
        _monitoring = false;
    }

    bool isMonitoring() override {
        return _monitoring;
    }

    void setChangeCallback(ClipboardChangeCallback callback) override {
        _changeCallback = callback;
    }

    uint32_t getSequenceNumber() override {
        return GetClipboardSequenceNumber();
    }

    bool clear() override {
        if (!open()) {
            return false;
        }

        bool result = EmptyClipboard() != FALSE;
        close();
        return result;
    }

    bool open() override {
        if (_isOpen) {
            return true;
        }
        _isOpen = OpenClipboard(NULL) != FALSE;
        return _isOpen;
    }

    void close() override {
        if (_isOpen) {
            CloseClipboard();
            _isOpen = false;
        }
    }

    bool isOpen() override {
        return _isOpen;
    }

    bool flush() override {
        return FlushClipboard() != FALSE;
    }

private:
    bool _isOpen;
    bool _monitoring;
    uint32_t _sequenceNumber;
    DWORD _lastSequenceNumber;
    UINT _htmlFormatId;
    UINT _rtfFormatId;
    UINT _nppTextLenFormatId;
    ClipboardChangeCallback _changeCallback;
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IClipboard& IClipboard::getInstance() {
    static ClipboardWin32 instance;
    return instance;
}

// ============================================================================
// ClipboardHistoryEntry implementation
// ============================================================================
ClipboardHistoryEntry::ClipboardHistoryEntry(const ClipboardData& d)
    : data(d), timestamp(0), isBinary(d.isBinary) {
    // Generate display text
    std::wstring wtext = d.toWString();
    if (wtext.empty() && !d.data.empty()) {
        // Binary data
        isBinary = true;
    }
}

// ============================================================================
// Windows Implementation of IClipboardHistory
// ============================================================================
class ClipboardHistoryWin32 : public IClipboardHistory {
public:
    ClipboardHistoryWin32() : _maxSize(20) {}

    void init() override {
        loadHistory();
    }

    void addEntry(const ClipboardData& data) override {
        // Check if already exists
        int existingIndex = findEntryIndex(data);
        if (existingIndex >= 0) {
            // Move to front
            ClipboardHistoryEntry entry = _entries[existingIndex];
            _entries.erase(_entries.begin() + existingIndex);
            _entries.insert(_entries.begin(), entry);
            return;
        }

        // Add new entry
        ClipboardHistoryEntry entry(data);
        entry.timestamp = GetTickCount64();

        // Generate display text
        if (entry.isBinary) {
            entry.displayText = L"[Binary data]";
        } else {
            std::wstring text = data.toWString();
            entry.displayText = ClipboardUtils::truncateForDisplay(text, 64);
        }

        _entries.insert(_entries.begin(), entry);

        // Trim to max size
        while (_entries.size() > _maxSize) {
            _entries.pop_back();
        }
    }

    ClipboardHistoryEntry getEntry(size_t index) override {
        if (index < _entries.size()) {
            return _entries[index];
        }
        return ClipboardHistoryEntry();
    }

    std::vector<ClipboardHistoryEntry> getAllEntries() override {
        return _entries;
    }

    size_t getEntryCount() override {
        return _entries.size();
    }

    void removeEntry(size_t index) override {
        if (index < _entries.size()) {
            _entries.erase(_entries.begin() + index);
        }
    }

    void clear() override {
        _entries.clear();
    }

    void setMaxSize(size_t maxSize) override {
        _maxSize = maxSize;
        while (_entries.size() > _maxSize) {
            _entries.pop_back();
        }
    }

    size_t getMaxSize() override {
        return _maxSize;
    }

    int findEntryIndex(const ClipboardData& data) override {
        for (size_t i = 0; i < _entries.size(); ++i) {
            if (_entries[i].data.data == data.data) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void saveHistory() override {
        // TODO: Save to settings
    }

    void loadHistory() override {
        // TODO: Load from settings
    }

private:
    std::vector<ClipboardHistoryEntry> _entries;
    size_t _maxSize;
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IClipboardHistory& IClipboardHistory::getInstance() {
    static ClipboardHistoryWin32 instance;
    return instance;
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace ClipboardUtils {

const char* formatToString(ClipboardFormat format) {
    switch (format) {
        case ClipboardFormat::Text: return "Text";
        case ClipboardFormat::UnicodeText: return "UnicodeText";
        case ClipboardFormat::Html: return "HTML";
        case ClipboardFormat::Rtf: return "RTF";
        case ClipboardFormat::Binary: return "Binary";
        case ClipboardFormat::Custom: return "Custom";
        default: return "Unknown";
    }
}

bool containsBinary(const std::vector<uint8_t>& data) {
    for (uint8_t byte : data) {
        if (byte < 32 && byte != '\t' && byte != '\n' && byte != '\r') {
            return true;
        }
    }
    return false;
}

std::wstring truncateForDisplay(const std::wstring& text, size_t maxLength) {
    if (text.length() <= maxLength) {
        return text;
    }

    std::wstring result = text.substr(0, maxLength - 3);
    result += L"...";
    return result;
}

std::wstring binaryToDisplay(const std::vector<uint8_t>& data, size_t maxLength) {
    std::wstring result = L"[";

    size_t displayBytes = std::min(data.size(), maxLength);
    for (size_t i = 0; i < displayBytes; ++i) {
        wchar_t hex[4];
        swprintf_s(hex, L"%02X ", data[i]);
        result += hex;
    }

    if (data.size() > maxLength) {
        result += L"...";
    }

    result += L"]";
    return result;
}

std::string convertToHtmlFormat(const std::string& html, const std::string& fragment) {
    // Windows HTML clipboard format header
    std::string result = "Version:0.9\r\n";
    result += "StartHTML:00000000\r\n";
    result += "EndHTML:00000000\r\n";
    result += "StartFragment:00000000\r\n";
    result += "EndFragment:00000000\r\n";

    std::string body = "<html><body>\r\n";
    body += "<!--StartFragment-->";
    body += fragment;
    body += "<!--EndFragment-->\r\n";
    body += "</body></html>";

    result += body;

    // Update offsets
    size_t startHtml = result.find("<html>");
    size_t endHtml = result.length();
    size_t startFragment = result.find("<!--StartFragment-->") + 20;
    size_t endFragment = result.find("<!--EndFragment-->");

    char buf[32];
    sprintf_s(buf, "%08zu", startHtml);
    result.replace(result.find("StartHTML:00000000") + 10, 8, buf);
    sprintf_s(buf, "%08zu", endHtml);
    result.replace(result.find("EndHTML:00000000") + 8, 8, buf);
    sprintf_s(buf, "%08zu", startFragment);
    result.replace(result.find("StartFragment:00000000") + 14, 8, buf);
    sprintf_s(buf, "%08zu", endFragment);
    result.replace(result.find("EndFragment:00000000") + 12, 8, buf);

    return result;
}

std::string extractTextFromHtml(const std::string& html) {
    // Simple HTML tag removal
    std::string result;
    bool inTag = false;

    for (char c : html) {
        if (c == '<') {
            inTag = true;
        } else if (c == '>') {
            inTag = false;
        } else if (!inTag) {
            result += c;
        }
    }

    return result;
}

} // namespace ClipboardUtils

} // namespace Platform
