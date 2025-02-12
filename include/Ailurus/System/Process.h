#pragma once

#include <string>
#include <optional>
#include <vector>

namespace Ailurus
{
    struct Process
    {
        using Handle = int64_t;
        using Pipe = int64_t;

    public:
        bool IsRunning() const;
        int WaitFinish() const;
        bool WriteStdin(const char* buffer, size_t size) const;
        std::optional<size_t> ReadStdout(char* buffer, size_t maxSize) const;
        std::optional<size_t> ReadStderr(char* buffer, size_t maxSize) const;

    public:
        Handle handle = -1;
        Pipe stdinPipe = -1;
        Pipe stdoutPipe = -1;
        Pipe stderrPipe = -1;

    public:
        static int32_t GetCurrentProcessId();
        static std::string GetProcessName(int32_t pid);
        static std::optional<Process> Create(const std::string& exeName, const std::vector<std::string>& argv);
    };

}
