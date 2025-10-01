#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_ANDROID

#include "Ailurus/System/Path.h"
#include "Ailurus/System/System.h"
#include <filesystem>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

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
        // Check if we're running as an Android application
        // This can be determined by checking if we have access to Android-specific APIs
        // For simplicity, we'll check if the executable path contains Android-specific patterns
        std::string executableDir = System::GetExecutableDirectory();
        
        // Check if we're in an Android app context (looking for typical Android paths)
        if (executableDir.find("/data/data/") != std::string::npos ||
            executableDir.find("/android_asset/") != std::string::npos ||
            executableDir.find("/storage/emulated/") != std::string::npos)
        {
            // We're in an Android app context
            // Try to get the application's files directory
            // Note: In a real implementation, you would get this from the Android Context
            // For now, we'll use a conventional Android app files path
            size_t packageStart = executableDir.find("/data/data/");
            if (packageStart != std::string::npos)
            {
                size_t packageEnd = executableDir.find("/", packageStart + 11); // 11 = length of "/data/data/"
                if (packageEnd != std::string::npos)
                {
                    std::string packagePath = executableDir.substr(0, packageEnd);
                    return packagePath + "/files";
                }
            }
            
            // Fallback: return the assets directory if we can detect it
            return "/android_asset";
        }
        
        // We're running as a Linux executable, use the executable directory
        return executableDir;
    }

    bool Path::IsAbsolutePath(const std::string& path)
    {
        if (path.empty())
            return false;

        std::filesystem::path p(path);
        return p.is_absolute();
    }
}

#endif // AILURUS_PLATFORM_ANDROID