#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"

namespace Czuch
{
	class RemoveNewEntityCommand : public IEditorCommand
	{
	public:
		RemoveNewEntityCommand(Scene* scene, Entity entity) :m_Scene(scene), m_Entity(entity) {}
		virtual ~RemoveNewEntityCommand() = default;

		void Execute() override
		{
			
		}
		void Undo() override
		{
			
		}
	private:
		Entity m_Entity;
		Entity m_Parent;
		Scene* m_Scene;
	};

}
