#include <memory>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Utility/Image.h>
#include <Ailurus/System/Path.h>
#include <Ailurus/Application/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Application/AssetsSystem/Texture/Texture.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/Resource/VulkanResourceManager.h>
#include <VulkanContext/Resource/Image/VulkanImage.h>
#include <VulkanContext/Resource/Image/VulkanSampler.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>

namespace Ailurus
{
	static std::optional<RenderPassType> JsonReadRenderPass(const std::string& path,
		const nlohmann::basic_json<>& renderPassConfig)
	{
		if (!renderPassConfig.contains("pass"))
		{
			Logger::LogError("Material render pass missing, {}", path);
			return std::nullopt;
		}

		const auto& passNameNode = renderPassConfig["pass"];
		if (!passNameNode.is_string())
		{
			Logger::LogError("Material render pass name error, {}, {}", path, passNameNode.dump());
			return std::nullopt;
		}

		RenderPassType pass;
		const std::string& passName = passNameNode.get<std::string>();
		if (!EnumReflection<RenderPassType>::TryFromString(passName, &pass))
		{
			Logger::LogError("Material render pass error, {}, {}", path, passName);
			return std::nullopt;
		}

		return pass;
	}

	static std::vector<const Shader*> JsonReadShader(const std::string& path, const nlohmann::basic_json<>& renderPassConfig)
	{
		std::vector<const Shader*> result;

		if (!renderPassConfig.contains("shader"))
		{
			Logger::LogError("Material render pass shader missing, {}", path);
			return result;
		}

		const auto& passShaderConfig = renderPassConfig["shader"];
		if (!passShaderConfig.is_array())
		{
			Logger::LogError("Material render pass shader config error, {}, {}", path, passShaderConfig.dump());
			return result;
		}

		for (const auto& shaderConfig : passShaderConfig)
		{
			if (!shaderConfig.contains("stage") || !shaderConfig.contains("source"))
			{
				Logger::LogError("Material render pass shader config missing stage or source, {}, {}", path, shaderConfig.dump());
				continue;
			}

			ShaderStage stage;
			const std::string& shaderStageName = shaderConfig["stage"].get<std::string>();
			if (!EnumReflection<ShaderStage>::TryFromString(shaderStageName, &stage))
			{
				Logger::LogError("Material shader stage error, {}, {}", path, shaderStageName);
				continue;
			}

			const std::string& shaderFilePath = shaderConfig["source"].get<std::string>();
			auto shaderFilePathSpv = Path::ResolvePath(String::Replace(shaderFilePath, "/Shader/", "/ShaderBin/") + ".spv");
			const Shader* pShader = Application::Get<RenderSystem>()->GetShaderLibrary()->GetShader(stage, shaderFilePathSpv);
			if (pShader == nullptr)
			{
				Logger::LogError("Material shader load error, {}, {}", path, shaderFilePathSpv);
				continue;
			}

			result.push_back(pShader);
		}

		return result;
	}

	static std::optional<uint32_t> JsonReadUniformBindingId(const std::string& path,
		const nlohmann::basic_json<>& uniformConfig)
	{
		if (!uniformConfig.contains("binding"))
		{
			Logger::LogError("Material render pass uniform config missing binding, {}, {}", path, uniformConfig.dump());
			return std::nullopt;
		}

		uint32_t bindingId = uniformConfig["binding"].get<uint32_t>();
		return bindingId;
	}

