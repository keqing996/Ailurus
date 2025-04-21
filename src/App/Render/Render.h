#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/Render/RenderPass/RenderPassType.h"
#include "Ailurus/Application/Render/Shader/ShaderLibrary.h"

namespace Ailurus
{
	class Material;
	class RenderPass;
	class CompMeshRender;

	class Render : public NonCopyable
	{
	public:
		~Render() override;

	public:
		void NeedRecreateSwapChain();

		// Material
		Material* GetMaterial(const std::string& name) const;
		Material* AddMaterial(const std::string& name);

		// Shader library
		ShaderLibrary* GetShaderLibrary() const;

		// Draw
		void RenderScene();
		void GraphicsWaitIdle() const;

	private:
		friend class Application;
		Render();

	private:
		void ReBuildSwapChain();
		void BuildRenderPass();
		void RenderForwardPass(std::vector<CompMeshRender*>& meshRenderList);
		void RenderMesh(const CompMeshRender* pMeshRender) const;

	private:
		bool _needRebuildSwapChain = false;
		std::unordered_map<std::string, std::unique_ptr<Material>> _materialMap;
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> _renderPassMap;
		const RenderPass* _pCurrentRenderPass = nullptr;

		// Shader library
		std::unique_ptr<ShaderLibrary> _pShaderLibrary;
	};
} // namespace Ailurus
