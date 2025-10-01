
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Application/SceneSystem/Component/CompCamera.h>
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
	Application::Create(800, 800, "Test", Application::Style{ });

	// Load assets and set up the scene
	auto modelRef = Application::Get<AssetsSystem>()->LoadModel("./Assets/Model/Cube.fbx");
	auto materialRef = Application::Get<AssetsSystem>()->LoadMaterial("./Assets/Material/DefaultMaterial.json");

	auto pEntityWeak = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pEntity = pEntityWeak.lock())
	{
		pEntity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
		pEntity->SetPosition({ 0.0f, 0.0f, 0.0f });
	}

	auto pCamera = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pCameraEntity = pCamera.lock())
	{
		auto pCam = pCameraEntity->AddComponent<CompCamera>(2.0f, 2.0f, 0.1f, 10.0f);
		Application::Get<RenderSystem>()->SetMainCamera(pCam);

		pCameraEntity->SetPosition({ 0.0f, 0.0f, 1.2f });
	}

	// Render
	Application::Loop([]() -> void {
		static float f = 0.0f;
		static int counter = 0;
		static bool show_demo_window = true;
		static bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	});

	Application::Destroy();

	return 0;
}