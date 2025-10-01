#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_APPLE

#include "Ailurus/System/Path.h"
#include "Ailurus/System/System.h"
#include <filesystem>
#include <CoreFoundation/CoreFoundation.h>

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
        std::filesystem::path relativePath = path;
        std::filesystem::path resolved = resourceRoot / relativePath;
        
        return resolved.string();
    }

    std::string Path::GetResourceRootPath()
    {
        // Check if we're running as an app bundle
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        if (mainBundle != nullptr)
        {
            // We're in an app bundle, get the Resources directory
            CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
            if (resourcesURL != nullptr)
            {
                char path[PATH_MAX];
                if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, 
                    reinterpret_cast<UInt8*>(path), PATH_MAX))
                {
                    CFRelease(resourcesURL);
                    return std::string(path);
                }
                CFRelease(resourcesURL);
            }
        }

        // Not in an app bundle or failed to get resources directory
        // Fall back to executable directory
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

#endif // AILURUS_PLATFORM_APPLE