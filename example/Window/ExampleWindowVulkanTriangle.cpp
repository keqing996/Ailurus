
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/Component/CompMeshRender.h>
#include <Ailurus/Application/Material/Material.h>

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

	auto pMesh = std::make_shared<Mesh>(
		vertices.data(), vertices.size() * sizeof(float),
		desc, IndexBufferFormat::UInt16,
		indices.data(), indices.size() * sizeof(uint16_t));

	auto pMaterial = std::make_shared<Material>();
	pMaterial->SetShader(RenderPassType::Forward, ShaderStage::Vertex, "./triangle.vert.spv");
	pMaterial->SetShader(RenderPassType::Forward, ShaderStage::Fragment, "./triangle.frag.spv");

	Application::Create(800, 600, "Triangle", Application::Style{});

	auto pEntityWeak = Application::GetSceneManager().CreateEntity();
	if (auto pEntity = pEntityWeak.lock())
	{
		auto pMeshRender = pEntity->AddComponent<CompMeshRender>(ComponentType::MeshRender);
		if (pMeshRender != nullptr)
		{
			pMeshRender->SetMesh(pMesh);
			pMeshRender->SetMaterial(pMaterial);
		}
	}

	Application::Loop(nullptr);
	Application::Destroy();

	return 0;
}