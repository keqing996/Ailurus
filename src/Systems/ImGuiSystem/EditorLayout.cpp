#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <imgui_internal.h>
#include "Ailurus/Application.h"
#include "Ailurus/Math/Math.hpp"
#include "Ailurus/Systems/ImGuiSystem/EditorLayout.h"
#include "Ailurus/Systems/ImGuiSystem/Font/MaterialIcons.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Breadcrumb.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Card.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/PropertyGrid.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/SearchBar.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Toast.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/TreeView.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h"
#include "Ailurus/Systems/SceneSystem/Component/CompCamera.h"
#include "Ailurus/Systems/SceneSystem/Component/CompLight.h"
#include "Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h"

namespace
{
	struct EditorState
	{
		bool layoutInitialized = false;
		uint32_t selectedEntityGuid = 0;
		uint32_t nameBufferGuid = 0;
		std::array<char, 128> nameBuffer{};
		std::array<char, 64> hierarchySearch{};
		std::array<char, 64> assetSearch{};
		std::vector<std::string> assetPath = { "Assets" };
		std::vector<std::string> consoleLines = {
			"Material shell initialized.",
			"Docking is enabled for the main viewport.",
			"Scene panels are bound to the live Graphics example state.",
		};
	};

	EditorState g_editorState;

	std::string GetEntityDisplayName(Ailurus::Entity* entity)
	{
		if (entity == nullptr)
			return "<null>";

		const std::string& name = entity->GetName();
		if (!name.empty())
			return name;

		return "Entity " + std::to_string(entity->GetGuid());
	}

	bool ContainsInsensitive(const std::string& value, const char* query)
	{
		if (query == nullptr || query[0] == '\0')
			return true;

		std::string loweredValue = value;
		std::string loweredQuery = query;
		std::transform(loweredValue.begin(), loweredValue.end(), loweredValue.begin(), [](unsigned char ch)
		{
			return static_cast<char>(std::tolower(ch));
		});
		std::transform(loweredQuery.begin(), loweredQuery.end(), loweredQuery.begin(), [](unsigned char ch)
		{
			return static_cast<char>(std::tolower(ch));
		});
		return loweredValue.find(loweredQuery) != std::string::npos;
	}

	Ailurus::Entity* GetSelectedEntity()
	{
		auto* scene = Ailurus::Application::Get<Ailurus::SceneSystem>();
		if (scene == nullptr || g_editorState.selectedEntityGuid == 0)
			return nullptr;

		return scene->GetEntity(g_editorState.selectedEntityGuid).lock().get();
	}

	void SyncSelectedEntityName(Ailurus::Entity* entity)
	{
		if (entity == nullptr)
		{
			g_editorState.nameBuffer.fill('\0');
			g_editorState.nameBufferGuid = 0;
			return;
		}

		if (g_editorState.nameBufferGuid == entity->GetGuid())
			return;

		g_editorState.nameBuffer.fill('\0');
		const std::string displayName = GetEntityDisplayName(entity);
		std::strncpy(g_editorState.nameBuffer.data(), displayName.c_str(), g_editorState.nameBuffer.size() - 1);
		g_editorState.nameBufferGuid = entity->GetGuid();
	}

