#pragma once

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
}