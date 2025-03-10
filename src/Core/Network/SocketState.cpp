#include "Ailurus/Network/SocketState.h"
#include <unordered_map>

namespace Ailurus
{
    static std::unordered_map<SocketState, std::string> gSocketEnumStringMap = {
        {SocketState::Success, "Success"},
        {SocketState::InvalidSocket, "InvalidSocket"},
        {SocketState::Busy, "Busy"},
        {SocketState::Disconnect, "Disconnect"},
        {SocketState::AddressFamilyNotMatch, "AddressFamilyNotMatch"},
        {SocketState::Error, "Error"},
    };

    static std::string UnknownSocketState = "Unknown";

    const std::string& SocketStateUtil::GetName(SocketState state)
    {
        auto itr = gSocketEnumStringMap.find(state);
        if (itr == gSocketEnumStringMap.end())
            return UnknownSocketState;

        return itr->second;
    }
}
