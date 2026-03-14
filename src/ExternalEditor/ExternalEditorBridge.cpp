#include "Ailurus/ExternalEditor/ExternalEditorBridge.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "Ailurus/Application.h"
#include "Ailurus/Math/Math.hpp"
#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h"
#include "Ailurus/Utility/Logger.h"
#include <libwebsockets.h>

namespace Ailurus
{
	namespace
	{
		template <typename T>
		class DoubleBufferQueue
		{
		public:
			void Push(T value)
			{
				std::scoped_lock lock(_mutex);
				_writeBuffer.push_back(std::move(value));
			}

			void Swap(std::vector<T>& out)
			{
				out.clear();
				std::scoped_lock lock(_mutex);
				out.swap(_writeBuffer);
			}

		private:
			std::mutex _mutex;
			std::vector<T> _writeBuffer;
		};

		enum class InboundMessageKind
		{
			ClientConnected,
			ClientDisconnected,
			ClientCommand,
			InvalidPayload,
			RequestFullSync,
		};

		struct InboundMessage
		{
			InboundMessageKind kind = InboundMessageKind::ClientCommand;
			uint64_t sessionId = 0;
			nlohmann::json payload;
			std::string errorMessage;
		};

		enum class OutboundMessageKind
		{
			FullSnapshot,
			DeltaBatch,
			ErrorMessage,
		};

		struct OutboundMessage
		{
			OutboundMessageKind kind = OutboundMessageKind::DeltaBatch;
			uint64_t sessionId = 0;
			uint64_t revision = 0;
			nlohmann::json payload;
		};

		struct EntityDeltaState
		{
			std::optional<std::string> name;
			bool hasParent = false;
			std::optional<uint32_t> parentGuid;
			bool hasTransform = false;
			Vector3f position = Vector3f::Zero;
			Quaternionf rotation = Quaternionf::Identity;
			Vector3f scale = Vector3f::One;
		};

		struct PendingSceneChanges
		{
			std::unordered_map<uint32_t, nlohmann::json> createdEntities;
			std::unordered_map<uint32_t, EntityDeltaState> updatedEntities;
			std::unordered_set<uint32_t> destroyedEntities;
			bool renderSettingsDirty = false;
			bool fullSnapshotRequested = false;
			std::string fullSnapshotReason;

			void ClearLiveChanges()
			{
				createdEntities.clear();
				updatedEntities.clear();
				destroyedEntities.clear();
				renderSettingsDirty = false;
			}

			void ClearAll()
			{
				ClearLiveChanges();
				fullSnapshotRequested = false;
				fullSnapshotReason.clear();
			}
		};

		static auto OutboundKindToString(OutboundMessageKind kind) -> const char*
		{
			switch (kind)
			{
				case OutboundMessageKind::FullSnapshot:
					return "fullSnapshot";
				case OutboundMessageKind::DeltaBatch:
					return "deltaBatch";
				case OutboundMessageKind::ErrorMessage:
					return "error";
			}

			return "unknown";
		}
	} // namespace

	struct ExternalEditorBridge::Impl
	{
		static constexpr size_t MAX_PENDING_NETWORK_MESSAGES = 256;

		Config config;
		SceneSystem* sceneSystem = nullptr;
		RenderSystem* renderSystem = nullptr;
		bool observersAttached = false;

		std::atomic<bool> running = false;
		std::atomic<bool> stopRequested = false;
		std::atomic<bool> clientConnected = false;
		std::atomic<uint64_t> nextSessionId = 1;
		std::atomic<void*> lwsContext = nullptr;

		std::thread networkThread;
		std::mutex startupMutex;
		std::condition_variable startupCv;
		bool startupResolved = false;
		bool startupSucceeded = false;

		DoubleBufferQueue<InboundMessage> inboundQueue;
		DoubleBufferQueue<OutboundMessage> outboundQueue;
		std::vector<InboundMessage> inboundScratch;
		std::vector<OutboundMessage> outboundScratch;

		uint64_t activeSessionId = 0;
		uint64_t currentRevision = 0;
		bool liveDeltaEnabled = false;
		PendingSceneChanges pendingChanges;
		std::string protocolNameStorage;
		std::array<lws_protocols, 2> protocols = {};
		lws* clientWsi = nullptr;
		uint64_t networkSessionId = 0;
		std::deque<std::string> pendingNetworkMessages;

