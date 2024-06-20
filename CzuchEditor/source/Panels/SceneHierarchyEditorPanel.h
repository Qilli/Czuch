#pragma once
#include "BaseEditorPanel.h"
#include"Subsystems/Scenes/Scene.h"

namespace Czuch
{
    class SceneHierarchyEditorPanel :
        public BaseEditorPanel
    {
    public:
        SceneHierarchyEditorPanel(Scene* activeScene);
        void SetActiveScene(Scene* activeScene);
        void FillUI() override;
    private:
		void DrawEntityNode(Entity entity);
    private:
        Entity m_SelectedEntity;
        Scene* m_ActiveScene;
    };
}


