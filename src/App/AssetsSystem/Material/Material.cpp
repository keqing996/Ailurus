#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Material/UniformVariableNumeric.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	class MaterialJsonHelper
	{
	public:
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
	};

	bool Material::LoadFromFile(const std::string& path)
	{
		std::ifstream fileStream(path);
		if (!fileStream.is_open())
		{
			Logger::LogError("Get material fail: {}", path);
			return false;
		}

		nlohmann::json json;
		fileStream >> json;
		fileStream.close();

		for (const auto& [passName, passShaderConfig] : json.items())
		{
			RenderPassType pass;
			if (!EnumReflection<RenderPassType>::TryFromString(passName, &pass))
			{
				Logger::LogError("Material render pass error, {}, {}", path, passName);
				continue;
			}

			if (passShaderConfig.contains("Shader"))
			{
				for (const auto& [shaderStageName, shaderPath] : passShaderConfig["Shader"].items())
				{
					ShaderStage stage;
					if (!EnumReflection<ShaderStage>::TryFromString(shaderStageName, &stage))
					{
						Logger::LogError("Material shader stage error, {}, {}", path, shaderStageName);
						continue;
					}

					const std::string& shaderFilePath = shaderPath.get<std::string>();
					const Shader* pShader = Application::Get<RenderSystem>()->GetShaderLibrary()->GetShader(stage, shaderFilePath);
					if (pShader == nullptr)
					{
						Logger::LogError("Material shader load error, {}, {}", path, shaderPath);
						continue;
					}

					_renderPassParaMap[pass].stageShaders[stage] = pShader;
				}
			}

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

		return true;
	}

	const Material::RenderPassParameters* Material::GetRenderPassParameters(RenderPassType pass) const
	{
		auto const passItr = _renderPassParaMap.find(pass);
		if (passItr == _renderPassParaMap.end())
			return nullptr;

		return &passItr->second;
	}
} // namespace Ailurus
