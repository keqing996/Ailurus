
#include <memory>
#include <vector>
#include <random>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <Ailurus/Application/SceneSystem/Component/CompCamera.h>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Application/TimeSystem/TimeSystem.h>
#include <Ailurus/Application/InputSystem/InputSystem.h>
#include "Ailurus/Application/SceneSystem/Entity/Entity.h"
#include <Ailurus/Math/Math.hpp>
#include <Ailurus/Utility/Logger.h>
#include "imgui.h"

using namespace Ailurus;

// Cube data structure
struct CubeData
{
	std::weak_ptr<Entity> entity;
	Vector3f rotationAxis;
	float rotationSpeed; // degrees per second
	float currentAngle;
};

void UpdateInput();
void UpdateUI();
void UpdateScene();
void OnScreenResize(Vector2i size);

std::vector<CubeData> cubes;
std::weak_ptr<Entity> pCameraEntity;

int Main(int argc, char* argv[])
{
	// Create the application instance
	Application::Create(1200, 800, "Test", Application::Style{ .canResize = true, .haveBorder = true });

	Application::SetCallbackOnWindowResize(OnScreenResize);

	// Load assets and set up the scene
	auto modelRef = Application::Get<AssetsSystem>()->LoadModel("./Assets/Model/Cube.fbx");
	auto materialRef = Application::Get<AssetsSystem>()->LoadMaterial("./Assets/Material/DefaultMaterial.json");

	// Random number generator setup
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> posDistX(-5.0f, 5.0f);
	std::uniform_real_distribution<float> posDistY(-3.0f, 3.0f);
	std::uniform_real_distribution<float> posDistZ(-5.0f, 5.0f);
	std::uniform_real_distribution<float> axisDist(-1.0f, 1.0f);
	std::uniform_real_distribution<float> speedDist(30.0f, 120.0f);

	// Create multiple cubes with random properties
	const int numCubes = 5;
	cubes.reserve(numCubes);

	for (int i = 0; i < numCubes; ++i)
	{
		auto pEntity = Application::Get<SceneSystem>()->CreateEntity();
		if (auto entity = pEntity.lock())
		{
			entity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
			
			// Random position
			Vector3f position = {
				posDistX(gen),
				posDistY(gen),
				posDistZ(gen)
			};
			entity->SetPosition(position);

			// Random rotation axis (normalized)
			Vector3f axis = {
				axisDist(gen),
				axisDist(gen),
				axisDist(gen)
			};
			float length = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
			if (length > 0.001f)
			{
				axis = axis / length;
			}
			else
			{
				axis = { 0.0f, 1.0f, 0.0f }; // Default to Y axis if random axis is too small
			}

			// Random rotation speed
			float speed = speedDist(gen);

			// Store cube data
			cubes.push_back({
				pEntity,
				axis,
				speed,
				0.0f
			});

			// Set initial rotation
			entity->SetRotation(Math::RotateAxis(axis, 0.0f));
		}
	}

	auto pCameraEntityLocal = Application::Get<SceneSystem>()->CreateEntity();
	pCameraEntity = pCameraEntityLocal;
	if (auto pCamera = pCameraEntityLocal.lock())
	{
		// Create camera with FOV and aspect ratio
		// 90 degree horizontal FOV, aspect ratio 1.0 (square), near 0.1, far 10.0
		auto pCam = pCamera->AddComponent<CompCamera>(0.2f, 0.2f, 0.1f, 10.0f);

		auto size = Application::GetSize();
		float aspectRatio = static_cast<float>(size.x) / static_cast<float>(size.y);
		pCam->Set(90.0f, aspectRatio, 0.1f, 10.0f); // fovHorizontal, aspect, near, far
		Application::Get<RenderSystem>()->SetMainCamera(pCam);

		pCamera->SetPosition({ 0.0f, 0.0f, 4.0f }); // Move camera back to see the cube better
	}

	// Render
	Application::Loop([]() -> void {
		UpdateInput();
		UpdateScene();
		UpdateUI();
	});

	Application::Destroy();

	return 0;
}

