#include <algorithm>
#include <cstdint>
#include <array>
#include <cmath>
#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Application/AssetsSystem/Mesh/Mesh.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Application/SceneSystem/SceneSystem.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Application/SceneSystem/Component/CompLight.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/SwapChain/VulkanSwapChain.h>
#include <VulkanContext/RenderTarget/RenderTargetManager.h>
#include <VulkanContext/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanContext/Pipeline/VulkanPipelineManager.h>
#include <VulkanContext/Pipeline/VulkanPipelineEntry.h>
#include <VulkanContext/DataBuffer/VulkanVertexBuffer.h>
#include <VulkanContext/DataBuffer/VulkanIndexBuffer.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Vertex/VulkanVertexLayoutManager.h>
#include <VulkanContext/Descriptor/VulkanDescriptorAllocator.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>
#include <VulkanContext/Descriptor/VulkanDescriptorWriter.h>
#include <VulkanContext/Resource/Image/VulkanSampler.h>
#include "Ailurus/Application/ImGuiSystem/ImGuiSystem.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Detail/RenderIntermediateVariable.h"

namespace Ailurus
{
	void RenderSystem::RenderPrepare()
	{
		const Matrix4x4f projMat = _pMainCamera->GetProjectionMatrix();
		const Matrix4x4f viewMat = _pMainCamera->GetViewMatrix();
		_pIntermediateVariable->viewProjectionMatrix = projMat * viewMat;
		_pIntermediateVariable->renderingMeshes.clear();
		_pIntermediateVariable->materialInstanceDescriptorsMap.clear();
		_pIntermediateVariable->renderingDescriptorSets.fill(vk::DescriptorSet{});
	}

	void RenderSystem::CollectRenderingContext()
	{
		auto& renderingMeshesMap = _pIntermediateVariable->renderingMeshes;
		renderingMeshesMap.clear();

		const auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
		for (const auto pEntity : allEntities)
		{
			const auto pMeshRender = pEntity->GetComponent<CompStaticMeshRender>();
			if (pMeshRender == nullptr)
				continue;

			const auto& modelRef = pMeshRender->GetModelAsset();
			if (!modelRef)
				continue;

			const auto& materialInstRef = pMeshRender->GetMaterialInstanceAsset();
			if (!materialInstRef)
				continue;

			const auto* pMaterial = materialInstRef->GetTargetMaterial();
			const auto* pMaterialInstance = materialInstRef.Get();
			for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
			{
				auto passType = static_cast<RenderPassType>(i);
				if (!pMaterial->HasRenderPass(passType))
					continue;

				const auto& allMeshes = modelRef->GetMeshes();
				for (const auto& pMesh : allMeshes)
				{
					const auto vertexLayoutId = pMesh->GetVertexLayoutId();

					renderingMeshesMap[passType].push_back(RenderingMesh{
						pMaterial,
						pMaterialInstance,
						vertexLayoutId,
						pMesh.get(),
						pEntity });
				}
			}
		}

		// Opaque meshes sorted by material & material instance
		auto& forwardPassMeshes = renderingMeshesMap[RenderPassType::Forward];
		std::sort(forwardPassMeshes.begin(), forwardPassMeshes.end(),
			[](const RenderingMesh& lhs, const RenderingMesh& rhs) -> bool {
				// Compare by Material
				if (lhs.pMaterial != rhs.pMaterial)
					return lhs.pMaterial < rhs.pMaterial;

				// Compare by MaterialInstance
				if (lhs.pMaterialInstance != rhs.pMaterialInstance)
					return lhs.pMaterialInstance < rhs.pMaterialInstance;

				// Compare by vertexLayoutId
				return lhs.vertexLayoutId < rhs.vertexLayoutId;
			});
	}

