#pragma once
#include "BaseEditorPanel.h"

namespace Czuch
{
    class SceneHierarchyEditorPanel :
        public BaseEditorPanel
    {
    public:
        SceneHierarchyEditorPanel(Scene* activeScene);
        void SetActiveScene(Scene* activeScene);
        void FillUI() override;
        void AddOnSelectedEntityListener(BaseEditorPanel* listener);
    private:
		void DrawEntityNode(Entity entity);
        void NotifyOnSelectedEntityListeners();
    private:
        std::vector<BaseEditorPanel*> m_OnSelectedEntityListeners;
        Scene* m_ActiveScene;
    };
}


