#include <nlohmann/json.hpp>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Material/UniformVariable/MaterialUniformVariableNumeric.h>
#include <Ailurus/Application/Application.h>
#include <VulkanSystem/Descriptor/VulkanDescriptorSet.h>

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

	Material::Material() = default;

	Material::~Material() = default;

	void Material::SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader)
	{
		const Shader* pShader = Application::Get<RenderSystem>()->GetShaderLibrary()->GetShader(stage, shader);
		_renderPassParaMap[pass].stageShaders[stage] = pShader;
	}

	std::optional<StageShaderArray> Material::GetStageShaderArray(RenderPassType pass) const
	{
		auto const passItr = _renderPassParaMap.find(pass);
		if (passItr == _renderPassParaMap.end())
			return std::nullopt;

		return passItr->second.stageShaders;
	}

	const VulkanDescriptorSet* Material::GetDescriptorSet() const
	{
		return _pVkDescriptorSet.get();
	}
} // namespace Ailurus
