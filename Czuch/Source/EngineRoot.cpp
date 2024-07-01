#include"czpch.h"
#include "EngineRoot.h"
#include "Core/TimeDiffCounter.h"
#include "Core/Time.h"
#include "Events/EventsTypes/ApplicationEvents.h"
#include "Renderer/Vulkan/VulkanRenderer.h"
#include "Subsystems/Assets/AssetsManager.h"
#include "Subsystems/Assets/AssetManagersTypes/ShaderAssetManager.h"
#include "Subsystems/Assets/AssetManagersTypes/TextureAssetManager.h"
#include "Subsystems/Assets/AssetManagersTypes/MaterialAssetManager.h"
#include "Subsystems/Assets/AssetManagersTypes/MaterialInstanceAssetManager.h"
#include "Subsystems/Assets/AssetManagersTypes/ModelAssetManager.h"
#include "Subsystems/Scenes/ScenesManager.h"
#include "Subsystems/UI/ImGUIManager.h"
#include "Subsystems/UI/Editor/EngineEditorSubsystem.h"
#include "Subsystems/Scenes/Components/CameraComponent.h"

namespace Czuch
{
	U32 WindowInfo::Width = 0;
	U32 WindowInfo::Height = 0;

	void EngineRoot::Init(const std::string& configFilePath, EngineEditorControl* control)
	{
		m_ShouldStopLoop = false;

		m_RenderSettings.dynamicRendering = false;
		m_RenderSettings.offscreenRendering = true;
		m_RenderSettings.validationMode = ValidationMode::Enabled;
		m_RenderSettings.engineMode = EngineMode::Editor;

		//create subsystems
		m_Logging = new Logging();
		m_EventsMgr = new EventsManager();

		//Init subsystems
		m_Logging->Init(&m_RenderSettings);
		m_EventsMgr->Init(&m_RenderSettings);

		const WindowParams wndParams{};

		WindowInfo::Width = wndParams.Width;
		WindowInfo::Height = wndParams.Height;

		//create window
		m_Window = Window::Create(wndParams);

		//create renderer
		m_Renderer = new VulkanRenderer(m_Window.get(), &m_RenderSettings);
		m_Renderer->Init();

		//create resources managers
		m_ResourcesMgr = new AssetsManager();
		m_ResourcesMgr->Init(&m_RenderSettings);
		m_ResourcesMgr->RegisterManager(new ShaderAssetManager(m_Renderer->GetDevice()), typeid(ShaderAsset));
		m_ResourcesMgr->RegisterManager(new TextureAssetManager(m_Renderer->GetDevice()), typeid(TextureAsset));
		m_ResourcesMgr->RegisterManager(new MaterialAssetManager(m_Renderer->GetDevice()), typeid(MaterialAsset));
		m_ResourcesMgr->RegisterManager(new MaterialInstanceAssetManager(m_Renderer->GetDevice()), typeid(MaterialInstanceAsset));
		m_ResourcesMgr->RegisterManager(new ModelAssetManager(m_Renderer->GetDevice()), typeid(ModelAsset));

		//create ui manager
		m_UIBaseMgr = new ImGUIManager(m_Renderer->GetDevice(), m_Window.get());
		m_UIBaseMgr->Init(&m_RenderSettings);

		//create editor subsystem
		m_EditorSubsystem = new EngineEditorSubsystem(control);
		m_EditorSubsystem->Init(&m_RenderSettings);

		//listen to events
		m_EventsMgr->AddListener(WindowClosedEvent::GetStaticEventType(), this);

		//create default assetse
		m_DefaultAssets = new BuildInAssets(m_Renderer->GetDevice(), m_ResourcesMgr);
		m_DefaultAssets->BuildAndLoad();

		//init scenes manager
		m_ScenesMgr = new ScenesManager(m_Renderer, m_ResourcesMgr);
		m_ScenesMgr->Init(&m_RenderSettings);

		//for editor mode add dockspace with editor ui
		m_UIBaseMgr->EnableEditorMode(m_RenderSettings.engineMode == EngineMode::Editor);
	}

	void EngineRoot::Shutdown()
	{
		m_ScenesMgr->Shutdown();
		delete m_ScenesMgr;
		delete m_DefaultAssets;

		m_EditorSubsystem->Shutdown();
		m_UIBaseMgr->Shutdown();

		//Shutdown
		m_ResourcesMgr->Shutdown();
		m_EventsMgr->Shutdown();
		m_Logging->Shutdown();

		//free memory
		delete m_EditorSubsystem;
		delete m_UIBaseMgr;
		delete m_ResourcesMgr;
		delete m_Renderer;
		delete m_EventsMgr;
		delete m_Logging;
	}

	void EngineRoot::Run()
	{
		F32 dt = 1 / 30.0f;
		TimeDiffCounter counter(TimeDiffCounter::Mode::Seconds);
		counter.StartCounter();

		while (!ShouldStopGameLoop())
		{
			//Update events
			auto deltaTime = UpdateDeltaTime(counter);
			m_EventsMgr->Update(deltaTime);
			m_Window->Update();
			m_ScenesMgr->Update(deltaTime);
			m_EditorSubsystem->Update(deltaTime);
			m_UIBaseMgr->SetSceneForUI(m_ScenesMgr->GetActiveScene());
			m_UIBaseMgr->Update(deltaTime);
			m_Renderer->DrawFrame();
			m_ScenesMgr->OnFinishFrame();
		}
		m_Renderer->AwaitDeviceIdle();
	}
	TimeDelta EngineRoot::UpdateDeltaTime(Czuch::TimeDiffCounter& counter)
	{
		//Update game engine dt
		F32 dt = counter.GetCounter(true);

		if (dt >= 1.0f)
		{
			dt = FPS30_TIME;
		}

		Time::DeltaTime = dt;
		return TimeDelta(dt);
	}
	bool EngineRoot::ShouldStopGameLoop()
	{
		return m_ShouldStopLoop;
	}
	void EngineRoot::OnEvent(Event& e)
	{
		if (e.GetEventType() == WindowClosedEvent::GetStaticEventType())
		{
			m_ShouldStopLoop = true;
		}
	}
}