		bool Start(const Config& inConfig)
		{
			if (running.load())
				return true;

			sceneSystem = Application::Get<SceneSystem>();
			renderSystem = Application::Get<RenderSystem>();
			if (sceneSystem == nullptr || renderSystem == nullptr)
			{
				Logger::LogError("ExternalEditorBridge: systems are unavailable, call Start after Application::Create");
				return false;
			}

			config = inConfig;
			pendingChanges.ClearAll();
			activeSessionId = 0;
			liveDeltaEnabled = false;
			stopRequested = false;
			clientConnected = false;

			sceneSystem->AddObserver(this, static_cast<SceneObserver*>(owner));
			renderSystem->AddSettingsObserver(this, static_cast<RenderSettingsObserver*>(owner));
			observersAttached = true;

			{
				std::scoped_lock lock(startupMutex);
				startupResolved = false;
				startupSucceeded = false;
			}

			running = true;
			networkThread = std::thread([this]() {
				NetworkThreadMain();
			});

			std::unique_lock lock(startupMutex);
			startupCv.wait(lock, [this]() {
				return startupResolved;
			});

			if (!startupSucceeded)
			{
				running = false;
				lock.unlock();
				Join();
				DetachObservers();
				return false;
			}

			Logger::LogInfo("ExternalEditorBridge: listening on ws://127.0.0.1:{}", config.port);
			return true;
		}

		void RequestStop()
		{
			if (!running.load() && !networkThread.joinable())
			{
				DetachObservers();
				return;
			}

			stopRequested = true;
			DetachObservers();
			WakeNetworkThread();
		}

		void Join()
		{
			if (networkThread.joinable())
				networkThread.join();

			running = false;
			clientConnected = false;
			activeSessionId = 0;
			liveDeltaEnabled = false;
			pendingChanges.ClearAll();
		}

		void PumpMainThreadPostEvent()
		{
			inboundQueue.Swap(inboundScratch);
			for (const auto& message : inboundScratch)
				HandleInboundMessage(message);
		}

		void PumpMainThreadPreRender()
		{
			if (activeSessionId == 0)
			{
				pendingChanges.ClearLiveChanges();
				return;
			}

			if (pendingChanges.fullSnapshotRequested)
			{
				currentRevision++;
				QueueOutboundMessage({
					.kind = OutboundMessageKind::FullSnapshot,
					.sessionId = activeSessionId,
					.revision = currentRevision,
					.payload = BuildFullSnapshotPayload(pendingChanges.fullSnapshotReason),
				});
				pendingChanges.ClearAll();
				liveDeltaEnabled = true;
				return;
			}

			if (!liveDeltaEnabled)
				return;

			nlohmann::json deltaPayload = BuildDeltaPayload();
			if (!deltaPayload.contains("changes") || deltaPayload["changes"].empty())
				return;

			currentRevision++;
			QueueOutboundMessage({
				.kind = OutboundMessageKind::DeltaBatch,
				.sessionId = activeSessionId,
				.revision = currentRevision,
				.payload = std::move(deltaPayload),
			});
		}

		void OnEntityCreated(const Entity& entity)
		{
			if (!ShouldCollectLiveChanges())
				return;

			pendingChanges.destroyedEntities.erase(entity.GetGuid());
			pendingChanges.updatedEntities.erase(entity.GetGuid());
			pendingChanges.createdEntities[entity.GetGuid()] = BuildEntitySnapshot(entity);
		}

		void OnEntityDestroyed(const Entity& entity)
		{
			if (!ShouldCollectLiveChanges())
				return;

			if (pendingChanges.createdEntities.erase(entity.GetGuid()) > 0)
			{
				pendingChanges.updatedEntities.erase(entity.GetGuid());
				return;
			}

			pendingChanges.updatedEntities.erase(entity.GetGuid());
			pendingChanges.destroyedEntities.insert(entity.GetGuid());
		}

		void OnEntityNameChanged(const Entity& entity)
		{
			if (!ShouldCollectLiveChanges())
				return;

			if (auto it = pendingChanges.createdEntities.find(entity.GetGuid()); it != pendingChanges.createdEntities.end())
			{
				it->second = BuildEntitySnapshot(entity);
				return;
			}

			auto& delta = pendingChanges.updatedEntities[entity.GetGuid()];
			delta.name = entity.GetName();
		}

