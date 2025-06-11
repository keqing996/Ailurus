#include <Ailurus/Application/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Assert.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>

namespace Ailurus
{
	/*
	static std::unique_ptr<MaterialUniformVariable> CreateUniformVar(const std::string& uniformBlockName, const std::string& uniformValueName, const nlohmann::basic_json<>& uniformVarConfig)
	{
		std::unique_ptr<MaterialUniformVariable> pUniVar = nullptr;

		const std::string& uniVarType = uniformVarConfig["Type"];
		if (uniVarType == EnumReflection<MaterialUniformVariableType>::ToString(MaterialUniformVariableType::Int))
		{
			int value = uniformVarConfig["Value"];
			pUniVar = std::make_unique<MaterialUniformVariableNumeric<int>>(uniformBlockName, uniformValueName, value);
		}
		else if (uniVarType == EnumReflection<MaterialUniformVariableType>::ToString(MaterialUniformVariableType::Uint))
		{
			unsigned int value = uniformVarConfig["Value"];
			pUniVar = std::make_unique<MaterialUniformVariableNumeric<unsigned int>>(uniformBlockName, uniformValueName, value);
		}
		else if (uniVarType == EnumReflection<MaterialUniformVariableType>::ToString(MaterialUniformVariableType::Float))
		{
			float value = uniformVarConfig["Value"];
			pUniVar = std::make_unique<MaterialUniformVariableNumeric<float>>(uniformBlockName, uniformValueName, value);
		}
		else if (uniVarType == EnumReflection<MaterialUniformVariableType>::ToString(MaterialUniformVariableType::Float2))
		{
			const float x = uniformVarConfig["X"];
			const float y = uniformVarConfig["Y"];
			pUniVar = std::make_unique<MaterialUniformVariableNumeric<Vector2f>>(uniformBlockName, uniformValueName, Vector2f{ x, y });
		}
		else if (uniVarType == EnumReflection<MaterialUniformVariableType>::ToString(MaterialUniformVariableType::Float3))
		{
			const float x = uniformVarConfig["X"];
			const float y = uniformVarConfig["Y"];
			const float z = uniformVarConfig["Z"];
			pUniVar = std::make_unique<MaterialUniformVariableNumeric<Vector3f>>(uniformBlockName, uniformValueName, Vector3f{ x, y, z });
		}
		else if (uniVarType == EnumReflection<MaterialUniformVariableType>::ToString(MaterialUniformVariableType::Float4))
		{
			const float x = uniformVarConfig["X"];
			const float y = uniformVarConfig["Y"];
			const float z = uniformVarConfig["Z"];
			const float w = uniformVarConfig["W"];
			pUniVar = std::make_unique<MaterialUniformVariableNumeric<Vector4f>>(uniformBlockName, uniformValueName, Vector4f{ x, y, z, w });
		}

		return pUniVar;
	}
*/
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
		for (const auto& uniformVariableConfig : uniformSetConfig)
		{
			
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











			

			if (passShaderConfig.contains("Uniform"))
			{
				for (auto uniformBlockNode : passShaderConfig["Uniform"])
				{
					const std::string& blockName = uniformBlockNode["BlockName"];
					uint32_t setId = uniformBlockNode["Set"];
					uint32_t bindingId = uniformBlockNode["Binding"];

					for (auto uniformValueNode : uniformBlockNode["Values"])
					{
						const std::string& varName = uniformValueNode["Name"];
						auto pUniVar = MaterialJsonHelper::CreateUniformVar(blockName, varName, uniformValueNode);
						if (pUniVar != nullptr)
							_renderPassParaMap[pass].uniformVariables.push_back(std::move(pUniVar));
					}
				}
			}
		}

		return AssetReference<ReadOnlyMaterialInstance>(pMaterialInstanceRaw);
	}

	AssetReference<ReadWriteMaterialInstance> AssetsSystem::CopyMaterialInstance(const AssetReference<ReadOnlyMaterialInstance>& materialInstance)
	{
	}
} // namespace Ailurus