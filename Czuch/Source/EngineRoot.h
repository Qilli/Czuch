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
	class UIBaseManager;
	class EngineEditorControl;
	class Camera;
	class EngineEditorSubsystem;

	class CZUCH_API EngineRoot:public Singleton<EngineRoot>,IEventsListener
	{
	public:
		void Init(const std::string& configFilePath,EngineEditorControl* editorControl);
		void Shutdown();
		void Run();
		Renderer& GetRenderer() const { return *m_Renderer; }
		ScenesManager& GetScenesManager() const { return *m_ScenesMgr; }
		UIBaseManager& GetUIBaseManager() const { return *m_UIBaseMgr; }
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
		UIBaseManager* m_UIBaseMgr;
		EngineEditorSubsystem* m_EditorSubsystem;
	private:
		BuildInAssets* m_DefaultAssets;
	private:
		bool m_ShouldStopLoop;
		RenderSettings m_RenderSettings;
	};
}