		void OnEntityParentChanged(const Entity& entity)
		{
			if (!ShouldCollectLiveChanges())
				return;

			if (auto it = pendingChanges.createdEntities.find(entity.GetGuid()); it != pendingChanges.createdEntities.end())
			{
				it->second = BuildEntitySnapshot(entity);
				return;
			}

			auto& delta = pendingChanges.updatedEntities[entity.GetGuid()];
			delta.hasParent = true;
			if (const Entity* parent = entity.GetParent(); parent != nullptr)
				delta.parentGuid = parent->GetGuid();
			else
				delta.parentGuid = std::nullopt;
		}

		void OnEntityTransformChanged(const Entity& entity)
		{
			if (!ShouldCollectLiveChanges())
				return;

			if (auto it = pendingChanges.createdEntities.find(entity.GetGuid()); it != pendingChanges.createdEntities.end())
			{
				it->second = BuildEntitySnapshot(entity);
				return;
			}

			auto& delta = pendingChanges.updatedEntities[entity.GetGuid()];
			delta.hasTransform = true;
			delta.position = entity.GetPosition();
			delta.rotation = entity.GetRotation();
			delta.scale = entity.GetScale();
		}

		void OnRenderSettingsChanged()
		{
			if (!ShouldCollectLiveChanges())
				return;

			pendingChanges.renderSettingsDirty = true;
		}

		bool IsRunning() const
		{
			return running.load();
		}

		bool IsClientConnected() const
		{
			return clientConnected.load();
		}

		uint16_t GetPort() const
		{
			return config.port;
		}

		uint64_t GetRevision() const
		{
			return currentRevision;
		}

		uint64_t GetActiveSessionId() const
		{
			return activeSessionId;
		}

		ExternalEditorBridge* owner = nullptr;

	private:
		void DetachObservers()
		{
			if (!observersAttached)
				return;

			if (Application::IsWindowValid())
			{
				if (sceneSystem != nullptr)
					sceneSystem->RemoveObserver(this);

				if (renderSystem != nullptr)
					renderSystem->RemoveSettingsObserver(this);
			}

			observersAttached = false;
			sceneSystem = nullptr;
			renderSystem = nullptr;
		}

		bool ShouldCollectLiveChanges() const
		{
			return activeSessionId != 0 && liveDeltaEnabled && !pendingChanges.fullSnapshotRequested;
		}

		void ScheduleFullSnapshot(const std::string& reason)
		{
			pendingChanges.ClearLiveChanges();
			pendingChanges.fullSnapshotRequested = true;
			pendingChanges.fullSnapshotReason = reason;
			liveDeltaEnabled = false;
		}

		void QueueOutboundMessage(OutboundMessage message)
		{
			outboundQueue.Push(std::move(message));
			WakeNetworkThread();
		}

		void WakeNetworkThread()
		{
			if (auto* context = static_cast<lws_context*>(lwsContext.load()); context != nullptr)
				lws_cancel_service(context);
		}

		void HandleInboundMessage(const InboundMessage& message)
		{
			switch (message.kind)
			{
				case InboundMessageKind::ClientConnected:
				{
					activeSessionId = message.sessionId;
					clientConnected = true;
					pendingChanges.ClearAll();
					ScheduleFullSnapshot("client-connected");
					break;
				}

				case InboundMessageKind::ClientDisconnected:
				{
					if (activeSessionId == message.sessionId)
					{
						activeSessionId = 0;
						clientConnected = false;
						liveDeltaEnabled = false;
						pendingChanges.ClearAll();
					}
					break;
				}

				case InboundMessageKind::InvalidPayload:
				{
					QueueErrorMessage(message.sessionId, message.errorMessage, false);
					break;
				}

				case InboundMessageKind::RequestFullSync:
				{
					if (activeSessionId == message.sessionId)
						ScheduleFullSnapshot(message.errorMessage.empty() ? "network-requested-resync" : message.errorMessage);
					break;
				}

				case InboundMessageKind::ClientCommand:
				{
					HandleCommandMessage(message);
					break;
				}
			}
		}

