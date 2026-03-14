#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "Ailurus/Systems/RenderSystem/RenderSystem.h"
#include "Ailurus/Systems/SceneSystem/SceneSystem.h"

namespace Ailurus
{
	class ExternalEditorBridge final : public SceneObserver, public RenderSettingsObserver
	{
	public:
		struct Config
		{
			Config()
				: port(12138)
				, protocolName("ailurus-editor")
			{
			}

			uint16_t port;
			std::string protocolName;
		};

	public:
		ExternalEditorBridge();
		~ExternalEditorBridge() override;

		bool Start();
		bool Start(const Config& config);
		void RequestStop();
		void Join();

		bool IsRunning() const;
		bool IsClientConnected() const;
		uint16_t GetPort() const;
		uint64_t GetRevision() const;
		uint64_t GetActiveSessionId() const;

		void PumpMainThreadPostEvent();
		void PumpMainThreadPreRender();

		void OnEntityCreated(const Entity& entity) override;
		void OnEntityDestroyed(const Entity& entity) override;
		void OnEntityNameChanged(const Entity& entity) override;
		void OnEntityParentChanged(const Entity& entity) override;
		void OnEntityTransformChanged(const Entity& entity) override;
		void OnRenderSettingsChanged() override;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
} // namespace Ailurus