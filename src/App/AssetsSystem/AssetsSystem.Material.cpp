#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"
#include <memory>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>
#include <Ailurus/Assert.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Utility/String.h>
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

	static void JsonReadShader(const std::string& path, RenderPassType pass, Material* pMaterial,
		const nlohmann::basic_json<>& renderPassConfig)
	{
		if (!renderPassConfig.contains("shader"))
		{
			Logger::LogError("Material render pass shader missing, {}, {}", path, EnumReflection<RenderPassType>::ToString(pass));
			return;
		}

		const auto& passShaderConfig = renderPassConfig["shader"];
		if (!passShaderConfig.is_array())
		{
			Logger::LogError("Material render pass shader config error, {}, {}", path, passShaderConfig.dump());
			return;
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

			// Add shader to material
			pMaterial->SetPassShader(pass, pShader);
		}
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
		std::vector<std::string>& accessNames,
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

		const std::string& name = uniformVarConfig["name"].get<std::string>();

		switch (varType)
		{
			case UniformVaribleType::Numeric:
			{
				accessNames.push_back(name);

				auto [valueType, value] = JsonReadUniformNumericValue(uniformVarConfig["value"]);
				auto pUniformVar = std::make_unique<UniformVariableNumeric>(name, valueType);
				outAccessValues.push_back({ String::Join(accessNames, ""),
					value });

				accessNames.pop_back();

				return pUniformVar;
			}
			case UniformVaribleType::Structure:
			{
				return nullptr;
			}
			case UniformVaribleType::Array:
			{

				return nullptr;
			}
			default:
			{
				Logger::LogError("Material render pass uniform config access type not supported, {}, {}", accessType);
				return nullptr;
			}
		}
	}

	static std::unique_ptr<UniformSet> JsonReadUniformSet(const std::string& path,
		const nlohmann::basic_json<>& renderPassConfig)
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

			uint32_t bindingId = bindingIdOpt.value();

			// Shader stage
			auto targetShaderStages = JsonReadUniformShaderStages(path, uniformConfig);
			if (targetShaderStages.empty())
				continue;

			// Variable
			if (!uniformConfig.contains("variable"))
			{
				Logger::LogError("Material render pass uniform config missing variable, {}, {}", path, uniformConfig.dump());
				continue;
			}

			std::vector<std::string> accessNames;
			std::vector<std::pair<std::string, UniformValue>> outAccessValues;
			auto pUniformVar = JsonReadUniformVariableRecursive(path, uniformConfig["variable"], 
				accessNames, outAccessValues);

			if (pUniformVar == nullptr)
				continue;


		}

		return pUniformSet;
	}

	AssetReference<ReadOnlyMaterialInstance> AssetsSystem::LoadMaterial(const std::string& path)
	{
		std::ifstream fileStream(path);
		if (!fileStream.is_open())
		{
			Logger::LogError("Get material fail: {}", path);
			return AssetReference<ReadOnlyMaterialInstance>(nullptr);
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
		auto pMaterialInstanceRaw = new ReadOnlyMaterialInstance(NextAssetId(), materialRef);
		_assetsMap[pMaterialInstanceRaw->GetAssetId()] = std::unique_ptr<ReadOnlyMaterialInstance>(pMaterialInstanceRaw);

		// Load
		for (const auto& renderPassConfig : json.array())
		{
			// Read render pass
			auto passOpt = JsonReadRenderPass(path, renderPassConfig);
			if (!passOpt.has_value())
				continue;

			RenderPassType pass = passOpt.value();

			// Read shader config
			JsonReadShader(path, pass, pMaterialRaw, renderPassConfig);

			// Read uniform set

			
		}

		return AssetReference<ReadOnlyMaterialInstance>(pMaterialInstanceRaw);
	}

	AssetReference<ReadWriteMaterialInstance> AssetsSystem::CopyMaterialInstance(const AssetReference<ReadOnlyMaterialInstance>& materialInstance)
	{
	}
} // namespace Ailurus