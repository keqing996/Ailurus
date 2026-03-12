#pragma once

#include <Ailurus/Network/EndPoint.h>
#include <Ailurus/Network/IpAddress.h>
#include "../Platform/PlatformApi.h"

namespace Ailurus
{
    class SocketUtil
    {
    public:
        SocketUtil() = delete;

    public:
        static int GetAddressFamily(IpAddress::Family family);

        static std::pair<int, int> GetTcpProtocol();

        static std::pair<int, int> GetUdpProtocol();

        static bool CreateSocketAddress(const EndPoint& endpoint, sockaddr* pResult, SockLen* structLen);
    };
}