		void HandleCommandMessage(const InboundMessage& message)
		{
			if (activeSessionId == 0 || activeSessionId != message.sessionId)
				return;

			if (!message.payload.is_object())
			{
				QueueErrorMessage(message.sessionId, "Command payload must be an object", false);
				return;
			}

			const std::string kind = message.payload.value("kind", "");
			if (kind != "command")
			{
				QueueErrorMessage(message.sessionId, "Unsupported message kind", false);
				return;
			}

			if (message.payload.value("sessionId", activeSessionId) != activeSessionId)
			{
				QueueErrorMessage(message.sessionId, "Session mismatch", true);
				ScheduleFullSnapshot("session-mismatch");
				return;
			}

			const auto payloadIt = message.payload.find("payload");
			if (payloadIt == message.payload.end() || !payloadIt->is_object())
			{
				QueueErrorMessage(message.sessionId, "Command is missing payload", false);
				return;
			}

			const std::string type = payloadIt->value("type", "");
			if (type == "requestFullSync")
			{
				ScheduleFullSnapshot("client-requested-resync");
				return;
			}

			const uint64_t baseRevision = message.payload.value("baseRevision", 0ull);
			if (baseRevision != currentRevision)
			{
				QueueErrorMessage(message.sessionId, "Revision mismatch; scheduling full sync", true);
				ScheduleFullSnapshot("revision-mismatch");
				return;
			}

			bool success = false;
			if (type == "setEntityName")
				success = ApplySetEntityName(*payloadIt);
			else if (type == "setEntityTransform")
				success = ApplySetEntityTransform(*payloadIt);
			else if (type == "setRenderSettings")
				success = ApplySetRenderSettings(*payloadIt);
			else
				QueueErrorMessage(message.sessionId, "Unsupported command type", false);

			if (!success && (type == "setEntityName" || type == "setEntityTransform" || type == "setRenderSettings"))
				ScheduleFullSnapshot("command-application-failed");
		}

		bool ApplySetEntityName(const nlohmann::json& payload)
		{
			if (sceneSystem == nullptr || !payload.contains("guid") || !payload.contains("name"))
			{
				QueueErrorMessage(activeSessionId, "setEntityName requires guid and name", false);
				return false;
			}

			auto entity = sceneSystem->GetEntity(payload["guid"].get<uint32_t>()).lock();
			if (!entity)
			{
				QueueErrorMessage(activeSessionId, "Entity not found", true);
				return false;
			}

			entity->SetName(payload["name"].get<std::string>());
			return true;
		}

		bool ApplySetEntityTransform(const nlohmann::json& payload)
		{
			if (sceneSystem == nullptr || !payload.contains("guid"))
			{
				QueueErrorMessage(activeSessionId, "setEntityTransform requires guid", false);
				return false;
			}

			auto entity = sceneSystem->GetEntity(payload["guid"].get<uint32_t>()).lock();
			if (!entity)
			{
				QueueErrorMessage(activeSessionId, "Entity not found", true);
				return false;
			}

			Vector3f position = entity->GetPosition();
			Quaternionf rotation = entity->GetRotation();
			Vector3f scale = entity->GetScale();

			if (payload.contains("position") && !ParseVector3(payload["position"], &position))
			{
				QueueErrorMessage(activeSessionId, "Invalid position payload", false);
				return false;
			}

			if (payload.contains("rotationEuler") && !ParseEulerRotation(payload["rotationEuler"], &rotation))
			{
				QueueErrorMessage(activeSessionId, "Invalid rotationEuler payload", false);
				return false;
			}

			if (payload.contains("scale") && !ParseVector3(payload["scale"], &scale))
			{
				QueueErrorMessage(activeSessionId, "Invalid scale payload", false);
				return false;
			}

			entity->SetPosition(position);
			entity->SetRotation(rotation);
			entity->SetScale(scale);
			return true;
		}

