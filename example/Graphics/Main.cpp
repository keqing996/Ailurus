
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/SceneSystem/Component/CompMeshRender.h>
#include <Ailurus/Application/RenderSystem/Material/Material.h>

using namespace Ailurus;

int Main(int argc, char* argv[])
{
	static std::array<float, 20> vertices = {
		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 1.0f, 1.0f, 1.0f
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	VertexAttributeDescription desc{ AttributeType::Vector2, AttributeType::Vector3 };

	Application::Create(800, 600, "Triangle", Application::Style{});
	{
		auto pMesh = std::make_shared<Mesh>(
		vertices.data(), vertices.size() * sizeof(float),
		desc, IndexBufferFormat::UInt16,
		indices.data(), indices.size() * sizeof(uint16_t));

		auto pMaterial = std::make_shared<Material>();
		pMaterial->SetShader(RenderPassType::Forward, ShaderStage::Vertex, "./Assets/ShaderBin/triangle.vert.spv");
		pMaterial->SetShader(RenderPassType::Forward, ShaderStage::Fragment, "./Assets/ShaderBin/triangle.frag.spv");

		auto pEntityWeak = Application::Get<SceneSystem>()->CreateEntity();
		if (auto pEntity = pEntityWeak.lock())
		{
			auto pMeshRender = pEntity->AddComponent<CompMeshRender>();
			if (pMeshRender != nullptr)
			{
				pMeshRender->SetMesh(pMesh);
				pMeshRender->SetMaterial(pMaterial);
			}
		}
	}

	Application::Loop(nullptr);
	Application::Destroy();

	return 0;
}