#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"

namespace Czuch
{
	class CreateNewEntityCommand : public IEditorCommand
	{
	public:
		CreateNewEntityCommand(Scene* scene,Entity parent):m_Scene(scene),m_Parent(parent){}
		virtual ~CreateNewEntityCommand() = default;

		void Execute() override
		{
			m_Entity=m_Scene->CreateEntity("Empty Entity", m_Parent);
		}
		void Undo() override
		{
			m_Scene->DestroyEntity(m_Entity);
		}

		Entity GetCreatedEntity() { return m_Entity; }
	private:
		Entity m_Entity;
		Entity m_Parent;
		Scene* m_Scene;
	};

}