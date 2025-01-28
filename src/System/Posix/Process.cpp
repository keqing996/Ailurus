#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_SUPPORT_POSIX

#include <string>
#include <optional>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Ailurus/System/Process.h"

auto Ailurus::Process::GetCurrentProcessId() -> int32_t
{
    return ::getpid();
}

auto Ailurus::Process::GetProcessName(const ProcessHandle& hProcess) -> std::string
{
    char buf[256];
    ::snprintf(buf, sizeof(buf), "/proc/%ld/status", reinterpret_cast<long>(hProcess));

    return {};
}

auto Ailurus::Process::CreateProcessAndWaitFinish(const std::string &commandLine) -> std::optional<int>
{
    pid_t pid = ::fork();
    if (pid == -1)
        return std::nullopt;

    if (pid == 0) // Child process
    {
        ::execlp(commandLine.c_str(), commandLine.c_str(), (char*)nullptr);
        _exit(1); // If execlp fails
    }
    else // Parent process
    {
        int status;
        // Wait for child to finish
        ::waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

auto Ailurus::Process::CreateProcessAndDetach(const std::string &commandLine) -> bool
{
    pid_t pid = ::fork();
    if (pid == -1)
        return false;

    if (pid == 0) // Child process
    {
        ::setsid();
        ::execlp(commandLine.c_str(), commandLine.c_str(), (char*)nullptr);
        _exit(1);
    }

    // Parent continues without waiting
    return true;
}

auto Ailurus::Process::CreateProcessWithPipe(const std::string &commandLine) -> std::optional<ProcessInfo>
{
    int pipeIn[2], pipeOut[2];
    if (::pipe(pipeIn) == -1 || ::pipe(pipeOut) == -1)
        return std::nullopt;

    pid_t pid = ::fork();
    if (pid == -1)
        return std::nullopt;

    if (pid == 0) // Child process
    {
        ::close(pipeIn[1]); // Close unused write end
        ::close(pipeOut[0]); // Close unused read end
        ::dup2(pipeIn[0], STDIN_FILENO); // Redirect stdin to pipe
        ::dup2(pipeOut[1], STDOUT_FILENO); // Redirect stdout to pipe

        ::execlp(commandLine.c_str(), commandLine.c_str(), (char*)nullptr);
        _exit(1); // If execlp fails
    }

    ProcessInfo processInfo {};
    processInfo.hProcess = reinterpret_cast<void*>(pid);
    processInfo.hPipeChildStdIn = reinterpret_cast<void*>(pipeIn[0]);
    processInfo.hPipeChildStdOut = reinterpret_cast<void*>(pipeOut[1]);

    return processInfo;
}

auto Ailurus::Process::WaitProcessFinish(const ProcessInfo &processInfo) -> int
{
    int status;
    pid_t pid = reinterpret_cast<pid_t>(processInfo.hProcess);
    ::waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

auto Ailurus::Process::SendDataToProcess(const ProcessInfo &processInfo, const char *buffer,
    int sendSize) -> std::optional<int>
{
    return ::write(reinterpret_cast<int>(processInfo.hPipeChildStdIn), buffer, sendSize);
}

auto Ailurus::Process::ReadDataFromProcess(const ProcessInfo &processInfo, char *buffer,
    int bufferSize) -> std::optional<int>
{
    return ::read(reinterpret_cast<int>(processInfo.hPipeChildStdOut), buffer, bufferSize);
}

auto Ailurus::Process::DetachProcess(const ProcessInfo &processInfo) -> void
{
    ::close(reinterpret_cast<int>(processInfo.hPipeChildStdIn));
    ::close(reinterpret_cast<int>(processInfo.hPipeChildStdOut));
}

#endif