	Ailurus::Widgets::TreeNode* BuildEntityNode(Ailurus::Entity* entity, const char* filter,
		std::vector<std::unique_ptr<Ailurus::Widgets::TreeNode>>& nodeStorage,
		std::vector<std::string>& labelStorage)
	{
		std::vector<Ailurus::Widgets::TreeNode*> childNodes;
		for (Ailurus::Entity* child : entity->GetChildren())
		{
			if (auto* childNode = BuildEntityNode(child, filter, nodeStorage, labelStorage))
				childNodes.push_back(childNode);
		}

		const std::string label = GetEntityDisplayName(entity);
		if (!ContainsInsensitive(label, filter) && childNodes.empty())
			return nullptr;

		labelStorage.push_back(label);
		auto node = std::make_unique<Ailurus::Widgets::TreeNode>();
		node->label = labelStorage.back().c_str();
		node->icon = entity->GetComponent<Ailurus::CompCamera>() != nullptr
			? Ailurus::MaterialIcons::Visibility
			: (entity->GetComponent<Ailurus::CompLight>() != nullptr ? Ailurus::MaterialIcons::AutoAwesome : Ailurus::MaterialIcons::Scene);
		node->isSelected = entity->GetGuid() == g_editorState.selectedEntityGuid;
		node->isExpanded = node->isSelected || !childNodes.empty();
		node->userData = entity;
		node->children = std::move(childNodes);

		Ailurus::Widgets::TreeNode* rawNode = node.get();
		nodeStorage.push_back(std::move(node));
		return rawNode;
	}

