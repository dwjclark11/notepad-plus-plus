// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Process.h"
#include "../../MISC/Process/Processus.h"
#include "../../MISC/Common/Common.h"
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <sstream>

namespace Platform {

// ============================================================================
// Windows Implementation of IProcess
// ============================================================================
class ProcessWin32 : public IProcess {
public:
    ProcessInfo run(const std::wstring& command,
                   const std::wstring& args,
                   const ProcessOptions& options) override {
        ProcessInfo info;

        if (options.elevated) {
            // Use ShellExecute for elevation
            HINSTANCE result = ::ShellExecuteW(nullptr, L"runas",
                                              command.c_str(),
                                              args.c_str(),
                                              options.workingDir.empty() ? nullptr : options.workingDir.c_str(),
                                              options.hidden ? SW_HIDE : SW_SHOWNORMAL);

            // ShellExecute returns a handle > 32 on success
            info.success = (reinterpret_cast<intptr_t>(result) > 32);
        } else {
            // Use CreateProcess for better control
            STARTUPINFOW si = {};
            si.cb = sizeof(si);
            if (options.hidden) {
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
            }

            PROCESS_INFORMATION pi = {};

            std::wstring cmdLine = command;
            if (!args.empty()) {
                cmdLine += L" " + args;
            }

            // Make a writable copy of command line
            std::vector<wchar_t> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
            cmdLineBuffer.push_back(L'\0');

            BOOL created = ::CreateProcessW(
                nullptr,                    // Application name
                cmdLineBuffer.data(),       // Command line
                nullptr,                    // Process security attributes
                nullptr,                    // Thread security attributes
                FALSE,                      // Inherit handles
                options.hidden ? CREATE_NO_WINDOW : 0,
                nullptr,                    // Environment
                options.workingDir.empty() ? nullptr : options.workingDir.c_str(),
                &si,
                &pi
            );

            if (created) {
                info.pid = pi.dwProcessId;
                info.nativeHandle = pi.hProcess;
                info.command = command;
                info.success = true;

                // Close thread handle, we don't need it
                ::CloseHandle(pi.hThread);

                if (options.waitForExit) {
                    ::WaitForSingleObject(pi.hProcess, INFINITE);
                }
            } else {
                info.success = false;
            }
        }

        return info;
    }

    ProcessResult runSync(const std::wstring& command,
                         const std::wstring& args,
                         const ProcessOptions& options) override {
        ProcessResult result;

        if (options.elevated) {
            // Use ShellExecuteEx with wait
            SHELLEXECUTEINFOW ShExecInfo = {};
            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
            ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
            ShExecInfo.lpVerb = L"runas";
            ShExecInfo.lpFile = command.c_str();
            ShExecInfo.lpParameters = args.empty() ? nullptr : args.c_str();
            ShExecInfo.lpDirectory = options.workingDir.empty() ? nullptr : options.workingDir.c_str();
            ShExecInfo.nShow = options.hidden ? SW_HIDE : SW_SHOWNORMAL;

            if (::ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess) {
                ::WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

                DWORD exitCode;
                if (::GetExitCodeProcess(ShExecInfo.hProcess, &exitCode)) {
                    result.exitCode = exitCode;
                    result.success = true;
                }

                ::CloseHandle(ShExecInfo.hProcess);
            } else {
                result.success = false;
                result.errorMessage = L"Failed to execute elevated process";
            }
        } else {
            // Use CreateProcess with output capture if needed
            HANDLE hStdOutRead = nullptr;
            HANDLE hStdOutWrite = nullptr;
            HANDLE hStdErrWrite = nullptr;

            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = nullptr;

            if (options.captureOutput) {
                // Create pipe for stdout
                if (!::CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
                    result.success = false;
                    result.errorMessage = L"Failed to create pipe";
                    return result;
                }

                // Ensure read handle is not inherited
                ::SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

                if (options.mergeStderr) {
                    hStdErrWrite = hStdOutWrite;
                }
            }

            STARTUPINFOW si = {};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = hStdOutWrite ? hStdOutWrite : ::GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = hStdErrWrite ? hStdErrWrite : ::GetStdHandle(STD_ERROR_HANDLE);

            if (options.hidden) {
                si.dwFlags |= STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
            }

            PROCESS_INFORMATION pi = {};

            std::wstring cmdLine = command;
            if (!args.empty()) {
                cmdLine += L" " + args;
            }

            std::vector<wchar_t> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
            cmdLineBuffer.push_back(L'\0');

            BOOL created = ::CreateProcessW(
                nullptr,
                cmdLineBuffer.data(),
                nullptr,
                nullptr,
                options.captureOutput ? TRUE : FALSE,
                options.hidden ? CREATE_NO_WINDOW : 0,
                nullptr,
                options.workingDir.empty() ? nullptr : options.workingDir.c_str(),
                &si,
                &pi
            );

            if (created) {
                // Close write ends in this process
                if (hStdOutWrite) {
                    ::CloseHandle(hStdOutWrite);
                    hStdOutWrite = nullptr;
                }

                // Read output if capturing
                if (options.captureOutput && hStdOutRead) {
                    char buffer[4096];
                    DWORD bytesRead;
                    std::ostringstream output;

                    while (::ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        output << buffer;
                    }

                    result.output = output.str();
                    ::CloseHandle(hStdOutRead);
                }

                ::WaitForSingleObject(pi.hProcess, INFINITE);

                DWORD exitCode;
                if (::GetExitCodeProcess(pi.hProcess, &exitCode)) {
                    result.exitCode = exitCode;
                    result.success = true;
                }

                ::CloseHandle(pi.hProcess);
                ::CloseHandle(pi.hThread);
            } else {
                result.success = false;
                result.errorMessage = L"Failed to create process";

                if (hStdOutRead) ::CloseHandle(hStdOutRead);
                if (hStdOutWrite) ::CloseHandle(hStdOutWrite);
            }
        }

        return result;
    }

