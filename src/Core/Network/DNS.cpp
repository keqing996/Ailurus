
#include "Ailurus/Network/DNS.h"
#include "Platform/PlatformApi.h"

namespace Ailurus
{
    std::vector<IpAddress> DNS::GetIpAddressByHostName(const std::string& str)
    {
        std::vector<IpAddress> result;

        addrinfo hints{};
        hints.ai_family = AF_UNSPEC; // ipv4 & ipv6

        addrinfo* pAddrResult = nullptr;
        if (::getaddrinfo(str.c_str(), nullptr, &hints, &pAddrResult) != 0)
            return result;

        if (pAddrResult == nullptr)
            return result;

        for (auto p = pAddrResult; p != nullptr; p = p->ai_next)
        {
            if (p->ai_family == AF_INET)
            {
                sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
                result.emplace_back(ntohl((ipv4->sin_addr).s_addr));
            }
            else if (p->ai_family == AF_INET6)
            {
                sockaddr_in6* ipv6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
                result.emplace_back((ipv6->sin6_addr).s6_addr);
            }
        }

        ::freeaddrinfo(pAddrResult);

        return result;
    }

    std::string DNS::GetHostName()
    {
        char hostname[256];
        if (::gethostname(hostname, sizeof(hostname)) == -1)
            return "";

        return hostname;
    }

    std::vector<IpAddress> DNS::GetLocalIpAddress()
    {
        return GetIpAddressByHostName(GetHostName());
    }
}
