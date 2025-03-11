#include <array>
#include "Ailurus/Window/Render/Render.h"
#include "Ailurus/Window/Render/DataBuffer/VertexBuffer.h"
#include "Ailurus/Window/Render/DataBuffer/IndexBuffer.h"
#include "Ailurus/Window/Render/Pipeline/Pipeline.h"
#include "Ailurus/Window/Render/InputAssemble/InputAssemble.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/VulkanContext.h"

namespace Ailurus
{
    Render::Render(const GetWindowSizeCallback& getWindowSize)
        : _getWindowSizeCallback(getWindowSize)
    {
        CreateStaticContext();

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

    Render::~Render()
    {
        DestroyDynamicContext();

        _pRenderObj.reset();

        DestroyStaticContext();
    }

    void Render::RenderScene()
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

    void Render::NeedRecreateSwapChain()
    {
        _needRebuildSwapChain = true;
    }

    ShaderLibrary* Render::GetShaderLibrary()
    {
        return _pShaderLibrary.get();
    }

    SwapChain* Render::GetSwapChain() const
    {
        return _pSwapChain.get();
    }

    void Render::CreateStaticContext()
    {
        if (!VulkanContext::Register(this, &_vkSurface))
            return;

        CreateCommandPool();

        _pShaderLibrary = std::make_unique<ShaderLibrary>(this);
    }

    void Render::DestroyStaticContext()
    {
        _pShaderLibrary.reset();

        VulkanContext::GetDevice().destroyCommandPool(_vkGraphicCommandPool);
        VulkanContext::Unregister(this);
    }

    void Render::CreateCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(VulkanContext::GetGraphicQueueIndex());

        _vkGraphicCommandPool = VulkanContext::GetDevice().createCommandPool(poolInfo);
    }

    void Render::CreateDynamicContext()
    {
        Vector2i windowSize = _getWindowSizeCallback();
        _pSwapChain = std::make_unique<SwapChain>(this, windowSize.x(), windowSize.y());
        _pForwardPass = std::make_unique<RenderPassForward>(this);
        _pAirport = std::make_unique<Airport>(this);
    }

    void Render::DestroyDynamicContext()
    {
        _pAirport.reset();
        _pForwardPass.reset();
        _pSwapChain.reset();
    }

    void Render::RecreateSwapChain()
    {
        DestroyDynamicContext();
        CreateDynamicContext();

        _needRebuildSwapChain = false;
    }
}
