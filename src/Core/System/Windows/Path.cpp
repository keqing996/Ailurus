#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/System/Path.h"
#include "Ailurus/System/System.h"
#include <filesystem>

namespace Ailurus
{
    std::string Path::ResolvePath(const std::string& path)
    {
        if (IsAbsolutePath(path))
        {
            return path;
        }

        // For relative paths, resolve against the resource root
        std::filesystem::path resourceRoot = GetResourceRootPath();
        resourceRoot = resourceRoot.remove_filename(); // Get directory of executable
        std::filesystem::path relativePath = path;
        std::filesystem::path resolved = resourceRoot / relativePath;
        
        return resolved.string();
    }

    std::string Path::GetResourceRootPath()
    {
        // For Windows, use the executable directory as resource root
        return System::GetExecutableDirectory();
    }

    bool Path::IsAbsolutePath(const std::string& path)
    {
        if (path.empty())
            return false;

        std::filesystem::path p(path);
        return p.is_absolute();
    }
}

#endif // AILURUS_PLATFORM_WINDOWS