#pragma once

#include <string>

namespace Ailurus
{
    class Path
    {
    public:
        Path() = delete;

    public:
        /// <summary>
        /// Resolves a given path to an absolute path.
        /// If the input path is already absolute, returns it as-is.
        /// If the input path is relative, returns an absolute path relative to the resource root folder.
        /// 
        /// Resource root folder behavior:
        /// - Apple platforms: Resources folder for app bundles, executable folder otherwise
        /// - Android: App resources folder for Android apps, executable folder for Linux executables
        /// - Other platforms: Executable folder
        /// </summary>
        /// <param name="path">The path to resolve (can be absolute or relative)</param>
        /// <returns>Absolute path pointing to the actual resource</returns>
        static auto ResolvePath(const std::string& path) -> std::string;

        /// <summary>
        /// Gets the resource root folder path for the current platform.
        /// This is the base folder used for resolving relative paths.
        /// </summary>
        /// <returns>Absolute path to the resource root folder</returns>
        static auto GetResourceRootPath() -> std::string;

        /// <summary>
        /// Checks if the given path is absolute.
        /// </summary>
        /// <param name="path">The path to check</param>
        /// <returns>True if the path is absolute, false if relative</returns>
        static auto IsAbsolutePath(const std::string& path) -> bool;
    };
}