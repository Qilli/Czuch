#pragma once
#include"Core/TimeDelta.h"
#include"Core/EngineCore.h"

namespace Czuch
{
	class CZUCH_API EngineEditorControl
	{
	public:
		virtual void FillUI(void* sceneViewportTexture) = 0;
		virtual void Update(TimeDelta delta) = 0;
		virtual void Init(void* ctx,RenderSettings* renderSettings)=0;
		virtual void AfterSystemInit() = 0;
		virtual void Shutdown()=0;
	};
}
