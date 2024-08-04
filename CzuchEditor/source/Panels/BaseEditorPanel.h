#pragma once
#include"Subsystems/Scenes/Scene.h"

namespace Czuch
{
	struct LockedVec3
	{
		LockedVec3() { isLocked = false; value = Vec3(0, 0, 0); }
		bool isLocked;
		Vec3 value;
	};


	class BaseEditorPanel
	{
	public:
		BaseEditorPanel() = default;
		virtual ~BaseEditorPanel() = default;
		virtual void FillUI() = 0;
		virtual void SelectedEntityChanged(Entity entity) {};
		virtual Entity GetSelectedEntity() { return m_SelectedEntity; }
	protected:
		inline bool IsEntityValid(Entity entity) {return entity.IsValid(); }
	protected:
		Entity m_SelectedEntity;
	};
}

