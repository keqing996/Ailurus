#pragma once

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
	class VulkanUniformBuffer;
	class UniformSet;
	class UniformSetMemory;
	struct RenderIntermediateVariable;

	class RenderSystem : public NonCopyable, public NonMovable
	{
	public:
		~RenderSystem();

	public:
		void NeedRecreateSwapChain();

		auto GetRenderPass(RenderPassType pass) const -> RenderPass*;
		auto GetGlobalUniformSet() const -> UniformSet*;

		// Shader library
		ShaderLibrary* GetShaderLibrary() const;

		// Camera
		void SetMainCamera(CompCamera* pCamera);
		CompCamera* GetMainCamera() const;

		// Draw
		void RenderScene();
		void GraphicsWaitIdle() const;

	private:
		friend class Application;
		RenderSystem();

	private:
		// Create
		void CreateIntermediateVariable();
		void BuildRenderPass();
		void BuildGlobalUniform();

		// Render
		void CollectCameraViewProjectionMatrix();
		void CollectOpaqueRenderingObject();
		void UpdateGlobalUniformBuffer();
		void ReBuildSwapChain();
		void RenderSpecificPass(RenderPassType pass, VulkanCommandBuffer* pCommandBuffer);

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
	};
} // namespace Ailurus
