#pragma once

#include <vector>
#include <Ailurus/Math/Matrix4x4.hpp>

namespace Ailurus
{
	class VulkanVertexLayout;
	class Mesh;
	class Material;
	class MaterialInstance;
	class Entity;

	struct RenderingMesh
	{
		const Material* pMaterial;
		const MaterialInstance* pMaterialInstance;
		uint64_t vertexLayoutId;
		const Mesh* pTargetMesh;
		
		// Additional information
		const Entity* pEntity;
	};

	struct RenderIntermediateVariable
	{
		// View and projection matrices
		Matrix4x4f viewProjectionMatrix;
		std::vector<RenderingMesh> renderingMeshes;
	};
}