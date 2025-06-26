#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPass.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <VulkanSystem/VulkanSystem.h>
#include <VulkanSystem/Buffer/VulkanUniformBuffer.h>
#include "Detail/RenderIntermediateVariable.h"

namespace Ailurus
{
	const char* RenderSystem::GLOBAL_UNIFORM_SET_NAME = "globalUniform";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_VIEW_PROJ_MAT = "viewProjectionMatrix";
	const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_CAMERA_POS = "cameraPosition";

	RenderSystem::RenderSystem()
	{
		Application::Get<VulkanSystem>()->CreateDynamicContext();

		_pShaderLibrary.reset(new ShaderLibrary());

		CreateIntermediateVariable();
		BuildGlobalUniform();
		BuildRenderPass();
	}

	RenderSystem::~RenderSystem()
	{
		Application::Get<VulkanSystem>()->DestroyDynamicContext();
	}

	void RenderSystem::NeedRecreateSwapChain()
	{
		_needRebuildSwapChain = true;
	}

	ShaderLibrary* RenderSystem::GetShaderLibrary() const
	{
		return _pShaderLibrary.get();
	}

	void RenderSystem::SetMainCamera(CompCamera* pCamera)
	{
		if (pCamera == nullptr)
			return;

		_pMainCamera = pCamera;
	}

	CompCamera* RenderSystem::GetMainCamera() const
	{
		return _pMainCamera;
	}

	void RenderSystem::GraphicsWaitIdle() const
	{
		Application::Get<VulkanSystem>()->GetDevice().waitIdle();
	}

	void RenderSystem::ReBuildSwapChain()
	{
		GraphicsWaitIdle();

		_renderPassMap.clear();

		Application::Get<VulkanSystem>()->DestroyDynamicContext();
		Application::Get<VulkanSystem>()->CreateDynamicContext();

		BuildRenderPass();

		_needRebuildSwapChain = false;
	}

	void RenderSystem::BuildRenderPass()
	{
		_renderPassMap[RenderPassType::Forward] = std::make_unique<RenderPass>(RenderPassType::Forward);
	}

	void RenderSystem::BuildGlobalUniform()
	{
		_pGlobalUniformSet = std::make_unique<UniformSet>(UniformSetUsage::General);

		// Create global uniform structure
		auto pGlobalUniformStructure = std::make_unique<UniformVariableStructure>();

		// Add view projection matrix
		pGlobalUniformStructure->AddMember(
			"viewProjectionMatrix", 
			std::make_unique<UniformVariableNumeric>(UniformValueType::Mat4));
		
		// Add camera position
		pGlobalUniformStructure->AddMember(
			"cameraPosition", 
			std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));

		auto pBindingPoint = std::make_unique<UniformBindingPoint>(
			0,
			std::vector<ShaderStage>{ ShaderStage::Vertex },
			"globalUniform",
			std::move(pGlobalUniformStructure)
		);

		_pGlobalUniformSet->AddBindingPoint(std::move(pBindingPoint));
		_pGlobalUniformSet->InitUniformBufferInfo();
		_pGlobalUniformSet->InitDescriptorSetLayout();

		_pGlobalUniformBuffer = std::make_unique<VulkanUniformBuffer>(_pGlobalUniformSet->GetUniformBufferSize());
	}

	auto RenderSystem::GetRenderPass(RenderPassType pass) const -> RenderPass*
	{
		const auto itr = _renderPassMap.find(pass);
		if (itr == _renderPassMap.end())
			return nullptr;

		return itr->second.get();
	}

	auto RenderSystem::GetGlobalUniformSet() const -> UniformSet*
	{
		return _pGlobalUniformSet.get();
	}

	auto RenderSystem::GetGlobalUniformBuffer() const -> VulkanUniformBuffer*
	{
		return _pGlobalUniformBuffer.get();
	}

	auto RenderSystem::GetGlobalUniformAccessNameViewProjMat() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_VIEW_PROJ_MAT;
		return value;
	}

	auto RenderSystem::GetGlobalUniformAccessNameCameraPos() -> const std::string&
	{
		static std::string value = std::string{ GLOBAL_UNIFORM_SET_NAME } + "." + GLOBAL_UNIFORM_ACCESS_CAMERA_POS;
		return value;
	}

} // namespace Ailurus
