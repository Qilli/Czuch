#pragma once
#include "./Subsystems/BaseSubsystem.h"

namespace Czuch
{
	class Scene;

	class CZUCH_API UIBaseManager : public BaseSubsystem<UIBaseManager>
	{
	public:
		virtual void Init() override {};
		virtual void Shutdown() override {};
		virtual void Update(TimeDelta timeDelta) override {};
		virtual void SetSceneForUI(Scene* scene) {};
		virtual void Draw() {};
		virtual void EnableEditorMode(bool enable) { m_EditorModeEnabled = enable; };
		virtual void* GetUIContext() { return m_UIContext; };
	protected:
		bool m_EditorModeEnabled = false;
		void* m_UIContext;
	};
}


