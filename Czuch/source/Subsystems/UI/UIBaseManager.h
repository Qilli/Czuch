#pragma once
#include "./Subsystems/BaseSubsystem.h"
#include "Events/IEventsListener.h"

namespace Czuch
{
	class Scene;

	class CZUCH_API UIBaseManager : public BaseSubsystem<UIBaseManager>,public IEventsListener
	{
	public:
		virtual void Init(EngineSettings* settings) override { BaseSubsystem::Init(settings); };
		virtual void Shutdown() override {};
		virtual void Update(TimeDelta timeDelta) override {};
		virtual void SetSceneForUI(Scene* scene) {};
		virtual void Draw() {};
		virtual void EnableEditorMode(bool enable) { m_EditorModeEnabled = enable; };
		virtual void* GetUIContext() { return m_UIContext; };
		void SetBlockEvents(bool block) { m_BlockEvents = block; }
	protected:
		bool m_EditorModeEnabled = false;
		void* m_UIContext;
		bool m_BlockEvents = false;
	};
}


