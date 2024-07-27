#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"
#include"Subsystems/Scenes/Components/Component.h"

namespace Czuch
{
	class RemoveNewEntityCommand : public IEditorCommand
	{
	public:
		RemoveNewEntityCommand(Scene* scene, Entity entity) :m_Scene(scene), m_Entity(entity) {}
		virtual ~RemoveNewEntityCommand() = default;
		void Execute() override
		{
			m_Entity.AddComponent<DestroyedComponent>();
			m_Entity.OnEachChild([](Entity e) { e.AddComponent<DestroyedComponent>(); },true,false);
		}
		void Undo() override
		{
			m_Entity.RemoveComponent<DestroyedComponent>();
			m_Entity.OnEachChild([](Entity e) { e.RemoveComponent<DestroyedComponent>(); },true,true);
		}

		void OnRemoveFromUndoStack() override
		{
			m_Scene->DestroyEntity(m_Entity);
		}
	private:
		Entity m_Entity;
		Entity m_Parent;
		Scene* m_Scene;
	};

}