		bool ApplySetRenderSettings(const nlohmann::json& payload)
		{
			if (renderSystem == nullptr)
			{
				QueueErrorMessage(activeSessionId, "RenderSystem unavailable", false);
				return false;
			}

			if (payload.contains("clearColor"))
			{
				std::array<float, 4> color;
				if (!ParseFloatArray<4>(payload["clearColor"], &color))
				{
					QueueErrorMessage(activeSessionId, "Invalid clearColor payload", false);
					return false;
				}
				renderSystem->SetClearColor(color[0], color[1], color[2], color[3]);
			}

			if (payload.contains("ambientColor"))
			{
				Vector3f ambient;
				if (!ParseVector3(payload["ambientColor"], &ambient))
				{
					QueueErrorMessage(activeSessionId, "Invalid ambientColor payload", false);
					return false;
				}
				renderSystem->SetAmbientColor(ambient.x, ambient.y, ambient.z);
			}

			if (payload.contains("ambientStrength"))
				renderSystem->SetAmbientStrength(payload["ambientStrength"].get<float>());

			if (payload.contains("shadow") && payload["shadow"].is_object())
			{
				const auto& shadow = payload["shadow"];
				if (shadow.contains("constantBias"))
					renderSystem->SetShadowConstantBias(shadow["constantBias"].get<float>());
				if (shadow.contains("slopeScale"))
					renderSystem->SetShadowSlopeScale(shadow["slopeScale"].get<float>());
				if (shadow.contains("normalOffset"))
					renderSystem->SetShadowNormalOffset(shadow["normalOffset"].get<float>());
			}

			if (payload.contains("vsyncEnabled"))
				renderSystem->SetVSyncEnabled(payload["vsyncEnabled"].get<bool>());

			if (payload.contains("msaaEnabled"))
				renderSystem->SetMSAAEnabled(payload["msaaEnabled"].get<bool>());

			if (payload.contains("skyboxEnabled"))
				renderSystem->SetSkyboxEnabled(payload["skyboxEnabled"].get<bool>());

			if (payload.contains("toneMapping") && payload["toneMapping"].is_object())
			{
				if (auto* toneMapping = renderSystem->GetToneMappingEffect(); toneMapping != nullptr)
				{
					const auto& toneMappingJson = payload["toneMapping"];
					if (toneMappingJson.contains("exposure"))
						toneMapping->SetExposure(toneMappingJson["exposure"].get<float>());
					if (toneMappingJson.contains("gamma"))
						toneMapping->SetGamma(toneMappingJson["gamma"].get<float>());
				}
			}

			if (payload.contains("bloom") && payload["bloom"].is_object())
			{
				if (auto* bloom = renderSystem->GetBloomMipChainEffect(); bloom != nullptr)
				{
					const auto& bloomJson = payload["bloom"];
					if (bloomJson.contains("threshold"))
						bloom->SetThreshold(bloomJson["threshold"].get<float>());
					if (bloomJson.contains("softKnee"))
						bloom->SetSoftKnee(bloomJson["softKnee"].get<float>());
					if (bloomJson.contains("intensity"))
						bloom->SetBloomIntensity(bloomJson["intensity"].get<float>());
					if (bloomJson.contains("blendFactor"))
						bloom->SetBlendFactor(bloomJson["blendFactor"].get<float>());
				}
			}

			return true;
		}

		nlohmann::json BuildFullSnapshotPayload(const std::string& reason) const
		{
			nlohmann::json payload;
			payload["reason"] = reason;
			payload["entities"] = nlohmann::json::array();

			if (sceneSystem != nullptr)
			{
				auto entities = sceneSystem->GetAllRawEntities();
				std::sort(entities.begin(), entities.end(), [](const Entity* left, const Entity* right) {
					return left->GetGuid() < right->GetGuid();
				});

				for (const Entity* entity : entities)
					payload["entities"].push_back(BuildEntitySnapshot(*entity));
			}

			payload["renderSettings"] = BuildRenderSettingsSnapshot();
			return payload;
		}

