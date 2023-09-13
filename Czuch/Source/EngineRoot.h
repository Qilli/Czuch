#pragma once

#include"Core/EngineCore.h"
#include"Subsystems/Logging.h"
#include"Core/TimeDiffCounter.h"
#include"Subsystems/EventsManager.h"

namespace Czuch
{
	class CZUCH_API EngineRoot
	{
	public:
		void Init(const std::string& configFilePath);
		void Shutdown();
		void Run();
	private:
		void UpdateDeltaTime(TimeDiffCounter& counter);
	private:
		bool ShouldStopGameLoop();
	private:
		Logging *m_Logging;
		EventsManager* m_EventsMgr;
	};
}

