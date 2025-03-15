#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"
#include"Subsystems/Scenes/Components/Component.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"

namespace Czuch
{
	class ChangeTransformCommand : public IEditorCommand
	{
	public:
		ChangeTransformCommand(Scene* scene, Entity entity,Vec3 pos,Vec3 rot,Vec3 scale) :m_Scene(scene), m_Entity(entity),m_Position(pos),m_Rotation(rot),m_Scale(scale)
		{

		}
		virtual ~ChangeTransformCommand() = default;
		void Execute() override
		{

		}
		void Undo() override
		{
			auto &transform=m_Entity.GetComponent<TransformComponent>();
			transform.SetLocalPosition(m_Position);
			transform.SetLocalEulerAngles(m_Rotation);
			transform.SetLocalScale(m_Scale);
			transform.ForceUpdateLocalTransform();
		}

		CzuchStr& ToString() override
		{
			if (m_Desc.empty())
			{
				m_Desc = "Cmd: Change Transform ";
				m_Desc += m_Entity.GetComponent<HeaderComponent>().GetHeader();
				m_Desc += " to ";
				m_Desc += "Pos: ";
				m_Desc += Vec3ToString(m_Position);
				m_Desc += " Rot: ";
				m_Desc += Vec3ToString(m_Rotation);
				m_Desc += " Scale: ";
				m_Desc += Vec3ToString(m_Scale);
				return m_Desc;
			}
			else
			{
				return m_Desc;
			}
		}

	private:
		Entity m_Entity;
		Vec3 m_Position;
		Vec3 m_Rotation;
		Vec3 m_Scale;
		Scene* m_Scene;
		CzuchStr m_Desc;
	};

}