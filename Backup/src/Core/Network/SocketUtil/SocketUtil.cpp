#include "SocketUtil.h"

#include <cstring>

namespace Ailurus
{
    int SocketUtil::GetAddressFamily(IpAddress::Family family)
    {
        switch (family)
        {
            case IpAddress::Family::IpV4:
                return AF_INET;
            case IpAddress::Family::IpV6:
                return AF_INET6;
        }

        return AF_INET;
    }

    std::pair<int, int> SocketUtil::GetTcpProtocol()
    {
        return std::make_pair(IPPROTO_TCP, SOCK_STREAM);
    }

    std::pair<int, int> SocketUtil::GetUdpProtocol()
    {
        return std::make_pair(IPPROTO_UDP, SOCK_DGRAM);
    }

    bool SocketUtil::CreateSocketAddress(const EndPoint& endpoint, sockaddr* pResult, SockLen* structLen)
    {
        if (pResult == nullptr || structLen == nullptr)
            return false;

        switch (endpoint.GetAddressFamily())
        {
            case IpAddress::Family::IpV4:
            {
                auto* pAddr = reinterpret_cast<sockaddr_in*>(pResult);
                std::memset(pAddr, 0, sizeof(sockaddr_in));
                pAddr->sin_addr.s_addr = htonl(endpoint.GetIp().GetV4Addr());
                pAddr->sin_family = AF_INET;
                pAddr->sin_port = htons(endpoint.GetPort());
                *structLen = sizeof(sockaddr_in);
                return true;
            }
            case IpAddress::Family::IpV6:
            {
                auto* pAddr = reinterpret_cast<sockaddr_in6*>(pResult);
                std::memset(pAddr, 0, sizeof(sockaddr_in6));
                ::memcpy(&pAddr->sin6_addr, endpoint.GetIp().GetV6Addr(), IpAddress::IPV6_ADDR_SIZE_BYTE);
                pAddr->sin6_family = AF_INET6;
                pAddr->sin6_scope_id = endpoint.GetV6ScopeId();
                pAddr->sin6_port = htons(endpoint.GetPort());
                *structLen = sizeof(sockaddr_in6);
                return true;
            }
            default:
                return false;
        }
    }
}
