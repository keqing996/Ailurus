#include <cstdint>
#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/Platform/Windows/WindowsDefine.h"
#include "Ailurus/System/Process.h"
#include "Ailurus/Utility/String.h"
#include <functional>
#include <Psapi.h>
#undef CreateProcess

namespace Ailurus
{
    static std::optional<size_t> ReadPipe(HANDLE pipe, char* buffer, int maxSize)
    {
        if (buffer == nullptr)
            return std::nullopt;

        DWORD readBytes = 0;
        const bool state = ::ReadFile(pipe, buffer, maxSize, &readBytes, nullptr);
        if (!state || readBytes == 0)
            return std::nullopt;

        return readBytes;
    }

    bool Process::IsRunning() const
    {
        HANDLE hProcess = reinterpret_cast<HANDLE>(handle);
        if (hProcess == nullptr || hProcess == INVALID_HANDLE_VALUE)
            return false;

        DWORD result = ::WaitForSingleObject(hProcess, 0);
        return result != WAIT_OBJECT_0;
    }

    int Process::WaitFinish() const
    {
        ::WaitForSingleObject(reinterpret_cast<HANDLE>(handle), INFINITE);

        DWORD dwExitCode = 0;
        ::GetExitCodeProcess(reinterpret_cast<HANDLE>(handle), &dwExitCode);

        ::CloseHandle(reinterpret_cast<HANDLE>(stdinPipe));
        ::CloseHandle(reinterpret_cast<HANDLE>(stdoutPipe));
        ::CloseHandle(reinterpret_cast<HANDLE>(stderrPipe));
        return dwExitCode;
    }

    bool Process::WriteStdin(const char* buffer, size_t size) const
    {
        if (buffer == nullptr)
            return false;

        DWORD realWrite = 0;
        const bool success = ::WriteFile(reinterpret_cast<HANDLE>(stdinPipe), buffer, size, &realWrite, nullptr);
        if (!success)
            return false;

        return realWrite;
    }

    std::optional<size_t> Process::ReadStdout(char* buffer, size_t maxSize) const
    {
        return ReadPipe(reinterpret_cast<HANDLE>(stdoutPipe), buffer, maxSize);
    }

    std::optional<size_t> Process::ReadStderr(char* buffer, size_t maxSize) const
    {
        return ReadPipe(reinterpret_cast<HANDLE>(stderrPipe), buffer, maxSize);
    }

    int32_t Process::GetCurrentProcessId()
    {
        return ::GetCurrentProcessId();
    }

    std::string Process::GetProcessName(int32_t pid)
    {
        HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess == nullptr)
            return "";

        WCHAR processName[MAX_PATH];
        if (::GetModuleBaseNameW(hProcess, nullptr, processName, sizeof(processName) / sizeof(WCHAR)))
        {
            ::CloseHandle(hProcess);
            return String::WideStringToString(std::wstring(processName));
        }