		nlohmann::json BuildDeltaPayload()
		{
			nlohmann::json payload;
			payload["changes"] = nlohmann::json::array();

			std::vector<uint32_t> createdGuids;
			createdGuids.reserve(pendingChanges.createdEntities.size());
			for (const auto& [guid, entityJson] : pendingChanges.createdEntities)
			{
				(void)entityJson;
				createdGuids.push_back(guid);
			}
			std::sort(createdGuids.begin(), createdGuids.end());

			for (uint32_t guid : createdGuids)
			{
				payload["changes"].push_back({
					{"type", "entityCreated"},
					{"entity", pendingChanges.createdEntities.at(guid)},
				});
			}

			std::vector<uint32_t> updatedGuids;
			updatedGuids.reserve(pendingChanges.updatedEntities.size());
			for (const auto& [guid, delta] : pendingChanges.updatedEntities)
			{
				(void)delta;
				updatedGuids.push_back(guid);
			}
			std::sort(updatedGuids.begin(), updatedGuids.end());

			for (uint32_t guid : updatedGuids)
			{
				const auto& delta = pendingChanges.updatedEntities.at(guid);
				nlohmann::json change = {
					{"type", "entityPatched"},
					{"guid", guid},
				};

				if (delta.name.has_value())
					change["name"] = *delta.name;

				if (delta.hasParent)
					change["parent"] = delta.parentGuid.has_value() ? nlohmann::json(*delta.parentGuid) : nlohmann::json(nullptr);

				if (delta.hasTransform)
					change["transform"] = BuildTransformPayload(delta.position, delta.rotation, delta.scale);

				payload["changes"].push_back(std::move(change));
			}

			std::vector<uint32_t> destroyedGuids(pendingChanges.destroyedEntities.begin(), pendingChanges.destroyedEntities.end());
			std::sort(destroyedGuids.begin(), destroyedGuids.end());

			for (uint32_t guid : destroyedGuids)
			{
				payload["changes"].push_back({
					{"type", "entityDestroyed"},
					{"guid", guid},
				});
			}

			if (pendingChanges.renderSettingsDirty)
			{
				payload["changes"].push_back({
					{"type", "renderSettingsUpdated"},
					{"renderSettings", BuildRenderSettingsSnapshot()},
				});
			}

			pendingChanges.ClearLiveChanges();
			return payload;
		}

		nlohmann::json BuildEntitySnapshot(const Entity& entity) const
		{
			nlohmann::json json;
			json["guid"] = entity.GetGuid();
			json["name"] = entity.GetName();
			json["parent"] = entity.GetParent() != nullptr ? nlohmann::json(entity.GetParent()->GetGuid()) : nlohmann::json(nullptr);
			json["transform"] = BuildTransformPayload(entity.GetPosition(), entity.GetRotation(), entity.GetScale());
			return json;
		}

		nlohmann::json BuildRenderSettingsSnapshot() const
		{
			nlohmann::json json;
			if (renderSystem == nullptr)
				return json;

			const auto clearColor = renderSystem->GetClearColor();
			const auto ambientColor = renderSystem->GetAmbientColor();
			json["clearColor"] = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };
			json["ambientColor"] = { ambientColor.x, ambientColor.y, ambientColor.z };
			json["ambientStrength"] = renderSystem->GetAmbientStrength();
			json["shadow"] = {
				{"constantBias", renderSystem->GetShadowConstantBias()},
				{"slopeScale", renderSystem->GetShadowSlopeScale()},
				{"normalOffset", renderSystem->GetShadowNormalOffset()},
			};
			json["vsyncEnabled"] = renderSystem->IsVSyncEnabled();
			json["msaaEnabled"] = renderSystem->IsMSAAEnabled();
			json["skyboxEnabled"] = renderSystem->IsSkyboxEnabled();

			if (const auto* toneMapping = renderSystem->GetToneMappingEffect(); toneMapping != nullptr)
			{
				json["toneMapping"] = {
					{"exposure", toneMapping->GetExposure()},
					{"gamma", toneMapping->GetGamma()},
				};
			}

			if (const auto* bloom = renderSystem->GetBloomMipChainEffect(); bloom != nullptr)
			{
				json["bloom"] = {
					{"threshold", bloom->GetThreshold()},
					{"softKnee", bloom->GetSoftKnee()},
					{"intensity", bloom->GetBloomIntensity()},
					{"blendFactor", bloom->GetBlendFactor()},
				};
			}

			return json;
		}

		static auto BuildTransformPayload(const Vector3f& position, const Quaternionf& rotation, const Vector3f& scale) -> nlohmann::json
		{
			const auto euler = Math::QuaternionToEulerAngles(rotation);
			return {
				{"position", { position.x, position.y, position.z }},
				{"rotationEuler", { euler.roll, euler.pitch, euler.yaw }},
				{"scale", { scale.x, scale.y, scale.z }},
			};
		}

		template <size_t Size>
		static bool ParseFloatArray(const nlohmann::json& json, std::array<float, Size>* out)
		{
			if (!json.is_array() || json.size() != Size)
				return false;

			for (size_t index = 0; index < Size; index++)
				(*out)[index] = json[index].get<float>();

			return true;
		}

		bool ParseVector3(const nlohmann::json& json, Vector3f* out) const
		{
			std::array<float, 3> values;
			if (!ParseFloatArray<3>(json, &values))
				return false;

			*out = { values[0], values[1], values[2] };
			return true;
		}

