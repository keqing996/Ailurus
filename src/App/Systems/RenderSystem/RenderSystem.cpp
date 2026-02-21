#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Resource/VulkanResourceManager.h>
#include <VulkanContext/Resource/Image/VulkanSampler.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>
#include "Ailurus/Utility/Logger.h"
#include "Detail/RenderIntermediateVariable.h"

namespace Ailurus
{
	const char* RenderSystem::GLOBAL_UNIFORM_SET_NAME = "globalUniform";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_VIEW_PROJ_MAT = "viewProjectionMatrix";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_CAMERA_POS = "cameraPosition";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_NUM_DIR_LIGHTS = "numDirectionalLights";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_NUM_POINT_LIGHTS = "numPointLights";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_NUM_SPOT_LIGHTS = "numSpotLights";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_DIR_LIGHT_DIRECTIONS = "dirLightDirections";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_DIR_LIGHT_COLORS = "dirLightColors";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_POSITIONS = "pointLightPositions";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_COLORS = "pointLightColors";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_ATTENUATIONS = "pointLightAttenuations";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_POSITIONS = "spotLightPositions";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_DIRECTIONS = "spotLightDirections";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_COLORS = "spotLightColors";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_ATTENUATIONS = "spotLightAttenuations";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_CUTOFFS = "spotLightCutoffs";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_CASCADE_VIEW_PROJ_MATRICES = "cascadeViewProjMatrices";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_CASCADE_SPLIT_DISTANCES = "cascadeSplitDistances";

	RenderSystem::RenderSystem(bool enableImGui, bool enable3D)
		: _enable3D(enable3D)
		, _enableImGui(enableImGui)
	{
		_pShaderLibrary.reset(new ShaderLibrary());

		CreateIntermediateVariable();
		BuildGlobalUniform();
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

	void RenderSystem::SetVSyncEnabled(bool enabled)
	{
		if (IsVSyncEnabled() == enabled)
			return;

		VulkanContext::SetVSyncEnabled(enabled);
		RequestRebuildSwapChain();
	}

	bool RenderSystem::IsVSyncEnabled() const
	{
		return VulkanContext::IsVSyncEnabled();
	}

	void RenderSystem::SetMSAAEnabled(bool enabled)
	{
		const bool isCurrentlyEnabled = IsMSAAEnabled();
		if (isCurrentlyEnabled == enabled)
			return;

		VulkanContext::SetMSAASamples(enabled ? vk::SampleCountFlagBits::e4 : vk::SampleCountFlagBits::e1);
		RequestRebuildSwapChain();
	}

	bool RenderSystem::IsMSAAEnabled() const
	{
		return VulkanContext::GetMSAASamples() != vk::SampleCountFlagBits::e1;
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

		VulkanContext::RebuildSwapChain();

		_needRebuildSwapChain = false;

		for (const auto& [key, postRebuildCallback] : _postSwapChainRebuildCallbacks)
		{
			if (postRebuildCallback)
				postRebuildCallback();
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

		// Add light counts
		pGlobalUniformStructure->AddMember(
			"numDirectionalLights",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Int));
		pGlobalUniformStructure->AddMember(
			"numPointLights",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Int));
		pGlobalUniformStructure->AddMember(
			"numSpotLights",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Int));

		// Add directional light arrays
		pGlobalUniformStructure->AddMember(
			"dirLightDirections",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_DIRECTIONAL_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"dirLightColors",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_DIRECTIONAL_LIGHTS));

		// Add point light arrays
		pGlobalUniformStructure->AddMember(
			"pointLightPositions",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_POINT_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"pointLightColors",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_POINT_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"pointLightAttenuations",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_POINT_LIGHTS));

		// Add spot light arrays
		pGlobalUniformStructure->AddMember(
			"spotLightPositions",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_SPOT_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"spotLightDirections",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_SPOT_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"spotLightColors",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_SPOT_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"spotLightAttenuations",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_SPOT_LIGHTS));
		pGlobalUniformStructure->AddMember(
			"spotLightCutoffs",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4),
				MAX_SPOT_LIGHTS));

		// Add CSM cascade matrices and split distances
		pGlobalUniformStructure->AddMember(
			"cascadeViewProjMatrices",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Mat4),
				4));  // 4 cascades
		pGlobalUniformStructure->AddMember(
			"cascadeSplitDistances",
			std::make_unique<UniformVariableArray>(
				std::make_unique<UniformVariableNumeric>(UniformValueType::Float),
				4));  // 4 cascades

		auto pBindingPoint = std::make_unique<UniformBindingPoint>(
			0,
			std::vector<ShaderStage>{ ShaderStage::Vertex, ShaderStage::Fragment },
			"globalUniform",
			std::move(pGlobalUniformStructure));

		_pGlobalUniformSet->AddBindingPoint(std::move(pBindingPoint));
		_pGlobalUniformSet->InitUniformBufferInfo();

		// Add shadow map sampler bindings (set=0, bindings 1-4) to the global descriptor layout
		std::vector<TextureBindingInfo> shadowMapBindings;
		for (uint32_t i = 0; i < RenderIntermediateVariable::CSM_CASCADE_COUNT; i++)
		{
			TextureBindingInfo bindingInfo;
			bindingInfo.bindingId = i + 1;
			bindingInfo.shaderStages = vk::ShaderStageFlagBits::eFragment;
			shadowMapBindings.push_back(bindingInfo);
		}
		_pGlobalUniformSet->InitDescriptorSetLayout(shadowMapBindings);

		_pGlobalUniformMemory = std::make_unique<UniformSetMemory>(_pGlobalUniformSet.get());

		// Create shadow map sampler
		_shadowSampler = VulkanContext::GetResourceManager()->CreateSampler();
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

	auto RenderSystem::GetGlobalUniformAccessNameNumDirLights() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_NUM_DIR_LIGHTS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameNumPointLights() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_NUM_POINT_LIGHTS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameNumSpotLights() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_NUM_SPOT_LIGHTS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameDirLightDirections() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_DIR_LIGHT_DIRECTIONS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameDirLightColors() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_DIR_LIGHT_COLORS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNamePointLightPositions() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_POSITIONS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNamePointLightColors() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_COLORS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNamePointLightAttenuations() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_ATTENUATIONS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameSpotLightPositions() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_POSITIONS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameSpotLightDirections() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_DIRECTIONS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameSpotLightColors() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_COLORS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameSpotLightAttenuations() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_ATTENUATIONS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameSpotLightCutoffs() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_CUTOFFS;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameCascadeViewProjMatrices() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_CASCADE_VIEW_PROJ_MATRICES;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameCascadeSplitDistances() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_CASCADE_SPLIT_DISTANCES;
		return value;
	}

} // namespace Ailurus