	static std::vector<ShaderStage> JsonReadUniformShaderStages(const std::string& path,
		const nlohmann::basic_json<>& uniformConfig)
	{
		if (!uniformConfig.contains("shaderStage"))
		{
			Logger::LogError("Material render pass uniform config missing shaderStage, {}, {}", path, uniformConfig.dump());
			return {};
		}

		const auto& shaderStageNode = uniformConfig["shaderStage"];
		if (!shaderStageNode.is_array())
		{
			Logger::LogError("Material render pass uniform config shaderStage not array, {}, {}", path, uniformConfig.dump());
			return {};
		}

		if (shaderStageNode.empty())
		{
			Logger::LogError("Material render pass uniform config shaderStage empty, {}, {}", path, uniformConfig.dump());
			return {};
		}

		std::vector<ShaderStage> targetShaderStages;
		for (const auto& stageNameNode : shaderStageNode)
		{
			if (!stageNameNode.is_string())
			{
				Logger::LogError("Material render pass uniform config shaderStage not string, {}, {}", path, uniformConfig.dump());
				continue;
			}

			ShaderStage stage;
			const std::string& stageName = stageNameNode.get<std::string>();
			if (!EnumReflection<ShaderStage>::TryFromString(stageName, &stage))
			{
				Logger::LogError("Material render pass uniform config shaderStage error, {}, {}", path, stageName);
				continue;
			}

			targetShaderStages.push_back(stage);
		}

		return targetShaderStages;
	}

	static std::pair<UniformValueType, UniformValue> JsonReadUniformNumericValue(const nlohmann::basic_json<>& uniformVarConfig)
	{
		UniformValueType valueType;
		const std::string& uniVarType = uniformVarConfig["type"].get<std::string>();
		if (!EnumReflection<UniformValueType>::TryFromString(uniVarType, &valueType))
		{
			Logger::LogError("Material render pass uniform variable type error, {}", uniVarType);
			return { UniformValueType::Int, UniformValue{ 0 } };
		}

		switch (valueType)
		{
			case UniformValueType::Int:
				return {
					UniformValueType::Int,
					uniformVarConfig["value"].get<int>()
				};
			case UniformValueType::Float:
				return {
					UniformValueType::Float,
					uniformVarConfig["value"].get<float>()
				};
			case UniformValueType::Vector2:
				return {
					UniformValueType::Vector2,
					Vector2f{
						uniformVarConfig["x"].get<float>(),
						uniformVarConfig["y"].get<float>() }
				};
			case UniformValueType::Vector3:
				return {
					UniformValueType::Vector3,
					Vector3f{
						uniformVarConfig["x"].get<float>(),
						uniformVarConfig["y"].get<float>(),
						uniformVarConfig["z"].get<float>() }
				};
			case UniformValueType::Vector4:
				return {
					UniformValueType::Vector4,
					Vector4f{
						uniformVarConfig["x"].get<float>(),
						uniformVarConfig["y"].get<float>(),
						uniformVarConfig["z"].get<float>(),
						uniformVarConfig["w"].get<float>() }
				};
			default:
				Logger::LogError("Material render pass uniform variable type not supported, {}", uniVarType);
				return {
					UniformValueType::Int,
					UniformValue{ 0 }
				};
		}
	}

