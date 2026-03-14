#include <Ailurus/Systems/RenderSystem/RenderSystem.h>
#include <Ailurus/Application.h>
#include <Ailurus/Systems/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Systems/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Systems/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/SwapChain/VulkanSwapChain.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Resource/VulkanResourceManager.h>
#include <VulkanContext/Resource/Image/VulkanSampler.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>
#include "Ailurus/Utility/Logger.h"
#include "Detail/RenderIntermediateVariable.h"
#include "Skybox/Skybox.h"
#include "IBL/IBLManager.h"

#include <cmath>

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
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR = "ambientColor";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_SHADOW_BIAS_PARAMS = "shadowBiasParams";

	RenderSystem::RenderSystem(bool enable3D, const std::string& skyboxHDRTexturePath)
		: _enable3D(enable3D)
	{
		_pShaderLibrary.reset(new ShaderLibrary());

		CreateIntermediateVariable();
		BuildGlobalUniform();

		// Initialize post-process chain
		const auto& swapChainConfig = VulkanContext::GetSwapChain()->GetConfig();
		_postProcessChain = std::make_unique<PostProcessChain>();
		_postProcessChain->Init(
			_pShaderLibrary.get(),
			swapChainConfig.extent.width,
			swapChainConfig.extent.height,
			vk::Format::eR16G16B16A16Sfloat);
		_postProcessChain->AddEffect<BloomMipChainEffect>();
		_postProcessChain->AddEffect<ToneMappingEffect>();
		_postProcessChain->InsertEffect<SSAOEffect>(0);

		// Initialize skybox
		if (enable3D)
		{
			_pSkybox = std::make_unique<Skybox>();
			_pSkybox->Init(_pShaderLibrary.get(),
				vk::Format::eR16G16B16A16Sfloat,
				vk::Format::eD32Sfloat,
				skyboxHDRTexturePath);

			// Initialize IBL
			_pIBLManager = std::make_unique<IBLManager>();
			auto cubemapView = _pSkybox->GetCubemapImageView();
			auto* pCubemapSampler = _pSkybox->GetCubemapSampler();
			if (cubemapView && pCubemapSampler)
			{
				auto cubemapSampler = pCubemapSampler->GetSampler();
				_pIBLManager->Precompute(cubemapView, cubemapSampler, _pShaderLibrary.get());
			}
		}
	}

	RenderSystem::~RenderSystem()
	{
		if (VulkanContext::Initialized())
			VulkanContext::WaitDeviceIdle();

		if (_postProcessChain)
		{
			_postProcessChain->Shutdown();
			_postProcessChain.reset();
		}

		if (_pSkybox)
		{
			_pSkybox->Shutdown();
			_pSkybox.reset();
		}

		if (_pIBLManager)
		{
			_pIBLManager->Shutdown();
			_pIBLManager.reset();
		}
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
		_pMainCamera = pCamera;
		SyncMainCameraAspectToSwapChain();
	}

	void RenderSystem::SyncMainCameraAspectToSwapChain()
	{
		if (_pMainCamera == nullptr || !_pMainCamera->IsPerspective())
			return;

		auto* pSwapChain = VulkanContext::GetSwapChain();
		if (pSwapChain == nullptr)
			return;

		const auto& extent = pSwapChain->GetConfig().extent;
		if (extent.width == 0 || extent.height == 0)
			return;

		const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
		if (std::abs(_pMainCamera->GetAspectRatio() - aspect) < 0.0001f)
			return;

		_pMainCamera->Set(
			_pMainCamera->GetHorizontalFOV(),
			aspect,
			_pMainCamera->GetNear(),
			_pMainCamera->GetFar());
	}

	CompCamera* RenderSystem::GetMainCamera() const
	{
		return _pMainCamera;
	}

	void RenderSystem::SetSkyboxEnabled(bool enabled)
	{
		_skyboxEnabled = enabled;
	}

	bool RenderSystem::IsSkyboxEnabled() const
	{
		return _skyboxEnabled;
	}

	void RenderSystem::SetClearColor(float r, float g, float b, float a)
	{
		_clearColor = {r, g, b, a};
	}

	std::array<float, 4> RenderSystem::GetClearColor() const
	{
		return _clearColor;
	}

	void RenderSystem::SetAmbientColor(float r, float g, float b)
	{
		_ambientColor = {r, g, b};
	}

	void RenderSystem::SetAmbientStrength(float strength)
	{
		_ambientStrength = strength;
	}

	void RenderSystem::SetShadowConstantBias(float bias)
	{
		_shadowConstantBias = bias;
	}

	void RenderSystem::SetShadowSlopeScale(float scale)
	{
		_shadowSlopeScale = scale;
	}

	void RenderSystem::SetShadowNormalOffset(float offset)
	{
		_shadowNormalOffset = offset;
	}

	const RenderStats& RenderSystem::GetRenderStats() const
	{
		return _renderStats;
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

		// Rebuild post-process chain with new swapchain dimensions
		if (_postProcessChain)
		{
			const auto& swapChainConfig = VulkanContext::GetSwapChain()->GetConfig();
			_postProcessChain->OnResize(
				swapChainConfig.extent.width,
				swapChainConfig.extent.height,
				vk::Format::eR16G16B16A16Sfloat);
		}

		// Rebuild skybox pipeline (MSAA samples may have changed)
		if (_pSkybox)
			_pSkybox->RebuildPipeline(vk::Format::eR16G16B16A16Sfloat, vk::Format::eD32Sfloat);

		SyncMainCameraAspectToSwapChain();

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

		// Add ambient color (xyz = color, w = strength)
		pGlobalUniformStructure->AddMember(
			"ambientColor",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4));

		// Add shadow bias parameters (x = constant bias, y = slope scale, z = normal offset, w = unused)
		pGlobalUniformStructure->AddMember(
			"shadowBiasParams",
			std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4));

		auto pBindingPoint = std::make_unique<UniformBindingPoint>(
			0,
			std::vector<ShaderStage>{ ShaderStage::Vertex, ShaderStage::Fragment },
			"globalUniform",
			std::move(pGlobalUniformStructure));

		_pGlobalUniformSet->AddBindingPoint(std::move(pBindingPoint));
		_pGlobalUniformSet->InitUniformBufferInfo();

		// Add shadow map sampler bindings (set=0, bindings 1-4) to the global descriptor layout
		std::vector<TextureBindingInfo> textureBindings;
		// Shadow maps (bindings 1-4)
		for (uint32_t i = 0; i < RenderIntermediateVariable::CSM_CASCADE_COUNT; i++)
		{
			TextureBindingInfo bindingInfo;
			bindingInfo.bindingId = i + 1;
			bindingInfo.shaderStages = vk::ShaderStageFlagBits::eFragment;
			textureBindings.push_back(bindingInfo);
		}
		// IBL textures (bindings 5-7)
		for (uint32_t binding = 5; binding <= 7; binding++)
		{
			TextureBindingInfo bindingInfo;
			bindingInfo.bindingId = binding;
			bindingInfo.shaderStages = vk::ShaderStageFlagBits::eFragment;
			textureBindings.push_back(bindingInfo);
		}
		_pGlobalUniformSet->InitDescriptorSetLayout(textureBindings);

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

	auto RenderSystem::GetGlobalUniformAccessNameAmbientColor() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameShadowBiasParams() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_SHADOW_BIAS_PARAMS;
		return value;
	}

} // namespace Ailurus
