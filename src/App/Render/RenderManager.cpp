#include "Ailurus/Application/Render/RenderManager.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/Material/Material.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Ailurus/Application/Component/CompMeshRender.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/DataBuffer/VertexBuffer.h"
#include "Vulkan/DataBuffer/IndexBuffer.h"
#include "Vulkan/RenderPass/RHIRenderPass.h"

namespace Ailurus
{
	RenderManager::RenderManager()
	{
		BuildRenderPass();
	}

	RenderManager::~RenderManager() = default;

	void RenderManager::NeedRecreateSwapChain()
	{
		_needRebuildSwapChain = true;
	}

	Material* RenderManager::GetMaterial(const std::string& name) const
	{
		if (const auto itr = _materialMap.find(name); itr != _materialMap.end())
			return itr->second.get();

		return nullptr;
	}

	Material* RenderManager::AddMaterial(const std::string& name)
	{
		_materialMap[name] = std::make_unique<Material>();
		return GetMaterial(name);
	}

	void RenderManager::RenderScene()
	{
		if (_needRebuildSwapChain)
			ReBuildSwapChain();

		if (!VulkanContext::WaitNextFrame(&_needRebuildSwapChain))
			return;

		// Prepare objects
		std::vector<Entity*> allEntities = Application::GetSceneManager().GetAllRawEntities();
		std::vector<CompMeshRender*> allMeshRender;
		allMeshRender.reserve(allEntities.size());
		for (auto pEntity : allEntities)
		{
			if (auto pMeshRender = pEntity->GetComponent<CompMeshRender>(ComponentType::MeshRender);
				pMeshRender != nullptr)
			{
				allMeshRender.push_back(pMeshRender);
			}
		}

		// Begin
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		VulkanContext::GetCurrentFrameCommandBuffer().begin(beginInfo);

		// Render pass
		RenderForwardPass(allMeshRender);

		// End
		VulkanContext::GetCurrentFrameCommandBuffer().end();

		// Fire
		VulkanContext::SubmitThisFrame(&_needRebuildSwapChain);
	}

	void RenderManager::GraphicsWaitIdle() const
	{
		VulkanContext::GetDevice().waitIdle();
	}

	void RenderManager::ReBuildSwapChain()
	{
		GraphicsWaitIdle();

		_renderPassMap.clear();
		VulkanContext::RebuildDynamicContext();
		BuildRenderPass();
		_needRebuildSwapChain = false;
	}

	void RenderManager::BuildRenderPass()
	{
		_renderPassMap[RenderPassType::Forward] = std::make_unique<RenderPass>(RenderPassType::Forward);
	}

	void RenderManager::RenderForwardPass(std::vector<CompMeshRender*>& meshRenderList)
	{
		if (_pCurrentRenderPass != nullptr)
		{
			Logger::LogError("Command buffer begin render pass while last render pass not ended");
			return;
		}

		_pCurrentRenderPass = _renderPassMap[RenderPassType::Forward].get();

		VulkanContext::GetCurrentFrameCommandBuffer().beginRenderPass(
			_pCurrentRenderPass->GetRHIRenderPass()->GetRenderPassBeginInfo(),
			{});

		for (const auto pMeshRender : meshRenderList)
			RenderMesh(pMeshRender);

		VulkanContext::GetCurrentFrameCommandBuffer().endRenderPass();

		_pCurrentRenderPass = nullptr;
	}

	void RenderManager::RenderMesh(const CompMeshRender* pMeshRender) const
	{
		if (pMeshRender == nullptr)
			return;

		if (_pCurrentRenderPass == nullptr)
		{
			Logger::LogError("Command buffer draw object but not in any render pass");
			return;
		}

		const auto commandBuffer = VulkanContext::GetCurrentFrameCommandBuffer();
		const auto pMesh = pMeshRender->GetMesh();
		const auto pMaterial = pMeshRender->GetMaterial();

		if (pMesh == nullptr || pMaterial == nullptr)
			return;

		auto optStageShaders = pMaterial->GetStageShaderArray(_pCurrentRenderPass->GetRenderPassType());
		if (!optStageShaders.has_value())
			return; // This object should not be drawn under this pass;

		// Bind pipeline
		PipelineConfig pipelineConfig;
		pipelineConfig.pMesh = pMesh;
		pipelineConfig.shaderStages = optStageShaders.value();
		vk::Pipeline pipeline = _pCurrentRenderPass->GetRHIRenderPass()->GetPipeline(pipelineConfig)->GetPipeline();
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

		// Set viewport & scissor
		auto extent = VulkanContext::GetSwapChainConfig().extent;
		vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
		vk::Rect2D scissor(vk::Offset2D{ 0, 0 }, extent);
		commandBuffer.setViewport(0, 1, &viewport);
		commandBuffer.setScissor(0, 1, &scissor);

		// Bind vertex buffer
		vk::Buffer vertexBuffers[] = { pMesh->GetVertexBuffer()->GetBuffer() };
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		// Draw by index
		auto pIndexBuffer = pMesh->GetIndexBuffer();
		if (pIndexBuffer != nullptr)
		{
			commandBuffer.bindIndexBuffer(pIndexBuffer->GetBuffer(), 0, pIndexBuffer->GetIndexType());
			commandBuffer.drawIndexed(pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}
		// Draw by vertex
		else
		{
			commandBuffer.draw(pMesh->GetVertexCount(), 1, 0, 0);
		}
	}
} // namespace Ailurus