	static std::unique_ptr<UniformVariable> JsonReadUniformVariableRecursive(const std::string& path,
		const nlohmann::basic_json<>& uniformVarConfig,
		const std::string& prefix,
		std::vector<std::pair<std::string, UniformValue>>& outAccessValues)
	{
		if (!uniformVarConfig.is_object())
		{
			Logger::LogError("Material render pass uniform config access node not object, {}", uniformVarConfig.dump());
			return nullptr;
		}

		if (!uniformVarConfig.contains("type"))
		{
			Logger::LogError("Material render pass uniform config access node missing type, {}", uniformVarConfig.dump());
			return nullptr;
		}

		UniformVaribleType varType;
		const std::string& accessType = uniformVarConfig["type"].get<std::string>();
		if (!EnumReflection<UniformVaribleType>::TryFromString(accessType, &varType))
		{
			Logger::LogError("Material render pass uniform config access type error, {}", accessType);
			return nullptr;
		}

		switch (varType)
		{
			case UniformVaribleType::Numeric:
			{
				auto [valueType, value] = JsonReadUniformNumericValue(uniformVarConfig["value"]);
				auto pUniformVar = std::make_unique<UniformVariableNumeric>(valueType);

				outAccessValues.emplace_back( prefix, value );

				return std::move(pUniformVar);
			}
			case UniformVaribleType::Structure:
			{
				auto pUniformVar = std::make_unique<UniformVariableStructure>();
				for (const auto& memberConfig : uniformVarConfig["members"])
				{
					// Name
					if (!memberConfig.contains("name"))
					{
						Logger::LogError("Material render pass uniform config member missing name, {}, {}", path, memberConfig.dump());
						continue;
					}

					const std::string& name = memberConfig["name"].get<std::string>();
					std::string elementName = prefix + "." + name;

					// Variable
					if (!memberConfig.contains("variable"))
					{
						Logger::LogError("Material render pass uniform config member missing variable, {}, {}", path, memberConfig.dump());
						continue;
					}

					const auto& variableConfig = memberConfig["variable"];
					auto pMemberVar = JsonReadUniformVariableRecursive(path, variableConfig, elementName, outAccessValues);
					if (pMemberVar == nullptr)
						continue;

					// Add member variable
					pUniformVar->AddMember(name, std::move(pMemberVar));
				}

				return std::move(pUniformVar);
			}
			case UniformVaribleType::Array:
			{
				auto pUniformVar = std::make_unique<UniformVariableArray>();
				size_t index = 0;
				for (const auto& memberConfig : uniformVarConfig["members"])
				{
					std::string elementName = prefix + "[" + std::to_string(index) + "]";

					auto pMemberVar = JsonReadUniformVariableRecursive(path, memberConfig, elementName, outAccessValues);
					if (pMemberVar == nullptr)
						continue;

					pUniformVar->AddMember(std::move(pMemberVar));

					index++;
				}

				return std::move(pUniformVar);
			}
			default:
			{
				Logger::LogError("Material render pass uniform config access type not supported, {}, {}", accessType);
				return nullptr;
			}
		}
	}

