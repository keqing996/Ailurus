#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/RenderPass/RenderPassType.h"

namespace Ailurus
{
	class Material;
	class MeshRender;

	class RenderManager : public NonCopyable
	{
	public:
		// Material
		Material* GetMaterial(const std::string& name) const;
		Material* AddMaterial(const std::string& name);

		// Draw
		void RenderScene();

	private:
		std::unordered_map<std::string, std::unique_ptr<Material>> _materialMap;
	};
} // namespace Ailurus