		bool ParseEulerRotation(const nlohmann::json& json, Quaternionf* out) const
		{
			std::array<float, 3> values;
			if (!ParseFloatArray<3>(json, &values))
				return false;

			EulerAnglesf euler;
			euler.roll = values[0];
			euler.pitch = values[1];
			euler.yaw = values[2];
			*out = Math::EulerAngleToQuaternion(euler);
			return true;
		}

		void QueueErrorMessage(uint64_t sessionId, const std::string& message, bool fullResyncScheduled)
		{
			if (sessionId == 0)
				return;

			QueueOutboundMessage({
				.kind = OutboundMessageKind::ErrorMessage,
				.sessionId = sessionId,
				.revision = currentRevision,
				.payload = {
					{"message", message},
					{"fullResyncScheduled", fullResyncScheduled},
				},
			});
		}

		static int LwsCallback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
		{
			(void)user;
			auto* bridge = static_cast<Impl*>(lws_context_user(lws_get_context(wsi)));
			return bridge != nullptr ? bridge->HandleLwsCallback(wsi, reason, in, len) : 0;
		}

		int HandleLwsCallback(lws* wsi, lws_callback_reasons reason, void* in, size_t len)
		{
			switch (reason)
			{
				case LWS_CALLBACK_ESTABLISHED:
				{
					if (clientWsi != nullptr && clientWsi != wsi)
					{
						unsigned char reasonText[] = "single-client-only";
						lws_close_reason(wsi, LWS_CLOSE_STATUS_POLICY_VIOLATION, reasonText, sizeof(reasonText) - 1);
						return -1;
					}

					clientWsi = wsi;
					networkSessionId = nextSessionId.fetch_add(1);
					clientConnected = true;
					pendingNetworkMessages.clear();
					inboundQueue.Push({
						.kind = InboundMessageKind::ClientConnected,
						.sessionId = networkSessionId,
					});
					break;
				}

				case LWS_CALLBACK_CLOSED:
				{
					if (clientWsi == wsi)
					{
						inboundQueue.Push({
							.kind = InboundMessageKind::ClientDisconnected,
							.sessionId = networkSessionId,
						});
						clientWsi = nullptr;
						networkSessionId = 0;
						clientConnected = false;
						pendingNetworkMessages.clear();
					}
					break;
				}

				case LWS_CALLBACK_RECEIVE:
				{
					if (wsi != clientWsi)
						break;

					try
					{
						std::string text(static_cast<const char*>(in), len);
						inboundQueue.Push({
							.kind = InboundMessageKind::ClientCommand,
							.sessionId = networkSessionId,
							.payload = nlohmann::json::parse(text),
						});
					}
					catch (const std::exception& ex)
					{
						inboundQueue.Push({
							.kind = InboundMessageKind::InvalidPayload,
							.sessionId = networkSessionId,
							.errorMessage = ex.what(),
						});
					}
					break;
				}

				case LWS_CALLBACK_SERVER_WRITEABLE:
				{
					if (wsi != clientWsi || pendingNetworkMessages.empty())
						break;

					std::string text = std::move(pendingNetworkMessages.front());
					pendingNetworkMessages.pop_front();
					std::vector<unsigned char> buffer(LWS_PRE + text.size());
					std::memcpy(buffer.data() + LWS_PRE, text.data(), text.size());

					const int written = lws_write(wsi, buffer.data() + LWS_PRE, text.size(), LWS_WRITE_TEXT);
					if (written < 0 || static_cast<size_t>(written) != text.size())
						return -1;

					if (!pendingNetworkMessages.empty())
						lws_callback_on_writable(wsi);
					break;
				}

				default:
					break;
			}

			return 0;
		}

