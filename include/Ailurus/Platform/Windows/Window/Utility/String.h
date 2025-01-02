#pragma once

#include <string>

namespace Utility
{
    std::string WideStringToString(const std::wstring& wStr);
    std::wstring StringToWideString(const std::string& str);
}
