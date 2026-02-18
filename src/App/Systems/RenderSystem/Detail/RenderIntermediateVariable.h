#pragma once

#include <array>
#include <vector>
#include <Ailurus/Math/Matrix4x4.hpp>
#include <Ailurus/Math/Vector4.hpp>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSet.h>

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

		// Light data (packed as Vector4f for GPU upload)
		int numDirectionalLights = 0;
		int numPointLights = 0;
		int numSpotLights = 0;
		std::vector<Vector4f> dirLightDirections;  // xyz = direction, w = unused
		std::vector<Vector4f> dirLightColors;      // xyz = color, w = intensity
		std::vector<Vector4f> pointLightPositions; // xyz = position, w = unused
		std::vector<Vector4f> pointLightColors;    // xyz = color, w = intensity
		std::vector<Vector4f> pointLightAttenuations; // xyz = attenuation, w = unused
		std::vector<Vector4f> spotLightPositions;  // xyz = position, w = unused
		std::vector<Vector4f> spotLightDirections; // xyz = direction, w = unused
		std::vector<Vector4f> spotLightColors;     // xyz = color, w = intensity
		std::vector<Vector4f> spotLightAttenuations; // xyz = attenuation, w = unused
		std::vector<Vector4f> spotLightCutoffs;    // x = cos(inner), y = cos(outer), zw = unused

		// CSM data
		static constexpr uint32_t CSM_CASCADE_COUNT = 4;
		std::array<Matrix4x4f, CSM_CASCADE_COUNT> cascadeViewProjMatrices;
		std::array<float, CSM_CASCADE_COUNT> cascadeSplitDistances;
	};
}