		void NetworkThreadMain()
		{
			protocolNameStorage = config.protocolName;
			protocols[0] = lws_protocols {
				.name = protocolNameStorage.c_str(),
				.callback = &Impl::LwsCallback,
				.per_session_data_size = 0,
				.rx_buffer_size = 64 * 1024,
			};
			protocols[1] = lws_protocols {};

			lws_context_creation_info info {};
			info.port = config.port;
			info.protocols = protocols.data();
			info.user = this;
			info.options = LWS_SERVER_OPTION_DISABLE_IPV6;

			auto* context = lws_create_context(&info);
			{
				std::scoped_lock lock(startupMutex);
				startupResolved = true;
				startupSucceeded = context != nullptr;
			}
			startupCv.notify_all();

			if (context == nullptr)
			{
				Logger::LogError("ExternalEditorBridge: failed to create libwebsockets context");
				return;
			}

			lwsContext = context;

			while (!stopRequested.load())
			{
				DrainOutboundMessages();
				lws_service(context, 25);
			}

			if (clientWsi != nullptr)
				lws_set_timeout(clientWsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);

			pendingNetworkMessages.clear();
			clientWsi = nullptr;
			networkSessionId = 0;
			clientConnected = false;
			lwsContext = nullptr;
			lws_context_destroy(context);
		}

		void DrainOutboundMessages()
		{
			outboundQueue.Swap(outboundScratch);
			for (const auto& message : outboundScratch)
			{
				if (message.sessionId == 0 || message.sessionId != networkSessionId)
					continue;

				if (message.kind == OutboundMessageKind::FullSnapshot)
					pendingNetworkMessages.clear();

				if (pendingNetworkMessages.size() >= MAX_PENDING_NETWORK_MESSAGES && message.kind != OutboundMessageKind::FullSnapshot)
				{
					pendingNetworkMessages.clear();
					inboundQueue.Push({
						.kind = InboundMessageKind::RequestFullSync,
						.sessionId = networkSessionId,
						.errorMessage = "outbound-backlog-resync",
					});
					break;
				}

				pendingNetworkMessages.push_back(SerializeOutboundMessage(message));
			}

			if (clientWsi != nullptr && !pendingNetworkMessages.empty())
				lws_callback_on_writable(clientWsi);
		}

		static auto SerializeOutboundMessage(const OutboundMessage& message) -> std::string
		{
			nlohmann::json json = {
				{"kind", OutboundKindToString(message.kind)},
				{"sessionId", message.sessionId},
				{"revision", message.revision},
				{"payload", message.payload},
			};
			return json.dump();
		}
	};

	ExternalEditorBridge::ExternalEditorBridge()
		: _impl(std::make_unique<Impl>())
	{
		_impl->owner = this;
	}

	ExternalEditorBridge::~ExternalEditorBridge()
	{
		RequestStop();
		Join();
	}

	bool ExternalEditorBridge::Start()
	{
		return Start(Config {});
	}

	bool ExternalEditorBridge::Start(const Config& config)
	{
		return _impl->Start(config);
	}

	void ExternalEditorBridge::RequestStop()
	{
		_impl->RequestStop();
	}

	void ExternalEditorBridge::Join()
	{
		_impl->Join();
	}

	bool ExternalEditorBridge::IsRunning() const
	{
		return _impl->IsRunning();
	}

	bool ExternalEditorBridge::IsClientConnected() const
	{
		return _impl->IsClientConnected();
	}

	uint16_t ExternalEditorBridge::GetPort() const
	{
		return _impl->GetPort();
	}

	uint64_t ExternalEditorBridge::GetRevision() const
	{
		return _impl->GetRevision();
	}

	uint64_t ExternalEditorBridge::GetActiveSessionId() const
	{
		return _impl->GetActiveSessionId();
	}

	void ExternalEditorBridge::PumpMainThreadPostEvent()
	{
		_impl->PumpMainThreadPostEvent();
	}

	void ExternalEditorBridge::PumpMainThreadPreRender()
	{
		_impl->PumpMainThreadPreRender();
	}

	void ExternalEditorBridge::OnEntityCreated(const Entity& entity)
	{
		_impl->OnEntityCreated(entity);
	}

	void ExternalEditorBridge::OnEntityDestroyed(const Entity& entity)
	{
		_impl->OnEntityDestroyed(entity);
	}

	void ExternalEditorBridge::OnEntityNameChanged(const Entity& entity)
	{
		_impl->OnEntityNameChanged(entity);
	}

	void ExternalEditorBridge::OnEntityParentChanged(const Entity& entity)
	{
		_impl->OnEntityParentChanged(entity);
	}

	void ExternalEditorBridge::OnEntityTransformChanged(const Entity& entity)
	{
		_impl->OnEntityTransformChanged(entity);
	}

	void ExternalEditorBridge::OnRenderSettingsChanged()
	{
		_impl->OnRenderSettingsChanged();
	}
} // namespace Ailurus