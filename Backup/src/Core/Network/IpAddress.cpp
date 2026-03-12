
#include "Ailurus/Network/IpAddress.h"
#include "Platform/PlatformApi.h"

#include <cstring>

namespace Ailurus
{
    IpAddress IpAddress::V4_ANY (0, 0, 0, 0);
    IpAddress IpAddress::V4_BROADCAST (255, 255, 255, 255);
    IpAddress IpAddress::V4_LOCAL_HOST (127, 0,0 , 1);

    IpAddress::IpAddress(uint32_t hostOrderIp)
        : _addrFamily(Family::IpV4)
        , _data()
        , _scopeId(0)
    {
        _data.ipV4Data = htonl(hostOrderIp);
    }

    IpAddress::IpAddress(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
        : _addrFamily(Family::IpV4)
        , _data()
        , _scopeId(0)
    {
        uint32_t hostOrderIp = static_cast<uint32_t>(byte1) << 24;
        hostOrderIp |= static_cast<uint32_t>(byte2) << 16;
        hostOrderIp |= static_cast<uint32_t>(byte3) << 8;
        hostOrderIp |= static_cast<uint32_t>(byte4);
        _data.ipV4Data = htonl(hostOrderIp);
    }

    IpAddress::IpAddress(const uint8_t* pAddr, uint32_t scopeId)
        : _addrFamily(Family::IpV6)
        , _data()
        , _scopeId(scopeId)
    {
        ::memcpy(_data.ipV6Data, pAddr, IPV6_ADDR_SIZE_BYTE);
    }

    IpAddress::Family IpAddress::GetFamily() const
    {
        return _addrFamily;
    }

    uint32_t IpAddress::GetV4Addr() const
    {
        return ntohl(_data.ipV4Data);
    }

    const uint8_t* IpAddress::GetV6Addr() const
    {
        return _data.ipV6Data;
    }

    uint32_t IpAddress::GetV6ScopeId() const
    {
        return _addrFamily == Family::IpV6 ? _scopeId : 0;
    }

    std::string IpAddress::ToString() const
    {
        switch (_addrFamily)
        {
            case Family::IpV4:
            {
                in_addr address{};
                address.s_addr = _data.ipV4Data;

                char resultBuffer[INET_ADDRSTRLEN];

                ::inet_ntop(AF_INET, &address, resultBuffer, INET_ADDRSTRLEN);

                return resultBuffer;
            }
            case Family::IpV6:
            {
                in6_addr address{};

                ::memcpy(&address.s6_addr, _data.ipV6Data, IPV6_ADDR_SIZE_BYTE);

                char resultBuffer[INET6_ADDRSTRLEN];

                ::inet_ntop(AF_INET6, &address, resultBuffer, INET6_ADDRSTRLEN);

                std::string result(resultBuffer);
                if (_scopeId != 0)
                {
                    result.push_back('%');
                    result += std::to_string(_scopeId);
                }

                return result;
            }
        }

        return {};
    }

    std::optional<IpAddress> IpAddress::TryParse(const std::string& str)
    {
        in_addr destinationV4 {};
        if (::inet_pton(AF_INET, str.c_str(), &destinationV4) == 1)
            return IpAddress(ntohl(destinationV4.s_addr));

        in6_addr destinationV6 {};
        if (::inet_pton(AF_INET6, str.c_str(), &destinationV6) == 1)
            return IpAddress{ reinterpret_cast<const uint8_t*>(&destinationV6.s6_addr), 0 };

        return std::nullopt;
    }

    bool operator==(const IpAddress& left, const IpAddress& right)
    {
        if (left._addrFamily != right._addrFamily)
            return false;

        switch (left._addrFamily)
        {
            case IpAddress::Family::IpV4:
            {
                return left._data.ipV4Data == right._data.ipV4Data;
            }
            case IpAddress::Family::IpV6:
            {
                const auto* leftData = reinterpret_cast<const uint64_t*>(left._data.ipV6Data);
                const auto* rightData = reinterpret_cast<const uint64_t*>(right._data.ipV6Data);
                return leftData[0] == rightData[0] && leftData[1] == rightData[1] && left._scopeId == right._scopeId;
            }
        }

        return false;
    }

    bool operator!=(const IpAddress& left, const IpAddress& right)
    {
        return !(left == right);
    }
}