	void RenderSystem::CollectLights()
	{
		// Clear previous light data
		auto& var = _pIntermediateVariable;
		var->numDirectionalLights = 0;
		var->numPointLights = 0;
		var->numSpotLights = 0;
		var->dirLightDirections.clear();
		var->dirLightColors.clear();
		var->pointLightPositions.clear();
		var->pointLightColors.clear();
		var->pointLightAttenuations.clear();
		var->spotLightPositions.clear();
		var->spotLightDirections.clear();
		var->spotLightColors.clear();
		var->spotLightAttenuations.clear();
		var->spotLightCutoffs.clear();

		// Collect lights from all entities
		const auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
		for (const auto pEntity : allEntities)
		{
			const auto pLight = pEntity->GetComponent<CompLight>();
			if (pLight == nullptr)
				continue;

			const LightType lightType = pLight->GetLightType();
			const Vector3f color = pLight->GetColor();
			const float intensity = pLight->GetIntensity();

			if (lightType == LightType::Directional && var->numDirectionalLights < MAX_DIRECTIONAL_LIGHTS)
			{
				const Vector3f direction = pLight->GetDirection();
				var->dirLightDirections.push_back(Vector4f(direction.x, direction.y, direction.z, 0.0f));
				var->dirLightColors.push_back(Vector4f(color.x, color.y, color.z, intensity));
				var->numDirectionalLights++;
			}
			else if (lightType == LightType::Point && var->numPointLights < MAX_POINT_LIGHTS)
			{
				const Vector3f position = pEntity->GetPosition();
				const Vector3f attenuation = pLight->GetAttenuation();
				var->pointLightPositions.push_back(Vector4f(position.x, position.y, position.z, 0.0f));
				var->pointLightColors.push_back(Vector4f(color.x, color.y, color.z, intensity));
				var->pointLightAttenuations.push_back(Vector4f(attenuation.x, attenuation.y, attenuation.z, 0.0f));
				var->numPointLights++;
			}
			else if (lightType == LightType::Spot && var->numSpotLights < MAX_SPOT_LIGHTS)
			{
				const Vector3f position = pEntity->GetPosition();
				const Vector3f direction = pLight->GetDirection();
				const Vector3f attenuation = pLight->GetAttenuation();
				const float innerCutoff = pLight->GetInnerCutoff();
				const float outerCutoff = pLight->GetOuterCutoff();
				
				// Convert degrees to radians and then to cosine
				const float innerRadians = innerCutoff * static_cast<float>(M_PI) / 180.0f;
				const float outerRadians = outerCutoff * static_cast<float>(M_PI) / 180.0f;
				const float cosInner = std::cos(innerRadians);
				const float cosOuter = std::cos(outerRadians);

				var->spotLightPositions.push_back(Vector4f(position.x, position.y, position.z, 0.0f));
				var->spotLightDirections.push_back(Vector4f(direction.x, direction.y, direction.z, 0.0f));
				var->spotLightColors.push_back(Vector4f(color.x, color.y, color.z, intensity));
				var->spotLightAttenuations.push_back(Vector4f(attenuation.x, attenuation.y, attenuation.z, 0.0f));
				var->spotLightCutoffs.push_back(Vector4f(cosInner, cosOuter, 0.0f, 0.0f));
				var->numSpotLights++;
			}
		}

		// Pad arrays to max size with zeros
		var->dirLightDirections.resize(MAX_DIRECTIONAL_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->dirLightColors.resize(MAX_DIRECTIONAL_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));

		var->pointLightPositions.resize(MAX_POINT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->pointLightColors.resize(MAX_POINT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->pointLightAttenuations.resize(MAX_POINT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));

		var->spotLightPositions.resize(MAX_SPOT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->spotLightDirections.resize(MAX_SPOT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->spotLightColors.resize(MAX_SPOT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->spotLightAttenuations.resize(MAX_SPOT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
		var->spotLightCutoffs.resize(MAX_SPOT_LIGHTS, Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
	}

	void RenderSystem::CalculateCascadeShadows()
	{
		auto& var = _pIntermediateVariable;
		
		// Only calculate CSM if we have at least one directional light
		if (var->numDirectionalLights == 0)
		{
			// Set identity matrices and zero distances
			for (uint32_t i = 0; i < RenderIntermediateVariable::CSM_CASCADE_COUNT; i++)
			{
				var->cascadeViewProjMatrices[i] = Matrix4x4f::Identity();
				var->cascadeSplitDistances[i] = 0.0f;
			}
			return;
		}

		// Get camera info
		const float nearPlane = _pMainCamera->GetNear();
		const float farPlane = _pMainCamera->GetFar();
		const Vector3f cameraPos = _pMainCamera->GetEntity()->GetPosition();
		const Quaternionf cameraRot = _pMainCamera->GetEntity()->GetRotation();
		
		// Use the first directional light for shadow calculation
		const Vector3f lightDir = Vector3f(
			var->dirLightDirections[0].x,
			var->dirLightDirections[0].y,
			var->dirLightDirections[0].z
		).Normalize();

		// Calculate cascade split distances using practical split scheme (blend of logarithmic and uniform)
		const float lambda = 0.5f; // Blend factor between uniform (0) and logarithmic (1)
		for (uint32_t i = 0; i < RenderIntermediateVariable::CSM_CASCADE_COUNT; i++)
		{
			float p = static_cast<float>(i + 1) / static_cast<float>(RenderIntermediateVariable::CSM_CASCADE_COUNT);
			float log = nearPlane * std::pow(farPlane / nearPlane, p);
			float uniform = nearPlane + (farPlane - nearPlane) * p;
			float d = lambda * log + (1.0f - lambda) * uniform;
			var->cascadeSplitDistances[i] = d;
		}

		// Camera forward direction
		Matrix4x4f cameraRotMat = Math::QuaternionToRotateMatrix(cameraRot);
		Vector3f forward = Vector3f(-cameraRotMat[0][2], -cameraRotMat[1][2], -cameraRotMat[2][2]).Normalize();
		Vector3f right = Vector3f(cameraRotMat[0][0], cameraRotMat[1][0], cameraRotMat[2][0]).Normalize();
		Vector3f up = Vector3f(cameraRotMat[0][1], cameraRotMat[1][1], cameraRotMat[2][1]).Normalize();
		
		// Get camera frustum size at near plane
		const float tanHalfFovy = std::tan(_pMainCamera->GetHeight() / (2.0f * _pMainCamera->GetNear()));
		const float aspectRatio = _pMainCamera->GetWidth() / _pMainCamera->GetHeight();
		
		// Previous cascade far plane (starts at near plane)
		float prevSplitDist = nearPlane;
		
		for (uint32_t cascadeIndex = 0; cascadeIndex < RenderIntermediateVariable::CSM_CASCADE_COUNT; cascadeIndex++)
		{
			float splitDist = var->cascadeSplitDistances[cascadeIndex];
			
			// Calculate frustum corner sizes for this cascade
			float nearHeight = tanHalfFovy * prevSplitDist;
			float nearWidth = nearHeight * aspectRatio;
			float farHeight = tanHalfFovy * splitDist;
			float farWidth = farHeight * aspectRatio;
			
			// Calculate 8 frustum corners in world space
			Vector3f frustumCorners[8];
			
			// Near plane corners
			Vector3f nearCenter = cameraPos + forward * prevSplitDist;
			frustumCorners[0] = nearCenter + up * nearHeight - right * nearWidth;
			frustumCorners[1] = nearCenter + up * nearHeight + right * nearWidth;
			frustumCorners[2] = nearCenter - up * nearHeight - right * nearWidth;
			frustumCorners[3] = nearCenter - up * nearHeight + right * nearWidth;
			
			// Far plane corners
			Vector3f farCenter = cameraPos + forward * splitDist;
			frustumCorners[4] = farCenter + up * farHeight - right * farWidth;
			frustumCorners[5] = farCenter + up * farHeight + right * farWidth;
			frustumCorners[6] = farCenter - up * farHeight - right * farWidth;
			frustumCorners[7] = farCenter - up * farHeight + right * farWidth;
			
			// Calculate frustum center
			Vector3f center(0.0f, 0.0f, 0.0f);
			for (const auto& corner : frustumCorners)
				center = center + corner;
			center = center / 8.0f;
			
			// Create light view matrix
			// Light position is behind the frustum center
			Vector3f lightPos = center - lightDir * 50.0f;
			
			// Build light view matrix manually (look at from lightPos to center)
			Vector3f lightForward = (center - lightPos).Normalize();
			Vector3f lightUp = Vector3f(0.0f, 1.0f, 0.0f);
			
			// If light direction is too close to world up, use alternative up vector
			if (std::abs(lightForward.Dot(lightUp)) > 0.99f)
				lightUp = Vector3f(1.0f, 0.0f, 0.0f);
			
			Vector3f lightRight = lightForward.Cross(lightUp).Normalize();
			lightUp = lightRight.Cross(lightForward).Normalize();
			
			// Build view matrix
			Matrix4x4f lightView{
				{ lightRight.x,   lightRight.y,   lightRight.z,   -lightRight.Dot(lightPos) },
				{ lightUp.x,      lightUp.y,      lightUp.z,      -lightUp.Dot(lightPos) },
				{ -lightForward.x, -lightForward.y, -lightForward.z, lightForward.Dot(lightPos) },
				{ 0.0f,           0.0f,           0.0f,           1.0f }
			};
			
			// Transform frustum corners to light space and find min/max
			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::lowest();
			
			for (const auto& corner : frustumCorners)
			{
				Vector4f lightSpaceCorner = lightView * Vector4f(corner.x, corner.y, corner.z, 1.0f);
				minX = std::min(minX, lightSpaceCorner.x);
				maxX = std::max(maxX, lightSpaceCorner.x);
				minY = std::min(minY, lightSpaceCorner.y);
				maxY = std::max(maxY, lightSpaceCorner.y);
				minZ = std::min(minZ, lightSpaceCorner.z);
				maxZ = std::max(maxZ, lightSpaceCorner.z);
			}
			
			// Extend the Z range to capture objects that might cast shadows into frustum
			const float zExtension = 50.0f;
			minZ -= zExtension;
			
			// Create orthographic projection for this cascade
			// Using the helper function from CompCamera.cpp
			float a11 = 2.0f / (maxX - minX);
			float a22 = 2.0f / (maxY - minY);
			float a33 = 2.0f / (maxZ - minZ);
			float a34 = -(maxZ + minZ) / (maxZ - minZ);
			
			Matrix4x4f lightProj = -1.0f * Matrix4x4f{
				{ a11,  0.0f, 0.0f, 0.0f },
				{ 0.0f, a22,  0.0f, 0.0f },
				{ 0.0f, 0.0f, a33,  a34  },
				{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
			
			// Store the combined light view-projection matrix
			var->cascadeViewProjMatrices[cascadeIndex] = lightProj * lightView;
			
			// Update for next cascade
			prevSplitDist = splitDist;
		}
	}

	void RenderSystem::CreateIntermediateVariable()
	{
		_pIntermediateVariable = std::make_unique<RenderIntermediateVariable>();
	}

	void RenderSystem::CheckRebuildSwapChain()
	{
		if (_needRebuildSwapChain)
			RebuildSwapChain();
	}

	void RenderSystem::RenderScene()
	{
		VulkanContext::RenderFrame(&_needRebuildSwapChain,
			[this](uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator) -> void {
				// Get swap chain for image layout transitions
				auto pSwapChain = VulkanContext::GetSwapChain();
				const auto& swapChainImages = pSwapChain->GetSwapChainImages();
				vk::Image currentImage = swapChainImages[swapChainImageIndex];
				const auto& swapChainImageViews = pSwapChain->GetSwapChainImageViews();
				vk::ImageView currentImageView = swapChainImageViews[swapChainImageIndex];
				vk::Extent2D extent = pSwapChain->GetConfig().extent;

				// Transition swapchain image to color attachment optimal
				pCommandBuffer->ImageMemoryBarrier(
					currentImage,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::AccessFlags{},
					vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eColorAttachmentOutput);

				if (_enable3D)
				{
					auto* pRenderTargetManager = VulkanContext::GetRenderTargetManager();
					vk::Image offscreenImage = pRenderTargetManager->GetOffscreenColorImage();
					vk::ImageView offscreenImageView = pRenderTargetManager->GetOffscreenColorImageView();

					// Transition offscreen HDR RT to color attachment optimal
					pCommandBuffer->ImageMemoryBarrier(
						offscreenImage,
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::eColorAttachmentOptimal,
						vk::AccessFlags{},
						vk::AccessFlagBits::eColorAttachmentWrite,
						vk::PipelineStageFlagBits::eTopOfPipe,
						vk::PipelineStageFlagBits::eColorAttachmentOutput);

					RenderPrepare();
					CollectRenderingContext();
					CollectLights();
					CalculateCascadeShadows();
					UpdateGlobalUniformBuffer(pCommandBuffer, pDescriptorAllocator);
					UpdateMaterialInstanceUniformBuffer(pCommandBuffer, pDescriptorAllocator);

					// Shadow pass
					RenderShadowPass(pCommandBuffer, pDescriptorAllocator);

					// Forward pass (renders to offscreen HDR RT)
					RenderPass(RenderPassType::Forward, pCommandBuffer);

					// Transition offscreen HDR RT to shader read only for post-process sampling
					pCommandBuffer->ImageMemoryBarrier(
						offscreenImage,
						vk::ImageLayout::eColorAttachmentOptimal,
						vk::ImageLayout::eShaderReadOnlyOptimal,
						vk::AccessFlagBits::eColorAttachmentWrite,
						vk::AccessFlagBits::eShaderRead,
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eFragmentShader);

					if (_postProcessChain && _postProcessChain->HasEnabledEffects())
					{
						// Execute post-process chain: offscreen RT → swapchain
						_postProcessChain->Execute(
							pCommandBuffer,
							offscreenImage, offscreenImageView,
							currentImage, currentImageView,
							extent, pDescriptorAllocator);
					}
					else
					{
						// No post-process: blit offscreen RT to swapchain
						pCommandBuffer->ImageMemoryBarrier(
							offscreenImage,
							vk::ImageLayout::eShaderReadOnlyOptimal,
							vk::ImageLayout::eTransferSrcOptimal,
							vk::AccessFlagBits::eShaderRead,
							vk::AccessFlagBits::eTransferRead,
							vk::PipelineStageFlagBits::eFragmentShader,
							vk::PipelineStageFlagBits::eTransfer);

						pCommandBuffer->ImageMemoryBarrier(
							currentImage,
							vk::ImageLayout::eColorAttachmentOptimal,
							vk::ImageLayout::eTransferDstOptimal,
							vk::AccessFlagBits::eColorAttachmentWrite,
							vk::AccessFlagBits::eTransferWrite,
							vk::PipelineStageFlagBits::eColorAttachmentOutput,
							vk::PipelineStageFlagBits::eTransfer);

						vk::ImageBlit blitRegion;
						blitRegion.setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
							.setSrcOffsets({ vk::Offset3D{0, 0, 0}, vk::Offset3D{static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), 1} })
							.setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
							.setDstOffsets({ vk::Offset3D{0, 0, 0}, vk::Offset3D{static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), 1} });

						pCommandBuffer->GetBuffer().blitImage(
							offscreenImage, vk::ImageLayout::eTransferSrcOptimal,
							currentImage, vk::ImageLayout::eTransferDstOptimal,
							blitRegion, vk::Filter::eLinear);

						pCommandBuffer->ImageMemoryBarrier(
							currentImage,
							vk::ImageLayout::eTransferDstOptimal,
							vk::ImageLayout::eColorAttachmentOptimal,
							vk::AccessFlagBits::eTransferWrite,
							vk::AccessFlagBits::eColorAttachmentWrite,
							vk::PipelineStageFlagBits::eTransfer,
							vk::PipelineStageFlagBits::eColorAttachmentOutput);
					}
				}

				// ImGui pass (renders on top of swapchain, which is in ColorAttachment layout)
				if (_enableImGui)
					RenderImGuiPass(swapChainImageIndex, pCommandBuffer);

				// Transition image to present layout after rendering
				pCommandBuffer->ImageMemoryBarrier(
					currentImage,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::ePresentSrcKHR,
					vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlags{},
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::PipelineStageFlagBits::eBottomOfPipe);
			});
	}

	void RenderSystem::UpdateGlobalUniformBuffer(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator)
	{
		auto& var = _pIntermediateVariable;

		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameViewProjMat() },
			var->viewProjectionMatrix);

		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameCameraPos() },
			_pMainCamera->GetEntity()->GetPosition());

		// Set light counts
		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameNumDirLights() },
			var->numDirectionalLights);

		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameNumPointLights() },
			var->numPointLights);

		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameNumSpotLights() },
			var->numSpotLights);

		// Set directional light arrays
		for (uint32_t i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++)
		{
			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameDirLightDirections(), i },
				var->dirLightDirections[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameDirLightColors(), i },
				var->dirLightColors[i]);
		}

		// Set point light arrays
		for (uint32_t i = 0; i < MAX_POINT_LIGHTS; i++)
		{
			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNamePointLightPositions(), i },
				var->pointLightPositions[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNamePointLightColors(), i },
				var->pointLightColors[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNamePointLightAttenuations(), i },
				var->pointLightAttenuations[i]);
		}

		// Set spot light arrays
		for (uint32_t i = 0; i < MAX_SPOT_LIGHTS; i++)
		{
			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameSpotLightPositions(), i },
				var->spotLightPositions[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameSpotLightDirections(), i },
				var->spotLightDirections[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameSpotLightColors(), i },
				var->spotLightColors[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameSpotLightAttenuations(), i },
				var->spotLightAttenuations[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameSpotLightCutoffs(), i },
				var->spotLightCutoffs[i]);
		}

		// Set CSM cascade matrices and split distances
		for (uint32_t i = 0; i < RenderIntermediateVariable::CSM_CASCADE_COUNT; i++)
		{
			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameCascadeViewProjMatrices(), i },
				var->cascadeViewProjMatrices[i]);

			_pGlobalUniformMemory->SetUniformValue(
				{ 0, GetGlobalUniformAccessNameCascadeSplitDistances(), i },
				var->cascadeSplitDistances[i]);
		}

		// Use cache to get or allocate descriptor set
		// Key: layout only (global uniform always uses same layout)
		auto globalUniformSetLayout = _pGlobalUniformSet->GetDescriptorSetLayout();
		
		VulkanDescriptorAllocator::CacheKey key;
		key.layout = globalUniformSetLayout->GetDescriptorSetLayout();
		key.bindingHash = 1; // Fixed hash for global uniform (to distinguish from material)
		
		auto globalDescriptorSet = pDescriptorAllocator->AllocateDescriptorSet(globalUniformSetLayout, &key);

		// Save set
		_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::General)] = globalDescriptorSet;

		// IMPORTANT: Always update descriptor set data (buffer content changes each frame)
		_pGlobalUniformMemory->UpdateToDescriptorSet(pCommandBuffer, globalDescriptorSet);

		// Write shadow map image views to the global descriptor set (bindings 1-4)
		if (_shadowSampler != nullptr)
		{
			auto* pRenderTargetManager = VulkanContext::GetRenderTargetManager();
			VulkanDescriptorWriter shadowWriter;
			for (uint32_t i = 0; i < pRenderTargetManager->GetShadowMapCascadeCount(); i++)
			{
				vk::ImageView shadowMapView = pRenderTargetManager->GetShadowMapImageView(i);
				if (shadowMapView)
					shadowWriter.WriteImage(i + 1, shadowMapView, _shadowSampler->GetSampler());
			}
			shadowWriter.UpdateSet(globalDescriptorSet);
		}
	}

	void RenderSystem::UpdateMaterialInstanceUniformBuffer(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator)
	{
		auto& opaqueMeshes = _pIntermediateVariable->renderingMeshes;
		for (auto& [pass, meshes] : opaqueMeshes)
		{
			auto& mapInstDescriptorSetMap = _pIntermediateVariable->materialInstanceDescriptorsMap[pass];
			for (auto& pRenderingMesh : meshes)
			{
				const auto* pMaterial = pRenderingMesh.pMaterial;
				const auto* pMaterialInstance = pRenderingMesh.pMaterialInstance;

				if (mapInstDescriptorSetMap.contains(pMaterialInstance))
					continue;

				// Skip passes that have no material uniform set (e.g., shadow pass)
				auto* pUniformSet = pMaterial->GetUniformSet(pass);
				if (pUniformSet == nullptr)
					continue;

				// Use cache to get or allocate descriptor set
				auto* pDescriptorLayout = pUniformSet->GetDescriptorSetLayout();
				
				// Create cache key based on layout and material instance
				VulkanDescriptorAllocator::CacheKey key;
				key.layout = pDescriptorLayout->GetDescriptorSetLayout();
				key.bindingHash = reinterpret_cast<size_t>(pMaterialInstance); // Use material instance pointer as hash
				
				auto descriptorSet = pDescriptorAllocator->AllocateDescriptorSet(pDescriptorLayout, &key);

				// Update material data to descriptor set (pass material instance for texture binding)
				pMaterialInstance->GetUniformSetMemory(pass)->UpdateToDescriptorSet(pCommandBuffer, descriptorSet, pMaterialInstance);

				// Record material instance descriptor set
				mapInstDescriptorSetMap[pMaterialInstance] = descriptorSet;
			}
		}
	}

	void RenderSystem::RenderShadowPass(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator)
	{
		auto* pRenderTargetManager = VulkanContext::GetRenderTargetManager();
		const uint32_t cascadeCount = pRenderTargetManager->GetShadowMapCascadeCount();
		const uint32_t shadowMapSize = 2048;

		auto& shadowMeshes = _pIntermediateVariable->renderingMeshes[RenderPassType::Shadow];

		for (uint32_t cascadeIndex = 0; cascadeIndex < cascadeCount; cascadeIndex++)
		{
			vk::Image shadowImage = pRenderTargetManager->GetShadowMapImage(cascadeIndex);
			vk::ImageView shadowImageView = pRenderTargetManager->GetShadowMapImageView(cascadeIndex);

			if (!shadowImage || !shadowImageView)
				continue;

			// Transition shadow map to depth attachment optimal
			pCommandBuffer->ImageMemoryBarrier(
				shadowImage,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				vk::AccessFlags{},
				vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eEarlyFragmentTests,
				vk::ImageAspectFlagBits::eDepth);

			// Begin depth-only rendering (clears and stores depth)
			pCommandBuffer->BeginDepthOnlyRendering(shadowImageView, vk::Extent2D{shadowMapSize, shadowMapSize});
			pCommandBuffer->SetViewportAndScissor(shadowMapSize, shadowMapSize);

			// Render shadow meshes for this cascade
			VulkanPipeline* pCurrentVkPipeline = nullptr;
			uint64_t currentVertexLayoutId = 0;

			for (const auto& renderingMesh : shadowMeshes)
			{
				if (currentVertexLayoutId != renderingMesh.vertexLayoutId)
				{
					currentVertexLayoutId = renderingMesh.vertexLayoutId;

					VulkanPipelineEntry pipelineEntry(RenderPassType::Shadow, renderingMesh.pMaterial->GetAssetId(), currentVertexLayoutId);
					pCurrentVkPipeline = VulkanContext::GetPipelineManager()->GetPipeline(pipelineEntry);
					if (pCurrentVkPipeline == nullptr)
					{
						Logger::LogError("RenderShadowPass: Shadow pipeline not found");
						continue;
					}

					pCommandBuffer->BindPipeline(pCurrentVkPipeline);

					// Shadow pass only uses the global descriptor set (no material-specific set)
					std::vector<vk::DescriptorSet> descriptorSets{
						_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::General)]
					};
					pCommandBuffer->BindDescriptorSet(pCurrentVkPipeline->GetPipelineLayout(), descriptorSets);
				}

				if (pCurrentVkPipeline == nullptr)
					continue;

				// Push model matrix and cascade index
				pCommandBuffer->PushConstantShadowData(pCurrentVkPipeline, renderingMesh.pEntity->GetModelMatrix(), cascadeIndex);

				// Bind vertex buffer
				const auto pVertexBuffer = renderingMesh.pTargetMesh->GetVertexBuffer();
				pCommandBuffer->BindVertexBuffer(pVertexBuffer);

				// Draw
				const auto pIndexBuffer = renderingMesh.pTargetMesh->GetIndexBuffer();
				if (pIndexBuffer != nullptr)
				{
					pCommandBuffer->BindIndexBuffer(pIndexBuffer);
					pCommandBuffer->DrawIndexed(pIndexBuffer->GetIndexCount());
				}
				else
				{
					pCommandBuffer->DrawNonIndexed(renderingMesh.pTargetMesh->GetVertexCount());
				}
			}

			pCommandBuffer->EndRendering();

			// Transition shadow map to shader read only for sampling in the forward pass
			pCommandBuffer->ImageMemoryBarrier(
				shadowImage,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal,
				vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits::eShaderRead,
				vk::PipelineStageFlagBits::eLateFragmentTests,
				vk::PipelineStageFlagBits::eFragmentShader,
				vk::ImageAspectFlagBits::eDepth);
		}
	}

	void RenderSystem::RenderPass(RenderPassType pass, VulkanCommandBuffer* pCommandBuffer)
	{
		if (_pIntermediateVariable->renderingMeshes.empty())
			return;

		// Get swap chain extent
		auto pSwapChain = VulkanContext::GetSwapChain();
		if (pSwapChain == nullptr)
		{
			Logger::LogError("SwapChain not found");
			return;
		}

		vk::Extent2D extent = pSwapChain->GetConfig().extent;

		// Check if MSAA is enabled
		bool useMSAA = VulkanContext::GetMSAASamples() != vk::SampleCountFlagBits::e1;
		auto* pRenderTargetManager = VulkanContext::GetRenderTargetManager();

		// Offscreen HDR RT is the render target for the forward pass
		vk::ImageView offscreenColorView = pRenderTargetManager->GetOffscreenColorImageView();

		if (useMSAA)
		{
			// With MSAA: render to MSAA color attachment and resolve to offscreen HDR RT
			vk::ImageView msaaColorView = pRenderTargetManager->GetMSAAColorImageView();
			vk::ImageView msaaDepthView = pRenderTargetManager->GetMSAADepthImageView();

			bool clearColor = (pass == RenderPassType::Forward);
			pCommandBuffer->BeginRendering(msaaColorView, msaaDepthView, offscreenColorView, extent, clearColor, true);
		}
		else
		{
			// Without MSAA: render directly to offscreen HDR RT
			vk::ImageView depthImageView = pRenderTargetManager->GetDepthImageView();

			bool clearColor = (pass == RenderPassType::Forward);
			pCommandBuffer->BeginRendering(offscreenColorView, depthImageView, nullptr, extent, clearColor, true);
		}

		// Intermediate variables
		const Material* pCurrentMaterial = nullptr;
		const MaterialInstance* pCurrentMaterialInstance = nullptr;
		uint64_t currentVertexLayoutId = 0;

		// Pipeline variables
		VulkanPipeline* pCurrentVkPipeline;
		for (const auto& renderingMesh : _pIntermediateVariable->renderingMeshes[pass])
		{
			if (renderingMesh.pMaterial != pCurrentMaterial)
			{
				pCurrentMaterial = renderingMesh.pMaterial;

				// Reset the material instance and the vertex layout
				pCurrentMaterialInstance = nullptr;
				currentVertexLayoutId = 0;
			}

			if (renderingMesh.pMaterialInstance != pCurrentMaterialInstance)
			{
				pCurrentMaterialInstance = renderingMesh.pMaterialInstance;

				_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::MaterialCustom)] =
					_pIntermediateVariable->materialInstanceDescriptorsMap[pass][pCurrentMaterialInstance];
			}

			if (currentVertexLayoutId != renderingMesh.vertexLayoutId)
			{
				currentVertexLayoutId = renderingMesh.vertexLayoutId;

				// Get vulkan pipeline
				VulkanPipelineEntry pipelineEntry(pass, pCurrentMaterial->GetAssetId(), currentVertexLayoutId);
				pCurrentVkPipeline = VulkanContext::GetPipelineManager()->GetPipeline(pipelineEntry);
				if (pCurrentVkPipeline == nullptr)
				{
					Logger::LogError("Pipeline not found for entry: {}", EnumReflection<RenderPassType>::ToString(pipelineEntry.renderPass));
					continue;
				}

				pCommandBuffer->BindPipeline(pCurrentVkPipeline);
				pCommandBuffer->SetViewportAndScissor();

				std::vector<vk::DescriptorSet> descriptorSets(_pIntermediateVariable->renderingDescriptorSets.begin(),
				                                               _pIntermediateVariable->renderingDescriptorSets.end());

				pCommandBuffer->BindDescriptorSet(pCurrentVkPipeline->GetPipelineLayout(), descriptorSets);
			}

			// Push constant model matrix
			pCommandBuffer->PushConstantModelMatrix(pCurrentVkPipeline, renderingMesh.pEntity->GetModelMatrix());

			// Bind vertex buffer
			const auto pVertexBuffer = renderingMesh.pTargetMesh->GetVertexBuffer();
			pCommandBuffer->BindVertexBuffer(pVertexBuffer);

			// Bind index buffer and draw
			const auto pIndexBuffer = renderingMesh.pTargetMesh->GetIndexBuffer();
			if (pIndexBuffer != nullptr)
			{
				pCommandBuffer->BindIndexBuffer(pIndexBuffer);
				pCommandBuffer->DrawIndexed(pIndexBuffer->GetIndexCount());
			}
			else
			{
				pCommandBuffer->DrawNonIndexed(renderingMesh.pTargetMesh->GetVertexCount());
			}
		}

		pCommandBuffer->EndRendering();
	}

	void RenderSystem::RenderImGuiPass(uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer)
	{
		auto pImGui = Application::Get<ImGuiSystem>();

		// Get swap chain image view and depth image view
		auto pSwapChain = VulkanContext::GetSwapChain();
		if (pSwapChain == nullptr)
		{
			Logger::LogError("SwapChain not found");
			return;
		}

		const auto& imageViews = pSwapChain->GetSwapChainImageViews();
		if (swapChainImageIndex >= imageViews.size())
		{
			Logger::LogError("Invalid swap chain image index");
			return;
		}

		vk::ImageView colorImageView = imageViews[swapChainImageIndex];
		vk::Extent2D extent = pSwapChain->GetConfig().extent;

		// ImGui doesn't need depth or MSAA, and should load existing content (clearColor=false)
		// Always render directly to swapchain image (no resolve needed)
		pCommandBuffer->BeginRendering(colorImageView, nullptr, nullptr, extent, false, false);
		pImGui->Render(pCommandBuffer);
		pCommandBuffer->EndRendering();
	}
} // namespace Ailurus