	static std::unique_ptr<UniformSet> JsonReadUniformSet(
		const std::string& path, 
		RenderPassType renderPass,
		const nlohmann::basic_json<>& renderPassConfig, 
		std::unordered_map<RenderPassType, UniformValueMap>& outAccessValues)
	{
		if (!renderPassConfig.contains("uniforms"))
			return nullptr;

		const auto& uniformSetConfig = renderPassConfig["uniforms"];
		if (!uniformSetConfig.is_array())
		{
			Logger::LogError("Material render pass uniform set config error, {}, {}", path, uniformSetConfig.dump());
			return nullptr;
		}

		if (uniformSetConfig.empty())
		{
			Logger::LogError("Material render pass uniform set config empty, {}", path);
			return nullptr;
		}

		auto pUniformSet = std::make_unique<UniformSet>(UniformSetUsage::MaterialCustom);
		for (const auto& uniformConfig : uniformSetConfig)
		{
			// Binding id
			auto bindingIdOpt = JsonReadUniformBindingId(path, uniformConfig);
			if (!bindingIdOpt.has_value())
				continue;

			// Shader stage
			auto targetShaderStages = JsonReadUniformShaderStages(path, uniformConfig);
			if (targetShaderStages.empty())
				continue;

			// Name
			if (!uniformConfig.contains("name"))
			{
				Logger::LogError("Material render pass uniform config missing name, {}, {}", path, uniformConfig.dump());
				continue;
			}
			const std::string& name = uniformConfig["name"].get<std::string>();

			// Variable
			if (!uniformConfig.contains("variable"))
			{
				Logger::LogError("Material render pass uniform config missing variable, {}, {}", path, uniformConfig.dump());
				continue;
			}

			std::vector<std::pair<std::string, UniformValue>> defaultAccessValues;
			auto pUniformVar = JsonReadUniformVariableRecursive(path, uniformConfig["variable"],
				name, defaultAccessValues);

			if (pUniformVar == nullptr)
				continue;

			// Create binding point
			auto pBindingPoint = std::make_unique<UniformBindingPoint>(*bindingIdOpt, targetShaderStages,
				name, std::move(pUniformVar));

			// Add binding point to a uniform set
			pUniformSet->AddBindingPoint(std::move(pBindingPoint));

		// Update access values
		for (const auto& [accessName, value] : defaultAccessValues)
		{
			auto unifromAccess = UniformAccess{ *bindingIdOpt, accessName };
			outAccessValues[renderPass][unifromAccess] = value;
		}
	}

	pUniformSet->InitUniformBufferInfo();
	// Note: InitDescriptorSetLayout() is called later after textures are loaded

	return std::move(pUniformSet);
}	static std::unordered_map<std::string, AssetRef<Texture>> JsonReadTextures(
		AssetsSystem* pAssetsSystem,
		const std::string& path,
		const nlohmann::basic_json<>& renderPassConfig)
	{
		std::unordered_map<std::string, AssetRef<Texture>> result;

		if (!renderPassConfig.contains("textures"))
			return result;

		const auto& texturesConfig = renderPassConfig["textures"];
		if (!texturesConfig.is_array())
		{
			Logger::LogError("Material render pass textures config error, {}, {}", path, texturesConfig.dump());
			return result;
		}

		auto* pResourceManager = VulkanContext::GetResourceManager();

		for (const auto& textureConfig : texturesConfig)
		{
			if (!textureConfig.contains("binding") || !textureConfig.contains("uniformVarName") || !textureConfig.contains("path"))
			{
				Logger::LogError("Material texture config missing binding, uniformVarName or path, {}, {}", path, textureConfig.dump());
				continue;
			}

			const uint32_t binding = textureConfig["binding"].get<uint32_t>();
			const std::string& uniformVarName = textureConfig["uniformVarName"].get<std::string>();
			const std::string& texturePath = textureConfig["path"].get<std::string>();
			const std::string& textureFullPath = Path::ResolvePath(texturePath);

			// Load image
			Image image(textureFullPath);
			if (image.GetPixelSize().first == 0 || image.GetPixelSize().second == 0)
			{
				Logger::LogError("Failed to load texture image: {}", textureFullPath);
				continue;
			}

			// Create Vulkan image
			VulkanImage* pVulkanImage = pResourceManager->CreateImage(image);
			if (pVulkanImage == nullptr)
			{
				Logger::LogError("Failed to create vulkan image for texture: {}", textureFullPath);
				continue;
			}

			// Create Vulkan sampler
			VulkanSampler* pVulkanSampler = pResourceManager->CreateSampler();
			if (pVulkanSampler == nullptr)
			{
				Logger::LogError("Failed to create vulkan sampler for texture: {}", textureFullPath);
				continue;
			}

			// Create texture asset
			uint64_t textureAssetId = pAssetsSystem->AllocateAssetId();
			auto* pTextureRaw = new Texture(textureAssetId);
			pTextureRaw->SetImage(pVulkanImage);
			pTextureRaw->SetSampler(pVulkanSampler);
			pTextureRaw->SetBindingId(binding);

			pAssetsSystem->RegisterAsset(textureAssetId, std::unique_ptr<Texture>(pTextureRaw));

			result.emplace(uniformVarName, AssetRef<Texture>(pTextureRaw));
		}

		return result;
	}

