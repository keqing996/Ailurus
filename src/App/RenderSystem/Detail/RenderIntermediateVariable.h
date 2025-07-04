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
		using MatInstDescriptorSetMap = std::unordered_map<const MaterialInstance*, VulkanDescriptorSet>;

		// View and projection matrices
		Matrix4x4f viewProjectionMatrix;

		// Rendering meshes
		std::unordered_map<RenderPassType, std::vector<RenderingMesh>> renderingMeshes;

		// Material instance descriptor sets map
		std::unordered_map<RenderPassType, MatInstDescriptorSetMap> materialInstanceDescriptorsMap;

		// Rendering descriptor sets
		std::array<VulkanDescriptorSet, EnumReflection<UniformSetUsage>::Size()> renderingDescriptorSets;
	};
}