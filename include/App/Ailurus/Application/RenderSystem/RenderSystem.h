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
	struct RenderIntermediateVariable;

	class RenderSystem : public NonCopyable, public NonMovable
	{
	public:
		~RenderSystem();

	public:
		void NeedRecreateSwapChain();

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
		auto GetRenderPass(RenderPassType pass) const -> RenderPass*;

		// Render
		void CollectCameraViewProjectionMatrix();
		void CollectMaterialMeshMap();
		void ReBuildSwapChain();
		void RenderForwardPass(VulkanCommandBuffer* pCommandBuffer);
		void RenderMaterialMeshes(const MaterialInstance* pMatInst, const std::vector<const Mesh*>& pMeshList, VulkanCommandBuffer* pCommandBuffer);
		void RenderMesh(const CompStaticMeshRender* pMeshRender, VulkanCommandBuffer* pCommandBuffer) const;

	private:
		bool _needRebuildSwapChain = false;
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> _renderPassMap;

		// Current main camera
		CompCamera* _pMainCamera = nullptr;

		// Shader library
		std::unique_ptr<ShaderLibrary> _pShaderLibrary;

		// Intermediate variables for every frame
		std::unique_ptr<RenderIntermediateVariable> _pIntermediateVariable;
	};
} // namespace Ailurus
