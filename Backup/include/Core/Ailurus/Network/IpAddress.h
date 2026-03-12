#pragma once

#include <string>
#include <cstdint>
#include <optional>

namespace Ailurus
{
    class IpAddress
    {
    public:
        enum class Family
        {
            IpV4,
            IpV6
        };

        static int constexpr IPV6_ADDR_SIZE_BYTE = 16;

    public:
        // V4 constructor
        explicit IpAddress(uint32_t hostOrderIp);
        IpAddress(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);

        // V6 constructor
        explicit IpAddress(const uint8_t* pAddr, uint32_t scopeId = 0);

        // operator
        friend bool operator==(const IpAddress& left, const IpAddress& right);
        friend bool operator!=(const IpAddress& left, const IpAddress& right);

    public:
        Family GetFamily() const;
        uint32_t GetV4Addr() const;
        const uint8_t* GetV6Addr() const;
        uint32_t GetV6ScopeId() const;
        std::string ToString() const;

    public:
        static IpAddress V4_LOCAL_HOST;
        static IpAddress V4_ANY;
        static IpAddress V4_BROADCAST;

        static std::optional<IpAddress> TryParse(const std::string& str);

    private:
        union IpData
        {
            uint32_t ipV4Data; // network order
            uint8_t ipV6Data[IPV6_ADDR_SIZE_BYTE];
        };

        Family _addrFamily;
        IpData _data;
        uint32_t _scopeId;
    };

    bool operator==(const IpAddress& left, const IpAddress& right);
    bool operator!=(const IpAddress& left, const IpAddress& right);
}
