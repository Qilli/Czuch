#include"czpch.h"
#include "EngineRoot.h"
#include "Core/TimeDiffCounter.h"
#include "Core/Time.h"
#include "Events/EventsTypes/ApplicationEvents.h"
#include "Renderer/Vulkan/VulkanRenderer.h"
#include "Subsystems/Assets/AssetsManager.h"
#include "Subsystems/Assets/AssetManagersTypes/ShaderAssetManager.h"
#include "Subsystems/Assets/AssetManagersTypes/TextureAssetManager.h"
namespace Czuch
{
	void EngineRoot::Init(const std::string& configFilePath)
	{
		m_ShouldStopLoop = false;

		//create subsystems
		m_Logging = new Logging();
		m_EventsMgr = new EventsManager();

		//Init subsystems
		m_Logging->Init();
		m_EventsMgr->Init();

		const WindowParams wndParams{};

		//create window
		m_Window = Window::Create(wndParams);

		//create renderer
		m_Renderer = new VulkanRenderer(m_Window.get(), ValidationMode::Enabled);
		m_Renderer->Init();

		//create resources managers
		m_ResourcesMgr = new AssetsManager();
		m_ResourcesMgr->Init();
		m_ResourcesMgr->RegisterManager(new ShaderAssetManager(m_Renderer->GetDevice()),typeid(ShaderAsset));
		m_ResourcesMgr->RegisterManager(new TextureAssetManager(m_Renderer->GetDevice()), typeid(TextureAsset));

		//listen to events
		m_EventsMgr->AddListener(WindowClosedEvent::GetStaticEventType(), this);
	}

	void EngineRoot::Shutdown()
	{
		//Shutdown
		m_ResourcesMgr->Shutdown();
		m_EventsMgr->Shutdown();
		m_Logging->Shutdown();
		
		//free memory
		delete m_ResourcesMgr;
		delete m_Renderer;
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
			m_Window->Update();
			UpdateDeltaTime(counter);
			m_Renderer->DrawFrame();
		}
		m_Renderer->AwaitDeviceIdle();
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
		return m_ShouldStopLoop;
	}
	void EngineRoot::OnEvent(const Event& e)
	{
		if (e.GetEventType() == WindowClosedEvent::GetStaticEventType())
		{
			m_ShouldStopLoop = true;
		}
	}
}

