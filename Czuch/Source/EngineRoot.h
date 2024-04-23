#pragma once

#include"Core/EngineCore.h"
#include"Subsystems/Logging.h"
#include"Core/TimeDiffCounter.h"
#include"Subsystems/EventsManager.h"
#include"Core/Window.h"
#include"Subsystems/Assets/BuildInAssets.h"

namespace Czuch
{
	class Renderer;
	class AssetsManager;
	class ScenesManager;

	class CZUCH_API EngineRoot: public IEventsListener
	{
	public:
		void Init(const std::string& configFilePath);
		void Shutdown();
		void Run();
		Renderer& GetRenderer() const { return *m_Renderer; }
		ScenesManager& GetScenesManager() const { return *m_ScenesMgr; }
		virtual void OnEvent(const Event& e) override;
	private:
		TimeDelta UpdateDeltaTime(TimeDiffCounter& counter);
	private:
		bool ShouldStopGameLoop();
	private:
		Logging *m_Logging;
		EventsManager* m_EventsMgr;
		Renderer* m_Renderer;
		Scope<Window> m_Window;
		AssetsManager* m_ResourcesMgr;
		ScenesManager *m_ScenesMgr;
	private:
		BuildInAssets* m_DefaultAssets;
	private:
		bool m_ShouldStopLoop;
	};
}

