#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"
#include"Subsystems/Scenes/Components/Component.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"	

namespace Czuch
{
	class ReparentEntityCommand : public IEditorCommand
	{
	public:
		ReparentEntityCommand(Scene* scene, Entity entity,Entity newParent) :m_Scene(scene), m_Entity(entity),m_Parent(newParent),m_OldParent(0) {}
		virtual ~ReparentEntityCommand() = default;
		void Execute() override
		{
			m_OldParent=m_Entity.GetComponent<TransformComponent>().GetParentGUID();
			m_Entity.GetComponent<TransformComponent>().SetParent(m_Parent);
		}
		void Undo() override
		{
			auto parentTemp = m_OldParent;
			m_OldParent = m_Entity.GetComponent<TransformComponent>().GetParentGUID();
			m_Entity.GetComponent<TransformComponent>().SetParent(m_Scene->GetEntityObjectWithGUID(parentTemp));
		}

	private:
		Entity m_Entity;
		Entity m_Parent;
		GUID m_OldParent;
		Scene* m_Scene;
	};

}