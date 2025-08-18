#pragma once

#include <memory>
#include "Ailurus/PlatformDefine.h"

namespace Ailurus
{
    class ImGuiVulkanBackEnd;

    class ImGuiSystem
    {
    public:
        ImGuiSystem();
        ~ImGuiSystem();

    private:
        std::unique_ptr<ImGuiVulkanBackEnd> _pVkImpl;
    };
}