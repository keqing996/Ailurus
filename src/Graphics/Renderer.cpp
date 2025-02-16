#include <array>
#include "Ailurus/Graphics/Renderer.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    namespace Verbose
    {
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        DebugReportExtCallback(
            VkDebugReportFlagsEXT,
            VkDebugReportObjectTypeEXT,
            std::uint64_t,
            std::size_t,
            std::int32_t,
            const char*,
            const char* pMessage,
            void*)
        {
            Logger::LogError(pMessage);
            return VK_FALSE;
        }

        static void LogInstanceLayerProperties()
        {
            auto allLayerProperties = vk::enumerateInstanceLayerProperties();

            Logger::LogInfo("Instance layer properties:");
            for (auto layerProperty: allLayerProperties)
                Logger::LogInfo("    {}", layerProperty.layerName.data());
        }

        static void LogInstanceExtensionProperties()
        {
            auto allExt = vk::enumerateInstanceExtensionProperties();
            Logger::LogInfo("Instance extensions:");
            for (auto ext: allExt)
                Logger::LogInfo("    {}", ext.extensionName.data());
        }

        static void LogPhysicalCards(const vk::Instance& vkInstance)
        {
            auto graphicCards = vkInstance.enumeratePhysicalDevices();
            Logger::LogInfo("All graphic cards:");
            for (auto& graphicCard: graphicCards)
            {
                auto property = graphicCard.getProperties();
                Logger::LogInfo("    {}", property.deviceName.data());
            }
        }

        static void LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface)
        {
            auto physicalDeviceProperty = vkPhysicalDevice.getProperties();
            Logger::LogInfo("Choose physical device: {}, API version: {}, vendor id: {}",
                            physicalDeviceProperty.deviceName.data(),
                            physicalDeviceProperty.apiVersion, physicalDeviceProperty.vendorID);

            auto queueFamilyProperties = vkPhysicalDevice.getQueueFamilyProperties();
            Logger::LogInfo("    Queue family size: {}", queueFamilyProperties.size());
            for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
            {
                auto& queueProperty = queueFamilyProperties[i];

                bool canPresent = vkPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), vkSurface);
                bool canGraphic = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eGraphics);
                bool canCompute = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eCompute);
                bool canTransfer = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eTransfer);

                Logger::LogInfo("        {}:\tPresent: {},\tGraphic: {}\t, Compute: {}\t, Transfer: {}",
                                i, canPresent, canGraphic, canCompute, canTransfer);
            }

#if false // Too long to show details
        Logger::LogInfo("    Physical device layer properties:");
        auto allLayerProperties = vkPhysicalDevice.enumerateDeviceLayerProperties();
        for (auto layerProperty: allLayerProperties)
            Logger::LogInfo("        {}", layerProperty.layerName.data());

        Logger::LogInfo("    Physical device extensions:");
        auto allExt = vkPhysicalDevice.enumerateDeviceExtensionProperties();
        for (auto ext: allExt)
            Logger::LogInfo("        {}", ext.extensionName.data());
