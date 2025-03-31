#pragma once

#include "EndPoint.h"

#include <vector>

namespace Ailurus
{
    class DNS
    {
    public:
        DNS() = delete;

    public:
        // Support both name & addr
        static std::vector<IpAddress> GetIpAddressByHostName(const std::string& str);
        static std::vector<IpAddress> GetLocalIpAddress();
        static std::string GetHostName();
    };
}