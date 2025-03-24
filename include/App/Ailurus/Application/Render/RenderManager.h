#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/RenderPass/RenderPassType.h"

namespace Ailurus
{
	class Material;
	class RenderPass;
	class MeshRender;
	class Flight;

	class RenderManager : public NonCopyable
	{
	public:
		RenderManager();
		~RenderManager() override;

	public:
		void NeedRecreateSwapChain();

		// Material
		Material* GetMaterial(const std::string& name) const;
		Material* AddMaterial(const std::string& name);

		// Draw
		void RenderScene(std::vector<MeshRender*>& objectList);

	private:
		void ReBuildSwapChain();
		void BuildRenderPass();
		void RenderForwardPass(std::vector<MeshRender*>& objectList, const Flight* pFlight);
		void RenderMesh(const Flight* pFlight, const MeshRender* pMeshRender) const;

	private:
		bool _needRebuildSwapChain = false;
		std::unordered_map<std::string, std::unique_ptr<Material>> _materialMap;
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> _renderPassMap;
		const RenderPass* _pCurrentRenderPass = nullptr;
	};
} // namespace Ailurus
