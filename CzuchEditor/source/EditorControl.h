#pragma once
#include "Czuch.h"
#include"Panels/SceneHierarchyEditorPanel.h"
#include"Panels/EntityInspectorEditorPanel.h"
#include"Commands/EditorCommandsControl.h"

namespace Czuch
{
	class EngineRoot;
	class EditorControl : public EngineEditorControl
	{
    public:
        EditorControl();
        virtual ~EditorControl();
        virtual void Init(void* context) override;
        virtual void Shutdown() override;
        virtual void Update(TimeDelta timeDelta) override;
        virtual void FillUI(void* sceneViewportTexture);
    protected:
        virtual void FillMainMenubar();
        void ShowMenuFile();
        bool UpdateOffscreenPass(U32 width,U32 height);
    private:
        EngineRoot* m_Root;
        SceneHierarchyEditorPanel* m_SceneHierarchyPanel;
        EntityInspectorEditorPanel* m_EntityInspectorPanel;
        bool m_OffscreenPassAdded;
        U32 m_Width,m_Height;
        std::function<void(U32,U32)> m_UpdateOffscreenPass;
        EditorCommandsControl* m_CommandsControl;
	};
}

