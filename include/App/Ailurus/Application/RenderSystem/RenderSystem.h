#pragma once

#include <functional>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Application/SceneSystem/Component/CompCamera.h"

namespace Ailurus
{
	class MaterialInstance;
	class Mesh;
	class RenderPass;
	class CompStaticMeshRender;
	class VulkanCommandBuffer;
	class VulkanDescriptorAllocator;
	class VulkanUniformBuffer;
	class UniformSet;
	class UniformSetMemory;
	struct RenderIntermediateVariable;

	class RenderSystem : public NonCopyable, public NonMovable
	{
	public:
		using PreSwapChainRebuild = std::function<void()>;
		using PostSwapChainRebuild = std::function<void()>;
		
	public:
		RenderSystem();
		~RenderSystem();

	public:
		void RequestRebuildSwapChain();

		auto GetRenderPass(RenderPassType pass) const -> RenderPass*;
		auto GetGlobalUniformSet() const -> UniformSet*;

		// Shader library
		ShaderLibrary* GetShaderLibrary() const;

		// Camera
		void SetMainCamera(CompCamera* pCamera);
		CompCamera* GetMainCamera() const;

		// Callbacks
		void AddCallbackPreSwapChainRebuild(void* key, const PreSwapChainRebuild& callback);
		void AddCallbackPostSwapChainRebuild(void* key, const PostSwapChainRebuild& callback);
		void RemoveCallbackPreSwapChainRebuild(void* key);
		void RemoveCallbackPostSwapChainRebuild(void* key);

		// Draw
		void RenderScene();
		void GraphicsWaitIdle() const;	

	private:
		// Create
		void CreateIntermediateVariable();
		void BuildRenderPass();
		void BuildGlobalUniform();

		// Render
		void RenderPrepare();
		void CollectRenderingContext();
		void UpdateGlobalUniformBuffer(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator);
		void UpdateMaterialInstanceUniformBuffer(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator);
		void RebuildSwapChain();
		void RenderSpecificPass(RenderPassType pass, uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer);
		void RenderImGuiPass(VulkanCommandBuffer* pCommandBuffer);

		// Global uniform
		static auto GetGlobalUniformAccessNameViewProjMat() -> const std::string&;
		static auto GetGlobalUniformAccessNameCameraPos() -> const std::string&;

	private:
		bool _needRebuildSwapChain = false;
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> _renderPassMap;

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
		std::unique_ptr<UniformSet> _pGlobalUniformSet;
		std::unique_ptr<UniformSetMemory> _pGlobalUniformMemory;

		// Callback functions map
		std::unordered_map<void*, PreSwapChainRebuild> _preSwapChainRebuildCallbacks;
		std::unordered_map<void*, PostSwapChainRebuild> _postSwapChainRebuildCallbacks;
	};
} // namespace Ailurus
