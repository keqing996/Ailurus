#include "Ailurus/Network/SocketState.h"
#include <array>

namespace Ailurus
{
    namespace
    {
        // Ordered lookup table avoids the static allocator churn of an unordered_map
        // while keeping the API surface untouched.
        static const std::array<std::string, 6> gSocketEnumStringMap = {
            "Success",
            "InvalidSocket",
            "Busy",
            "Disconnect",
            "AddressFamilyNotMatch",
            "Error",
        };

        static const std::string UnknownSocketState = "Unknown";
    }

    const std::string& SocketStateUtil::GetName(SocketState state)
    {
        const auto index = static_cast<size_t>(state);
        if (index < gSocketEnumStringMap.size())
            return gSocketEnumStringMap[index];

        return UnknownSocketState;
    }
}
