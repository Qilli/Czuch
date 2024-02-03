#pragma once

#include"Core/EngineCore.h"
#include"Subsystems/Logging.h"
#include"Core/TimeDiffCounter.h"
#include"Subsystems/EventsManager.h"
#include"Core/Window.h"

namespace Czuch
{
	class Renderer;
	class AssetsManager;

	class CZUCH_API EngineRoot: public IEventsListener
	{
	public:
		void Init(const std::string& configFilePath);
		void Shutdown();
		void Run();
		virtual void OnEvent(const Event& e) override;
	private:
		void UpdateDeltaTime(TimeDiffCounter& counter);
	private:
		bool ShouldStopGameLoop();
	private:
		Logging *m_Logging;
		EventsManager* m_EventsMgr;
		Renderer* m_Renderer;
		Scope<Window> m_Window;
		AssetsManager* m_ResourcesMgr;
	private:
		bool m_ShouldStopLoop;
	};
}