#endif
        }
    }

    Renderer::Renderer(
        const GetWindowSizeCallback& getWindowSize,
        const GetWindowInstanceExtension& getWindowInstExt,
        const WindowCreateSurfaceCallback& createSurface,
        const WindowDestroySurfaceCallback& destroySurface,
        bool enableValidationLayer)
        : _getWindowSizeCallback(getWindowSize)
          , _getWindowInstExtensionsCallback(getWindowInstExt)
          , _windowDestroySurfaceCallback(destroySurface)
    {
        CreateStaticContext(enableValidationLayer, createSurface);

        // temp
        _pVertShader = std::make_unique<Shader>(this, ShaderStage::Vertex, "./triangle.vert.spv");
        _pFragShader = std::make_unique<Shader>(this, ShaderStage::Fragment, "./triangle.frag.spv");

        CreateDynamicContext();
    }

    Renderer::~Renderer()
    {
        DestroyDynamicContext();
        DestroyStaticContext();
    }

    void Renderer::Render()
    {
        if (_needRebuildSwapChain)
            RecreateSwapChain();

        auto vkSwapChain = _pSwapChain->GetSwapChain();
        auto vkRenderPass = _pRenderPass->GetRenderPass();
        auto vkBackBuffers = _pBackBuffer->GetBackBuffers();
        auto timeout = std::numeric_limits<uint64_t>::max();

        auto flight = _pAirport->GetNextFlight();

        auto acquireImage = _vkLogicalDevice.acquireNextImageKHR(vkSwapChain, timeout, flight.imageReadySemaphore);
        if (acquireImage.result == vk::Result::eErrorOutOfDateKHR || acquireImage.result == vk::Result::eSuboptimalKHR)
            _needRebuildSwapChain = true;

        if (acquireImage.result == vk::Result::eErrorOutOfDateKHR)
            return;

        if (acquireImage.result != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to acquire next image, result = {}", static_cast<int>(acquireImage.result));
            return;
        }

        uint32_t imageIndex = acquireImage.value;

        RecordCommand(flight.commandBuffer, vkRenderPass, vkBackBuffers[imageIndex]);

        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(flight.imageReadySemaphore)
                .setSignalSemaphores(flight.renderFinishSemaphore)
                .setWaitDstStageMask(waitStages)
                .setCommandBuffers(flight.commandBuffer);

        auto submit = _vkGraphicQueue.submit(1, &submitInfo, flight.fence);
        if (submit != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to submit command buffer, result = {}", static_cast<int>(submit));
            return;
        }

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(flight.renderFinishSemaphore)
                .setSwapchains(vkSwapChain)
                .setImageIndices(imageIndex);

        auto present = _vkPresentQueue.presentKHR(presentInfo);
        if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR)
            _needRebuildSwapChain = true;

        if (present == vk::Result::eErrorOutOfDateKHR)
            return;

        if (present != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
            return;
        }

        _pAirport->UpdateFlightPlan();
    }

    void Renderer::RecordCommand(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass,
                                 vk::Framebuffer targetFrameBuffer)
    {
        commandBuffer.reset();

        auto extent = _pSwapChain->GetSwapChainConfig().extent;

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        commandBuffer.begin(beginInfo); {
            vk::ClearValue clearColor({0.0f, 0.0f, 0.0f, 1.0f});

            vk::RenderPassBeginInfo renderPassInfo;
            renderPassInfo.setRenderPass(renderPass)
                    .setFramebuffer(targetFrameBuffer)
                    .setRenderArea(vk::Rect2D{
                        vk::Offset2D{0, 0},
                        extent
                    })
                    .setClearValues(clearColor);

            commandBuffer.beginRenderPass(renderPassInfo, {}); {
                vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
                vk::Rect2D scissor(vk::Offset2D{0, 0}, extent);

                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pPipeline->GetPipeline());
                commandBuffer.setViewport(0, 1, &viewport);
                commandBuffer.setScissor(0, 1, &scissor);
                commandBuffer.draw(3, 1, 0, 0);
            }
            commandBuffer.endRenderPass();
        }
        commandBuffer.end();
    }

    void Renderer::NeedRecreateSwapChain()
    {
        _needRebuildSwapChain = true;
    }

    vk::Instance Renderer::GetInstance() const
    {
        return _vkInstance;
    }

    vk::SurfaceKHR Renderer::GetSurface() const
    {
        return _vkSurface;
    }

    vk::PhysicalDevice Renderer::GetPhysicalDevice() const
    {
        return _vkPhysicalDevice;
    }

    vk::Device Renderer::GetLogicalDevice() const
    {
        return _vkLogicalDevice;
    }

    const Renderer::QueueIndex& Renderer::GetQueueIndex() const
    {
        return _queueIndex;
    }

    vk::Queue Renderer::GetGraphicQueue() const
    {
        return _vkGraphicQueue;
    }

    vk::Queue Renderer::GetPresentQueue() const
    {
        return _vkPresentQueue;
    }

    vk::CommandPool Renderer::GetCommandPool() const
    {
        return _vkGraphicCommandPool;
    }

    void Renderer::CreateStaticContext(bool enableValidation, const WindowCreateSurfaceCallback& createSurface)
    {
        Verbose::LogInstanceLayerProperties();
        Verbose::LogInstanceExtensionProperties();

        CreateInstance(enableValidation);

        if (enableValidation)
            CreatDebugReportCallbackExt();

        CreateSurface(createSurface);

        Verbose::LogPhysicalCards(_vkInstance);

        ChoosePhysicsDevice();

        Verbose::LogChosenPhysicalCard(_vkPhysicalDevice, _vkSurface);

        CreateLogicDevice();

        CreateCommandPool();
    }

    void Renderer::DestroyStaticContext()
    {
        _vkLogicalDevice.destroyCommandPool(_vkGraphicCommandPool);
        _vkLogicalDevice.destroy();

        _windowDestroySurfaceCallback(_vkInstance, _vkSurface);

        if (_vkDebugReportCallbackExt)
        {
            vk::DispatchLoaderDynamic dynamicLoader(_vkInstance, vkGetInstanceProcAddr);
            _vkInstance.destroyDebugReportCallbackEXT(_vkDebugReportCallbackExt, nullptr, dynamicLoader);
        }

        _vkInstance.destroy();
    }

    void Renderer::CreateInstance(bool enableValidation)
    {
        // Validation layers
        static const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
        std::vector<const char*> validationLayers;
        if (enableValidation)
        {
            auto allLayerProperties = vk::enumerateInstanceLayerProperties();
            for (auto layerProperty: allLayerProperties)
            {
                if (layerProperty.layerName == VALIDATION_LAYER_NAME)
                    validationLayers.push_back(VALIDATION_LAYER_NAME);
            }
        }

        // Extensions
        std::vector<const char*> requiredExtensions;
        if (enableValidation)
            requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        std::vector<const char*> windowInstExtensions = _getWindowInstExtensionsCallback();
        requiredExtensions.insert(requiredExtensions.end(),
                                  windowInstExtensions.begin(), windowInstExtensions.end());

        // Create instance
        vk::ApplicationInfo applicationInfo;
        applicationInfo
                .setPApplicationName("Ailurus")
                .setApiVersion(VK_API_VERSION_1_3)
                .setPEngineName("No Engine");

        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo
                .setPApplicationInfo(&applicationInfo)
                .setPEnabledLayerNames(validationLayers)
                .setPEnabledExtensionNames(requiredExtensions);

        _vkInstance = vk::createInstance(instanceCreateInfo, nullptr);
    }

    void Renderer::CreatDebugReportCallbackExt()
    {
        auto flags = vk::DebugReportFlagBitsEXT::eWarning
                     | vk::DebugReportFlagBitsEXT::ePerformanceWarning
                     | vk::DebugReportFlagBitsEXT::eError;

        vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
        debugReportCallbackCreateInfo
                .setFlags(flags)
                .setPfnCallback(Verbose::DebugReportExtCallback);

        vk::DispatchLoaderDynamic dynamicLoader(_vkInstance, vkGetInstanceProcAddr);

        _vkDebugReportCallbackExt = _vkInstance.createDebugReportCallbackEXT(
            debugReportCallbackCreateInfo, nullptr, dynamicLoader);
    }

    void Renderer::CreateSurface(const WindowCreateSurfaceCallback& createSurface)
    {
        _vkSurface = createSurface(_vkInstance);
    }

    void Renderer::ChoosePhysicsDevice()
    {
        auto graphicCards = _vkInstance.enumeratePhysicalDevices();

        vk::PhysicalDeviceType lastFoundDeviceType = vk::PhysicalDeviceType::eOther;
        for (auto& graphicCard: graphicCards)
        {
            auto property = graphicCard.getProperties();

            // Discrete gpu found, no need to continue.
            if (property.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                _vkPhysicalDevice = graphicCard;
                break;
            }

            // Integrated gpu found, record it and keep finding discrete gpu.
            if (property.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            {
                _vkPhysicalDevice = graphicCard;
                lastFoundDeviceType = vk::PhysicalDeviceType::eIntegratedGpu;
            }

            // Cpu found, record it only if that no gpu was found.
            if (property.deviceType == vk::PhysicalDeviceType::eCpu
                && lastFoundDeviceType != vk::PhysicalDeviceType::eIntegratedGpu)
            {
                _vkPhysicalDevice = graphicCard;
                lastFoundDeviceType = vk::PhysicalDeviceType::eCpu;
            }
        }
    }

    void Renderer::CreateLogicDevice()
    {
        // Find graphic queue and present queue.
        auto queueFamilyProperties = _vkPhysicalDevice.getQueueFamilyProperties();
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            auto& property = queueFamilyProperties[i];
            if ((property.queueFlags & vk::QueueFlagBits::eGraphics) && !_queueIndex.graphicQueueIndex.has_value())
                _queueIndex.graphicQueueIndex = static_cast<uint32_t>(i);

            bool canPresent = _vkPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _vkSurface);
            if (canPresent && !_queueIndex.presentQueueIndex.has_value())
                _queueIndex.presentQueueIndex = static_cast<uint32_t>(i);

            if (_queueIndex.graphicQueueIndex.has_value() && _queueIndex.presentQueueIndex.has_value())
                break;
        }

        // Queue create info
        const float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
        queueCreateInfoList.emplace_back();
        queueCreateInfoList.back().setQueueCount(1)
                .setQueuePriorities(queuePriority)
                .setQueueFamilyIndex(_queueIndex.graphicQueueIndex.value());

        if (_queueIndex.presentQueueIndex.value() != _queueIndex.graphicQueueIndex.value())
        {
            queueCreateInfoList.emplace_back();
            queueCreateInfoList.back()
                    .setQueueCount(1)
                    .setQueuePriorities(queuePriority)
                    .setQueueFamilyIndex(_queueIndex.presentQueueIndex.value());
        }

        // Features
        vk::PhysicalDeviceFeatures physicalDeviceFeatures;
        physicalDeviceFeatures.setSamplerAnisotropy(true);

        // Swap chain is required
        const char* extensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        vk::DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo
                .setPEnabledExtensionNames(extensions)
                .setQueueCreateInfos(queueCreateInfoList)
                .setPEnabledFeatures(&physicalDeviceFeatures);

        _vkLogicalDevice = _vkPhysicalDevice.createDevice(deviceCreateInfo);
        _vkGraphicQueue = _vkLogicalDevice.getQueue(_queueIndex.graphicQueueIndex.value(), 0);
        _vkPresentQueue = _vkLogicalDevice.getQueue(_queueIndex.presentQueueIndex.value(), 0);
    }

    void Renderer::CreateCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(_queueIndex.graphicQueueIndex.value());

        _vkGraphicCommandPool = _vkLogicalDevice.createCommandPool(poolInfo);
    }

    void Renderer::CreateDynamicContext()
    {
        Vector2i windowSize = _getWindowSizeCallback();
        _pSwapChain = std::make_unique<SwapChain>(this, windowSize.x(), windowSize.y());
        _pRenderPass = std::make_unique<RenderPass>(this, _pSwapChain.get());
        _pBackBuffer = std::make_unique<BackBuffer>(this, _pSwapChain.get(), _pRenderPass.get());

        _pPipeline = std::make_unique<Pipeline>(this, _pRenderPass.get());
        _pPipeline->AddShader(_pVertShader.get());
        _pPipeline->AddShader(_pFragShader.get());
        _pPipeline->GeneratePipeline();

        _pAirport = std::make_unique<Airport>(this);
    }

    void Renderer::DestroyDynamicContext()
    {
        _pAirport.reset();
        _pPipeline.reset();
        _pBackBuffer.reset();
        _pRenderPass.reset();
        _pSwapChain.reset();
    }

    void Renderer::RecreateSwapChain()
    {
        DestroyDynamicContext();
        CreateDynamicContext();

        _needRebuildSwapChain = false;
    }
}
