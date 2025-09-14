#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/RenderPass/VulkanRenderPass.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include "Ailurus/Utility/Logger.h"
#include "Detail/RenderIntermediateVariable.h"

namespace Ailurus
{
	const char* RenderSystem::GLOBAL_UNIFORM_SET_NAME = "globalUniform";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_VIEW_PROJ_MAT = "viewProjectionMatrix";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_CAMERA_POS = "cameraPosition";

	RenderSystem::RenderSystem(bool enableImGui, bool enable3D)
		: _enable3D(enable3D)
		, _enableImGui(enableImGui)
	{
		_pShaderLibrary.reset(new ShaderLibrary());

		CreateIntermediateVariable();
		BuildGlobalUniform();
		BuildRenderPass();
	}

	RenderSystem::~RenderSystem()
	{
	}

	void RenderSystem::RequestRebuildSwapChain()
	{
		_needRebuildSwapChain = true;
	}

	ShaderLibrary* RenderSystem::GetShaderLibrary() const
	{
		return _pShaderLibrary.get();
	}

	void RenderSystem::SetMainCamera(CompCamera* pCamera)
	{
		if (pCamera == nullptr)
			return;

		_pMainCamera = pCamera;
	}

	CompCamera* RenderSystem::GetMainCamera() const
	{
		return _pMainCamera;
	}

	void RenderSystem::AddCallbackPreSwapChainRebuild(void* key, const PreSwapChainRebuild& callback)
	{
		if (_preSwapChainRebuildCallbacks.contains(key))
			Logger::LogWarn("RenderSystem: PreSwapChainRebuild callback already exists, key = {}", key);

		_preSwapChainRebuildCallbacks[key] = callback;
	}

	void RenderSystem::AddCallbackPostSwapChainRebuild(void* key, const PostSwapChainRebuild& callback)
	{
		if (_postSwapChainRebuildCallbacks.contains(key))
			Logger::LogWarn("RenderSystem: PostSwapChainRebuild callback already exists, key = {}", key);

		_postSwapChainRebuildCallbacks[key] = callback;
	}

	void RenderSystem::RemoveCallbackPreSwapChainRebuild(void* key)
	{
		_preSwapChainRebuildCallbacks.erase(key);
	}

	void RenderSystem::RemoveCallbackPostSwapChainRebuild(void* key)
	{
		_postSwapChainRebuildCallbacks.erase(key);
	}

	void RenderSystem::GraphicsWaitIdle() const
	{
		VulkanContext::WaitDeviceIdle();
	}

	void RenderSystem::RebuildSwapChain()
	{
		GraphicsWaitIdle();

		for (const auto& [key, preRebuildCallback] : _preSwapChainRebuildCallbacks)
		{
			if (preRebuildCallback)
				preRebuildCallback();
		}

		_renderPassMap.clear();

		VulkanContext::RebuildSwapChain();

		BuildRenderPass();

		_needRebuildSwapChain = false;

		for (const auto& [key, postRebuildCallback] : _postSwapChainRebuildCallbacks)
		{
			if (postRebuildCallback)
				postRebuildCallback();
		}
	}

	void RenderSystem::BuildRenderPass()
	{
		// Forward RenderPass
		if (_enable3D)
		{
			Ailurus::VulkanRenderPassConfig config;
			Ailurus::VulkanRenderPassConfig::FrameBufferOperation fbOp;
			fbOp.usage = FrameBufferUsage::PresentBackBuffer;
			fbOp.multiSampling = MultiSamplingType::None;
			fbOp.clearAttachment = true;
			fbOp.writeAttachment = true;
			config.frameBufferOperations.push_back(fbOp);
			std::vector<vk::ClearValue> clearValues = { vk::ClearValue{ std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f} } };
			_renderPassMap[RenderPassType::Forward] = std::make_unique<VulkanRenderPass>(config, RenderPassType::Forward, clearValues);
		}

		// ImGui RenderPass
		if (_enableImGui)
		{
			Ailurus::VulkanRenderPassConfig config;
			Ailurus::VulkanRenderPassConfig::FrameBufferOperation fbOp;
			fbOp.usage = FrameBufferUsage::PresentBackBuffer;
			fbOp.multiSampling = MultiSamplingType::None;
			fbOp.clearAttachment = false;
			fbOp.writeAttachment = true;
			config.frameBufferOperations.push_back(fbOp);
			std::vector<vk::ClearValue> clearValues = { vk::ClearValue{ std::array<float,4>{0.45f, 0.55f, 0.60f, 1.0f} } };
			_renderPassMap[RenderPassType::ImGui] = std::make_unique<VulkanRenderPass>(config, RenderPassType::ImGui, clearValues);
		}
	}

	void RenderSystem::BuildGlobalUniform()
	{
		_pGlobalUniformSet = std::make_unique<UniformSet>(UniformSetUsage::General);

		// Create global uniform structure
		auto pGlobalUniformStructure = std::make_unique<UniformVariableStructure>();

		// Add view projection matrix
		pGlobalUniformStructure->AddMember(
			"viewProjectionMatrix",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Mat4));

		// Add camera position
		pGlobalUniformStructure->AddMember(
			"cameraPosition",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));

		auto pBindingPoint = std::make_unique<UniformBindingPoint>(
			0,
			std::vector<ShaderStage>{ ShaderStage::Vertex },
			"globalUniform",
			std::move(pGlobalUniformStructure));

		_pGlobalUniformSet->AddBindingPoint(std::move(pBindingPoint));
		_pGlobalUniformSet->InitUniformBufferInfo();
		_pGlobalUniformSet->InitDescriptorSetLayout();

		_pGlobalUniformMemory = std::make_unique<UniformSetMemory>(_pGlobalUniformSet.get());
	}

	auto RenderSystem::GetGlobalUniformSet() const -> UniformSet*
	{
		return _pGlobalUniformSet.get();
	}

	auto RenderSystem::GetGlobalUniformAccessNameViewProjMat() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_VIEW_PROJ_MAT;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameCameraPos() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_CAMERA_POS;
		return value;
	}

auto RenderSystem::GetRenderPass(RenderPassType pass) const -> VulkanRenderPass*
{
	auto it = _renderPassMap.find(pass);
	if (it == _renderPassMap.end())
		return nullptr;
	return it->second.get();
}

} // namespace Ailurus
