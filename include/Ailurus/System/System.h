#pragma once

#include <string>
#include <cstdint>

namespace Ailurus
{
    class System
    {
    public:
        System() = delete;

    public:
        static auto GetMachineName() -> std::string;
        static auto GetCurrentUserName() -> std::string;

    public:
        static auto GetEnvironmentVariable(const std::string& keyName) -> std::string;
        static auto SetEnvironmentVariable(const std::string& keyName, const std::string& value) -> void;

    public:
        static auto GetHomeDirectory() -> std::string;
        static auto GetCurrentDirectory() -> std::string;
        static auto SetCurrentDirectory(const std::string& path) -> bool;
        static auto GetExecutableDirectory() -> std::string;
        static auto GetTempDirectory() -> std::string;

    public:
        static auto GetLastError() -> uint32_t;
    };
}