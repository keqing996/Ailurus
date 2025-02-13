#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus::VerboseLogger::Vulkan
{
    void SetEnable(bool enable);

    void LogInstanceLayerProperties();

    void LogInstanceExtensionProperties();

    void LogPhysicalCards(const vk::Instance& vkInstance);

    void LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface);
}