	void DrawSceneSummaryCard()
	{
		auto* renderSystem = Ailurus::Application::Get<Ailurus::RenderSystem>();
		if (renderSystem == nullptr)
			return;

		const Ailurus::RenderStats& stats = renderSystem->GetRenderStats();
		const bool visible = Ailurus::Widgets::BeginCard("Renderer");
		if (visible)
		{
			ImGui::Text("Draw Calls: %u", stats.drawCalls);
			ImGui::Text("Triangles: %u", stats.triangleCount);
			ImGui::Text("Entities: %u", stats.entityCount);
			ImGui::Text("Culled: %u", stats.culledEntityCount);
			ImGui::Text("Frame Time: %.2f ms", stats.frameTimeMs);

			auto clearColor = renderSystem->GetClearColor();
			float clearColorValues[4] = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };
			if (Ailurus::Widgets::BeginPropertyGrid("SceneSummaryProperties"))
			{
				if (Ailurus::Widgets::PropertyColor("Clear Color", clearColorValues))
				{
					renderSystem->SetClearColor(clearColorValues[0], clearColorValues[1], clearColorValues[2], clearColorValues[3]);
				}

				bool skyboxEnabled = renderSystem->IsSkyboxEnabled();
				if (Ailurus::Widgets::PropertyBool("Skybox", &skyboxEnabled))
					renderSystem->SetSkyboxEnabled(skyboxEnabled);

				Ailurus::Vector3f ambient = renderSystem->GetAmbientColor();
				float ambientValues[3] = { ambient.x, ambient.y, ambient.z };
				if (Ailurus::Widgets::PropertyVec3("Ambient Color", ambientValues))
					renderSystem->SetAmbientColor(ambientValues[0], ambientValues[1], ambientValues[2]);

				float ambientStrength = renderSystem->GetAmbientStrength();
				if (Ailurus::Widgets::PropertyFloat("Ambient Strength", &ambientStrength, 0.0f, 1.5f))
					renderSystem->SetAmbientStrength(ambientStrength);

				Ailurus::Widgets::EndPropertyGrid();
			}
		}
		Ailurus::Widgets::EndCard();
	}

	void DrawPostProcessCard()
	{
		auto* renderSystem = Ailurus::Application::Get<Ailurus::RenderSystem>();
		if (renderSystem == nullptr)
			return;

		auto* postProcess = renderSystem->GetPostProcessChain();
		if (postProcess == nullptr)
			return;

		auto* toneMapping = dynamic_cast<Ailurus::ToneMappingEffect*>(postProcess->GetEffect("ToneMapping"));
		auto* bloom = dynamic_cast<Ailurus::BloomMipChainEffect*>(postProcess->GetEffect("BloomMipChain"));

		const bool visible = Ailurus::Widgets::BeginCard("Post Processing");
		if (visible)
		{
			if (Ailurus::Widgets::BeginPropertyGrid("PostProcessProperties"))
			{
				if (toneMapping != nullptr)
				{
					bool enabled = toneMapping->IsEnabled();
					if (Ailurus::Widgets::PropertyBool("Tone Mapping", &enabled))
						toneMapping->SetEnabled(enabled);
					float exposure = toneMapping->GetExposure();
					if (Ailurus::Widgets::PropertyFloat("Exposure", &exposure, 0.1f, 3.0f))
						toneMapping->SetExposure(exposure);
					float gamma = toneMapping->GetGamma();
					if (Ailurus::Widgets::PropertyFloat("Gamma", &gamma, 1.0f, 3.0f))
						toneMapping->SetGamma(gamma);
				}

				if (bloom != nullptr)
				{
					bool enabled = bloom->IsEnabled();
					if (Ailurus::Widgets::PropertyBool("Bloom", &enabled))
						bloom->SetEnabled(enabled);
					float threshold = bloom->GetThreshold();
					if (Ailurus::Widgets::PropertyFloat("Threshold", &threshold, 0.0f, 4.0f))
						bloom->SetThreshold(threshold);
					float intensity = bloom->GetBloomIntensity();
					if (Ailurus::Widgets::PropertyFloat("Intensity", &intensity, 0.0f, 2.0f))
						bloom->SetBloomIntensity(intensity);
				}

				Ailurus::Widgets::EndPropertyGrid();
			}
		}
		Ailurus::Widgets::EndCard();
	}

	void DrawInspectorCard(Ailurus::Entity* entity)
	{
		if (entity == nullptr)
			return;

		SyncSelectedEntityName(entity);

		const bool transformVisible = Ailurus::Widgets::BeginCard("Transform");
		if (transformVisible)
		{
			if (Ailurus::Widgets::BeginPropertyGrid("InspectorTransform"))
			{
				if (Ailurus::Widgets::PropertyText("Name", g_editorState.nameBuffer.data(), g_editorState.nameBuffer.size()))
					entity->SetName(g_editorState.nameBuffer.data());

				Ailurus::Vector3f position = entity->GetPosition();
				float positionValues[3] = { position.x, position.y, position.z };
				if (Ailurus::Widgets::PropertyVec3("Position", positionValues))
					entity->SetPosition({ positionValues[0], positionValues[1], positionValues[2] });

				Ailurus::Vector3f scale = entity->GetScale();
				float scaleValues[3] = { scale.x, scale.y, scale.z };
				if (Ailurus::Widgets::PropertyVec3("Scale", scaleValues))
					entity->SetScale({ scaleValues[0], scaleValues[1], scaleValues[2] });

				auto rotation = Ailurus::Math::QuaternionToEulerAngles(entity->GetRotation());
				float rotationValues[3] = { rotation.roll, rotation.pitch, rotation.yaw };
				if (Ailurus::Widgets::PropertyVec3("Rotation", rotationValues))
				{
					entity->SetRotation(Ailurus::Math::EulerAngleToQuaternion(Ailurus::EulerAnglesf(rotationValues[0], rotationValues[1], rotationValues[2])));
				}

				Ailurus::Widgets::EndPropertyGrid();
			}
		}
		Ailurus::Widgets::EndCard();

		if (Ailurus::CompCamera* camera = entity->GetComponent<Ailurus::CompCamera>())
		{
			const bool cameraVisible = Ailurus::Widgets::BeginCard("Camera");
			if (cameraVisible)
			{
				bool perspective = camera->IsPerspective();
				if (Ailurus::Widgets::BeginPropertyGrid("InspectorCamera"))
				{
					if (Ailurus::Widgets::PropertyBool("Perspective", &perspective))
						camera->SetPerspective(perspective);
					float fov = camera->GetHorizontalFOV();
					if (Ailurus::Widgets::PropertyFloat("Horizontal FOV", &fov, 20.0f, 140.0f))
						camera->Set(fov, camera->GetAspectRatio(), camera->GetNear(), camera->GetFar());
					float nearPlane = camera->GetNear();
					if (Ailurus::Widgets::PropertyFloat("Near", &nearPlane, 0.01f, camera->GetFar()))
						camera->Set(camera->GetHorizontalFOV(), camera->GetAspectRatio(), nearPlane, camera->GetFar());
					float farPlane = camera->GetFar();
					if (Ailurus::Widgets::PropertyFloat("Far", &farPlane, camera->GetNear(), 500.0f))
						camera->Set(camera->GetHorizontalFOV(), camera->GetAspectRatio(), camera->GetNear(), farPlane);
					Ailurus::Widgets::EndPropertyGrid();
				}
			}
			Ailurus::Widgets::EndCard();
		}

		if (Ailurus::CompLight* light = entity->GetComponent<Ailurus::CompLight>())
		{
			const bool lightVisible = Ailurus::Widgets::BeginCard("Light");
			if (lightVisible)
			{
				static const char* lightTypes[] = { "Directional", "Point", "Spot" };
				int currentType = static_cast<int>(light->GetLightType());
				if (Ailurus::Widgets::BeginPropertyGrid("InspectorLight"))
				{
					if (Ailurus::Widgets::PropertyEnum("Type", &currentType, lightTypes, 3))
						light->SetLightType(static_cast<Ailurus::LightType>(currentType));

					Ailurus::Vector3f lightColor = light->GetColor();
					float colorValues[4] = { lightColor.x, lightColor.y, lightColor.z, 1.0f };
					if (Ailurus::Widgets::PropertyColor("Color", colorValues))
						light->SetColor({ colorValues[0], colorValues[1], colorValues[2] });

					float intensity = light->GetIntensity();
					if (Ailurus::Widgets::PropertyFloat("Intensity", &intensity, 0.0f, 8.0f))
						light->SetIntensity(intensity);

					Ailurus::Vector3f direction = light->GetDirection();
					float directionValues[3] = { direction.x, direction.y, direction.z };
					if (Ailurus::Widgets::PropertyVec3("Direction", directionValues))
						light->SetDirection({ directionValues[0], directionValues[1], directionValues[2] });

					Ailurus::Widgets::EndPropertyGrid();
				}
			}
			Ailurus::Widgets::EndCard();
		}

		if (entity->GetComponent<Ailurus::CompStaticMeshRender>() != nullptr)
		{
			const bool rendererVisible = Ailurus::Widgets::BeginCard("Renderer");
			if (rendererVisible)
			{
				ImGui::TextUnformatted("Static mesh renderer attached.");
			}
			Ailurus::Widgets::EndCard();
		}
	}
}

