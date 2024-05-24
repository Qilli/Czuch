#pragma once
#include "UIBaseManager.h"


namespace Czuch
{
	class GraphicsDevice;
	class VulkanDevice;
	class Window;
    class CZUCH_API ImGUIManager :
        public UIBaseManager
    {
    public:
		ImGUIManager(GraphicsDevice* device,Window* wnd);
		virtual ~ImGUIManager();

		void Init() override;
		void Update(TimeDelta timeDelta) override;
		void Shutdown() override;
    private:
		VulkanDevice* m_Device;
		Window* m_Window;
    };

}


