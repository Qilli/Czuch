#pragma once
#include"Core/Singleton.h"
#include"Core/TimeDelta.h"
#include"Core/EngineCore.h"

namespace Czuch
{
	template <typename T > class CZUCH_API BaseSubsystem : public Singleton<T>
	{
	public:
		virtual void Init(RenderSettings* settings) { m_RenderSettings = settings; }
		virtual void Shutdown() = 0;
		virtual void Update(TimeDelta timeDelta) = 0;
		virtual void OnFinishFrame() {}
		virtual bool IsInEditorMode() { return m_RenderSettings->engineMode == EngineMode::Editor; }
	protected:
		RenderSettings* m_RenderSettings;
	};
}

