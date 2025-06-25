#pragma once

#include <unordered_map>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>

namespace Ailurus
{
	class VulkanVertexLayout;
	class Mesh;
	class Material;
	class Entity;

	struct RenderingMeshInfo
	{
		const Mesh* pTargetMesh;
		const Entity* pEntity;
	};

	struct RenderInfoPerVertexLayout
	{
		const VulkanVertexLayout* pVertexLayout;
		std::vector<RenderingMeshInfo> meshes;
	};

	struct RenderInfoPerMaterial
	{
		const Material* pMaterial;
		std::unordered_map<uint64_t, RenderInfoPerVertexLayout> vertexLayoutsMap;
	};

	struct RenderInfoPerPass
	{
		std::unordered_map<uint32_t, RenderInfoPerMaterial> materialsMap;
	};

	struct RenderInfo
	{
		std::unordered_map<RenderPassType, RenderInfoPerPass> renderPassesMap;
	};

}