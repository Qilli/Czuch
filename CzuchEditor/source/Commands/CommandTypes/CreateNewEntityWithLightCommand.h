#pragma once
#include"../IEditorCommand.h"
#include"Subsystems/Scenes/Scene.h"
#include"Subsystems/Scenes/Components/LightComponent.h"


namespace Czuch
{
	class CreateNewEntityWithLightCommand : public IEditorCommand
	{
	public:
		CreateNewEntityWithLightCommand(Scene* scene, Entity parent, LightType lightType)
			: m_Scene(scene), m_Parent(parent), m_LightType(lightType) {}
		virtual ~CreateNewEntityWithLightCommand() = default;

		void Execute() override
		{
			std::string entityName;
			switch (m_LightType)
			{
			case LightType::Directional:
					entityName = "Directional Light";
					break;
			case LightType::Point:
					entityName = "Point Light";
					break;
			case LightType::Spot:
					entityName = "Spot Light";
					break;
			}
			m_Entity = m_Scene->CreateEntity(entityName, m_Parent);
			auto *component = &m_Entity.AddComponent<LightComponent>();
			// Add the appropriate light component based on m_LightType
			switch (m_LightType)
			{
			case LightType::Directional:
				component->SetLightType(LightType::Directional);
				break;
			case LightType::Point:
				component->SetLightType(LightType::Point);
				break;
			case LightType::Spot:
				component->SetLightType(LightType::Spot);
				break;
			}
		}

		void Undo() override
		{
			m_Scene->DestroyEntity(m_Entity);
		}

		Entity GetCreatedEntity() { return m_Entity; }

		CzuchStr& ToString() override
		{
			if (m_Desc.empty())
			{
				m_Desc = "Cmd: Create ";
				switch (m_LightType) {
				case LightType::Directional: m_Desc += "Directional Light"; break;
				case LightType::Point:     m_Desc += "Point Light"; break;
				case LightType::Spot:        m_Desc += "Spot Light"; break;
				}
				m_Desc += " ";
				m_Desc += m_Entity.GetComponent<HeaderComponent>().GetHeader();
				return m_Desc;
			}
			else
			{
				return m_Desc;
			}
		}

	private:
		Entity m_Entity;
		Entity m_Parent;
		Scene* m_Scene;
		LightType m_LightType;
		CzuchStr m_Desc;
	};
}