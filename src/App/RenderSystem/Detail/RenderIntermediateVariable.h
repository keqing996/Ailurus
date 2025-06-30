#pragma once

#include <array>
#include <vector>
#include <Ailurus/Math/Matrix4x4.hpp>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <VulkanSystem/Descriptor/VulkanDescriptorSet.h>

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

		// Rendering meshes
		std::vector<RenderingMesh> renderingMeshes;

		// Rendering descriptor sets
		std::array<VulkanDescriptorSet, EnumReflection<UniformSetUsage>::Size()> renderingDescriptorSets;
	};
}