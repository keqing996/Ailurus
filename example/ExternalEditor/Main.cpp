#include <Ailurus/Application.h>
#include <Ailurus/ExternalEditor/ExternalEditorBridge.h>
#include <Ailurus/Systems/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Systems/AssetsSystem/Material/Material.h>
#include <Ailurus/Systems/AssetsSystem/Model/Model.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h>
#include <Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h>
#include <Ailurus/Systems/SceneSystem/Component/CompCamera.h>
#include <Ailurus/Systems/SceneSystem/Component/CompLight.h>
#include <Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Utility/Logger.h>

using namespace Ailurus;

int Main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (!Application::Create(1600, 1200, "Ailurus External Editor Prototype", Application::Style {
		.canResize = true,
		.haveBorder = true,
		.enableRender3D = true,
		.skyboxHDRTexturePath = "./Assets/Texture/skybox_2k.hdr",
	}))
		return 1;

	ExternalEditorBridge editorBridge;
	Application::SetCallbackOnMainLoopPostEvent([&editorBridge]() {
		editorBridge.PumpMainThreadPostEvent();
	});
	Application::SetCallbackOnMainLoopPreRender([&editorBridge]() {
		editorBridge.PumpMainThreadPreRender();
	});
	Application::SetCallbackOnWindowPreDestroyed([&editorBridge]() {
		editorBridge.RequestStop();
		editorBridge.Join();
	});

	auto* assets = Application::Get<AssetsSystem>();
	auto* scene = Application::Get<SceneSystem>();
	auto* renderSystem = Application::Get<RenderSystem>();

	auto modelRef = assets->LoadModel("./Assets/Model/Cube.fbx");
	auto materialRef = assets->LoadMaterial("./Assets/Material/PBRMaterial.json");

	auto root = scene->CreateEntity().lock();
	if (root)
		root->SetName("Scene Root");

	for (int index = 0; index < 4; ++index)
	{
		auto cube = scene->CreateEntity().lock();
		if (!cube)
			continue;

		cube->SetName("Cube " + std::to_string(index + 1));
		cube->SetParent(root.get());
		cube->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
		cube->SetPosition({ -3.0f + static_cast<float>(index) * 2.0f, 0.0f, 0.5f + static_cast<float>(index) * 0.3f });
		cube->SetScale({ 0.8f, 0.8f, 0.8f });
	}

	auto cameraEntity = scene->CreateEntity().lock();
	if (cameraEntity)
	{
		cameraEntity->SetName("Main Camera");
		auto* camera = cameraEntity->AddComponent<CompCamera>(0.2f, 0.2f, 0.1f, 60.0f);
		renderSystem->SetMainCamera(camera);
		cameraEntity->SetPosition({ 0.0f, 5.0f, 9.5f });
		cameraEntity->SetRotation(Math::RotateAxis(Vector3f { 1.0f, 0.0f, 0.0f }, -0.5f));
	}

	auto sunEntity = scene->CreateEntity().lock();
	if (sunEntity)
	{
		sunEntity->SetName("Sun Light");
		auto* light = sunEntity->AddComponent<CompLight>();
		light->SetLightType(LightType::Directional);
		light->SetDirection({ -0.3f, -0.9f, -0.2f });
		light->SetColor({ 1.0f, 0.95f, 0.88f });
		light->SetIntensity(3.0f);
	}

	auto fillEntity = scene->CreateEntity().lock();
	if (fillEntity)
	{
		fillEntity->SetName("Fill Light");
		auto* light = fillEntity->AddComponent<CompLight>();
		light->SetLightType(LightType::Directional);
		light->SetDirection({ 0.5f, -0.35f, 0.55f });
		light->SetColor({ 0.65f, 0.75f, 0.9f });
		light->SetIntensity(0.7f);
	}

	renderSystem->SetClearColor(0.14f, 0.19f, 0.28f, 1.0f);
	renderSystem->SetAmbientColor(1.0f, 1.0f, 1.0f);
	renderSystem->SetAmbientStrength(0.22f);
	renderSystem->SetShadowConstantBias(0.005f);
	renderSystem->SetShadowSlopeScale(0.01f);
	renderSystem->SetShadowNormalOffset(0.02f);

	if (auto* toneMapping = renderSystem->GetToneMappingEffect(); toneMapping != nullptr)
	{
		toneMapping->SetExposure(0.95f);
		toneMapping->SetGamma(2.2f);
	}

	if (auto* bloom = renderSystem->GetBloomMipChainEffect(); bloom != nullptr)
	{
		bloom->SetThreshold(1.1f);
		bloom->SetSoftKnee(0.5f);
		bloom->SetBloomIntensity(0.15f);
		bloom->SetBlendFactor(0.7f);
	}

	if (!editorBridge.Start())
	{
		Application::Destroy();
		return 1;
	}

	Logger::LogInfo("External editor prototype: connect your browser UI to ws://127.0.0.1:{}", editorBridge.GetPort());

	Application::Loop([]() {
	});
	return 0;
}