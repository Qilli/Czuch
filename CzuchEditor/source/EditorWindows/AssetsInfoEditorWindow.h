#pragma once
#include "BaseEditorWindow.h"

namespace Czuch
{
    class AssetsInfoEditorWindow :
        public BaseEditorWindow
    {
	public:
		AssetsInfoEditorWindow(const char* name);
		virtual ~AssetsInfoEditorWindow();
		virtual void FillUI() override;
		virtual void Init(EngineRoot* root) override;
    };

}


