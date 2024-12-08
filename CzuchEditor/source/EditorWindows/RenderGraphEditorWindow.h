#pragma once
#include "BaseEditorWindow.h"

namespace Czuch
{
	class Renderer;
    class RenderGraphEditorWindow :
        public BaseEditorWindow
    {
	public:
		RenderGraphEditorWindow(const char* name);
		virtual ~RenderGraphEditorWindow();
		virtual void FillUI() override;
		virtual void Init(EngineRoot* root) override;
	private:
		Renderer* m_Renderer;
    };

}


