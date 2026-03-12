#pragma once

#include <string>

namespace Ailurus
{
    enum class SocketState
    {
        Success,
        InvalidSocket,
        Busy,
        Disconnect,
        AddressFamilyNotMatch,
        Error
    };

    class SocketStateUtil
    {
    public:
        SocketStateUtil() = delete;

        static const std::string& GetName(SocketState state);
    };
}