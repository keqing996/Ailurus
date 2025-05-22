#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Application/SceneSystem/Component/CompCamera.h"

namespace Ailurus
{
	class Material;
	class RenderPass;
	class CompStaticMeshRender;

	class RenderSystem : public NonCopyable, public NonMovable
	{
	public:
		~RenderSystem();

	public:
		void NeedRecreateSwapChain();

		// Material
		Material* GetMaterial(const std::string& name) const;
		Material* AddMaterial(const std::string& name);

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
		void ReBuildSwapChain();
		void BuildRenderPass();
		void RenderForwardPass(std::vector<CompStaticMeshRender*>& meshRenderList);
		void RenderMesh(const CompStaticMeshRender* pMeshRender, class VulkanCommandBuffer* pCommandBuffer) const;

	private:
		bool _needRebuildSwapChain = false;
		std::unordered_map<std::string, std::unique_ptr<Material>> _materialMap;
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> _renderPassMap;
		const RenderPass* _pCurrentRenderPass = nullptr;

		// Current main camera
		CompCamera* _pMainCamera = nullptr;

		// Shader library
		std::unique_ptr<ShaderLibrary> _pShaderLibrary;
	};
} // namespace Ailurus
