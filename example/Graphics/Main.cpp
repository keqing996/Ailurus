
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Application/SceneSystem/Component/CompCamera.h>
#include <Ailurus/Application/SceneSystem/Component/CompLight.h>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Application/TimeSystem/TimeSystem.h>
#include "Ailurus/Math/Math.hpp"
#include "Ailurus/Utility/Logger.h"

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
	Application::Create(800, 800, "Test", Application::Style{ 
		.canResize = true,
		.haveBorder = true,
		.enableRenderImGui = true,
		.enableRender3D = true
	});

	// Load assets and set up the scene
	auto modelRef = Application::Get<AssetsSystem>()->LoadModel("./Assets/Model/Cube.fbx");
	auto materialRef = Application::Get<AssetsSystem>()->LoadMaterial("./Assets/Material/DefaultMaterial.json");

	auto pEntityWeak = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pEntity = pEntityWeak.lock())
	{
		pEntity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
		pEntity->SetPosition({ 0.0f, 0.0f, 0.0f });
		pEntity->SetRotation(Math::RotateAxis(Vector3f{ 0.0f, 1.0f, 0.0f }, 45.0f));
	}

	auto pCamera = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pCameraEntity = pCamera.lock())
	{
		// Use a reasonable frustum: smaller near plane size for proper perspective
		// Width and height at near plane should be small (like 0.2m at 0.1m distance)
		// This gives ~90 degree FOV which is standard for games
		auto pCam = pCameraEntity->AddComponent<CompCamera>(0.2f, 0.2f, 0.1f, 10.0f);
		Application::Get<RenderSystem>()->SetMainCamera(pCam);

		pCameraEntity->SetPosition({ 0.0f, 0.0f, 3.0f });  // Move camera back to see the cube better
	}

	// Create a directional light
	auto pLightEntity = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pLight = pLightEntity.lock())
	{
		auto pLightComp = pLight->AddComponent<CompLight>();
		pLightComp->SetLightType(LightType::Directional);
		pLightComp->SetDirection({ -0.5f, -1.0f, -0.3f });
		pLightComp->SetColor({ 1.0f, 1.0f, 1.0f });
		pLightComp->SetIntensity(1.0f);
	}

	// Render
	Application::Loop([pEntityWeak]() -> void {
		auto deltaTime = Application::Get<TimeSystem>()->DeltaTime() / 1000.0f;

		// Rotate the cube
		if (auto pEntity = pEntityWeak.lock())
		{
			static float rotationAngle = 0.0f;
			rotationAngle += 90.0f * deltaTime;  // 90 degrees per second
			rotationAngle = std::fmod(rotationAngle, 360.0f);
			
			// Create rotation quaternion around Y axis
			pEntity->SetRotation(Math::RotateAxis(Vector3f{ 0.0f, 1.0f, 0.0f }, rotationAngle));
		}

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