    bool shellOpen(const std::wstring& fileOrUrl,
                  const std::wstring& args,
                  bool elevated) override {
        HINSTANCE result = ::ShellExecuteW(
            nullptr,
            elevated ? L"runas" : L"open",
            fileOrUrl.c_str(),
            args.empty() ? nullptr : args.c_str(),
            nullptr,
            SW_SHOWNORMAL
        );

        return reinterpret_cast<intptr_t>(result) > 32;
    }

    bool shellOpenWith(const std::wstring& application,
                      const std::wstring& file,
                      const std::wstring& args,
                      bool elevated) override {
        std::wstring fullArgs = L"\"" + file + L"\"";
        if (!args.empty()) {
            fullArgs += L" " + args;
        }

        HINSTANCE result = ::ShellExecuteW(
            nullptr,
            elevated ? L"runas" : L"open",
            application.c_str(),
            fullArgs.c_str(),
            nullptr,
            SW_SHOWNORMAL
        );

        return reinterpret_cast<intptr_t>(result) > 32;
    }

    bool waitForProcess(const ProcessInfo& info, uint32_t timeoutMs) override {
        if (!info.nativeHandle) return false;

        HANDLE hProcess = static_cast<HANDLE>(info.nativeHandle);
        DWORD result = ::WaitForSingleObject(hProcess, timeoutMs);

        return result == WAIT_OBJECT_0;
    }

    bool terminateProcess(const ProcessInfo& info, bool force) override {
        (void)force;  // Windows doesn't distinguish between graceful and force

        if (!info.nativeHandle) {
            // Try to open process by PID
            HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, info.pid);
            if (!hProcess) return false;

            BOOL terminated = ::TerminateProcess(hProcess, 1);
            ::CloseHandle(hProcess);
            return terminated != FALSE;
        }

        HANDLE hProcess = static_cast<HANDLE>(info.nativeHandle);
        return ::TerminateProcess(hProcess, 1) != FALSE;
    }

    bool isProcessRunning(const ProcessInfo& info) override {
        return isProcessRunning(info.pid);
    }

    bool getExitCode(const ProcessInfo& info, uint32_t& exitCode) override {
        if (!info.nativeHandle) return false;

        HANDLE hProcess = static_cast<HANDLE>(info.nativeHandle);
        DWORD code;
        if (::GetExitCodeProcess(hProcess, &code)) {
            exitCode = code;
            return true;
        }
        return false;
    }

