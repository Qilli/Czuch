#pragma once
#include "UIBaseManager.h"


namespace Czuch
{
	class GraphicsDevice;
	class VulkanDevice;
	class Window;
	class EngineEditorControl;
    class CZUCH_API ImGUIManager :
        public UIBaseManager
    {
    public:
		ImGUIManager(GraphicsDevice* device,Window* wnd);
		virtual ~ImGUIManager();

		void Init(EngineSettings* settings) override;
		void Update(TimeDelta timeDelta) override;
		void Shutdown() override;
		void SetSceneForUI(Scene* scene) override;
		void OnEvent(Event& e) override;
    private:
		VulkanDevice* m_Device;
		Window* m_Window;
    };

}


