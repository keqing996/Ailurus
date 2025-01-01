#include <cstdint>
#include "Ailurus/PlatformDefine.h"

#if PLATFORM_WINDOWS

#include "Ailurus/Platform/Windows/WindowsDefine.h"
#include "Ailurus/System/Process.h"
#include "Ailurus/Utility/String.h"
#include <functional>
#include <Psapi.h>
#undef CreateProcess

namespace Ailurus
{
    int32_t Process::GetCurrentProcessId()
    {
        return ::GetCurrentProcessId();
    }

    ProcessHandle Process::GetProcessHandle(int32_t processId)
    {
        ProcessHandle handle{};
        handle.handle = ::OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
        return handle;
    }

    void Process::ReleaseProcessHandle(const ProcessHandle& pProcess)
    {
        ::CloseHandle(pProcess.handle);
    }

    std::string Process::GetProcessName(const ProcessHandle& hProcess)
    {
        wchar_t nameBuffer[256 + 1] = {};
        const DWORD length = ::GetProcessImageFileNameW(hProcess.handle, nameBuffer, 256);
        if (length == 0)
            return {};

        return String::WideStringToString(nameBuffer);
    }

    std::optional<int> Process::CreateProcessAndWaitFinish(const std::string& commandLine)
    {
        STARTUPINFOW si;
        ::ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;
        ::ZeroMemory( &pi, sizeof(pi) );

        std::wstring commandLineW = String::StringToWideString(commandLine);

        // Start the child process.
        if(!::CreateProcessW(
                nullptr,                                    // No module name (use command line)
                const_cast<LPWSTR>(commandLineW.c_str()),   // Command line
                nullptr,                                    // Process handle not inheritable
                nullptr,                                    // Thread handle not inheritable
                FALSE,                                      // Set handle inheritance to FALSE
                0,                                          // No creation flags
                nullptr,                                    // Use parent's environment block
                nullptr,                                    // Use parent's starting directory
                &si,                                        // Pointer to STARTUPINFO structure
                &pi )                                       // Pointer to PROCESS_INFORMATION structure
                )
        {
            return std::nullopt;
        }

        ::WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD dwExitCode = 0;
        ::GetExitCodeProcess(pi.hProcess, &dwExitCode);

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
        return dwExitCode;
    }

    bool Process::CreateProcessAndDetach(const std::string& commandLine)
    {
        STARTUPINFOW si;
        ::ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;
        ::ZeroMemory( &pi, sizeof(pi) );

        std::wstring commandLineW = String::StringToWideString(commandLine);

        // Start the child process.
        if(!::CreateProcessW(
                nullptr,                                    // No module name (use command line)
                const_cast<LPWSTR>(commandLineW.c_str()),   // Command line
                nullptr,                                    // Process handle not inheritable
                nullptr,                                    // Thread handle not inheritable
                FALSE,                                      // Set handle inheritance to FALSE
                0,                                          // No creation flags
                nullptr,                                    // Use parent's environment block
                nullptr,                                    // Use parent's starting directory
                &si,                                        // Pointer to STARTUPINFO structure
                &pi )                                       // Pointer to PROCESS_INFORMATION structure
                )
        {
            return false;
        }

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);