    bool isProcessRunning(uint32_t pid) override {
        if (pid == 0) return false;

        HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!hProcess) return false;

        DWORD exitCode;
        BOOL gotCode = ::GetExitCodeProcess(hProcess, &exitCode);
        ::CloseHandle(hProcess);

        return gotCode && exitCode == STILL_ACTIVE;
    }

    uint32_t getCurrentProcessId() override {
        return ::GetCurrentProcessId();
    }

    std::wstring getCurrentProcessPath() override {
        wchar_t buffer[MAX_PATH];
        DWORD size = ::GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (size > 0 && size < MAX_PATH) {
            return std::wstring(buffer, size);
        }
        return std::wstring();
    }

    bool isCurrentProcessElevated() override {
        HANDLE hToken = nullptr;
        if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            return false;
        }

        TOKEN_ELEVATION elevation;
        DWORD size;
        BOOL elevated = FALSE;

        if (::GetTokenInformation(hToken, TokenElevation, &elevation,
                                  sizeof(elevation), &size)) {
            elevated = elevation.TokenIsElevated;
        }

        ::CloseHandle(hToken);
        return elevated != FALSE;
    }

    bool restartElevated(const std::wstring& args) override {
        std::wstring exePath = getCurrentProcessPath();
        if (exePath.empty()) return false;

        HINSTANCE result = ::ShellExecuteW(
            nullptr,
            L"runas",
            exePath.c_str(),
            args.empty() ? nullptr : args.c_str(),
            nullptr,
            SW_SHOWNORMAL
        );

        return reinterpret_cast<intptr_t>(result) > 32;
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IProcess& IProcess::getInstance() {
    static ProcessWin32 instance;
    return instance;
}

// ============================================================================
// Static Helper Implementations
// ============================================================================
std::wstring IProcess::quoteArgument(const std::wstring& arg) {
    // Check if quoting is needed
    if (arg.find_first_of(L" \"\t\n\v") == std::wstring::npos) {
        return arg;
    }

    // Quote and escape the argument
    std::wstring result = L"\"";
    for (wchar_t c : arg) {
        if (c == L'"') {
            result += L"\\\"";
        } else if (c == L'\\') {
            result += L"\\\\";
        } else {
            result += c;
        }
    }
    result += L'"';
    return result;
}

std::wstring IProcess::buildCommandLine(const std::wstring& program,
                                       const std::vector<std::wstring>& args) {
    std::wstring cmdLine = quoteArgument(program);

    for (const auto& arg : args) {
        cmdLine += L" " + quoteArgument(arg);
    }

    return cmdLine;
}

std::wstring IProcess::findExecutable(const std::wstring& name) {
    wchar_t buffer[MAX_PATH];
    DWORD result = ::SearchPathW(nullptr, name.c_str(), L".exe", MAX_PATH, buffer, nullptr);
    if (result > 0 && result < MAX_PATH) {
        return std::wstring(buffer, result);
    }
    return std::wstring();
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace ProcessUtils {

ProcessResult execute(const std::wstring& command,
                     const std::vector<std::wstring>& args,
                     const std::wstring& workingDir) {
    std::wstring cmdLine = command;
    for (const auto& arg : args) {
        cmdLine += L" " + IProcess::quoteArgument(arg);
    }

    ProcessOptions options;
    options.workingDir = workingDir;
    options.captureOutput = true;
    options.waitForExit = true;

    return IProcess::getInstance().runSync(command, cmdLine.substr(command.length()), options);
}

bool openDocument(const std::wstring& path) {
    return IProcess::getInstance().shellOpen(path, L"", false);
}

bool openUrl(const std::wstring& url) {
    return IProcess::getInstance().shellOpen(url, L"", false);
}

bool runDetached(const std::wstring& command,
                const std::vector<std::wstring>& args,
                const std::wstring& workingDir) {
    std::wstring cmdLine;
    for (const auto& arg : args) {
        if (!cmdLine.empty()) cmdLine += L" ";
        cmdLine += IProcess::quoteArgument(arg);
    }

    ProcessOptions options;
    options.workingDir = workingDir;

    ProcessInfo info = IProcess::getInstance().run(command, cmdLine, options);
    return info.success;
}

} // namespace ProcessUtils

} // namespace Platform
