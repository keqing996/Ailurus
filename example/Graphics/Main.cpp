
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>

using namespace Ailurus;

int Main(int argc, char* argv[])
{
	/* Backup
	static std::array<float, 20> vertices = {
		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 1.0f, 1.0f, 1.0f
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};
	*/

	// Create the application instance
	Application::Create(800, 600, "Triangle", Application::Style{});

	// Load assets and set up the scene
	auto modelRef = Application::Get<AssetsSystem>()->LoadModel("./Assets/Model/Primitive/Cube.fbx");
	auto materialRef = Application::Get<AssetsSystem>()->LoadMaterial("./Assets/Material/DefaultMaterial.json");

	auto pEntityWeak = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pEntity = pEntityWeak.lock())
	{
		pEntity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
	}

	auto pCamera = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pCameraEntity = pCamera.lock())
	{
		auto pCam = pCameraEntity->AddComponent<CompCamera>(-2, 2, -2, 2, 0.1f, 100.0f, true);
		Application::Get<RenderSystem>()->SetMainCamera(pCam);

		pCameraEntity->SetPosition({ -3.0f, 0.0f, 5.0f });
	}

	// Render
	Application::Loop(nullptr);
	Application::Destroy();

	return 0;
}