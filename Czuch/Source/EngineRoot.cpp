#include"czpch.h"
#include "EngineRoot.h"
#include "Core/TimeDiffCounter.h"
#include "Core/Time.h"

namespace Czuch
{
	void EngineRoot::Init(const std::string& configFilePath)
	{
		//create subsystems
		m_Logging = new Logging();
		m_EventsMgr = new EventsManager();

		//Init subsystems
		m_Logging->Init();
		m_EventsMgr->Init();
	}

	void EngineRoot::Shutdown()
	{
		//Shutdown
		m_EventsMgr->Shutdown();
		m_Logging->Shutdown();
		
		//free memory
		delete m_EventsMgr;
		delete m_Logging;
	}

	void EngineRoot::Run()
	{
		F32 dt = 1 / 30.0f;
		TimeDiffCounter counter(TimeDiffCounter::Mode::Seconds);
		
		while (!ShouldStopGameLoop())
		{
			//Update events
			m_EventsMgr->Update();

			UpdateDeltaTime(counter);
		}
	}
	void EngineRoot::UpdateDeltaTime(Czuch::TimeDiffCounter& counter)
	{
		//Update game engine dt
		F32 dt = counter.GetCounter(true);

		if (dt >= 1.0f)
		{
			dt = FPS30_TIME;
		}

		Time::DeltaTime = dt;
	}
	bool EngineRoot::ShouldStopGameLoop()
	{
		return false;
	}
}

