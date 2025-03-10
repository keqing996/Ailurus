#pragma once

#include "IpAddress.h"

namespace Ailurus
{
    class EndPoint
    {
    public:
        EndPoint(const IpAddress& ip, uint16_t port);
        EndPoint(const IpAddress& ip, uint16_t port, uint32_t scopeId);

        friend bool operator==(const EndPoint& left, const EndPoint& right);
        friend bool operator!=(const EndPoint& left, const EndPoint& right);

    public:
        const IpAddress& GetIp() const;
        IpAddress::Family GetAddressFamily() const;
        uint16_t GetPort() const;
        uint32_t GetV6ScopeId() const;

    private:
        IpAddress _ip;
        uint16_t _port;
        uint32_t _v6ScopeId;
    };

    bool operator==(const EndPoint& left, const EndPoint& right);
    bool operator!=(const EndPoint& left, const EndPoint& right);


}
