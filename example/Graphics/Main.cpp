
#include <Ailurus/Application.h>
#include <Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Systems/SceneSystem/Component/CompCamera.h>
#include <Ailurus/Systems/SceneSystem/Component/CompLight.h>
#include <Ailurus/Systems/AssetsSystem/Material/Material.h>
#include <Ailurus/Systems/AssetsSystem/Model/Model.h>
#include <Ailurus/Systems/TimeSystem/TimeSystem.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h>

#include <vector>
#include <cmath>

using namespace Ailurus;

int Main(int argc, char* argv[])
{
	// Create the application instance
	Application::Create(800, 800, "Test", Application::Style{ 
		.canResize = true,
		.haveBorder = true,
		.enableRenderImGui = true,
		.enableRender3D = true,
		.skyboxHDRTexturePath = "./Assets/Texture/skybox_2k.hdr"
	});

	// Load assets and set up the scene
	auto modelRef = Application::Get<AssetsSystem>()->LoadModel("./Assets/Model/Cube.fbx");
	auto materialRef = Application::Get<AssetsSystem>()->LoadMaterial("./Assets/Material/PBRMaterial.json");

	// Create multiple cubes to verify lighting from different angles
	struct CubeInfo
	{
		Vector3f position;
		Vector3f rotationAxis;
		float rotationSpeed; // degrees per second
		float scale;
	};

	std::vector<CubeInfo> cubeInfos = {
		// Center cube
		{{ 0.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f},  90.0f, 1.0f},
		// Left - near red point light
		{{-2.5f,  0.0f,  0.0f}, {1.0f, 1.0f, 0.0f},  60.0f, 0.7f},
		// Right - near green point light
		{{ 2.5f,  0.0f,  0.0f}, {1.0f, 0.0f, 1.0f},  75.0f, 0.7f},
		// Top - near blue point light
		{{ 0.0f,  2.0f,  0.0f}, {0.0f, 0.0f, 1.0f}, 120.0f, 0.6f},
		// Bottom
		{{ 0.0f, -2.0f,  0.0f}, {1.0f, 0.0f, 0.0f},  45.0f, 0.6f},
		// Front-left
		{{-1.8f,  1.0f,  1.5f}, {1.0f, 1.0f, 1.0f}, 100.0f, 0.5f},
		// Front-right
		{{ 1.8f,  1.0f,  1.5f}, {0.0f, 1.0f, 1.0f},  80.0f, 0.5f},
		// Back-left
		{{-1.8f, -1.0f, -1.5f}, {1.0f, 0.5f, 0.0f},  55.0f, 0.5f},
		// Back-right
		{{ 1.8f, -1.0f, -1.5f}, {0.5f, 1.0f, 0.5f},  65.0f, 0.5f},
	};

	std::vector<std::weak_ptr<Entity>> cubeEntities;
	for (const auto& info : cubeInfos)
	{
		auto entity = Application::Get<SceneSystem>()->CreateEntity();
		if (auto pEntity = entity.lock())
		{
			pEntity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
			pEntity->SetPosition(info.position);
			pEntity->SetScale({ info.scale, info.scale, info.scale });
		}
		cubeEntities.push_back(entity);
	}

	auto pCamera = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pCameraEntity = pCamera.lock())
	{
		auto pCam = pCameraEntity->AddComponent<CompCamera>(0.2f, 0.2f, 0.1f, 50.0f);
		Application::Get<RenderSystem>()->SetMainCamera(pCam);

		pCameraEntity->SetPosition({ 0.0f, 3.0f, 10.0f });
		pCameraEntity->SetRotation(Math::RotateAxis(Vector3f{ 1.0f, 0.0f, 0.0f }, -15.0f));
	}

	// ===== Phase 5: Bright scene configuration =====
	// Sky-blue clear color (similar to Unity's default sky gradient)
	Application::Get<RenderSystem>()->SetClearColor(0.4f, 0.6f, 0.85f);

	// HDR skybox already contributes significant IBL, so keep ambient modest for SDR output.
	Application::Get<RenderSystem>()->SetAmbientColor(1.0f, 1.0f, 1.0f);
	Application::Get<RenderSystem>()->SetAmbientStrength(0.22f);

	// Tone mapping tuned for an SDR display. The HDR skybox still provides high range input,
	// but the output needs lower exposure than the earlier bright-gradient setup.
	auto* pPostProcess = Application::Get<RenderSystem>()->GetPostProcessChain();
	if (auto* pToneMap = dynamic_cast<ToneMappingEffect*>(pPostProcess->GetEffect("ToneMapping")))
	{
		pToneMap->SetExposure(0.95f);
	}
	if (auto* pBloom = dynamic_cast<BloomMipChainEffect*>(pPostProcess->GetEffect("BloomMipChain")))
	{
		pBloom->SetThreshold(1.1f);
		pBloom->SetBloomIntensity(0.12f);
	}

	// Create a strong directional light (main sun) - steep angle for visible shadows
	auto pLightEntity = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pLight = pLightEntity.lock())
	{
		auto pLightComp = pLight->AddComponent<CompLight>();
		pLightComp->SetLightType(LightType::Directional);
		pLightComp->SetDirection({ -0.4f, -0.8f, -0.3f });
		pLightComp->SetColor({ 1.0f, 0.95f, 0.9f });
		pLightComp->SetIntensity(3.0f);
	}

	// Weak fill light from opposite side - gives depth without color cast
	auto pFillLight = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pFill = pFillLight.lock())
	{
		auto pLightComp = pFill->AddComponent<CompLight>();
		pLightComp->SetLightType(LightType::Directional);
		pLightComp->SetDirection({ 0.5f, -0.3f, 0.5f });
		pLightComp->SetColor({ 0.6f, 0.7f, 0.85f });
		pLightComp->SetIntensity(0.8f);
	}

	// Render
	Application::Loop([cubeEntities, cubeInfos]() -> void {
		auto deltaTime = Application::Get<TimeSystem>()->DeltaTime() / 1000.0f;

		// Rotate all cubes with different axes and speeds
		static std::vector<float> rotationAngles(cubeEntities.size(), 0.0f);
		for (size_t i = 0; i < cubeEntities.size(); i++)
		{
			if (auto pEntity = cubeEntities[i].lock())
			{
				rotationAngles[i] += cubeInfos[i].rotationSpeed * deltaTime;
				rotationAngles[i] = std::fmod(rotationAngles[i], 360.0f);
				pEntity->SetRotation(Math::RotateAxis(cubeInfos[i].rotationAxis, rotationAngles[i]));
			}
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