#include <array>
#include "Ailurus/Graphics/Renderer.h"
#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"
#include "Ailurus/Graphics/Pipeline/Pipeline.h"
#include "Ailurus/Graphics/InputAssemble/InputAssemble.h"
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

        _pRenderObj = std::make_unique<RenderObject>(this);

        static std::array vertices = {
            -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, 1.0f, 1.0f, 1.0f
        };

        const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        auto pVertexBuffer = std::make_unique<VertexBuffer>(this, reinterpret_cast<const char*>(vertices.data()),
            vertices.size() * sizeof(float));
        auto pIndexBuffer = std::make_unique<IndexBuffer>(this, indices);

        auto pInputAssemble = std::make_unique<InputAssemble>(this,
            std::move(pVertexBuffer),
            InputAttribute{AttributeType::Vector2, AttributeType::Vector3},
            std::move(pIndexBuffer));

        auto pVertexShader = _pShaderLibrary->GetShader<ShaderStage::Vertex>("./triangle.vert.spv");
        auto pFragmentShader = _pShaderLibrary->GetShader<ShaderStage::Fragment>("./triangle.frag.spv");

        _pRenderObj->SetInputAssemble(std::move(pInputAssemble));
        _pRenderObj->SetRenderPassShader<RenderPassType::Forward>(pVertexShader);
        _pRenderObj->SetRenderPassShader<RenderPassType::Forward>(pFragmentShader);

        CreateDynamicContext();
    }

    Renderer::~Renderer()
    {
        DestroyDynamicContext();

        _pRenderObj.reset();

        DestroyStaticContext();
    }

    void Renderer::Render()
    {
        if (_needRebuildSwapChain)
            RecreateSwapChain();

        auto flight = _pAirport->WaitNextFlight(_pSwapChain.get(), &_needRebuildSwapChain);
        if (!flight.has_value())
            return;

        flight->Begin();
        {
            flight->BeginRenderPass(_pForwardPass.get());
            {
                flight->DrawObject(_pRenderObj.get());
            }
            flight->EndRenderPass();
        }
        flight->End();

        _pAirport->TakeOff(flight.value(), _pSwapChain.get(), &_needRebuildSwapChain);
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

    uint32_t Renderer::GetGraphicQueueIndex() const
    {
        return _graphicQueueIndex;
    }

    uint32_t Renderer::GetPresentQueueIndex() const
    {
        return _presentQueueIndex;
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

    ShaderLibrary* Renderer::GetShaderLibrary()
    {
        return _pShaderLibrary.get();
    }

    SwapChain* Renderer::GetSwapChain() const
    {
        return _pSwapChain.get();
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

        _pShaderLibrary = std::make_unique<ShaderLibrary>(this);
    }

    void Renderer::DestroyStaticContext()
    {
        _pShaderLibrary.reset();

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
                if (std::string(layerProperty.layerName.data()) == VALIDATION_LAYER_NAME)
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
        std::optional<uint32_t> optGraphicQueue = std::nullopt;
        std::optional<uint32_t> optPresentQueue = std::nullopt;
        auto queueFamilyProperties = _vkPhysicalDevice.getQueueFamilyProperties();
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            auto& property = queueFamilyProperties[i];
            if ((property.queueFlags & vk::QueueFlagBits::eGraphics) && !optGraphicQueue.has_value())
                optGraphicQueue = static_cast<uint32_t>(i);

            bool canPresent = _vkPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _vkSurface);
            if (canPresent && !optPresentQueue.has_value())
                optPresentQueue = static_cast<uint32_t>(i);

            if (optGraphicQueue.has_value() && optPresentQueue.has_value())
                break;
        }

        if (!optGraphicQueue.has_value() || !optPresentQueue.has_value())
        {
            Logger::LogError("Fail to get graphic queue and present queue.");
            return;
        }

        _graphicQueueIndex = *optGraphicQueue;
        _presentQueueIndex = *optPresentQueue;

        // Queue create info
        const float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
        queueCreateInfoList.emplace_back();
        queueCreateInfoList.back().setQueueCount(1)
                .setQueuePriorities(queuePriority)
                .setQueueFamilyIndex(_graphicQueueIndex);

        if (_presentQueueIndex != _graphicQueueIndex)
        {
            queueCreateInfoList.emplace_back();
            queueCreateInfoList.back()
                    .setQueueCount(1)
                    .setQueuePriorities(queuePriority)
                    .setQueueFamilyIndex(_presentQueueIndex);
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
        _vkGraphicQueue = _vkLogicalDevice.getQueue(_graphicQueueIndex, 0);
        _vkPresentQueue = _vkLogicalDevice.getQueue(_presentQueueIndex, 0);
    }

    void Renderer::CreateCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(_graphicQueueIndex);

        _vkGraphicCommandPool = _vkLogicalDevice.createCommandPool(poolInfo);
    }

    void Renderer::CreateDynamicContext()
    {
        Vector2i windowSize = _getWindowSizeCallback();
        _pSwapChain = std::make_unique<SwapChain>(this, windowSize.x(), windowSize.y());
        _pForwardPass = std::make_unique<RenderPassForward>(this);
        _pAirport = std::make_unique<Airport>(this);
    }

    void Renderer::DestroyDynamicContext()
    {
        _pAirport.reset();
        _pForwardPass.reset();
        _pSwapChain.reset();
    }

    void Renderer::RecreateSwapChain()
    {
        DestroyDynamicContext();
        CreateDynamicContext();

        _needRebuildSwapChain = false;
    }
}
