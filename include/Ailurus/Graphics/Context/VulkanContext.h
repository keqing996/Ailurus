#pragma once

#include <functional>
#include "vulkan/vulkan.hpp"
#include "QueueIndex.h"
#include "../../Math/Vector.hpp"

namespace Ailurus
{
    class VulkanContext
    {
        using GetWindowSizeCallback = std::function<Vector2i()>;
        using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
        using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
        using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

    public:
        VulkanContext(const GetWindowSizeCallback& getWindowSize,
            const GetWindowInstanceExtension& getWindowInstExt,
            const WindowCreateSurfaceCallback& createSurface,
            const WindowDestroySurfaceCallback& destroySurface,
            bool enableValidationLayer);

        ~VulkanContext();

    public:
        vk::Instance GetInstance() const;
        vk::SurfaceKHR GetSurface() const;
        vk::PhysicalDevice GetPhysicalDevice() const;
        vk::Device GetLogicalDevice() const;
        const QueueIndex& GetQueueIndex() const;
        vk::Queue GetGraphicQueue() const;
        vk::Queue GetPresentQueue() const;
        const std::vector<vk::CommandBuffer>& GetCommandBuffers() const;
        const std::vector<vk::Semaphore>& GetImageAvailableSemaphores() const;
        const std::vector<vk::Semaphore>& GetRenderFinishSemaphores() const;
        const std::vector<vk::Fence>& GetFences() const;

        inline static uint32_t PARALLEL_FRAME_COUNT = 2;

    private:
        void CreateInstance(bool enableValidation);
        void CreatDebugReportCallbackExt();
        void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
        void ChoosePhysicsDevice();
        void CreateLogicDevice();
        void CreateCommandPoolAndCommandBuffer();
        void CreateSemaphoresAndFences();

    private:
        GetWindowSizeCallback _getWindowSizeCallback;
        GetWindowInstanceExtension _getWindowInstExtensionsCallback;
        WindowDestroySurfaceCallback _windowDestroySurfaceCallback;

        vk::Instance _vkInstance = nullptr;
        vk::DebugReportCallbackEXT _vkDebugReportCallbackExt = nullptr;
        vk::SurfaceKHR _vkSurface = nullptr;
        vk::PhysicalDevice _vkPhysicalDevice = nullptr;
        vk::Device _vkLogicalDevice = nullptr;
        QueueIndex _queueIndex;
        vk::Queue _vkGraphicQueue = nullptr;
        vk::Queue _vkPresentQueue = nullptr;

        vk::CommandPool _vkGraphicCommandPool = nullptr;
        std::vector<vk::CommandBuffer> _vkCommandBuffers;

        std::vector<vk::Semaphore> _imageAvailableSemaphores;
        std::vector<vk::Semaphore> _renderFinishedSemaphores;
        std::vector<vk::Fence> _inFlightFences;
    };
}
