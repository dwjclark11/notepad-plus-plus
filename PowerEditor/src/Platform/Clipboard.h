// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace Platform {

// Clipboard format types
enum class ClipboardFormat {
    Text,           // Plain text (CF_TEXT / UTF-8)
    UnicodeText,    // Unicode text (CF_UNICODETEXT)
    Html,           // HTML format
    Rtf,            // Rich Text Format
    Binary,         // Binary data with length
    Custom          // Custom format
};

// Clipboard data structure
struct ClipboardData {
    ClipboardFormat format = ClipboardFormat::Text;
    std::vector<uint8_t> data;
    std::string customFormatName;  // For custom formats
    bool isBinary = false;

    ClipboardData() = default;
    explicit ClipboardData(const std::string& text);
    explicit ClipboardData(const std::wstring& text);
    explicit ClipboardData(const std::vector<uint8_t>& binaryData);

    // Convert to string
    std::string toString() const;
    std::wstring toWString() const;

    // Check if empty
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
};

// Clipboard change callback
using ClipboardChangeCallback = std::function<void()>;

// ============================================================================
// IClipboard Interface
// ============================================================================
class IClipboard {
public:
    virtual ~IClipboard() = default;

    // Singleton accessor
    static IClipboard& getInstance();

    // ------------------------------------------------------------------------
    // Basic Clipboard Operations
    // ------------------------------------------------------------------------

    /// Check if clipboard has data
    virtual bool hasData() = 0;

    /// Check if clipboard has specific format
    virtual bool hasFormat(ClipboardFormat format) = 0;

    /// Check if clipboard has custom format
    virtual bool hasCustomFormat(const std::string& formatName) = 0;

    // ------------------------------------------------------------------------
    // Text Operations
    // ------------------------------------------------------------------------

    /// Get text from clipboard
    virtual std::wstring getText() = 0;

    /// Set text to clipboard
    virtual bool setText(const std::wstring& text) = 0;

    /// Set text with binary length info (for preserving binary content)
    virtual bool setTextWithLength(const std::wstring& text, size_t originalLength) = 0;

    // ------------------------------------------------------------------------
    // Binary Data Operations
    // ------------------------------------------------------------------------

    /// Get binary data from clipboard
    virtual ClipboardData getData() = 0;

    /// Get data with specific format
    virtual ClipboardData getData(ClipboardFormat format) = 0;

    /// Set binary data to clipboard
    virtual bool setData(const ClipboardData& data) = 0;

    /// Set multiple formats at once
    virtual bool setData(const std::vector<ClipboardData>& dataItems) = 0;

    // ------------------------------------------------------------------------
    // HTML/RTF Operations
    // ------------------------------------------------------------------------

    /// Get HTML from clipboard
    virtual std::string getHtml() = 0;

    /// Set HTML to clipboard (with plain text fallback)
    virtual bool setHtml(const std::string& html, const std::string& text) = 0;

    /// Get RTF from clipboard
    virtual std::string getRtf() = 0;

    /// Set RTF to clipboard
    virtual bool setRtf(const std::string& rtf, const std::string& text) = 0;

    // ------------------------------------------------------------------------
    // Custom Format Operations
    // ------------------------------------------------------------------------

    /// Register a custom clipboard format
    virtual uint32_t registerFormat(const std::string& formatName) = 0;

    /// Get custom format data
    virtual std::vector<uint8_t> getCustomData(const std::string& formatName) = 0;

    /// Set custom format data
    virtual bool setCustomData(const std::string& formatName, const std::vector<uint8_t>& data) = 0;

    // ------------------------------------------------------------------------
    // Clipboard History / Monitoring
    // ------------------------------------------------------------------------

    /// Start monitoring clipboard changes
    virtual void startMonitoring() = 0;

    /// Stop monitoring clipboard changes
    virtual void stopMonitoring() = 0;

    /// Check if monitoring is active
    virtual bool isMonitoring() = 0;

    /// Set callback for clipboard changes
    virtual void setChangeCallback(ClipboardChangeCallback callback) = 0;

    /// Get current clipboard sequence number (changes on each clipboard update)
    virtual uint32_t getSequenceNumber() = 0;

    // ------------------------------------------------------------------------
    // Utility Functions
    // ------------------------------------------------------------------------

    /// Clear clipboard
    virtual bool clear() = 0;

    /// Open clipboard for operations (returns false if another app has it open)
    virtual bool open() = 0;

    /// Close clipboard
    virtual void close() = 0;

    /// Check if clipboard is currently open
    virtual bool isOpen() = 0;

    /// Flush clipboard (Windows-specific, no-op on Linux)
    virtual bool flush() = 0;
};

// ============================================================================
// Clipboard History Entry
// ============================================================================
struct ClipboardHistoryEntry {
    ClipboardData data;
    uint64_t timestamp;
    std::wstring displayText;
    bool isBinary = false;

    ClipboardHistoryEntry() = default;
    explicit ClipboardHistoryEntry(const ClipboardData& d);
};

// ============================================================================
// Clipboard History Manager
// ============================================================================
class IClipboardHistory {
public:
    virtual ~IClipboardHistory() = default;

    // Singleton accessor
    static IClipboardHistory& getInstance();

    /// Initialize the history manager
    virtual void init() = 0;

    /// Add entry to history
    virtual void addEntry(const ClipboardData& data) = 0;

    /// Get entry at index
    virtual ClipboardHistoryEntry getEntry(size_t index) = 0;

    /// Get all entries
    virtual std::vector<ClipboardHistoryEntry> getAllEntries() = 0;

    /// Get number of entries
    virtual size_t getEntryCount() = 0;

    /// Remove entry at index
    virtual void removeEntry(size_t index) = 0;

    /// Clear all history
    virtual void clear() = 0;

    /// Set maximum history size
    virtual void setMaxSize(size_t maxSize) = 0;

    /// Get maximum history size
    virtual size_t getMaxSize() = 0;

    /// Find entry index by data comparison
    virtual int findEntryIndex(const ClipboardData& data) = 0;

    /// Save history to settings
    virtual void saveHistory() = 0;

    /// Load history from settings
    virtual void loadHistory() = 0;
};

// ============================================================================
// Utility Functions
// ============================================================================
namespace ClipboardUtils {

/// Convert clipboard format to string
const char* formatToString(ClipboardFormat format);

/// Check if data contains binary content
bool containsBinary(const std::vector<uint8_t>& data);

/// Truncate text for display with ellipsis
std::wstring truncateForDisplay(const std::wstring& text, size_t maxLength);

/// Truncate binary data for display
std::wstring binaryToDisplay(const std::vector<uint8_t>& data, size_t maxLength);

/// Convert HTML to clipboard HTML format (Windows HTML format header)
std::string convertToHtmlFormat(const std::string& html, const std::string& fragment);

/// Extract plain text from HTML
std::string extractTextFromHtml(const std::string& html);

} // namespace ClipboardUtils

} // namespace Platform
