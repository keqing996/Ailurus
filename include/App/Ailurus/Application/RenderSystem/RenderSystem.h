#pragma once

#include <functional>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Application/SceneSystem/Component/CompCamera.h"
#include "Ailurus/Application/RenderSystem/PostProcess/PostProcessChain.h"

namespace Ailurus
{
	class MaterialInstance;
	class Mesh;
	class CompStaticMeshRender;
	class VulkanCommandBuffer;
	class VulkanDescriptorAllocator;
	class VulkanUniformBuffer;
	class VulkanSampler;
	class UniformSet;
	class UniformSetMemory;
	struct RenderIntermediateVariable;

	class RenderSystem : public NonCopyable, public NonMovable
	{
	public:
		using PreSwapChainRebuild = std::function<void()>;
		using PostSwapChainRebuild = std::function<void()>;
		
	public:
		RenderSystem(bool enableImGui, bool enable3D);
		~RenderSystem();

	public:
		void RequestRebuildSwapChain();
		auto GetGlobalUniformSet() const -> UniformSet*;

		// Shader library
		ShaderLibrary* GetShaderLibrary() const;

		// Camera
		void SetMainCamera(CompCamera* pCamera);
		CompCamera* GetMainCamera() const;

		// VSync
		void SetVSyncEnabled(bool enabled);
		bool IsVSyncEnabled() const;

		// MSAA
		void SetMSAAEnabled(bool enabled);
		bool IsMSAAEnabled() const;

		// Callbacks
		void AddCallbackPreSwapChainRebuild(void* key, const PreSwapChainRebuild& callback);
		void AddCallbackPostSwapChainRebuild(void* key, const PostSwapChainRebuild& callback);
		void RemoveCallbackPreSwapChainRebuild(void* key);
		void RemoveCallbackPostSwapChainRebuild(void* key);

		// Draw
		void CheckRebuildSwapChain();
		void RenderScene();
		void GraphicsWaitIdle() const;	

	private:
		// Create
		void CreateIntermediateVariable();
		void BuildGlobalUniform();

		// Render
		void RenderPrepare();
		void CollectRenderingContext();
		void CollectLights();
		void CalculateCascadeShadows();
		void UpdateGlobalUniformBuffer(VulkanCommandBuffer* pCommandBuffer, class VulkanDescriptorAllocator* pDescriptorAllocator);
		void UpdateMaterialInstanceUniformBuffer(VulkanCommandBuffer* pCommandBuffer, class VulkanDescriptorAllocator* pDescriptorAllocator);
		void RebuildSwapChain();
		void RenderPass(RenderPassType pass, uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer);
		void RenderShadowPass(VulkanCommandBuffer* pCommandBuffer, class VulkanDescriptorAllocator* pDescriptorAllocator);
		void RenderImGuiPass(uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer);

		// Global uniform
		static auto GetGlobalUniformAccessNameViewProjMat() -> const std::string&;
		static auto GetGlobalUniformAccessNameCameraPos() -> const std::string&;
		static auto GetGlobalUniformAccessNameNumDirLights() -> const std::string&;
		static auto GetGlobalUniformAccessNameNumPointLights() -> const std::string&;
		static auto GetGlobalUniformAccessNameNumSpotLights() -> const std::string&;
		static auto GetGlobalUniformAccessNameDirLightDirections() -> const std::string&;
		static auto GetGlobalUniformAccessNameDirLightColors() -> const std::string&;
		static auto GetGlobalUniformAccessNamePointLightPositions() -> const std::string&;
		static auto GetGlobalUniformAccessNamePointLightColors() -> const std::string&;
		static auto GetGlobalUniformAccessNamePointLightAttenuations() -> const std::string&;
		static auto GetGlobalUniformAccessNameSpotLightPositions() -> const std::string&;
		static auto GetGlobalUniformAccessNameSpotLightDirections() -> const std::string&;
		static auto GetGlobalUniformAccessNameSpotLightColors() -> const std::string&;
		static auto GetGlobalUniformAccessNameSpotLightAttenuations() -> const std::string&;
		static auto GetGlobalUniformAccessNameSpotLightCutoffs() -> const std::string&;
		static auto GetGlobalUniformAccessNameCascadeViewProjMatrices() -> const std::string&;
		static auto GetGlobalUniformAccessNameCascadeSplitDistances() -> const std::string&;

	private:
		bool _needRebuildSwapChain = false;
		bool _enable3D = false;
		bool _enableImGui = false;

		// Current main camera
		CompCamera* _pMainCamera = nullptr;

		// Shader library
		std::unique_ptr<ShaderLibrary> _pShaderLibrary;

		// Intermediate variables for every frame
		std::unique_ptr<RenderIntermediateVariable> _pIntermediateVariable;

		// Uniforms set for global usage
		static const char* GLOBAL_UNIFORM_SET_NAME;
		static const char* GLOBAL_UNIFORM_ACCESS_VIEW_PROJ_MAT;
		static const char* GLOBAL_UNIFORM_ACCESS_CAMERA_POS;
		static const char* GLOBAL_UNIFORM_ACCESS_NUM_DIR_LIGHTS;
		static const char* GLOBAL_UNIFORM_ACCESS_NUM_POINT_LIGHTS;
		static const char* GLOBAL_UNIFORM_ACCESS_NUM_SPOT_LIGHTS;
		static const char* GLOBAL_UNIFORM_ACCESS_DIR_LIGHT_DIRECTIONS;
		static const char* GLOBAL_UNIFORM_ACCESS_DIR_LIGHT_COLORS;
		static const char* GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_POSITIONS;
		static const char* GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_COLORS;
		static const char* GLOBAL_UNIFORM_ACCESS_POINT_LIGHT_ATTENUATIONS;
		static const char* GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_POSITIONS;
		static const char* GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_DIRECTIONS;
		static const char* GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_COLORS;
		static const char* GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_ATTENUATIONS;
		static const char* GLOBAL_UNIFORM_ACCESS_SPOT_LIGHT_CUTOFFS;
		static const char* GLOBAL_UNIFORM_ACCESS_CASCADE_VIEW_PROJ_MATRICES;
		static const char* GLOBAL_UNIFORM_ACCESS_CASCADE_SPLIT_DISTANCES;
		static constexpr int MAX_DIRECTIONAL_LIGHTS = 4;
		static constexpr int MAX_POINT_LIGHTS = 8;
		static constexpr int MAX_SPOT_LIGHTS = 4;
		std::unique_ptr<UniformSet> _pGlobalUniformSet;
		std::unique_ptr<UniformSetMemory> _pGlobalUniformMemory;

		// Shadow map sampler (owned by VulkanResourceManager, valid for the duration of VulkanContext)
		VulkanSampler* _shadowSampler = nullptr;

		// Post-process chain
		std::unique_ptr<PostProcessChain> _postProcessChain;

		// Callback functions map
		std::unordered_map<void*, PreSwapChainRebuild> _preSwapChainRebuildCallbacks;
		std::unordered_map<void*, PostSwapChainRebuild> _postSwapChainRebuildCallbacks;
	};
} // namespace Ailurus
