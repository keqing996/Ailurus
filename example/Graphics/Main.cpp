
#include <Ailurus/Application.h>
#include <Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Systems/SceneSystem/Component/CompCamera.h>
#include <Ailurus/Systems/SceneSystem/Component/CompLight.h>
#include <Ailurus/Systems/AssetsSystem/Material/Material.h>
#include <Ailurus/Systems/AssetsSystem/Model/Model.h>
#include <Ailurus/Systems/ImGuiSystem/EditorLayout.h>
#include <Ailurus/Systems/ImGuiSystem/Widgets/Toast.h>
#include <Ailurus/Systems/TimeSystem/TimeSystem.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h>

#include <vector>
#include <cmath>

using namespace Ailurus;

int Main(int argc, char* argv[])
{
	// Create the application instance
	Application::Create(1600, 1200, "Test", Application::Style{ 
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
		const char* name;
		Vector3f position;
		Vector3f rotationAxis;
		float rotationSpeed; // degrees per second
		float scale;
		Entity* parent = nullptr;
	};

	auto centerGroup = Application::Get<SceneSystem>()->CreateEntity();
	auto accentGroup = Application::Get<SceneSystem>()->CreateEntity();
	Entity* pCenterGroup = centerGroup.lock().get();
	Entity* pAccentGroup = accentGroup.lock().get();
	if (pCenterGroup != nullptr)
		pCenterGroup->SetName("Center Cluster");
	if (pAccentGroup != nullptr)
		pAccentGroup->SetName("Accent Cluster");

	std::vector<CubeInfo> cubeInfos = {
		// Center cube
		{"Center Cube", { 0.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f},  90.0f, 1.0f, pCenterGroup},
		// Left - near red point light
		{"Left Cube", {-2.5f,  0.0f,  0.0f}, {1.0f, 1.0f, 0.0f},  60.0f, 0.7f, pCenterGroup},
		// Right - near green point light
		{"Right Cube", { 2.5f,  0.0f,  0.0f}, {1.0f, 0.0f, 1.0f},  75.0f, 0.7f, pCenterGroup},
		// Top - near blue point light
		{"Top Cube", { 0.0f,  2.0f,  0.0f}, {0.0f, 0.0f, 1.0f}, 120.0f, 0.6f, pCenterGroup},
		// Bottom
		{"Bottom Cube", { 0.0f, -2.0f,  0.0f}, {1.0f, 0.0f, 0.0f},  45.0f, 0.6f, pCenterGroup},
		// Front-left
		{"Front Left Cube", {-1.8f,  1.0f,  1.5f}, {1.0f, 1.0f, 1.0f}, 100.0f, 0.5f, pAccentGroup},
		// Front-right
		{"Front Right Cube", { 1.8f,  1.0f,  1.5f}, {0.0f, 1.0f, 1.0f},  80.0f, 0.5f, pAccentGroup},
		// Back-left
		{"Back Left Cube", {-1.8f, -1.0f, -1.5f}, {1.0f, 0.5f, 0.0f},  55.0f, 0.5f, pAccentGroup},
		// Back-right
		{"Back Right Cube", { 1.8f, -1.0f, -1.5f}, {0.5f, 1.0f, 0.5f},  65.0f, 0.5f, pAccentGroup},
	};

	std::vector<std::weak_ptr<Entity>> cubeEntities;
	for (const auto& info : cubeInfos)
	{
		auto entity = Application::Get<SceneSystem>()->CreateEntity();
		if (auto pEntity = entity.lock())
		{
			pEntity->SetName(info.name);
			pEntity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
			pEntity->SetParent(info.parent);
			pEntity->SetPosition(info.position);
			pEntity->SetScale({ info.scale, info.scale, info.scale });
		}
		cubeEntities.push_back(entity);
	}

	auto pCamera = Application::Get<SceneSystem>()->CreateEntity();
	if (auto pCameraEntity = pCamera.lock())
	{
		pCameraEntity->SetName("Main Camera");
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
		pLight->SetName("Sun Light");
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
		pFill->SetName("Fill Light");
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

		auto* imgui = Application::Get<ImGuiSystem>();
		const ImGuiID dockspaceId = imgui->BeginDockSpace("AilurusEditorDockSpace", ImGuiDockNodeFlags_PassthruCentralNode);
		EditorLayout::SetupDefaultLayout(dockspaceId);

		static bool showWelcomeToast = true;
		if (showWelcomeToast)
		{
			Widgets::PushToast("Graphics example upgraded to the docked Material editor shell.", Widgets::ToastType::Success, 4.0f);
			showWelcomeToast = false;
		}

		EditorLayout::DrawHierarchy();
		EditorLayout::DrawSceneView();
		EditorLayout::DrawInspector();
		EditorLayout::DrawConsole();
		EditorLayout::DrawAssetBrowser();
		Widgets::RenderToasts();
		imgui->EndDockSpace();
	});

	return 0;
}