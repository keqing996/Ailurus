#pragma once

#include <string>
#include <optional>

namespace Ailurus
{
    using ProcessHandle = void*;
    using PipeHandle = void*;

    struct ProcessInfo
    {
        ProcessHandle hProcess;
        PipeHandle hPipeChildStdIn;
        PipeHandle hPipeChildStdOut;
    };

    class Process
    {
    public:
        Process() = delete;

    public:
        static auto GetCurrentProcessId() -> int32_t;
        static auto GetProcessName(const ProcessHandle& hProcess) -> std::string;

        static auto CreateProcessAndWaitFinish(const std::string& commandLine) -> std::optional<int>;
        static auto CreateProcessAndDetach(const std::string& commandLine) -> bool;
        static auto CreateProcessWithPipe(const std::string& commandLine) -> std::optional<ProcessInfo>;
        static auto WaitProcessFinish(const ProcessInfo& processInfo) -> int;
        static auto SendDataToProcess(const ProcessInfo& processInfo, const char* buffer, int sendSize) -> std::optional<int>;
        static auto ReadDataFromProcess(const ProcessInfo& processInfo, char* buffer, int bufferSize) -> std::optional<int>;
        static auto DetachProcess(const ProcessInfo& processInfo) -> void;

    };

}