#pragma once
#include "./Subsystems/BaseSubsystem.h"
#include "EngineEditorControl.h"

namespace Czuch
{
	class EngineRoot;
	class CZUCH_API EngineEditorSubsystem : public BaseSubsystem<EngineEditorSubsystem>
	{
    public:
        EngineEditorSubsystem(EngineEditorControl* editorControl);
        virtual void Init(RenderSettings* settings) override;
        virtual void AfterSystemInit();
        virtual void Shutdown() override;
        virtual void Update(TimeDelta timeDelta) override;
        virtual void FillUI();
    private:
        EngineRoot* m_Root;
        EngineEditorControl* m_EditorControl;
	};
}

