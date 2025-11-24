#pragma once

#include"Core/EngineCore.h"
#include"Subsystems/Logging.h"
#include"Core/TimeDiffCounter.h"
#include"Subsystems/EventsManager.h"
#include"Core/Window.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include"Subsystems/InputManager.h"

namespace Czuch
{
	class Renderer;
	class AssetsManager;
	class ScenesManager;
	class UIBaseManager;
	class EngineEditorControl;
	struct Camera;
	class EngineEditorSubsystem;

	enum class CZUCH_API EngineStateMode
	{
		Editor,
		Playmode
	};

	enum class CZUCH_API UpdateMode
	{
		Locked,
		Unlocked
	};

	class CZUCH_API EngineRoot:public Singleton<EngineRoot>,IEventsListener
	{
	public:
		void Init(const std::string& configFilePath,EngineEditorControl* editorControl);
		void Shutdown();
		void Run();
		Renderer& GetRenderer() const { return *m_Renderer; }
		ScenesManager& GetScenesManager() const { return *m_ScenesMgr; }
		UIBaseManager& GetUIBaseManager() const { return *m_UIBaseMgr; }
		Window& GetWindow() const { return *m_Window; }
		virtual void OnEvent(Event& e) override;
		inline void SetGameMode(EngineMode mode) { m_EngineSettings.engineMode = mode; }
		EngineMode GetGameMode() const { return m_EngineSettings.engineMode; }
		EngineStateMode GetEngineStateMode() const { return m_EngineStateMode; }
		void SetEngineStateMode(EngineStateMode mode) { m_EngineStateMode = mode; }
		UpdateMode GetUpdateMode() const { return m_UpdateMode; }
		void SetUpdateMode(UpdateMode mode) { m_UpdateMode = mode; }
		void TryBuildDefaultAssets();
	public:
		static EngineSettings& GetEngineSettings() { return m_EngineSettings; }
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
		InputManager* m_InputMgr;
		EngineEditorSubsystem* m_EditorSubsystem;
	private:
		BuildInAssets* m_DefaultAssets;
	private:
		bool m_ShouldStopLoop;
		EngineStateMode m_EngineStateMode;
		UpdateMode m_UpdateMode;
	private:
		static EngineSettings m_EngineSettings;
	};
}