void UpdateInput()
{
	auto pCamera = pCameraEntity.lock();
	if (!pCamera)
		return;

	auto pInput = Application::Get<InputSystem>();
	auto deltaTime = (float)Application::Get<TimeSystem>()->DeltaTime() / 1000.0f;

	// Movement speed (units per second)
	const float moveSpeed = 1.5f;
	// Rotation speed (degrees per second)
	const float rotateSpeed = 45.0f;

	// Get current camera transform
	Vector3f position = pCamera->GetPosition();
	Quaternion<float> rotation = pCamera->GetRotation();

	// Calculate forward, right vectors from camera rotation
	Vector3f forward = rotation * Vector3f{ 0.0f, 0.0f, -1.0f };
	Vector3f right = rotation * Vector3f{ 1.0f, 0.0f, 0.0f };

	// WASD movement
	if (pInput->IsButtonPressed(ButtonType::KeyboardW))
	{
		position = position + forward * moveSpeed * deltaTime;
	}
	if (pInput->IsButtonPressed(ButtonType::KeyboardS))
	{
		position = position - forward * moveSpeed * deltaTime;
	}
	if (pInput->IsButtonPressed(ButtonType::KeyboardA))
	{
		position = position - right * moveSpeed * deltaTime;
	}
	if (pInput->IsButtonPressed(ButtonType::KeyboardD))
	{
		position = position + right * moveSpeed * deltaTime;
	}

	// Arrow keys rotation
	static float yaw = 0.0f;   // Left-Right rotation (around Y axis)
	static float pitch = 0.0f; // Up-Down rotation (around X axis)

	if (pInput->IsButtonPressed(ButtonType::KeyboardArrowLeft))
	{
		yaw += rotateSpeed * deltaTime;
	}
	if (pInput->IsButtonPressed(ButtonType::KeyboardArrowRight))
	{
		yaw -= rotateSpeed * deltaTime;
	}
	if (pInput->IsButtonPressed(ButtonType::KeyboardArrowUp))
	{
		pitch -= rotateSpeed * deltaTime;
	}
	if (pInput->IsButtonPressed(ButtonType::KeyboardArrowDown))
	{
		pitch += rotateSpeed * deltaTime;
	}

	// Clamp pitch to avoid gimbal lock
	pitch = std::clamp(pitch, -89.0f, 89.0f);

	// Create rotation from yaw and pitch
	Quaternion<float> yawRotation = Math::RotateAxis(Vector3f{ 0.0f, 1.0f, 0.0f }, yaw);
	Quaternion<float> pitchRotation = Math::RotateAxis(Vector3f{ 1.0f, 0.0f, 0.0f }, pitch);
	rotation = yawRotation * pitchRotation;

	// Apply transforms
	pCamera->SetPosition(position);
	pCamera->SetRotation(rotation);
}

void UpdateScene()
{
	auto deltaTime = Application::Get<TimeSystem>()->DeltaTime() / 1000.0f;

	// Rotate all cubes
	for (auto& cubeData : cubes)
	{
		if (auto pEntity = cubeData.entity.lock())
		{
			// Update rotation angle
			cubeData.currentAngle += cubeData.rotationSpeed * deltaTime;
			cubeData.currentAngle = std::fmod(cubeData.currentAngle, 360.0f);

			// Apply rotation
			pEntity->SetRotation(Math::RotateAxis(cubeData.rotationAxis, cubeData.currentAngle));
		}
	}
}

void UpdateUI()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGuiIO& io = ImGui::GetIO();

	ImGui::Begin("Settings");

	ImGui::ColorEdit3("clear color", (float*)&clear_color);

	bool enable3D = Application::GraphicsSetting::IsEnable3D();
	if (ImGui::Checkbox("Enable 3D", &enable3D))
	{
		Application::GraphicsSetting::SetEnable3D(enable3D);
	}

	int frameRate = static_cast<int>(Application::GraphicsSetting::GetTargetFrameRate());
	if (ImGui::SliderInt("FrameRate", &frameRate, 20, 240))
	{
		Application::GraphicsSetting::SetTargetFrameRate(static_cast<uint32_t>(frameRate));
	}

	bool msaa = Application::GraphicsSetting::IsMSAAEnabled();
	if (ImGui::Checkbox("MSAA", &msaa))
	{
		Application::GraphicsSetting::SetMSAAEnabled(msaa);
	}

	bool vSync = Application::GraphicsSetting::IsVSyncEnabled();
	if (ImGui::Checkbox("VSync", &vSync))
	{
		Application::GraphicsSetting::SetVSyncEnabled(vSync);
	}

	if (ImGui::Button("Rebuild Swap Chain"))
	{
		auto pRenderSystem = Application::Get<RenderSystem>();
		if (pRenderSystem)
			pRenderSystem->RequestRebuildSwapChain();
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}

void OnScreenResize(Vector2i size)
{
	auto pCamera = Application::Get<RenderSystem>()->GetMainCamera();
	if (pCamera)
	{
		float aspectRatio = static_cast<float>(size.x) / static_cast<float>(size.y);
		float fov = pCamera->GetHorizontalFOV();
		pCamera->Set(fov, aspectRatio, 0.1f, 10.0f);
	}
}