        return true;
    }

    std::optional<ProcessInfo>  Process::CreateProcessWithPipe(const std::string& commandLine)
    {
        HANDLE hChildStdIn_Read = nullptr;
        HANDLE hChildStdIn_Write = nullptr;
        HANDLE hChildStdOut_Read = nullptr;
        HANDLE hChildStdOut_Write = nullptr;

        SECURITY_ATTRIBUTES securityAttributes;
        ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        securityAttributes.bInheritHandle = true;   // Set the bInheritHandle flag so pipe handles are inherited.
        securityAttributes.lpSecurityDescriptor = nullptr;

        // Create one-way pipe for child process STDOUT
        if (!::CreatePipe(&hChildStdOut_Read, &hChildStdOut_Write, &securityAttributes, 0))
            return std::nullopt;

        // Ensure read handle to pipe for STDOUT is not inherited
        if (!::SetHandleInformation(hChildStdOut_Read, HANDLE_FLAG_INHERIT, 0))
            return std::nullopt;

        // Create one-way pipe for child process STDIN
        if (!::CreatePipe(&hChildStdIn_Read, &hChildStdIn_Write, &securityAttributes, 0))
            return std::nullopt;

        // Ensure write handle to pipe for STDIN is not inherited
        if (!::SetHandleInformation(hChildStdIn_Write, HANDLE_FLAG_INHERIT, 0))
            return std::nullopt;

        STARTUPINFOW startupInfo;
        ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
        startupInfo.cb = sizeof(STARTUPINFO);
        startupInfo.hStdError = hChildStdOut_Write;
        startupInfo.hStdOutput = hChildStdOut_Write;
        startupInfo.hStdInput = hChildStdIn_Read;
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION processInfo;
        ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

        std::wstring commandLineW = String::StringToWideString(commandLine);

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
        // ╚══════════════════╝                ╚══════════════════╝

        ::CloseHandle(hChildStdIn_Read);
        ::CloseHandle(hChildStdOut_Write);

        // After close child process's handles:
        // ╔══════════════════╗                ╔══════════════════╗
        // ║  Parent Process  ║                ║  Child Process   ║
        // ╠══════════════════╣                ╠══════════════════╣
        // ║                  ║                ║                  ║
        // ║  hStdInPipeWrite ╟───────────────>║                  ║
        // ║                  ║                ║                  ║
        // ║  hStdOutPipeRead ║<───────────────╢                  ║
        // ║                  ║                ║                  ║
        // ╚══════════════════╝                ╚══════════════════╝

        ProcessInfo result;
        result.hProcess.handle = processInfo.hProcess;
        result.hThread.handle = processInfo.hThread;
        result.hPipeChildStdOut.handle = hChildStdOut_Read;
        result.hPipeChildStdIn.handle = hChildStdIn_Write;
        return result;
    }

    int Process::WaitProcessFinish(const ProcessInfo& processInfo)
    {
        // Close left handle
        ::CloseHandle(processInfo.hPipeChildStdIn.handle);
        ::CloseHandle(processInfo.hPipeChildStdOut.handle);

        // Wait until child process exits
        ::WaitForSingleObject(processInfo.hProcess.handle, INFINITE);

        DWORD dwExitCode = 0;
        ::GetExitCodeProcess(processInfo.hProcess.handle, &dwExitCode);

        // Close process and thread handles
        ::CloseHandle(processInfo.hProcess.handle);
        ::CloseHandle(processInfo.hThread.handle);

        return dwExitCode;
    }

    std::optional<int> Process::SendDataToProcess(const ProcessInfo& processInfo, const char* buffer, int sendSize)
    {
        if (buffer == nullptr)
            return std::nullopt;

        DWORD realWrite = 0;
        const bool success = ::WriteFile(processInfo.hPipeChildStdIn.handle, buffer, sendSize, &realWrite, nullptr);
        if (!success)
            return std::nullopt;

        return realWrite;
    }

    std::optional<int> Process::ReadDataFromProcess(const ProcessInfo& processInfo, char* buffer, int bufferSize)
    {
        if (buffer == nullptr)
            return std::nullopt;

        DWORD readBytes = 0;
        const bool state = ::ReadFile(processInfo.hPipeChildStdOut.handle, buffer, bufferSize, &readBytes, nullptr);
        if (!state || readBytes == 0)
            return std::nullopt;

        return readBytes;
    }

    void Process::DetachProcess(const ProcessInfo& processInfo)
    {
        // Close left handle
        ::CloseHandle(processInfo.hPipeChildStdIn.handle);
        ::CloseHandle(processInfo.hPipeChildStdOut.handle);

        // Close process and thread handles
        ::CloseHandle(processInfo.hProcess.handle);
        ::CloseHandle(processInfo.hThread.handle);
    }
}

#endif