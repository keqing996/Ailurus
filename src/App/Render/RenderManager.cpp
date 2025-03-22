#include "Ailurus/Application/Render/RenderManager.h"
#include "Ailurus/Application/Material/Material.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Ailurus/Application/Render/MeshRender.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Vulkan/Airport/Airport.h"
#include "Vulkan/RenderPass/RHIRenderPass.h"

namespace Ailurus
{
	static vk::Format ConvertToVkFormat(AttributeType type)
	{
		switch (type)
		{
			case AttributeType::Vector2:
				return vk::Format::eR32G32Sfloat;
			case AttributeType::Vector3:
				return vk::Format::eR32G32B32Sfloat;
			case AttributeType::Vector4:
				return vk::Format::eR32G32B32A32Sfloat;
		}

		Logger::LogError("Fail to convert attribute type to vk format, attribute type = {}",
			EnumReflection<AttributeType>::ToString(type));
		return vk::Format::eUndefined;
	}

	static std::vector<vk::VertexInputAttributeDescription> GetMeshVulkanAttributeDescription(const Mesh* pMesh)
	{
		std::vector<vk::VertexInputAttributeDescription> result;

		uint32_t offset = 0;
		auto attributes = pMesh->GetInputAttribute().GetAttributes();
		for (auto i = 0; i < attributes.size(); i++)
		{
			AttributeType attr = attributes[i];

			vk::VertexInputAttributeDescription attributeDescriptions;
			attributeDescriptions.setBinding(0)
				.setLocation(i)
				.setFormat(ConvertToVkFormat(attr))
				.setOffset(offset);

			result.push_back(attributeDescriptions);

			offset += VertexAttributeDescription::SizeOf(attr);
		}

		return result;
	}

	RenderManager::RenderManager()
	{
		_renderPassMap[RenderPassType::Forward] = std::make_unique<RenderPass>(RenderPassType::Forward);
	}

	RenderManager::~RenderManager() = default;

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
			VulkanContext::RebuildDynamicContext();

		auto pSwapChain = VulkanContext::GetSwapChain();
		auto opFlight = VulkanContext::GetAirport()->WaitNextFlight(pSwapChain, &_needRebuildSwapChain);
		if (!opFlight.has_value())
			return;

		auto flight = opFlight.value();

		// Begin
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		flight.commandBuffer.begin(beginInfo);

		// Render pass
		RenderForwardPass(&flight);

		// End
		flight.commandBuffer.end();

		// Fire
		VulkanContext::GetAirport()->TakeOff(flight, pSwapChain, &_needRebuildSwapChain);
	}

	void RenderManager::RenderForwardPass(const Flight* pFlight)
	{
		if (_pCurrentRenderPass != nullptr)
		{
			Logger::LogError("Command buffer begin render pass while last render pass not ended");
			return;
		}

		_pCurrentRenderPass = _renderPassMap[RenderPassType::Forward].get();

		pFlight->commandBuffer.beginRenderPass(_pCurrentRenderPass->GetRHIRenderPass()
												   ->GetRenderPassBeginInfo(*pFlight),
			{});

		//	flight->DrawObject(_pRenderObj.get());

		pFlight->commandBuffer.endRenderPass();

		_pCurrentRenderPass = nullptr;
	}

	void RenderManager::RenderMesh(const Flight* pFlight, const MeshRender* pMeshRender)
	{
		if (_pCurrentRenderPass == nullptr)
		{
			Logger::LogError("Command buffer draw object but not in any render pass");
			return;
		}

		auto commandBuffer = pFlight->commandBuffer;
		auto pMesh = pMeshRender->GetMesh();
		auto pMaterial = pMeshRender->GetMaterial();

		if (pMesh == nullptr || pMaterial == nullptr)
			return;

		auto optStageShaders = pMaterial->GetRenderPassShaders(_pCurrentRenderPass->GetRenderPassType());
		if (!optStageShaders.has_value())
			return; // This object should not be drawn under this pass;

		const InputAssemble* pInputAssemble = pRenderObject->GetInputAssemble();

		// Bind pipeline
		PipelineConfig pipelineConfig;
		pipelineConfig.pInputAssemble = pInputAssemble;
		pipelineConfig.shaderStages = *optStageShaders.value();
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pCurrentRenderPass->GetPipeline(pipelineConfig)->GetPipeline());

		// Set viewport & scissor
		auto extent = _pRenderer->GetSwapChain()->GetSwapChainConfig().extent;
		vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
		vk::Rect2D scissor(vk::Offset2D{0, 0}, extent);
		commandBuffer.setViewport(0, 1, &viewport);
		commandBuffer.setScissor(0, 1, &scissor);

		// Bind vertex buffer
		vk::Buffer vertexBuffers[] = { pInputAssemble->GetVertexBuffer()->GetBuffer() };
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		// Draw by index
		auto pIndexBuffer = pInputAssemble->GetIndexBuffer();
		if (pIndexBuffer != nullptr)
		{
			commandBuffer.bindIndexBuffer(pIndexBuffer->GetBuffer(), 0, pIndexBuffer->GetIndexType());
			commandBuffer.drawIndexed(pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}
		// Draw by vertex
		else
		{
			commandBuffer.draw(pInputAssemble->GetVertexCount(), 1, 0, 0);
		}
	}

} // namespace Ailurus
