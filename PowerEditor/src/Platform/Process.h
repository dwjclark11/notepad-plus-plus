// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace PlatformLayer {

// Process execution options
struct ProcessOptions {
    bool elevated = false;           // Request elevation (Windows UAC / Linux pkexec)
    bool hidden = false;             // Hide window/console
    bool waitForExit = false;        // Wait for process to complete
    bool captureOutput = false;      // Capture stdout/stderr
    bool mergeStderr = false;        // Merge stderr into stdout
    std::wstring workingDir;         // Working directory
    std::vector<std::wstring> envVars; // Environment variables (KEY=VALUE)

    ProcessOptions() = default;
};

// Process execution result
struct ProcessResult {
    bool success = false;
    uint32_t exitCode = 0;
    std::string output;              // Captured stdout (if captureOutput was true)
    std::string errorOutput;         // Captured stderr (if captureOutput was true)
    std::wstring errorMessage;       // Error message if success is false

    ProcessResult() = default;
};

// Process information for running processes
struct ProcessInfo {
    uint32_t pid = 0;
    std::wstring command;
    void* nativeHandle = nullptr;    // Platform-specific handle

    ProcessInfo() = default;
};

// Process type enumeration
enum class ProcessType {
    GUI,        // GUI application
    Console,    // Console application
    Background  // Background/daemon process
};

// ============================================================================
// IProcess Interface
// ============================================================================
class IProcess {
public:
    virtual ~IProcess() = default;

    // Singleton accessor
    static IProcess& getInstance();

    // ------------------------------------------------------------------------
    // Basic Process Execution
    // ------------------------------------------------------------------------

    // Run a process asynchronously (non-blocking)
    // Returns process info which can be used with waitForProcess()
    virtual ProcessInfo run(const std::wstring& command,
                           const std::wstring& args,
                           const ProcessOptions& options = {}) = 0;

    // Run a process synchronously (blocking)
    // Returns the process result including exit code
    virtual ProcessResult runSync(const std::wstring& command,
                                 const std::wstring& args,
                                 const ProcessOptions& options = {}) = 0;

    // ------------------------------------------------------------------------
    // Shell Execution (open files/URLs with default handler)
    // ------------------------------------------------------------------------

    // Open a file or URL with the default application
    virtual bool shellOpen(const std::wstring& fileOrUrl,
                          const std::wstring& args = L"",
                          bool elevated = false) = 0;

    // Open a file with a specific application
    virtual bool shellOpenWith(const std::wstring& application,
                              const std::wstring& file,
                              const std::wstring& args = L"",
                              bool elevated = false) = 0;

    // ------------------------------------------------------------------------
    // Process Management
    // ------------------------------------------------------------------------

    // Wait for a process to complete
    // Returns false if timeout occurred
    virtual bool waitForProcess(const ProcessInfo& info,
                               uint32_t timeoutMs = 0xFFFFFFFF) = 0;

    // Terminate a process
    virtual bool terminateProcess(const ProcessInfo& info,
                                 bool force = false) = 0;

    // Check if a process is still running
    virtual bool isProcessRunning(const ProcessInfo& info) = 0;

    // Get the exit code of a completed process
    virtual bool getExitCode(const ProcessInfo& info, uint32_t& exitCode) = 0;

    // ------------------------------------------------------------------------
    // Process Queries
    // ------------------------------------------------------------------------

    // Check if a process with given PID exists
    virtual bool isProcessRunning(uint32_t pid) = 0;

    // Get the current process ID
    virtual uint32_t getCurrentProcessId() = 0;

    // Get the current process executable path
    virtual std::wstring getCurrentProcessPath() = 0;

    // ------------------------------------------------------------------------
    // Elevated Execution
    // ------------------------------------------------------------------------

    // Check if current process is running with elevated privileges
    virtual bool isCurrentProcessElevated() = 0;

    // Restart current process with elevation
    // Returns immediately; application should exit after calling this
    virtual bool restartElevated(const std::wstring& args = L"") = 0;

    // ------------------------------------------------------------------------
    // Utility Functions
    // ------------------------------------------------------------------------

    // Quote an argument for safe passing to a process
    static std::wstring quoteArgument(const std::wstring& arg);

    // Build a command line from program and arguments
    static std::wstring buildCommandLine(const std::wstring& program,
                                        const std::vector<std::wstring>& args);

    // Find an executable in PATH
    static std::wstring findExecutable(const std::wstring& name);
};

// ============================================================================
// Convenience Class (similar to original Process)
// ============================================================================
class Process {
public:
    Process(const wchar_t* cmd, const wchar_t* args, const wchar_t* cDir)
        : _command(cmd ? cmd : L""),
          _args(args ? args : L""),
          _curDir(cDir ? cDir : L"") {}

    Process(const std::wstring& cmd, const std::wstring& args,
           const std::wstring& cDir)
        : _command(cmd), _args(args), _curDir(cDir) {}

    // Run asynchronously
    void run(bool isElevationRequired = false) const {
        ProcessOptions options;
        options.elevated = isElevationRequired;
        options.workingDir = _curDir;
        IProcess::getInstance().run(_command, _args, options);
    }

    // Run synchronously and return exit code
    unsigned long runSync(bool isElevationRequired = false) const {
        ProcessOptions options;
        options.elevated = isElevationRequired;
        options.workingDir = _curDir;
        options.waitForExit = true;
        ProcessResult result = IProcess::getInstance().runSync(_command, _args, options);
        return result.exitCode;
    }

protected:
    std::wstring _command;
    std::wstring _args;
    std::wstring _curDir;
};

// ============================================================================
// Utility Functions
// ============================================================================
namespace ProcessUtils {

// Execute a command and capture output
ProcessResult execute(const std::wstring& command,
                     const std::vector<std::wstring>& args,
                     const std::wstring& workingDir = L"");

// Open a document with its default application
bool openDocument(const std::wstring& path);

// Open a URL in the default browser
bool openUrl(const std::wstring& url);

// Run a command in the background (detached process)
bool runDetached(const std::wstring& command,
                const std::vector<std::wstring>& args,
                const std::wstring& workingDir = L"");

} // namespace ProcessUtils

} // namespace PlatformLayer
