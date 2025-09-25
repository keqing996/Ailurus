
#include "Ailurus/Network/EndPoint.h"

namespace Ailurus
{
    EndPoint::EndPoint(const IpAddress& ip, uint16_t port)
        : _ip(ip)
        , _port(port)
        , _v6ScopeId(ip.GetFamily() == IpAddress::Family::IpV6 ? ip.GetV6ScopeId() : 0)
    {
    }

    EndPoint::EndPoint(const IpAddress& ip, uint16_t port, uint32_t scopeId)
        : _ip(ip)
        , _port(port)
        , _v6ScopeId(scopeId)
    {
    }

    const IpAddress& EndPoint::GetIp() const
    {
        return _ip;
    }

    IpAddress::Family EndPoint::GetAddressFamily() const
    {
        return _ip.GetFamily();
    }

    uint16_t EndPoint::GetPort() const
    {
        return _port;
    }

    uint32_t EndPoint::GetV6ScopeId() const
    {
        return _v6ScopeId;
    }

    bool operator==(const EndPoint& left, const EndPoint& right)
    {
        if (left._port != right._port)
            return false;

        if (left._ip != right._ip)
            return false;

        if (left.GetAddressFamily() == IpAddress::Family::IpV6
            && left._v6ScopeId != right._v6ScopeId)
            return false;

        return true;
    }

    bool operator!=(const EndPoint& left, const EndPoint& right)
    {
        return !(left == right);
    }
}