        ::CloseHandle(hProcess);
        return "";
    }

    std::optional<Process> Process::Create(const std::string& exeName, const std::vector<std::string>& argv)
    {
        HANDLE hChildStdIn_Read = nullptr;
        HANDLE hChildStdIn_Write = nullptr;
        HANDLE hChildStdOut_Read = nullptr;
        HANDLE hChildStdOut_Write = nullptr;
        HANDLE hChildStdErr_Read = nullptr;
        HANDLE hChildStdErr_Write = nullptr;

        SECURITY_ATTRIBUTES securityAttributes;
        ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        securityAttributes.bInheritHandle = true;   // Set the bInheritHandle flag so pipe handles are inherited.
        securityAttributes.lpSecurityDescriptor = nullptr;

        auto CloseAllHandle = [&]() -> void
        {
            if (hChildStdIn_Read) ::CloseHandle(hChildStdIn_Read);
            if (hChildStdIn_Write) ::CloseHandle(hChildStdIn_Write);
            if (hChildStdOut_Read) ::CloseHandle(hChildStdOut_Read);
            if (hChildStdOut_Write) ::CloseHandle(hChildStdOut_Write);
            if (hChildStdErr_Read) ::CloseHandle(hChildStdErr_Read);
            if (hChildStdErr_Write) ::CloseHandle(hChildStdErr_Write);
        };

        if (!::CreatePipe(&hChildStdOut_Read,   &hChildStdOut_Write,    &securityAttributes, 0) ||
            !::CreatePipe(&hChildStdErr_Read,   &hChildStdErr_Write,    &securityAttributes, 0) ||
            !::CreatePipe(&hChildStdIn_Read,    &hChildStdIn_Write,     &securityAttributes, 0) ||
            !::SetHandleInformation(hChildStdOut_Read, HANDLE_FLAG_INHERIT, 0) ||
            !::SetHandleInformation(hChildStdErr_Read, HANDLE_FLAG_INHERIT, 0) ||
            !::SetHandleInformation(hChildStdIn_Write, HANDLE_FLAG_INHERIT, 0))
        {
            CloseAllHandle();
            return std::nullopt;
        }

        STARTUPINFOW startupInfo;
        ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
        startupInfo.cb = sizeof(STARTUPINFO);
        startupInfo.hStdError = hChildStdOut_Write;
        startupInfo.hStdOutput = hChildStdOut_Write;
        startupInfo.hStdInput = hChildStdIn_Read;
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION processInfo;
        ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

        std::string totalCmd = exeName + ' ' + String::Join(argv, ' ');
        std::wstring commandLineW = String::StringToWideString(totalCmd);

        // Start the child process.
        if(!::CreateProcessW(
                nullptr,
                const_cast<LPWSTR>(commandLineW.c_str()),
                nullptr,
                nullptr,
                TRUE,
                0,
                nullptr,
                nullptr,
                &startupInfo,
                &processInfo ))
        {
            CloseAllHandle();
            return std::nullopt;
        }

        // Before close child process's handles:
        // ╔══════════════════╗                ╔══════════════════╗
        // ║  Parent Process  ║                ║  Child Process   ║
        // ╠══════════════════╣                ╠══════════════════╣
        // ║                  ║                ║                  ║
        // ║  hStdInPipeWrite ╟───────────────>║  hStdInPipeRead  ║
        // ║                  ║                ║                  ║
        // ║  hStdOutPipeRead ║<───────────────╢ hStdOutPipeWrite ║
        // ║                  ║                ║                  ║
        // ║  hStdErrPipeRead ║<───────────────╢ hStdErrPipeWrite ║
        // ║                  ║                ║                  ║
        // ╚══════════════════╝                ╚══════════════════╝

        ::CloseHandle(hChildStdIn_Read);
        ::CloseHandle(hChildStdOut_Write);
        ::CloseHandle(hChildStdErr_Write);

        // After close child process's handles:
        // ╔══════════════════╗                ╔══════════════════╗
        // ║  Parent Process  ║                ║  Child Process   ║
        // ╠══════════════════╣                ╠══════════════════╣
        // ║                  ║                ║                  ║
        // ║  hStdInPipeWrite ╟───────────────>║                  ║
        // ║                  ║                ║                  ║
        // ║  hStdOutPipeRead ║<───────────────╢                  ║
        // ║                  ║                ║                  ║
        // ║  hStdErrPipeRead ║<───────────────╢                  ║
        // ║                  ║                ║                  ║
        // ╚══════════════════╝                ╚══════════════════╝

        Process result;
        result.handle = reinterpret_cast<int64_t>(processInfo.hProcess);
        result.stdinPipe = reinterpret_cast<int64_t>(hChildStdIn_Write);
        result.stdoutPipe = reinterpret_cast<int64_t>(hChildStdOut_Read);
        result.stderrPipe = reinterpret_cast<int64_t>(hChildStdErr_Read);
        return result;
    }
}

#endif