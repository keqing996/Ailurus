#include <memory>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>
#include <Ailurus/Assert.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialUniformAccess.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>

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
			if (!shaderConfig.contains("stage") || !shaderConfig.contains("path"))
			{
				Logger::LogError("Material render pass shader config missing stage or path, {}, {}", path, shaderConfig.dump());
				continue;
			}

			ShaderStage stage;
			const std::string& shaderStageName = shaderConfig["stage"].get<std::string>();
			if (!EnumReflection<ShaderStage>::TryFromString(shaderStageName, &stage))
			{
				Logger::LogError("Material shader stage error, {}, {}", path, shaderStageName);
				continue;
			}

			const std::string& shaderFilePath = shaderConfig["path"].get<std::string>();
			const Shader* pShader = Application::Get<RenderSystem>()->GetShaderLibrary()->GetShader(stage, shaderFilePath);
			if (pShader == nullptr)
			{
				Logger::LogError("Material shader load error, {}, {}", path, shaderFilePath);
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
			case UniformValueType::Vector2f:
				return {
					UniformValueType::Vector2f,
					Vector2f{
						uniformVarConfig["x"].get<float>(),
						uniformVarConfig["y"].get<float>() }
				};
			case UniformValueType::Vector3f:
				return {
					UniformValueType::Vector3f,
					Vector3f{
						uniformVarConfig["x"].get<float>(),
						uniformVarConfig["y"].get<float>(),
						uniformVarConfig["z"].get<float>() }
				};
			case UniformValueType::Vector4f:
				return {
					UniformValueType::Vector4f,
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

		if (!uniformVarConfig.contains("type") || !uniformVarConfig.contains("name"))
		{
			Logger::LogError("Material render pass uniform config access node missing type or name, {}", uniformVarConfig.dump());
			return nullptr;
		}

		UniformVaribleType varType;
		const std::string& accessType = uniformVarConfig["type"].get<std::string>();
		if (!EnumReflection<UniformVaribleType>::TryFromString(accessType, &varType))
		{
			Logger::LogError("Material render pass uniform config access type error, {}, {}", accessType);
			return nullptr;
		}

		switch (varType)
		{
			case UniformVaribleType::Numeric:
			{
				auto [valueType, value] = JsonReadUniformNumericValue(uniformVarConfig["value"]);
				auto pUniformVar = std::make_unique<UniformVariableNumeric>(valueType);

				outAccessValues.push_back({ prefix, value });

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

	static std::unique_ptr<UniformSet> JsonReadUniformSet(const std::string& path,
		const nlohmann::basic_json<>& renderPassConfig, std::vector<std::tuple<uint32_t, std::string, UniformValue>>& outAccessValues)
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

			// Add binding point to uniform set
			pUniformSet->AddBindingPoint(std::move(pBindingPoint));

			// Update access values
			for (const auto& [accessName, value] : defaultAccessValues)
				outAccessValues.push_back({ *bindingIdOpt, accessName, value });
		}

		pUniformSet->InitUniformBuffer();
		return std::move(pUniformSet);
	}

	AssetReference<MaterialInstance> AssetsSystem::LoadMaterial(const std::string& path)
	{
		std::ifstream fileStream(path);
		if (!fileStream.is_open())
		{
			Logger::LogError("Get material fail: {}", path);
			return AssetReference<MaterialInstance>(nullptr);
		}

		nlohmann::json json;
		fileStream >> json;
		fileStream.close();

		// Create material
		auto pMaterialRaw = new Material(NextAssetId());
		_fileAssetToIdMap[path] = pMaterialRaw->GetAssetId();
		_assetsMap[pMaterialRaw->GetAssetId()] = std::unique_ptr<Material>(pMaterialRaw);

		// Create material asset reference
		AssetReference<Material> materialRef(pMaterialRaw);

		// Create material instance
		auto pMaterialInstanceRaw = new MaterialInstance(NextAssetId(), materialRef);
		_assetsMap[pMaterialInstanceRaw->GetAssetId()] = std::unique_ptr<MaterialInstance>(pMaterialInstanceRaw);

		// Load
		for (const auto& renderPassConfig : json.array())
		{
			// Read render pass
			auto passOpt = JsonReadRenderPass(path, renderPassConfig);
			if (!passOpt.has_value())
				continue;

			// Read shader config
			auto shaders = JsonReadShader(path, renderPassConfig);
			for (auto pShader : shaders)
				pMaterialRaw->SetPassShader(*passOpt, pShader);

			// Read uniform set
			std::vector<std::tuple<uint32_t, std::string, UniformValue>> accessValues;
			auto pUniformSet = JsonReadUniformSet(path, renderPassConfig, accessValues);
			if (pUniformSet == nullptr)
				continue;

			// Add uniform set to material
			pMaterialRaw->SetUniformSet(*passOpt, std::move(pUniformSet));

			// Update material instance uniform values
			for (const auto& [bindingId, accessName, value] : accessValues)
				pMaterialInstanceRaw->uniformValueMap[{*passOpt, bindingId, accessName}] = value;
		}

		return AssetReference<MaterialInstance>(pMaterialInstanceRaw);
	}

	AssetReference<MaterialInstance> AssetsSystem::CopyMaterialInstance(const AssetReference<MaterialInstance>& materialInstance)
	{
		auto pTargetMaterialRef = materialInstance.Get()->targetMaterial;

		auto pNewMaterialInstanceRaw = new MaterialInstance(NextAssetId(), pTargetMaterialRef);
		pNewMaterialInstanceRaw->uniformValueMap = materialInstance.Get()->uniformValueMap;

		_assetsMap[pNewMaterialInstanceRaw->GetAssetId()] = std::unique_ptr<MaterialInstance>(pNewMaterialInstanceRaw);

		return AssetReference<MaterialInstance>(pNewMaterialInstanceRaw);
	}
} // namespace Ailurus