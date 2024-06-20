#pragma once
#include"Subsystems/Scenes/Scene.h"

namespace Czuch
{
	static class ImGuiExtension
	{
	public:
		
	};


	class BaseEditorPanel
	{
	public:
		BaseEditorPanel() = default;
		virtual ~BaseEditorPanel() = default;
		virtual void FillUI() = 0;
		virtual void SelectedEntityChanged(Entity entity) {};
	protected:
		inline bool IsEntityValid(Entity entity) {return entity.IsValid(); }
	protected:
		Entity m_SelectedEntity;
	};
}

