#pragma once

#include "../../PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include <string>
#include <optional>
#include <vector>
#include <shobjidl.h>

namespace Ailurus
{
    class Dialog
    {
    public:
        Dialog() = delete;

    public:
        struct FileTypeFilter
        {
            std::string name;
            std::string suffix;
        };

    public:
        static std::optional<std::string> ShowShellDialogAndGetResult(IFileDialog* pFileDialog);

        static std::optional<std::string> OpenFile(const std::string& titleMsg, const std::vector<FileTypeFilter>* pFilter);

        static std::optional<std::string> SaveFile(const std::string& titleMsg, const std::string& defaultName, const std::vector<FileTypeFilter>* pFilter);

        static std::optional<std::string> OpenDirectory(const std::string& titleMsg);
    };

}

#endif