namespace Ailurus
{
	void EditorLayout::SetupDefaultLayout(ImGuiID dockspaceId)
	{
		if (dockspaceId == 0)
			return;

		ImGuiDockNode* existingNode = ImGui::DockBuilderGetNode(dockspaceId);
		if (g_editorState.layoutInitialized && existingNode != nullptr)
			return;

		g_editorState.layoutInitialized = true;
		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

		ImGuiID center = dockspaceId;
		ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.22f, nullptr, &center);
		ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.28f, nullptr, &center);
		ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.30f, nullptr, &center);

		ImGui::DockBuilderDockWindow("Hierarchy", left);
		ImGui::DockBuilderDockWindow("Scene View", center);
		ImGui::DockBuilderDockWindow("Inspector", right);
		ImGui::DockBuilderDockWindow("Console", bottom);
		ImGui::DockBuilderDockWindow("Assets", bottom);
		ImGui::DockBuilderFinish(dockspaceId);
	}

	void EditorLayout::DrawSceneView()
	{
		ImGui::SetNextWindowBgAlpha(0.38f);
		if (!ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::End();
			return;
		}

		ImGui::TextUnformatted("Graphics example scene remains active behind this dockspace.");
		DrawSceneSummaryCard();
		DrawPostProcessCard();
		ImGui::End();
	}

	void EditorLayout::DrawHierarchy()
	{
		if (!ImGui::Begin("Hierarchy"))
		{
			ImGui::End();
			return;
		}

		auto* scene = Application::Get<SceneSystem>();
		if (scene == nullptr)
		{
			ImGui::TextUnformatted("Scene system unavailable.");
			ImGui::End();
			return;
		}

		Widgets::SearchBar("Search entities", g_editorState.hierarchySearch.data(), g_editorState.hierarchySearch.size());
		std::vector<Entity*> entities = scene->GetAllRawEntities();
		if (g_editorState.selectedEntityGuid == 0 && !entities.empty())
			g_editorState.selectedEntityGuid = entities.front()->GetGuid();

		std::vector<std::unique_ptr<Widgets::TreeNode>> nodeStorage;
		std::vector<std::string> labelStorage;
		nodeStorage.reserve(entities.size());
		labelStorage.reserve(entities.size());
		std::vector<Widgets::TreeNode*> roots;
		roots.reserve(entities.size());

		for (Entity* entity : entities)
		{
			if (entity->GetParent() != nullptr)
				continue;

			if (Widgets::TreeNode* root = BuildEntityNode(entity, g_editorState.hierarchySearch.data(), nodeStorage, labelStorage))
				roots.push_back(root);
		}

		if (Widgets::TreeNode* clicked = Widgets::TreeView("HierarchyTree", roots))
		{
			if (auto* entity = static_cast<Entity*>(clicked->userData))
				g_editorState.selectedEntityGuid = entity->GetGuid();
		}

		ImGui::End();
	}

	void EditorLayout::DrawInspector()
	{
		if (!ImGui::Begin("Inspector"))
		{
			ImGui::End();
			return;
		}

		Entity* entity = GetSelectedEntity();
		if (entity == nullptr)
		{
			ImGui::TextUnformatted("Select an entity from Hierarchy.");
			ImGui::End();
			return;
		}

		DrawInspectorCard(entity);
		ImGui::End();
	}

	void EditorLayout::DrawConsole()
	{
		if (!ImGui::Begin("Console"))
		{
			ImGui::End();
			return;
		}

		const bool visible = Widgets::BeginCard("Session Console");
		if (visible)
		{
			if (ImGui::Button("Raise Toast"))
				Widgets::PushToast("Material shell is active inside the Graphics example.", Widgets::ToastType::Info);
			ImGui::SameLine();
			if (ImGui::Button("Success Toast"))
				Widgets::PushToast("Post-process parameters updated.", Widgets::ToastType::Success);

			ImGui::Separator();
			for (const std::string& line : g_editorState.consoleLines)
				ImGui::BulletText("%s", line.c_str());
		}
		Widgets::EndCard();

		ImGui::End();
	}

	void EditorLayout::DrawAssetBrowser()
	{
		if (!ImGui::Begin("Assets"))
		{
			ImGui::End();
			return;
		}

		Widgets::Breadcrumb(g_editorState.assetPath);
		Widgets::SearchBar("Filter assets", g_editorState.assetSearch.data(), g_editorState.assetSearch.size());

		static const std::array<std::pair<const char*, const char*>, 6> assetEntries = {{
			{ MaterialIcons::Folder, "Model/Cube.fbx" },
			{ MaterialIcons::Folder, "Material/PBRMaterial.json" },
			{ MaterialIcons::Image, "Texture/skybox_2k.hdr" },
			{ MaterialIcons::Image, "Texture/container.png" },
			{ MaterialIcons::Folder, "Shader/pbr.frag" },
			{ MaterialIcons::Folder, "Shader/PostProcess/tonemapping.frag" },
		}};

		const bool visible = Widgets::BeginCard("Example Assets");
		if (visible)
		{
			for (const auto& [icon, label] : assetEntries)
			{
				if (!ContainsInsensitive(label, g_editorState.assetSearch.data()))
					continue;
				ImGui::BulletText("%s %s", icon, label);
			}
		}
		Widgets::EndCard();

		ImGui::End();
	}
}