	AssetRef<MaterialInstance> AssetsSystem::LoadMaterial(const std::string& inPath)
	{
		auto path = Path::ResolvePath(inPath);

		std::ifstream fileStream(path);
		if (!fileStream.is_open())
		{
			Logger::LogError("Get material fail: {}", path);
			return AssetRef<MaterialInstance>(nullptr);
		}

		nlohmann::json json;
		fileStream >> json;
		fileStream.close();

		if (!json.is_array())
		{
			Logger::LogError("Material json is not array: {}", path);
			return AssetRef<MaterialInstance>(nullptr);
		}

		// Create material
		auto pMaterialRaw = new Material(NextAssetId());
		_fileAssetToIdMap[path] = pMaterialRaw->GetAssetId();
		_assetsMap[pMaterialRaw->GetAssetId()] = std::unique_ptr<Material>(pMaterialRaw);

		// Create material asset reference
		const AssetRef<Material> materialRef(pMaterialRaw);

		// Load
		std::unordered_map<RenderPassType, UniformValueMap> accessValues;
		for (const auto& renderPassConfig : json)
	{
		// Read render pass
		auto passOpt = JsonReadRenderPass(path, renderPassConfig);
		if (!passOpt.has_value())
			continue;

		// Read shader config
		auto shaders = JsonReadShader(path, renderPassConfig);

		// Read the uniform set (may be null for passes with no material uniforms, e.g. shadow pass)
		auto pUniformSet = JsonReadUniformSet(path, *passOpt, renderPassConfig, accessValues);

		// Read and load textures
		auto textures = JsonReadTextures(this, path, renderPassConfig);

		if (pUniformSet != nullptr)
		{
			// Build texture binding information for descriptor set layout
			std::vector<TextureBindingInfo> textureBindings;
			for (const auto& [uniformVarName, textureRef] : textures)
			{
				if (auto* pTexture = textureRef.Get())
				{
					TextureBindingInfo bindingInfo;
					bindingInfo.bindingId = pTexture->GetBindingId();
					bindingInfo.shaderStages = vk::ShaderStageFlagBits::eFragment; // Textures typically used in fragment shader
					textureBindings.push_back(bindingInfo);
				}
			}

			// Initialize descriptor set layout with both uniform buffers and textures
			pUniformSet->InitDescriptorSetLayout(textureBindings);
		}

		// Set shader and uniform
		pMaterialRaw->SetPassShaderAndUniform(*passOpt, shaders, std::move(pUniformSet));

		// Set textures
		for (const auto& [uniformVarName, textureRef] : textures)
		{
			pMaterialRaw->SetPassTexture(*passOpt, uniformVarName, textureRef);
		}
	}		// Create material instance
		auto pMaterialInstanceRaw = new MaterialInstance(NextAssetId(), materialRef);

		// Update material instance uniform values
		for (const auto& [renderPass, uniformValueMap] : accessValues)
		{
			for (const auto& [access, value] : uniformValueMap)
			{
				UniformSetMemory* pRenderPassUniformMemory = pMaterialInstanceRaw->_renderPassUniformBufferMap[renderPass].get();
				pRenderPassUniformMemory->SetUniformValue(access, value);
			}
		}

		_assetsMap[pMaterialInstanceRaw->GetAssetId()] = std::unique_ptr<MaterialInstance>(pMaterialInstanceRaw);

		return AssetRef<MaterialInstance>(pMaterialInstanceRaw);
	}

	AssetRef<MaterialInstance> AssetsSystem::CopyMaterialInstance(const AssetRef<MaterialInstance>& materialInstance)
	{
		const auto pMatInstRaw = materialInstance.Get();
		const auto pMatRaw = pMatInstRaw->_targetMaterial;

		const auto pMatInstRawNew = new MaterialInstance(NextAssetId(), pMatRaw);
		for (const auto& [renderPass, pPassUniformMemory] : pMatInstRaw->_renderPassUniformBufferMap)
		{
			auto& uniformValueMap = pPassUniformMemory->GetUniformValueMap();
			for (const auto& [access, value] : uniformValueMap)
			{
				pMatInstRawNew->SetUniformValue(renderPass, access, value);
			}
		}

		_assetsMap[pMatInstRawNew->GetAssetId()] = std::unique_ptr<MaterialInstance>(pMatInstRawNew);

		return AssetRef<MaterialInstance>(pMatInstRawNew);
	}
} // namespace Ailurus